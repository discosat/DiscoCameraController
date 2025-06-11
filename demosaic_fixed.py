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

def demosaic_bayer(bayer_image, pattern='RGGB', flip_vertical=True):
    """Demosaic Bayer pattern to RGB image with proper orientation."""
    height, width = bayer_image.shape
    
    # Convert to 8-bit for OpenCV processing (scale from 12-bit to 8-bit)
    bayer_8bit = (bayer_image >> 4).astype(np.uint8)
    
    # Flip image vertically if needed (cameras often capture upside down)
    if flip_vertical:
        bayer_8bit = cv2.flip(bayer_8bit, 0)  # 0 means vertical flip
    
    # Convert based on Bayer pattern
    # Note: OpenCV expects BGR output, not RGB
    if pattern == 'RGGB':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BayerRG2BGR)
    elif pattern == 'GRBG':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BayerGR2BGR)
    elif pattern == 'GBRG':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BayerGB2BGR)
    elif pattern == 'BGGR':
        color_image = cv2.cvtColor(bayer_8bit, cv2.COLOR_BayerBG2BGR)
    else:
        raise ValueError(f"Unknown Bayer pattern: {pattern}")
    
    return color_image

def try_all_patterns(bayer_image, flip_vertical=True):
    """Try all Bayer patterns and save results."""
    patterns = ['RGGB', 'GRBG', 'GBRG', 'BGGR']
    
    # Based on the C++ code, the camera tries BayerGR12 first, then BayerRG12
    # So GRBG pattern is most likely correct
    
    for pattern in patterns:
        try:
            color_image = demosaic_bayer(bayer_image, pattern, flip_vertical)
            
            # Save with flip indicator in filename
            flip_str = '_flipped' if flip_vertical else ''
            output_filename = f'demosaic_{pattern.lower()}{flip_str}.jpg'
            cv2.imwrite(output_filename, color_image)
            print(f"Saved {output_filename} using {pattern} pattern")
            
            # Also save as PNG for better quality
            png_filename = f'demosaic_{pattern.lower()}{flip_str}.png'
            cv2.imwrite(png_filename, color_image)
            
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
    
    # Try all Bayer patterns with vertical flip (correct orientation)
    print("\nTrying all Bayer patterns with vertical flip (correct orientation)...")
    try_all_patterns(bayer_image, flip_vertical=True)
    
    # Also try without flip for comparison
    print("\nTrying all Bayer patterns without flip (upside down)...")
    try_all_patterns(bayer_image, flip_vertical=False)
    
    # Save the raw Bayer data as grayscale for inspection
    bayer_8bit = (bayer_image >> 4).astype(np.uint8)
    cv2.imwrite('bayer_raw_original.jpg', bayer_8bit)
    cv2.imwrite('bayer_raw_flipped.jpg', cv2.flip(bayer_8bit, 0))
    print("\nSaved raw Bayer data as grayscale (both orientations)")
    
    # Provide recommendation based on C++ code analysis
    print("\n" + "="*60)
    print("RECOMMENDATION:")
    print("Based on the C++ code, the camera is configured to use BayerGR12 format.")
    print("This corresponds to the GRBG Bayer pattern.")
    print("The most likely correct image is: demosaic_grbg_flipped.jpg")
    print("="*60)

if __name__ == "__main__":
    main()