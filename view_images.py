#!/usr/bin/env python3
import cv2
import numpy as np
from pathlib import Path

def view_image(filename):
    """Display an image file."""
    if not Path(filename).exists():
        print(f"File {filename} does not exist")
        return
    
    img = cv2.imread(filename)
    if img is None:
        print(f"Could not load image: {filename}")
        return
    
    # Get image info
    height, width = img.shape[:2]
    print(f"\n{filename}:")
    print(f"  Dimensions: {width}x{height}")
    print(f"  Channels: {img.shape[2] if len(img.shape) > 2 else 1}")
    print(f"  Data type: {img.dtype}")
    print(f"  Value range: {img.min()} - {img.max()}")
    
    # Resize for display if too large
    display_img = img.copy()
    if width > 1200 or height > 800:
        scale = min(1200/width, 800/height)
        new_width = int(width * scale)
        new_height = int(height * scale)
        display_img = cv2.resize(display_img, (new_width, new_height))
        print(f"  Resized for display: {new_width}x{new_height}")
    
    # Show image
    cv2.imshow(filename, display_img)
    key = cv2.waitKey(0)
    cv2.destroyAllWindows()
    
    return key

def main():
    # List all the generated images
    image_files = [
        'bayer_raw.jpg',
        'demosaic_rggb.jpg', 
        'demosaic_grbg.jpg',
        'demosaic_gbrg.jpg',
        'demosaic_bggr.jpg'
    ]
    
    print("Available images:")
    for i, filename in enumerate(image_files):
        print(f"{i+1}. {filename}")
    
    print("\nPress any key to continue to next image, 'q' to quit")
    
    for filename in image_files:
        key = view_image(filename)
        if key == ord('q'):
            break
    
    print("Image viewing complete.")

if __name__ == "__main__":
    main()