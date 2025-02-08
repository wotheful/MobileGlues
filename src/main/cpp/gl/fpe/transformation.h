//
// Created by Swung 0x48 on 2025/2/8.
//

#ifndef FOLD_CRAFT_LAUNCHER_TRANSFORMATION_H
#define FOLD_CRAFT_LAUNCHER_TRANSFORMATION_H

#include "../gl.h"
#include "../gles/loader.h"
#include "../gl/log.h"
#include "fpe.hpp"

#ifdef __cplusplus
extern "C" {
#endif

GLAPI GLAPIENTRY void glMatrixMode( GLenum mode );
GLAPI GLAPIENTRY void glLoadIdentity();
GLAPI GLAPIENTRY void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val);

#ifdef __cplusplus
}
#endif

#endif //FOLD_CRAFT_LAUNCHER_TRANSFORMATION_H
