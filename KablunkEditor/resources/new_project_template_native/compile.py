from distutils.command.build import build
import subprocess
import sys

def main():
    project_name = "$PROJECT_NAME$"
    cl_exec_path: str = R"%comspec% /k 'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat' x86_amd64"
    
    VALID_BUILD_TARGETS = ["Debug", "Release", "Distribution"]
    
    build_target: str = "Debug"
    if len(sys.argv) > 1:
        for i, arg in enumerate(sys.argv):
            if arg in VALID_BUILD_TARGETS:
                build_target = arg
        else:
            print(f"no build target specified, defaulting to {build_target}")
    
    cl_args: str = f"msbuild {project_name}.sln -target:{project_name}:{build_target}"

    
    with subprocess.Popen(args=cl_args, executable=cl_exec_path) as process:
        print(process.stdout.readlines())
        print(process.stderr.readlines())

if __name__ == "__main__":
    main()