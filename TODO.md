# TODOS:

## Next Time
- [X] RenderGraph Rewrite
    - [X] Resource Helpers
        - [X] Build Pipeline From Struct
        - [X] Pipeline Layout 
            - [X] Helper
    - [X] Building Graph
        - [X] design doc + mem layout
        - [X] RenderPass builder
        - [X] Gen Resources
            - [X] Buffer Sizing
            - [X] Image Sizing
        - [X] Gen VkRenderPasses
        - [X] build runtime Pipeline
        - [X] Shader Compilation
        - [X] FrameBuffer Info
    - [X] Cleanup
        - [X] Destroy Info
        - [X] Destroy RenderPipeline
        - [X] Destroy FrameBuffers
        - [X] Destroy Images
    - [X] Using Graph
        - [X] Dispatch Commands
            - [X] Renderpass Begin
            - [X] Bind Vertex + Pipeline
            - [X] Pipeline Barrier
            - [X] Swapchain TRANSFER_DST
        - [X] Resize
- [ ] Improvements
    - [ ] Vertex Buffer Support
        - [X] Add Buffer
        - [X] Get Buffer Handle (void* for dyn and Transfer op for vert data)
            - [X] void* for dyn
            - [X] transfer for static
        - [X] Add Vertex Spec
            - [ ] Update Vertex Spec to handle more data
                - [X] vec2
                - [X] vec3
                - [ ] vec4
    - [X] Switch to Updated Cutils
    - [ ] Index Buffer Support
        - [ ] Add Buffer
        - [ ] Alter Draw command to use index buffer
    - [X] Get Framebuffer to use correct size
        - [X] Store on Pass using shrinking algorithm
        - [X] Allow setting viewport (Put on Pass)
    - [ ] Pipeline Config Funcs
    - [X] Descriptor Sets
        - [X] Update Pipeline Layouts
        - [X] Descriptor Sets
        - [X] Calc Number of sets required
        - [X] create layout pool
        - [X] Allocate Uniform Buffers
    - [X] Textures
        - [X] Add Binding Type
        - [X] Add to image
    - [ ] Shader Storage (only read and compile once)
    - [X] Texture Data


## Quality of Life:
- [X] Fix all tests printing
- [X] Update RenderGraph Code
    - [X] Split off Read and Write code
    - [X] fix dynArray to be a proper type
- [ ] Improve Tests
    - [ ] Add asserts to all tests
    - [ ] Redirect logs to file
    - [ ] Split large tests into better tests
- [ ] more unit style tests (test internals)
- [ ] consider caching for platform configuration
- [ ] Update SimpleBuild
- [ ] Update CUtils
- [ ] Investigate Better Memory tests
     - [ ] VkAllocationCallbacks (track allocations)
     - [ ] Force driver to free enumerate memory?

## Side Features
- [ ] New Allocators
    - [ ] Scratch arena
        - Dynamic in debug, static in release
        - Should be fairly simple to reuse
        - must never invalidate pointers
    - [ ] Fixed size Freelist
        - Must support dynamic allocation in debug and
        static allocation in release
        - Should include fast reset
    - [ ] Replace lists with appropriate allocators
    - [ ] Provide options to user for External facing allocators
- [ ] Code Introspection and Generation
    - [ ] Generate Struct descriptions
    - [ ] Dynamic print

## Core:
- [ ] Handle all events
- [ ] Custom title
    - [ ] Window Decoration?
- [ ] Add more backends
    - [ ] Wayland
    - [ ] Win11
