try:
    import requests
except ModuleNotFoundError:
    print("python requests module needs to be installed!")
try:
    from fake_useragent import UserAgent
except ModuleNotFoundError:
    print("python fake_useragent module needs to be installed!")
    
from pathlib import Path
import os
import zipfile


PREMAKE_BINARY_URL = 'https://github.com/premake/premake-core/releases/download/v5.0.0-alpha16/premake-5.0.0-alpha16-windows.zip'
PREMAKE_LOCAL_PATH = 'vendor/premake/bin'
PREMAKE_BINARY_LOCAL_PATH = f'{PREMAKE_LOCAL_PATH}/premake5.exe'
PREMAKE_ZIP_LOCAL_PATH = f'{PREMAKE_LOCAL_PATH}/premake5.zip'

def download_file(url, path) -> str:
    with open(path, 'wb') as file:
        user_agent = UserAgent()
        headers = { 'User-Agent' : user_agent.chrome }
        response = requests.get(url, headers = headers, stream = True)
        total = response.headers.get('content-length')
        print("Starting Download")
        if total is None:
            file.write(response.content)
        else:
            downloaded = 0
            total = int(total)
            for data in response.iter_content(chunk_size = max(int(total/1000), 1024*1024)):
                downloaded += len(data)
                file.write(data)

                print(f'Downloading... ({downloaded}/{total})')

                # TODO add metrics for downloading
    print()

def premake_binary_exists() -> bool:
    return Path(PREMAKE_BINARY_LOCAL_PATH).is_file()
    

def install_premake() -> None:
    if premake_binary_exists():
        print("Premake binary already exists!")
        return

    # Create folder for binary
    os.chdir('../')
    Path(PREMAKE_LOCAL_PATH).mkdir(parents = True, exist_ok = True)
    
    # Download
    print(f'Downloading Premake Binary from \'{PREMAKE_BINARY_URL}\' into \'{PREMAKE_ZIP_LOCAL_PATH}\'')
    download_file(PREMAKE_BINARY_URL, PREMAKE_ZIP_LOCAL_PATH)
    print("Zip downloaded")
    
    print("Unzipping...")
    with zipfile.ZipFile(PREMAKE_ZIP_LOCAL_PATH, 'r') as zipped_file:
        zipped_file.extractall(PREMAKE_LOCAL_PATH)
    
    print("Cleaning up...")
    if os.path.exists(PREMAKE_ZIP_LOCAL_PATH):
        os.remove(PREMAKE_ZIP_LOCAL_PATH)
    else:
        print("something went wrong, zip does not seem to exist!")
    print("Done")    

if __name__ == "__main__":
    install_premake()
