# Radar Data Visualization System

This project simulates, processes, and visualizes dynamic radar signal data. It generates synthetic radar frames with moving targets, stores them in a database, and displays them through an interactive web visualization.

## Features

- Dynamic radar signal simulation with moving targets
- Reduced angular resolution (180°) and range bins (50) for efficient processing
- MySQL database integration for storing radar frame data
- Express.js server for handling data upload and retrieval
- D3.js-powered visualization with animated frame playback
- Security features including rate limiting and Helmet protection

## System Architecture

1. **Data Generation** (`RadarDataGen.cpp`): C++ program that simulates moving targets and generates radar frames
2. **Database** (`db.sql`): MySQL database schema for storing radar frame data
3. **Backend Server** (`server.js`): Express.js server that handles data upload, storage, and retrieval
4. **Upload Utility** (`upload-frames.js`): Helper script to upload generated frames to the server
5. **Frontend Visualization** (`index.html`): Web interface for visualizing radar data using D3.js

## Setup Instructions

### Prerequisites

- Node.js and npm
- MySQL database server
- C++ compiler (g++ or equivalent)
- Basic knowledge of JavaScript and C++

### Installation

1. **Set up the database**:
   ```bash
   mysql -u username -p < db.sql
   ```

2. **Configure environment variables**:
   Create a `.env` file with the following:
   ```
   DB_HOST=localhost
   DB_USER=your_username
   DB_PASSWORD=your_password
   DB_NAME=radar_db
   PORT=3000
   ```

3. **Install Node.js dependencies**:
   ```bash
   npm install express mysql2 dotenv helmet express-rate-limit axios
   ```

4. **Compile the radar data generator**:
   ```bash
   g++ RadarDataGen.cpp -o radar_generator -std=c++17
   ```

### Running the System

1. **Generate radar data**:
   ```bash
   ./radar_generator
   ```
   This will create text files in a `radar_frames` directory.

2. **Start the server**:
   ```bash
   node server.js
   ```

3. **Upload frames to the database**:
   ```bash
   node upload-frames.js
   ```

4. **Access the visualization**:
   Open your browser and navigate to `http://localhost:3000`

## System Components

### RadarDataGen.cpp

The C++ program simulates radar data by:
- Creating randomly positioned moving targets
- Updating target positions based on their velocities
- Generating a radar signal grid for each frame
- Saving frame data to text files

### Database Schema

Simple MySQL structure with:
- `radar_frames` table to store frame data
- Indexed by frame number for faster retrieval

### Express Server (server.js)

- RESTful API for frame upload and retrieval
- Enhanced security with Helmet and rate limiting
- Error handling and graceful shutdown capability
- Database connection pooling

### Upload Utility (upload-frames.js)

- Reads frame files from the `radar_frames` directory
- Sends each frame to the server via HTTP POST requests
- Implements error handling and logging

### Visualization (index.html)

- D3.js-based SVG visualization
- Animated playback of radar frames
- Debug information display
- Error handling and user feedback

## Limitations and Considerations

- The system currently supports up to 300 frames per simulation
- Frame data uploads are rate-limited to 300 per minute
- The visualization refreshes every 200ms
- Security features include CSP headers, rate limiting, and parameterized queries

## Troubleshooting

- **No frames available**: Ensure the radar generator has been run and frames are created
- **Database connection error**: Verify your database credentials and server status
- **Upload failures**: Check server logs for specific error details
- **Visualization not showing**: Open browser console for debugging information

## Further Development

- Add authentication for secure frame uploads
- Implement real-time data streaming using WebSockets
- Create more sophisticated target movement patterns
- Add filters and analysis tools to the visualization
