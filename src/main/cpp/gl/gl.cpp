//
// Created by Swung0x48 on 2024/10/8.
//

#include "../includes.h"
#include "gl.h"
#include "log.h"
#include "../gles/loader.h"
#include "mg.h"
#include "gl_mg.h"

#define DEBUG 1

static void glClearDepth_static(GLclampd depth) {
    LOG()
    GLES.glClearDepthf((float)depth);
    CHECK_GL_ERROR
}

void glClearDepth(GLclampd depth) {
    glClearDepth_static(depth);
}

void glHint(GLenum target, GLenum mode) {
    LOG()
    LOG_D("glHint, target = %s, mode = %s", glEnumToString(target), glEnumToString(mode))
}
