const express = require('express');
const mysql = require('mysql2/promise');
const fs = require('fs').promises;
const path = require('path');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
require('dotenv').config();

const app = express();
const PORT = process.env.PORT || 3000;

// Security Middleware
app.use(helmet()); // Adds various HTTP headers for security

// Rate Limiting
const limiter = rateLimit({
    windowMs: 15 * 60 * 1000, // 15 minutes
    max: 100, // Limit each IP to 100 requests per windowMs
    standardHeaders: true, // Return rate limit info in RateLimit-* headers
    legacyHeaders: false, // Disable X-RateLimit-* headers
});
app.use(limiter);

// Middleware
app.use(express.json({
    limit: '1mb' // Prevent very large payloads
}));
app.use(express.static('public'));

// Validate Environment Variables
function validateEnvVariables() {
    const requiredVars = ['DB_HOST', 'DB_USER', 'DB_PASSWORD', 'DB_NAME'];
    const missingVars = requiredVars.filter(varName => !process.env[varName]);

    if (missingVars.length > 0) {
        console.error(`Missing required environment variables: ${missingVars.join(', ')}`);
        process.exit(1);
    }
}

// Sanitize and validate database configuration
function getDbConfig() {
    return {
        host: process.env.DB_HOST,
        user: process.env.DB_USER,
        password: process.env.DB_PASSWORD,
        database: process.env.DB_NAME,
        connectionLimit: 10,
        waitForConnections: true,
        queueLimit: 0,
        // Enable SSL connection for production
        ...(process.env.NODE_ENV === 'production' && {
            ssl: {
                rejectUnauthorized: true
            }
        })
    };
}

// Logging utility
const logger = {
    info: (message) => {
        if (process.env.LOG_LEVEL === 'info') {
            console.log(`[INFO] ${message}`);
        }
    },
    error: (message, error) => {
        console.error(`[ERROR] ${message}`, error);
    }
};

// Create a connection pool
let pool;
function initializeDatabase() {
    try {
        validateEnvVariables();
        const dbConfig = getDbConfig();
        pool = mysql.createPool(dbConfig);
        logger.info('Database connection pool created successfully');
        return pool;
    } catch (error) {
        logger.error('Failed to initialize database', error);
        process.exit(1);
    }
}

// Create radar frames table
async function setupRadarFramesTable() {
    try {
        const connection = await pool.getConnection();
        try {
            await connection.query(`
                CREATE TABLE IF NOT EXISTS radar_frames (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    frame_number INT UNIQUE,
                    data LONGTEXT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                )
            `);
            logger.info('Radar frames table is ready');
        } finally {
            connection.release();
        }
    } catch (error) {
        logger.error('Database table setup failed', error);
    }
}

// Upload endpoint with improved error handling
app.post('/upload', async (req, res) => {
    try {
        const frameDir = path.join(__dirname, 'radar_frames');
        const files = await fs.readdir(frameDir);

        const uploadPromises = files
            .filter(file => file.startsWith('frame_') && file.endsWith('.txt'))
            .map(async (file) => {
                const frameMatch = file.match(/frame_(\d+)\.txt/);
                if (!frameMatch) return;

                const frameNumber = parseInt(frameMatch[1], 10);
                const filePath = path.join(frameDir, file);
                const data = await fs.readFile(filePath, 'utf-8');

                try {
                    await pool.execute(
                        'INSERT INTO radar_frames (frame_number, data) VALUES (?, ?) ON DUPLICATE KEY UPDATE data = ?', 
                        [frameNumber, data, data]
                    );
                    logger.info(`Uploaded frame ${frameNumber}`);
                } catch (dbError) {
                    logger.error(`Error uploading frame ${frameNumber}`, dbError);
                }
            });

        await Promise.all(uploadPromises);
        res.status(200).send('Frames uploaded successfully');
    } catch (error) {
        logger.error('Upload process failed', error);
        res.status(500).send('Error uploading frames');
    }
});

// Retrieve frames endpoint with pagination
app.get('/frames', async (req, res) => {
    try {
        const page = parseInt(req.query.page) || 1;
        const limit = parseInt(req.query.limit) || 300;
        const offset = (page - 1) * limit;

        const [results] = await pool.execute(
            'SELECT * FROM radar_frames ORDER BY frame_number LIMIT ? OFFSET ?',
            [limit, offset]
        );

        const [countResult] = await pool.execute(
            'SELECT COUNT(*) as total FROM radar_frames'
        );

        res.json({
            frames: results,
            currentPage: page,
            totalFrames: countResult[0].total,
            hasMore: results.length === limit
        });
    } catch (error) {
        logger.error('Frames retrieval failed', error);
        res.status(500).send('Error retrieving frames');
    }
});

// Start server
async function startServer() {
    try {
        // Initialize database
        initializeDatabase();
        
        // Setup tables
        await setupRadarFramesTable();

        // Start listening
        app.listen(PORT, () => {
            logger.info(`Server running on http://localhost:${PORT}`);
            logger.info(`Environment: ${process.env.NODE_ENV || 'development'}`);
        });
    } catch (error) {
        logger.error('Server startup failed', error);
        process.exit(1);
    }
}

// Graceful shutdown
process.on('SIGINT', async () => {
    logger.info('Shutting down server gracefully');
    if (pool) {
        await pool.end();
    }
    process.exit(0);
});

// Handle unhandled promise rejections
process.on('unhandledRejection', (reason, promise) => {
    logger.error('Unhandled Rejection at:', promise, 'reason:', reason);
});

// Start the server
startServer();