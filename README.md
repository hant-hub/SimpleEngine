SimpleEngine is a simple game engine written
in C, using the Vulkan rendering API. It aims to be
a cross platform game engine capable of rendering,
audio, etc.

> In future we plan to have zero compile time dependencies.


## TODO:
- [ ] Transition to newer version of SimpleBuild
- [ ] Imperative Style Pipeline Creation
    - [ ] Pipeline Info
    - [ ] Depth Attachments
    - [ ] Input Attachments
    - [ ] Preserve Attachments
    - [ ] Pipeline Caching and Creation
    - [ ] FrameBuffer Creation
    - [ ] Multipass targeting
    - [ ] Command Buffer creation
- [ ] Shader analysis
    - [ ] GLSL preprocessor
    - [ ] Automatic Descriptor set creation


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
