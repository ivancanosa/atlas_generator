# Atlas Generator
A generic atlas generator for 2D packaging of images.

## Requirements
- CMake +3.10
- A C++17-compatible compiler

## Features
- Generates multiple atlases with specified maximum width and height.
- Accepts a list of image files or directories as input.

## How to use
```bash
Usage: atlas [--help] [--version] [--height VAR] [--width VAR] [-d VAR] [-j VAR] [images]...

Positional arguments:
  images         list of image paths to include in the atlas [nargs: 0 or more]

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --height       height of each atlas bin [nargs=0..1] [default: 1920]
  --width        width of each atlas bin [nargs=0..1] [default: 1080]
  -d             output dir of atlas images [nargs=0..1] [default: "atlas_images"]
  -j             output dir of atlas json description [nargs=0..1] [default: "atlas.json"]

```

## Compilation
```bash
# Create and navigate to the build directory
mkdir build
cd build

# Run CMake to generate build files
cmake ../src

# Compile the project
make

# Run tests
ctest

# Install
sudo cmake --install .  
```

