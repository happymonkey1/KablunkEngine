import os
import Utils
from pathlib import Path
import zipfile
import shutil

PROGRAM_FILES = os.enviorn.get("PROGRAMFILES")
BOOST_LIBRARY_URL = "https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.zip"
BOOST_LOCAL_PATH = f"{PROGRAM_FILES}/boost"
BOOST_DOWNLOAD_TEMP_PATH = f"{BOOST_LOCAL_PATH}/temp"
BOOST_ZIP_PATH = f"{BOOST_DOWNLOAD_TEMP_PATH}/boost_1_80_0.zip"

def install_boost():
    """download, extract, and cleanup boost library"""
    
    Path(BOOST_LOCAL_PATH).mkdir(parents=True, exist_ok=True)
    Path(BOOST_DOWNLOAD_TEMP_PATH).mkdir(parents=True, exist_ok=True)
    
    # download
    print(f"installing '{BOOST_LIBRARY_URL}' to '{BOOST_ZIP_PATH}'...")
    Utils.DownloadFile(BOOST_LOCAL_PATH, BOOST_ZIP_PATH)
    print("done")
    
    # extract
    print("unzipping...")
    with zipfile.ZipFile(BOOST_ZIP_PATH, 'r') as zip_file:
        zip_file.extractall(BOOST_LOCAL_PATH)
    print("done.")
    
    # cleanup downloaded files
    print("cleaning temporary files...")
    shutil.rmtree(BOOST_DOWNLOAD_TEMP_PATH)
    print("done.")
    print("re-run this script after installation.")

def install_boost_prompt():
    """prompt user whether they would like to install boost, and install if yes"""
    
    print("would you like to install boost?")
    install = Utils.YesOrNo()
    if install:
        install_boost()
        quit()
    
def check_boost_install():
    """determine whether boost is already installed on the system"""
    
    if not os.path.isdir(BOOST_LOCAL_PATH):
        print("boost sdk is not installed.")
        install_boost_prompt()
        return False
    
    print(f"Boost found at {BOOST_LOCAL_PATH}")
    return True