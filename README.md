# CoWorld

A real-time 3D graphics engine built with OpenGL, featuring skeletal animation, dynamic lighting, and an interactive sandbox environment.

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-green.svg)
![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

## Overview

coWorld is a from-scratch 3D engine that demonstrates modern OpenGL rendering techniques. The project centers around an interactive scene with an animated cow character, featuring full skeletal animation with layered body part control, Phong lighting, collision detection, and a debug GUI.

## Features

- **Skeletal Animation System**
  - Bone hierarchy with weighted vertex skinning
  - Keyframe interpolation (position, rotation, scale)
  - Layered animations (head, tail, locomotion can animate independently)
  - Support for GLTF, FBX, and other Assimp-compatible formats

- **Rendering**
  - Phong lighting model with point, directional, and spot lights
  - Cubemap skybox
  - Textured and untextured material support
  - Configurable render settings (wireframe mode, etc.)

- **Interaction**
  - Free-fly camera with collision detection
  - First-person POV camera mode
  - Real-time lighting adjustments via ImGui menu
  - Keyboard/mouse input abstraction layer

- **Architecture**
  - Clean separation: Application → Scene → Models/Camera/Lights
  - RAII resource management for OpenGL objects
  - Configurable via centralized `Config` namespace

## Requirements

- C++17 compiler
- OpenGL 3.3+
- CMake 3.10+

### Dependencies

| Library | Purpose |
|---------|---------|
| [GLFW](https://www.glfw.org/) | Window & input |
| [GLAD](https://glad.dav1d.de/) | OpenGL loader |
| [GLM](https://github.com/g-truc/glm) | Math library |
| [Assimp](https://github.com/assimp/assimp) | Model loading |
| [stb_image](https://github.com/nothings/stb) | Texture loading |
| [Dear ImGui](https://github.com/ocornut/imgui) | Debug GUI |

## Building

```bash
git clone https://github.com/yourusername/coWorld.git
cd coWorld
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
./coWorld
```

### Controls

| Key | Action |
|-----|--------|
| `W/A/S/D` | Camera movement |
| `E/Q` | Camera up/down |
| `F` | Fast movement |
| `Arrow keys` | Move cow |
| `1-4` | Head animation controls |
| `Z/X/C` | Tail animation controls |
| `Tab` | Cycle cameras |
| `M` | Toggle menu |
| `Esc` | Quit |

## Project Structure

```
coWorld/
├── src/
│   ├── main.cpp              # Entry point
│   ├── application.cpp       # Main loop & window management
│   ├── scene.cpp             # Scene graph & rendering
│   ├── camera.cpp            # Free-fly & POV cameras
│   ├── model.cpp             # Static model loading
│   ├── animated_model.cpp    # Skeletal model loading
│   ├── animator.cpp          # Animation playback & blending
│   ├── bone.cpp              # Bone transforms & interpolation
│   ├── shader.cpp            # GLSL shader management
│   ├── mesh.cpp              # Static mesh rendering
│   ├── animated_mesh.cpp     # Skinned mesh rendering
│   ├── light.cpp             # Light types (point/dir/spot)
│   ├── texture.cpp           # Texture loading & binding
│   ├── skybox.cpp            # Cubemap skybox
│   ├── input_manager.cpp     # Input abstraction
│   └── menu_system.cpp       # ImGui debug interface
├── shaders/
│   ├── object.vs / .fs       # Main object shader
│   ├── animated.vs / .fs     # Skeletal animation shader
│   └── skybox.vs / .fs       # Skybox shader
├── assets/
│   ├── models/
│   └── textures/
└── CMakeLists.txt
```

## Technical Details

### Animation Pipeline

1. **Loading**: Assimp extracts bone hierarchy and keyframes from model files
2. **Bone Mapping**: Bones are filtered and mapped to vertex weights
3. **Playback**: `Animator` advances time and interpolates keyframes
4. **Skinning**: Final bone matrices are uploaded to GPU for vertex transformation

### Layered Animation

The animator supports independent animation layers:
- **Locomotion**: Body movement (walking, running)
- **Head**: Look direction, expressions
- **Tail**: Secondary motion

Layers can play different animations simultaneously and blend based on bone assignments.

