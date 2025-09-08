#!/usr/bin/env python3
import numpy as np
import cv2
from pathlib import Path

def read_bayer_buffer(filename, width, height, bits_per_pixel=12):
    """Read raw Bayer buffer from file."""
    with open(filename, 'rb') as f:
        data = f.read()
    
    # For 12-bit data stored in 16-bit format
    if bits_per_pixel == 12:
        # Convert bytes to 16-bit values (little-endian)
        raw_data = np.frombuffer(data, dtype=np.uint16)
        # Mask to 12-bit values
        raw_data = raw_data & 0x0FFF
    else:
        raw_data = np.frombuffer(data, dtype=np.uint8)
    
    # Reshape to image dimensions
    image = raw_data.reshape(height, width)
    return image

def demosaic_bayer(bayer_image, pattern='RGGB'):
    """Demosaic Bayer pattern to RGB image."""
    height, width = bayer_image.shape
    
    # Convert to 8-bit for OpenCV processing (scale from 12-bit to 8-bit)
    bayer_8bit = (bayer_image >> 4).astype(np.uint8)
    
    # Convert based on Bayer pattern
    if pattern == 'RGGB':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BAYER_RG2RGB)
    elif pattern == 'GRBG':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BAYER_GR2RGB)
    elif pattern == 'GBRG':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BAYER_GB2RGB)
    elif pattern == 'BGGR':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BAYER_BG2RGB)
    else:
        raise ValueError(f"Unknown Bayer pattern: {pattern}")
    
    return color_image

def try_all_patterns(bayer_image):
    """Try all Bayer patterns and save results."""
    patterns = ['RGGB', 'GRBG', 'GBRG', 'BGGR']
    
    for pattern in patterns:
        try:
            color_image = demosaic_bayer(bayer_image, pattern)
            output_filename = f'demosaic_{pattern.lower()}.jpg'
            cv2.imwrite(output_filename, color_image)
            print(f"Saved {output_filename} using {pattern} pattern")
        except Exception as e:
            print(f"Failed to process {pattern}: {e}")

def main():
    # Image parameters from the captured output (full resolution)
    width = 2464
    height = 2056
    bits_per_pixel = 12
    
    # Read the buffer
    print("Reading Bayer buffer...")
    bayer_image = read_bayer_buffer('buffer_dump.bin', width, height, bits_per_pixel)
    
    print(f"Bayer image shape: {bayer_image.shape}")
    print(f"Bayer image min/max values: {bayer_image.min()}/{bayer_image.max()}")
    
    # Try all Bayer patterns
    print("Trying all Bayer patterns...")
    try_all_patterns(bayer_image)
    
    # Also save the raw Bayer data as grayscale for inspection
    bayer_8bit = (bayer_image >> 4).astype(np.uint8)
    cv2.imwrite('bayer_raw.jpg', bayer_8bit)
    print("Saved bayer_raw.jpg (raw Bayer data as grayscale)")

if __name__ == "__main__":
    main()