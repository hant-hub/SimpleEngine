#ifndef SE_USER_H
#define SE_USER_H
/*
 *
 * Header File for Communication between the engine and the 
 * Application DLL. 
 *
 * Here should be shared data structure definitions, as well
 * as function pointer definitions etc.
 *
 */

#include <platform.h>

#include <render/render.h>
#include <render/vertex.h>
#include <render/pipeline.h>
#include <render/memory.h>
#include <render/utils.h>


#define SE_INIT_FUNC(x) void(x)(SE_window* w)
#define SE_UPDATE_FUNC(x) void(x)(SE_window* w)
#define SE_DRAW_FUNC(x) void(x)(SE_window* w)

typedef SE_INIT_FUNC(*SE_init_func);
typedef SE_UPDATE_FUNC(*SE_update_func);
typedef SE_DRAW_FUNC(*SE_draw_func);

typedef struct SE_user_state {
    SE_init_func init;
    SE_update_func update;
    SE_draw_func draw;
} SE_user_state;




#endif
