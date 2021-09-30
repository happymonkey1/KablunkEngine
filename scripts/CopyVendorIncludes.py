from os import walk, path, getenv
from shutil import copytree
import sys
from pathlib import Path, PurePath

IGNORE_DIRS = ["examples", "extern", "test"]
PRINT_COPIED_DIR_PATHS = False

def strip_quotes_around_path(path) -> str:
    if path[0] == '\'' or path[0] == '\"' or path[len(path) - 1] == '\'' or path[len(path) - 1] == '\"':
        path.strip('\'')
        path.strip('\"')
        
    return path

def add_quotes_around_path(path) -> str:
    if path[0] != '\'' or path[0] != '"' or path[len(path) - 1] != '\'' or path[len(path) - 1] != '"':
        path.strip('\'')
        path.strip('"')
        return '\'' + path + '\''

def ignore_dirs_check(path) -> bool:
    for ignore in IGNORE_DIRS:
        if ignore in path:
            return False
    return True

def copy_include_dir(source, dest_path):
    vendor_dirs = [dir for dir in source.iterdir() if dir.is_dir()]
    include_dirs = []

    # Set to false to only copy include directories
    # currently does not work since some vendors do not have include dirs
    all_directories = True 

    print("Locating vendor directories!")
    for vendor in vendor_dirs:
        found = [dir for dir in vendor.iterdir() if dir.is_dir() and dir.stem == "include"]
        if found != []:
            include_dirs += found
        else:
            include_dirs.append(vendor)

    print("Done")

    print("\nCopying vendor directories to destination!")
    for include_dir in include_dirs:
        relative_path = PurePath(include_dir).relative_to(source)
        full_dest_path = PurePath(dest_path) / relative_path
        if PRINT_COPIED_DIR_PATHS:
            print(full_dest_path)
        copytree(include_dir, full_dest_path, dirs_exist_ok=True)
    print("Done")

'''
    for root, subdirs, _ in walk(source):
        for directory in subdirs:
            if directory == "include":
                src_path = path.join(root, directory)
                if not ignore_dirs_check(src_path):
                    continue
                #print(src_path)
                print("GETTING HERE")
                print(copytree(src_path, dest_path))'''


def main():
    src_path = strip_quotes_around_path(getenv('KABLUNK_DIR') + "\\KablunkEngine\\vendor\\")
    print(f"\nSource: {src_path}")
    dest_path = strip_quotes_around_path(sys.argv[1])
    print(f"Destination: {dest_path}\n")

    copy_include_dir(Path(src_path), Path(dest_path))

if __name__ == "__main__":
    main()
    