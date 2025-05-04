#include "platform/wayland/wayland.h"
#include <platform.h>
#include <render/render.h>
#include <render/pipeline.h>

#include <math/mat.h>
#include <stdio.h>

#include <user.h>

SE_INIT_FUNC(Init) {
}

SE_UPDATE_FUNC(Update) {
}

SE_DRAW_FUNC(Draw) {
}


SE_user_state state = {
    .init = Init,
    .update = Update,
    .draw = Draw,
};
