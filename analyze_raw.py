#!/usr/bin/env python3
import numpy as np
from pathlib import Path

def analyze_bayer_raw():
    """Analyze the raw Bayer buffer data."""
    # Image parameters (full resolution)
    width = 2464
    height = 2056
    
    with open('buffer_dump.bin', 'rb') as f:
        data = f.read()
    
    # Convert to 16-bit values (little-endian)
    raw_data = np.frombuffer(data, dtype=np.uint16)
    # Mask to 12-bit values
    raw_data = raw_data & 0x0FFF
    
    # Reshape to image dimensions
    image = raw_data.reshape(height, width)
    
    print(f"Raw Bayer Data Analysis:")
    print(f"Shape: {image.shape}")
    print(f"Data type: {image.dtype}")
    print(f"Min value: {image.min()}")
    print(f"Max value: {image.max()}")
    print(f"Mean value: {image.mean():.2f}")
    print(f"Std deviation: {image.std():.2f}")
    
    # Check for saturation
    saturated_pixels = np.sum(image >= 4095)  # 12-bit max
    total_pixels = width * height
    saturation_percent = (saturated_pixels / total_pixels) * 100
    print(f"Saturated pixels (4095): {saturated_pixels} ({saturation_percent:.2f}%)")
    
    # Check for zero pixels
    zero_pixels = np.sum(image == 0)
    zero_percent = (zero_pixels / total_pixels) * 100
    print(f"Zero pixels: {zero_pixels} ({zero_percent:.2f}%)")
    
    # Sample different regions
    print(f"\nSample regions:")
    regions = [
        ("Top-left", image[0:100, 0:100]),
        ("Top-right", image[0:100, -100:]),
        ("Center", image[height//2-50:height//2+50, width//2-50:width//2+50]),
        ("Bottom-left", image[-100:, 0:100]),
        ("Bottom-right", image[-100:, -100:])
    ]
    
    for name, region in regions:
        print(f"  {name}: min={region.min()}, max={region.max()}, mean={region.mean():.1f}")
    
    # Check histogram
    hist, bins = np.histogram(image, bins=50, range=(0, 4095))
    print(f"\nHistogram analysis:")
    print(f"  Most common value: {bins[np.argmax(hist)]:.0f}")
    print(f"  Values in top 10% (>3686): {np.sum(image > 3686)} pixels")
    print(f"  Values in bottom 10% (<410): {np.sum(image < 410)} pixels")
    
    # Check for patterns (like half-image issues)
    left_half = image[:, :width//2]
    right_half = image[:, width//2:]
    top_half = image[:height//2, :]
    bottom_half = image[height//2:, :]
    
    print(f"\nHalf-image analysis:")
    print(f"  Left half:   mean={left_half.mean():.1f}, max={left_half.max()}")
    print(f"  Right half:  mean={right_half.mean():.1f}, max={right_half.max()}")
    print(f"  Top half:    mean={top_half.mean():.1f}, max={top_half.max()}")
    print(f"  Bottom half: mean={bottom_half.mean():.1f}, max={bottom_half.max()}")
    
    # Look for row-by-row patterns
    row_means = np.mean(image, axis=1)
    col_means = np.mean(image, axis=0)
    
    print(f"\nRow/Column patterns:")
    print(f"  Row means: min={row_means.min():.1f}, max={row_means.max():.1f}, std={row_means.std():.2f}")
    print(f"  Col means: min={col_means.min():.1f}, max={col_means.max():.1f}, std={col_means.std():.2f}")
    
    # Save a small crop as text for inspection
    crop = image[1000:1020, 1000:1020]  # 20x20 crop from middle
    print(f"\n20x20 sample from center (row 1000-1020, col 1000-1020):")
    for row in crop:
        print("  " + " ".join(f"{val:4d}" for val in row))

if __name__ == "__main__":
    analyze_bayer_raw()