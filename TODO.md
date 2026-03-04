# TODOS:

## Next Time
- [ ] RenderGraph Rewrite
    - [ ] Building Graph
        - [ ] design doc + mem layout
        - [ ] RenderPass builder
        - [ ] Gen Resources
        - [ ] Gen VkRenderPasses
        - [ ] build runtime Pipeline
    - [ ] Using Graph
        - [ ] Dispatch Commands
        - [ ] Resize


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
