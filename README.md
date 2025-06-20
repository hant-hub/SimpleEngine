SimpleEngine is a simple game engine written
in C, using the Vulkan rendering API. It aims to be
a cross platform game engine capable of rendering,
audio, etc.

> In future we plan to have zero compile time dependencies.

## TODO:
- [X] Better Internal/User Arguement Sync
    - [X] Rearchitect includes to allow top level user include
    - [X] Separate out types and function declarations
    - [X] Replace function declarations with macros
    - [ ] Extend Preprocessor to autopopulate Macros and API
- [ ] Transition to newer version of SimpleBuild
    - [X] Linux Build
    - [ ] Windows Build
- [ ] Imperative Style Pipeline Creation
    - [X] Pipeline Info
    - [ ] Depth Attachments
    - [ ] Input Attachments
    - [ ] Preserve Attachments
    - [ ] Pipeline Caching and Creation
    - [ ] FrameBuffer Creation
         - [X] One per Renderpass
         - [ ] SwapChain Special Handling
    - [ ] Multipass targeting
- [ ] Shader analysis
    - [ ] GLSL preprocessor
    - [ ] Automatic Descriptor set creation
- [ ] Additional Features
    - [ ] Obj parser, should be fairly easy
    - [ ] More Opaque Passes
        - [ ] Text
        - [ ] Sprite
        - [ ] Solid Blocks (Solid colors)
    - [ ] Post Processing Passes
        - [ ] Sharpness
        - [ ] Color Mapping
        - [ ] Saturation


### Low Priority
- [ ] Custom printfs
- [ ] Custom Block Allocator
> Note: The two above features would allow for complete independence
from libc. Although it may be less efficient without a lot of work.
- [ ] Dynamic Array implementation
- [ ] Multibuffer Vertex Spec (split into multiple buffers)
- [ ] Image Loading
- [ ] Audio
- [ ] Threading
- [ ] Pipeline Cache
