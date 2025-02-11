//
// Created by Swung 0x48 on 2025/2/8.
//

#ifndef MOBILEGLUES_FPE_HPP
#define MOBILEGLUES_FPE_HPP

#include "../gl.h"
#include "../log.h"
#include "transformation.h"
#include "state.h"
#include "vertexpointer.h"

extern struct glstate_t g_glstate;

// 0 - keep DrawArray, 1 - switch to DrawElements
int commit_fpe_state_on_draw(GLenum* mode, GLint* first, GLsizei* count);

#endif //MOBILEGLUES_FPE_HPP
