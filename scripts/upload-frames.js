const axios = require('axios');
const fs = require('fs');
const path = require('path');

async function uploadRadarFrames() {
    try {
        console.log('Starting radar frames upload...');
        
        // Check if frames directory exists and has files
        const framesDir = path.join(__dirname, '..', 'radar_frames');
        const files = fs.readdirSync(framesDir);
        
        if (files.length === 0) {
            console.error('No frame files found. Run the generator first.');
            process.exit(1);
        }

        const response = await axios.post('http://localhost:3000/upload');
        console.log('Upload successful:', response.data);
    } catch (error) {
        console.error('Upload failed:', error.message);
        if (error.response) {
            console.error('Response data:', error.response.data);
            console.error('Response status:', error.response.status);
        } else if (error.request) {
            console.error('No response received:', error.request);
        } else {
            console.error('Error setting up request:', error.message);
        }
        process.exit(1);
    }
}

uploadRadarFrames();