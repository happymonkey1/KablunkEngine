import os
import subprocess

import Vulkan
import Boost

# Change from Scripts directory to root
cur_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(cur_dir)
os.chdir('../')
print(f"script is running in '{os.getcwd()}'")
cur_dir = os.getcwd()

KB_ENGINE_ENVIRONMENT_VARIABLE_NAME = "KABLUNK_DIR"
print(f"setting ${KB_ENGINE_ENVIRONMENT_VARIABLE_NAME}='{cur_dir}'")
os.environ[KB_ENGINE_ENVIRONMENT_VARIABLE_NAME] = cur_dir

if (not Boost.check_boost_install()):
    print("Boost not installed.")

if (not Vulkan.CheckVulkanSDK()):
    print("Vulkan SDK not installed.")

print("retrieving and updating submodules...")
subprocess.call(["git", "lfs", "pull"])
subprocess.call(
    ["git", "submodule", "update", "--init", "--recursive", "--remote"]
)

print("Running premake...")
premake_path = f"\"{os.environ[KB_ENGINE_ENVIRONMENT_VARIABLE_NAME]}/vendor/premake/bin/premake5.exe\""
print(f"premake bin path {premake_path}")
print(f"working directory {os.getcwd()}")
subprocess.call(
    [f"{premake_path}", "vs2022"],
    env={KB_ENGINE_ENVIRONMENT_VARIABLE_NAME:os.environ[KB_ENGINE_ENVIRONMENT_VARIABLE_NAME]}
)
