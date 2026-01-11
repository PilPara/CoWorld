#!/bin/zsh
set -e

# Ensure dependencies are installed
brew list glfw || brew install glfw
brew list assimp || brew install assimp

# Point CMake to Homebrew packages
export CMAKE_PREFIX_PATH="/opt/homebrew"

# Configure + build
cmake -S . -B build \
  -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

# Symlink compile_commands.json for LSP (clangd)
ln -sf build/compile_commands.json compile_commands.json

# Run the program
./build/CowWorld
