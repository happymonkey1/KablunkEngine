# KablunkEngine 

Personal project to develop a basic game engine. Currently in development, meaning features might be broken or not implemented yet. 

## Setup

Run scripts/Setup.py to install required libraries `python scripts/Setup.py`  
Run scripts/Windows-GenProjects.bat to generate project files.  
Run scripts/Windows-GitUpdateSubmodules.bat to fetch/update vendor submodules.  

Compatible with Visual Studio 2022, msvc compiler, and windows 10.  

Required Libraries/Binaries (should be installed by setup.py)
- Boost
- Vulkan
- Premake

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
- [x] Datastructures & Algorithms (allocators, map, priority queue, simple c++ parser, etc.)
- [x] DLL memory sharing / IPC

## In progress

- [ ] Text renderer
- [ ] In-house UI
- [ ] Reflection
- [ ] Audio
- [ ] Build system

## Roadmap

https://trello.com/b/cYhcr9CG/kablunkengine

## Credits

Inspiration, help, and guidance from [@TheCherno](https://www.youtube.com/channel/UCQ-W1KE9EYfdxhL6S4twUNw) (YouTube) from watching his game engine development series. 
