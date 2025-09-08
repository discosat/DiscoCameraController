#!/usr/bin/env python3
import cv2
import numpy as np
from pathlib import Path

def analyze_image(filename):
    """Analyze an image file and print statistics."""
    if not Path(filename).exists():
        print(f"File {filename} does not exist")
        return
    
    img = cv2.imread(filename)
    if img is None:
        print(f"Could not load image: {filename}")
        return
    
    height, width = img.shape[:2]
    channels = img.shape[2] if len(img.shape) > 2 else 1
    
    print(f"\n{filename}:")
    print(f"  Dimensions: {width}x{height}")
    print(f"  Channels: {channels}")
    print(f"  Data type: {img.dtype}")
    print(f"  File size: {Path(filename).stat().st_size} bytes")
    
    # Calculate statistics for each channel
    if channels == 3:
        # BGR image
        b, g, r = cv2.split(img)
        print(f"  Blue  channel: min={b.min()}, max={b.max()}, mean={b.mean():.1f}")
        print(f"  Green channel: min={g.min()}, max={g.max()}, mean={g.mean():.1f}")
        print(f"  Red   channel: min={r.min()}, max={r.max()}, mean={r.mean():.1f}")
    else:
        # Grayscale image
        print(f"  Grayscale: min={img.min()}, max={img.max()}, mean={img.mean():.1f}")
    
    # Check for common issues
    if img.max() == 0:
        print("  WARNING: Image is completely black!")
    elif img.max() < 50:
        print("  WARNING: Image appears very dark (max value < 50)")
    elif img.min() == img.max():
        print("  WARNING: Image has no variation (all pixels same value)")
    
    # Check if image looks like it has content
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) if channels == 3 else img
    edges = cv2.Canny(gray, 50, 150)
    edge_count = np.count_nonzero(edges)
    edge_percentage = (edge_count / (width * height)) * 100
    print(f"  Edge content: {edge_percentage:.2f}% of pixels have edges")
    
    if edge_percentage > 5:
        print("  ✓ Image appears to have good detail/content")
    elif edge_percentage > 1:
        print("  ⚠ Image has some content but may be blurry or low contrast")
    else:
        print("  ✗ Image appears to have very little content")

def main():
    image_files = [
        'bayer_raw.jpg',
        'demosaic_rggb.jpg', 
        'demosaic_grbg.jpg',
        'demosaic_gbrg.jpg',
        'demosaic_bggr.jpg'
    ]
    
    print("Image Analysis Report")
    print("=" * 50)
    
    for filename in image_files:
        analyze_image(filename)
    
    print("\n" + "=" * 50)
    print("Analysis complete.")

if __name__ == "__main__":
    main()