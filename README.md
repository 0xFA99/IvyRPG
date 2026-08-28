# IvyRPG

A small 2D RPG built in **C** with a custom game architecture around [raylib](https://www.raylib.com/).

IvyRPG started as a game project and gradually became a playground for exploring low-level programming, memory management, rendering, asset handling, and performance-oriented software design.

> This project is primarily a learning and experimentation project. Expect the architecture to evolve as I learn and refactor things.

## Features

- **Tilemap & autotiling** for building the game world
- **2D rendering** with raylib
- **Camera system** and virtual-resolution rendering
- **Collision detection** and player movement
- **Scene management** for title, gameplay, and options
- **Entity and system modules** for keeping game code separated by responsibility
- **Asset management** for game resources
- **Texture management** and preprocessed binary assets
- **Audio support** for WAV and OGG assets
- **Custom memory allocation** with a linear arena allocator
- **Localization, profiles, and inventory systems**

The codebase is intentionally split into modules such as `core`, `graphics`, `systems`, `scenes`, `entities`, `audio`, and `arena`.

## Building

### Requirements

- C compiler with C11 support (GCC or Clang)
- CMake 3.20 or newer
- Linux or Windows
- The project includes its raylib libraries under `lib/`

### Debug build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

The Debug configuration enables additional diagnostics. On Linux it also enables AddressSanitizer and UndefinedBehaviorSanitizer.

Run the executable from the build output:

```bash
./build/bin/IvyRPG
```

### Release build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Release builds use a Unity build and enable link-time optimization when supported by the compiler.

## Project Structure

```text
IvyRPG/
├── assets/      # Preprocessed game assets
├── include/     # Public headers
├── lib/         # Platform-specific raylib libraries
├── src/
│   ├── arena/   # Memory allocation
│   ├── audio/   # Audio handling
│   ├── core/    # Game and input core
│   ├── entities/# Game entities
│   ├── graphics/# Rendering, tilemaps, camera, collision
│   ├── scenes/  # Game scenes
│   ├── systems/ # Game-wide systems and managers
│   └── utils/   # Shared utilities
└── CMakeLists.txt
```

## Technical Focus

IvyRPG is also a place to experiment with questions that go beyond the game itself:

- How should memory ownership work in a C codebase?
- How can game data be represented and loaded efficiently?
- How should systems be split without introducing unnecessary abstraction?
- How can the build be configured differently for debugging and performance?
- How much control can be kept in a small, dependency-light architecture?

The project is continuously refactored as these questions are explored.

## Status

IvyRPG is an active personal project. The game and architecture are still evolving, and parts of the code may change significantly over time.

## License

See [LICENSE](LICENSE) for the current license.

---

Built as a practical way to learn **C, game architecture, memory management, graphics programming, and performance-oriented software development**.
