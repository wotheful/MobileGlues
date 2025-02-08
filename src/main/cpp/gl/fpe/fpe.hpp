//
// Created by Swung 0x48 on 2025/2/8.
//

#ifndef FOLD_CRAFT_LAUNCHER_FPE_HPP
#define FOLD_CRAFT_LAUNCHER_FPE_HPP

#include <glm/glm.hpp>
#include "../gl.h"
#include "../log.h"
#include <array>

#define DEBUG 0

struct glstate_t {
    glm::mat4 matrices[4];
    GLenum matrix_mode = GL_MODELVIEW;

    inline int matrix_idx() {
        switch (matrix_mode) {
            case GL_MODELVIEW:
                return 0;
            case GL_PROJECTION:
                return 1;
            case GL_TEXTURE:
                return 2;
            case GL_COLOR:
                return 3;
        }
        LOG_E("Error: 1282");
        return 0;
    }
};

extern struct glstate_t g_glstate;

#endif //FOLD_CRAFT_LAUNCHER_FPE_HPP
