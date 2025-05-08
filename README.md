SimpleEngine is a simple game engine written
in C, using the Vulkan rendering API. It aims to be
a cross platform game engine capable of rendering,
audio, etc.


## TODO:
- [ ] Transition to newer version of SimpleBuild
- [ ] Imperative Style Pipeline Creation
- [ ] Shader analysis
    - [ ] GLSL preprocessor
    - [ ] Automatic Descriptor set creation


### Low Priority
- [ ] Custom printfs
- [ ] Custom Block Allocator
> Note: The two above features would allow for complete independence
from glibc. Although it may be less efficient without a lot of work.
- [ ] Dynamic Array implementation
- [ ] Multibuffer Vertex Spec (split into multiple buffers)
- [ ] Image Loading
- [ ] Audio
- [ ] Threading
