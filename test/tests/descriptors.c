#include "graphics/graphics.h"
#include "se.h"
#include "cutils.h"
#include <stddef.h>

#include <core/introspect.h>


int main() {
    setdirExe();
    InitSE();
    SEwindow* win = CreateWindow(GlobalAllocator, "test", NULL);
    //SERenderPipelineInfo* r = SECreatePipeline(win);

    SEStructSpec vertSpec[] = {
        (SEStructSpec){
            .name = sstring("x"),
            .offset = 0,
            .type = SE_VAR_TYPE_F32,
            .size = sizeof(f32),
        },
        (SEStructSpec){
            .name = sstring("y"),
            .offset = sizeof(f32),
            .type = SE_VAR_TYPE_F32,
            .size = sizeof(f32),
        },
        (SEStructSpec){
            .name = sstring("z"),
            .offset = sizeof(f32) * 2,
            .type = SE_VAR_TYPE_F32,
            .size = sizeof(f32),
        },
    };

    //SEBeginRenderPass(r);
    //AddVertexBinding(r, SE_BINDING_VERTEX, vertSpec, ARRAY_SIZE(vertSpec));
    //SEEndRenderPass(r);
    

    /* API PLAN
        typedef struct Vertex {
            sem_v2f pos;
            sem_v3f color;
        } Vertex;
        
        typedef struct Uniform {
            f32 arg;
        } Uniform;

        SEStructSpec Vertex_spec[] = {
            {sstring("pos"), TYPE_V2F, sizeof(sem_v2f), offsetof(Vertex, pos)}, 
            ...
        };

        SEStructSpec Uniform_spec[] = {
            {sstring("arg"), TYPE_FLOAT, sizeof(f32), offsetof(Uniform, arg)},
        };

        ...

        int main() {
            ...
            SERenderInfo* r;
            u32 id = AddLayout(r, vert_src, frag_src, Vertex_spec);

            BeginPass();
            UseLayout(id);
            EndPass();
            ...
        }

        
    */
    typedef struct Vertex {
        u32 i;
        u32 j;
    } Vertex;
    offsetof(Vertex, j);

    while (0){
        Poll(win);


        if (win->keystate[KEY_ESC] == KEY_PRESSED) break;
    }

    DestroyWindow(win);
    return 0;
}
