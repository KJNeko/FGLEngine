Titor rendering engine.

- Written with c++23
- Vulkan Rendering API
- Uses Tracy profiling tool.

# TODO:

---

### 1. Components

#### 1.1 Models

- [ ] Polish model loading
- [ ] Add in editor model viewer (Render models as preview image)

---

### 2. Performance

- [ ] GPU object culling
- [ ] Better object storage tree

---

### 3. SLANG

- [x] Convert all shaders to slang
- [x] Figure out SubpassInput grouping, If it's even possible(?)
- [x] Finish slang compilation
- [ ] Implement reflection testing
- [ ] Implement reflection assertions (Assert classes are expected sizes)

### 4. Physics

- [ ] Implement common physics functions
- [ ] Implement CPU raycasts
- [ ] Implement physics tick system
- [ ] Physics components
- [ ] Editor viewers for physics

### 5. Cameras

- [x] Verify multi-camera works
- [ ] Implement multi-camera viewer in editor
- [ ] Implement editor camera spawner

### 6. Editor

- [ ] Implement object spawner
- [ ] Implement component modification for objects
- [ ] Figure out how glTF does components saving/loading. If it even does?

### 7. Fixes (CURRENT)
- [ ] Fix the memory management systems
- [ ] Fix the transfer system

Long term plans:
- [Nanite-like implementation.](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine?application_version=5.0)
- Explicit and strict VRAM targets for certain settings.
- Explicit model loading control. ( Allows for better control of model loading )
- Scripts ( Eventually )
- Multicore focus.

# Building

- Install vulkan sdk with vulkan 1.4 at minimum
- Clone w/ submodules
- `cmake -DCMAKE_BUILD_TYPE=Debug -B build`
- `cmake --build build -j<THREAD_COUNT>`
