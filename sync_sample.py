import os
import shutil

def copy_sample_back(src_dir, dest_dir):
    """
    Recursively copy the contents of src_dir (the edited sample in Assets)
    back to dest_dir (the canonical Samples~ folder in your package).
    """
    if not os.path.exists(src_dir):
        print(f"Source directory does not exist: {src_dir}")
        return
    
    # Create destination if missing
    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    
    for root, dirs, files in os.walk(src_dir):
        # Compute relative path from src_dir root
        rel_path = os.path.relpath(root, src_dir)
        dest_root = os.path.join(dest_dir, rel_path) if rel_path != '.' else dest_dir
        
        # Ensure destination directories exist
        for d in dirs:
            dir_path = os.path.join(dest_root, d)
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)
        
        # Copy files
        for f in files:
            src_file = os.path.join(root, f)
            dest_file = os.path.join(dest_root, f)
            shutil.copy2(src_file, dest_file)  # copy2 to preserve metadata
            print(f"Copied {src_file} -> {dest_file}")

if __name__ == "__main__":
    # Example usage:
    src = "BRT-Unity-DemoProject/Assets/Samples/BRT_Example"
    dest = "brt-unity-package/Samples~/BRT_Example"
    copy_sample_back(src, dest)
