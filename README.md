# KablunkEngine 

[![Build status](https://ci.appveyor.com/api/projects/status/wmtieejd651lye65?svg=true)](https://ci.appveyor.com/project/happymonkey1/kablunkengine)

Personal project to develop a basic 2D game engine. 

## Setup

Run scripts/Windows-GenProjects.bat to generate project files

## Issues

- [ ] Should switch submodules to all be forks to include premake files

## Engine

### Satisfactory Implemention

- [x] Basic 3D rendering
- [x] Functional 2D renderer
- [x] Window manager
- [x] Event system
- [x] Basic Orthographic Camera System
- [x] Framebuffer
- [x] Makefile implementation - Premake
- [x] ECS
- [x] Native Scripting

### Planned / In Progress (In no particular order)

- [ ] Line Renderer
- [ ] Scripting -  C#, python, lua, etc
- [ ] Hot Reloading Scripts
- [ ] More Rendering APIs - Vulkan, DirectX, Metal (LOL)
- [x] Task / Threading Abstraction
- [ ] Multithreading Parts of Engine - Rendering and Physics
- [ ] Particle System
- [ ] Better Debuging - Memory, CPU usage, etc
- [ ] Virtual File System
- [ ] Runtime Builds
- [ ] Multiplatform support - Development and Packaging
- [ ] Audio Support - OpenAL
- [ ] Mesh Rendering
- [ ] 3D Animations
- [ ] 2D / 3D? Lighting
- [ ] 2D Physics - Box2D
- [ ] 3D Physics - PhysX, Bullet
- [ ] Better Shader System - SPIR-V
- [ ] Networking - LOL
- [ ] Material System
- [ ] Physically Based Renderer
- [ ] 3D Model to 2D Texture System (what is this called?)
- [ ] UI system (Either abstracting over ImGui or inhouse)
- [ ] Asset Manager
- [x] UUIDs
- [ ] Binary Serialization - Cereal
- [ ] Editor Projects

### Cleanup

- [x] Refactor Input Code Situation
- [ ] Code Review - Member variable renaming, etc
- [x] Improve Camera System

## Editor

### Satisfactory Implemention

- [x] Dockspace
- [x] Viewports
- [x] Theme
- [x] Scene Camera

### Planned / In Progress (In no particular order)

- [x] Scene Heirarchy
- [x] Scene Loading and Unloading
- [ ] Scene Play and Stop
- [x] UI Components Editing, Transform, Camera, SpriteRenderer, etc
- [x] Component Gismos
- [ ] Entity Selection Outline in Viewport
- [x] Content Manager
- [ ] Asset Browser

## Other

- [ ] Fix Continuous Integration
- [ ] Add unit testing
- [ ] Refactor Serialization

## Credits

Inspiration and guidance from [@TheCherno](https://www.youtube.com/channel/UCQ-W1KE9EYfdxhL6S4twUNw) on YouTube. 

## Conventions

Mostly follows [Stroustrup's](https://www.stroustrup.com/bs_faq2.html) style with some personal preferences.

