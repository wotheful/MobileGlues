//
// Created by Swung 0x48 on 2025/2/8.
//

#include "state.h"

#define DEBUG 0

void glEnable(GLenum cap) {
    LOG()
    LOG_D("glEnable, cap = 0x%x", cap);

    LOAD_GLES_FUNC(glEnable)

    gles_glEnable(cap);
}

void glDisable(GLenum cap) {
    LOG()
    LOG_D("glDisable, cap = 0x%x", cap);

    LOAD_GLES_FUNC(glDisable)

    gles_glDisable(cap);
}

void glClientActiveTexture(GLenum texture) {
    LOG()
    LOG_D("glClientActiveTexture(GL_TEXTURE%d)", texture - GL_TEXTURE0)

    g_glstate.state.client_active_texture = texture;
}