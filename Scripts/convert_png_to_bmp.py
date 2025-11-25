import os
import cv2
import numpy as np

def convert_png_to_bmp_recursive(root_dir):
    """
    Recursively finds all PNG files in a directory and converts them to BMP format
    in the same directory. Transparent pixels are replaced with magenta (255, 0, 255).

    Args:
        root_dir (str): The root directory to start searching from.
    """
    converted_count = 0
    magenta_bgr = (255, 0, 255)  # OpenCV uses BGR order, so magenta is (B=255, G=0, R=255)

    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.lower().endswith(".png"):
                try:
                    input_path = os.path.join(dirpath, filename)
                    output_filename = os.path.splitext(filename)[0] + ".bmp"
                    output_path = os.path.join(dirpath, output_filename)

                    # Read the PNG image using OpenCV, keeping the alpha channel
                    png_image = cv2.imread(input_path, cv2.IMREAD_UNCHANGED)
                    
                    if png_image is None:
                        print(f"Warning: Could not read image: {input_path}")
                        continue

                    # Check if the image has an alpha channel
                    if png_image.shape[2] == 4:
                        # Split the image into BGR and an alpha channel
                        bgr = png_image[:, :, :3]
                        alpha = png_image[:, :, 3]

                        # Create a magenta background
                        background = np.full(bgr.shape, magenta_bgr, dtype=np.uint8)

                        # Create a boolean mask from the alpha channel
                        # True where alpha is 0 (fully transparent)
                        is_transparent = alpha == 0

                        # Use the mask to choose between original BGR and magenta background
                        # Where 'is_transparent' is True, use 'background', otherwise use 'bgr'
                        final_bgr = np.where(is_transparent[:, :, np.newaxis], background, bgr)
                        
                        # Save the resulting 3-channel image
                        cv2.imwrite(output_path, final_bgr)
                    else:
                        # If there is no alpha channel, just save the image as is
                        cv2.imwrite(output_path, png_image)

                    print(f"Converted {input_path} to {output_path}")
                    converted_count += 1
                except Exception as e:
                    print(f"Error converting {filename}: {e}")

    print(f"\nConversion complete. Converted {converted_count} files.")

if __name__ == "__main__":
    # The script is in the 'Scripts' directory, so we go up one level
    # to find the 'Resources' directory.
    # The current working directory is the project root, so we can just use 'Resources'
    start_directory = 'Resources'
    
    if not os.path.isdir(start_directory):
        print(f"Error: The target directory '{os.path.abspath(start_directory)}' does not exist.")
    else:
        print(f"Starting conversion in '{os.path.abspath(start_directory)}'...")
        convert_png_to_bmp_recursive(start_directory)
