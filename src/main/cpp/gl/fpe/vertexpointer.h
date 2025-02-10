//
// Created by Swung 0x48 on 2025/2/10.
//

#ifndef FOLD_CRAFT_LAUNCHER_VERTEXPOINTER_H
#define FOLD_CRAFT_LAUNCHER_VERTEXPOINTER_H

#include "../gl.h"
#include "../gles/loader.h"
#include "../gl/log.h"
#include "fpe.hpp"

#ifdef __cplusplus
extern "C" {
#endif

GLAPI GLAPIENTRY void glVertexPointer(GLint size,
                                      GLenum type,
                                      GLsizei stride,
                                      const void *pointer);

#ifdef __cplusplus
}
#endif

#endif //FOLD_CRAFT_LAUNCHER_VERTEXPOINTER_H
