//
// Created by Swung 0x48 on 2025/2/8.
//

#include "transformation.h"
#include <glm/ext/matrix_clip_space.hpp>
#define DEBUG 0

void glMatrixMode( GLenum mode ) {
    LOG()

    switch (mode) {
        case GL_MODELVIEW:
        case GL_PROJECTION:
        case GL_TEXTURE:
        case GL_COLOR:
            g_glstate.matrix_mode = mode;
        default:
            break;
    }
}

void glLoadIdentity() {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] = glm::mat4();
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] = glm::ortho(left, right, bottom, top, near_val, far_val);
}
