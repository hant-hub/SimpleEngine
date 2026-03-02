# TODOS:

## Next Time
- [X] Add Funcs for Allocating Buffers
- [X] Extend RenderGraph
    - [X] Insert Buffer into RenderGraph
    - [X] Extend SEPass to bind Vertex Buffers
    - [X] Bind Vertex Buffers
    - [X] Map Dynamic Buffers
    - [X] Transfer Queue for Static Buffers
- [X] Clean up
    - [X] Unmap Dynamic Buffers
- [X] Move Buffers into RenderGraph
    - [X] Remove Allocations from SEVulkan
        - [X] Remove BufAllocators
        - [X] SEsettings
        - [X] ConfigMaxGPU Mem
    - [X] Fix Allocation Funcs
        - [X] Remove Config Buf Allocator
        - [X] Fix Alloc Buffer to use Rendergraph
    - [X] Rendergraph allocate
        - [X] Move allocations into RenderGraph
        - [X] Change Allocate Buffer to use RenderGraph
        - [X] Move ConfigMaxGPU Mem into init
    - [X] Buffer Allocate
        - [X] Step to Allocate Memory
        - [X] Create Resource
        - [X] Retrieve Buffer
            - [X] Dynamic Void*
            - [X] Static Upload
- [ ] Fix Internal Api
    - [ ] Convert Internal graphics API to use SEVulkan instead of SEwindow
- [ ] Resize Support
    - [ ] Rework Attachments
        - [ ] Allocators for images
        - [ ] Attachment Description (size, format, etc)
        - [ ] FrameBuffer Description (which attachments)
        - [ ] Change Graph to blit to swapchain
            - [ ] Mark Back Buffer
            - [ ] Future Prune unused passes
    - [ ] Swapchain Recreation
    - [ ] Attachment Recreation?
        - [ ] Attachment registry
    - [ ] Framebuffer Recreation


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

## Graphics:
- [ ] New Resource Allocation
    - [ ] Flags for Memory Types
        - [ ] Textures
        - [X] Vertex Buffers
        - [ ] Index Buffers
        - [ ] Render Targets
    - [ ] Memory Management
        - [X] Allocator (free list based)
        - [ ] alignment calcs
        - [ ] Memory Heap scanning
        - [ ] Alignment and Resource fixing
    - [X] Buffers
        - [X] Allocate
        - [X] Host Coherent
        - [X] Transfer Queue (just use pipeline barrier lol)
    - [ ] Descriptor Sets
    - [ ] Pipeline Layouts
        - [ ] Store Resource type in RenderGraph
    - [ ] Textures
        - [ ] Host Coherent (what you'd use for sand sim)
        - [ ] Transfer Queue
    - [ ] Integrate with Rendergraph
- [ ] Add Uniform analysis
- [ ] Draw Commands
    - [ ] Raw Draw (no indicies)
    - [ ] Draw with indicies
    - [ ] Bind Buffer (vertex, index, etc)
- [ ] Rendergraph Improvements
    - [ ] Pipeline Customization
    - [ ] Multiqueue support
    - [ ] Resize and swapchain recreation
    - [ ] Merge compatible passes into single Renderpass
    - [ ] Renderpass Reordering
    - [ ] Offline baking
- [ ] Compute considerations
    - [ ] Maybe add second queue? (will require additional synchronization)
    - [ ] Renderpass type

## Core:
- [ ] Handle all events
- [ ] Custom title
    - [ ] Window Decoration?
- [ ] Add more backends
    - [ ] Wayland
    - [ ] Win11
