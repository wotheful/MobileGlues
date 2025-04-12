//
// Created by Swung0x48 on 2024/10/8.
//

#include "../includes.h"
#include "gl.h"
#include "log.h"
#include "../gles/loader.h"
#include "mg.h"
#include "gl_mg.h"

#define DEBUG 0

void glClearDepth(GLclampd depth) {
    LOG()
    GLES.glClearDepthf((float)depth);
    CHECK_GL_ERROR
}

void glHint(GLenum target, GLenum mode) {
    LOG()
    LOG_D("glHint, target = %s, mode = %s", glEnumToString(target), glEnumToString(mode))
    GLES.glHint(target, mode);
}

void glDepthMask(GLboolean flag) {
    LOG()
    GLES.glDepthMask(flag);
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    LOG()
    GLES.glColorMask(red, green, blue, alpha);
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    LOG()
    GLES.glViewport(x, y, width, height);
}

void glClear (GLbitfield mask) {
    LOG()
    GLES.glClear(mask);
}

void glClearColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    LOG()
    GLES.glClearColor(red, green, blue, alpha);
}
