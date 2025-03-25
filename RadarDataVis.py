import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as colors

class RadarVisualizer:
    def __init__(self, max_range=10000, resolution=360, range_bins=100):
        self.max_range = max_range
        self.resolution = resolution
        self.range_bins = range_bins

    def read_radar_data(self, file_path="radar_data.txt"):
        """Read radar data from file."""
        with open(file_path, "r") as file:
            data = [list(map(float, line.strip().split())) for line in file.readlines()]
        return np.array(data)

    def polar_plot(self, data):
        """Create a polar radar plot similar to classic radar display."""
        plt.figure(figsize=(12, 12), facecolor='black')
        ax = plt.subplot(111, projection='polar')
        
        # Create angle and radius grids
        theta = np.linspace(0, 2*np.pi, self.resolution, endpoint=False)
        radii = np.linspace(0, self.max_range, self.range_bins)
        
        # Mild data preprocessing
        data = np.where(data < 0.01, 0, data)
        
        # Create mesh grid
        T, R = np.meshgrid(theta, radii)
        
        # Use log scale for better visualization of weak signals
        norm = colors.LogNorm(vmin=0.01, vmax=data.max())
        
        # Plot using pcolormesh for radar-like appearance
        pc = ax.pcolormesh(T, R, data.T, cmap='viridis', norm=norm, alpha=0.8, shading='auto')
        
        # Customize radar-like appearance
        ax.set_theta_zero_location('N')  # North at the top
        ax.set_theta_direction(-1)       # Clockwise rotation
        
        # Add white grid lines
        ax.grid(True, color='white', alpha=0.5, linestyle='-')
        
        # Customize plot
        plt.title('Radar Signal Visualization', color='green', fontsize=15)
        plt.colorbar(pc, label='Signal Strength', ax=ax, fraction=0.046, pad=0.1)
        
        # Set background to black and text to green for radar-like appearance
        ax.set_facecolor('black')
        ax.tick_params(colors='green')
        for spine in ax.spines.values():
            spine.set_edgecolor('green')
        
        plt.tight_layout()
        plt.show()

    def run(self):
        """Main visualization method."""
        radar_data = self.read_radar_data()
        self.polar_plot(radar_data)

if __name__ == "__main__":
    visualizer = RadarVisualizer()
    visualizer.run()