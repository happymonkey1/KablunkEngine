# KablunkEngine

Personal project to develop a basic game engine. Currently in development, meaning features might be broken or not implemented yet.

## Setup

Note: requires python to run setup scripts.  
Run scripts/Setup.py to install required libraries `python scripts/Setup.py`  
Run scripts/Windows-GenProjects.bat to generate project files.  
Run scripts/Windows-GitUpdateSubmodules.bat to fetch/update vendor submodules.

Compatible with Visual Studio 2022, msvc compiler, and windows 10.

Required Libraries/Binaries (should be installed by setup.py)

- Vulkan
  - Requires VULKAN_SDK environment variable (again should be automated)
- Premake
  - Requires installation in `%KABLUNK_DIR%/vendor/premake/bin/premake5.exe` (again should be automated)
- Boost 1.84.0
  - **Must** be installed under `C:/Program Files/boost/boost_1_84_0/`
- MSVC developer command prompt must be on system path
  - Add `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\`
- Catch2 Test adaptor for Visual Studio (Windows)

## Features

- [x] 2D batch renderer and basic 3D renderer
- [x] C# scripting
- [x] C++ scripting and runtime dll loading
- [x] Editor
- [x] Vulkan backend, hardware accelerated rendering, and renderer abstraction layer
- [x] Entity Component System
- [x] Asset Manager
- [x] Project System
- [x] Extendable event system
- [x] Datastructures & Algorithms (allocators, map, priority queue, simple runtime c++ parser, etc.)
- [x] DLL memory sharing / IPC

## In progress

- [ ] Text renderer
- [ ] In-house UI
- [ ] Reflection
- [ ] Audio
- [ ] Build system
- [ ] Networking Abstraction
- [ ] Runtime c++ hot reloading
- [ ] PBR renderer

## Roadmap

https://trello.com/b/cYhcr9CG/kablunkengine

## Credits

Inspiration, help, and guidance from [@TheCherno](https://www.youtube.com/channel/UCQ-W1KE9EYfdxhL6S4twUNw) (YouTube) from watching his game engine development series.
