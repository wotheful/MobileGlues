//
// Created by Swung0x48 on 2024/10/8.
//

#include "../includes.h"
#include "GL/gl.h"
#include "GL/glcorearb.h"
#include "log.h"
#include "../gles/loader.h"
#include "mg.h"
#include "../config/settings.h"

#define DEBUG 0

void glClearDepth(GLclampd depth) {
    LOG()
    GLES.glClearDepthf((float)depth);
    CHECK_GL_ERROR
}

void glHint(GLenum target, GLenum mode) {
    LOG()
    LOG_D("glHint, target = %s, mode = %s", glEnumToString(target), glEnumToString(mode))
    CHECK_GL_ERROR
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    LOG()
    LOG_D("glClearColor, red = %f, green = %f, blue = %f, alpha = %f", red, green, blue, alpha)

    // Hook Mojang interface background color
    if (fabsf(red - 0.937255f) < 0.01f &&
        fabsf(green - 0.196078f) < 0.01f &&
        fabsf(blue - 0.239216f) < 0.01f &&
        fabsf(alpha - 1.000000f) < 0.01f) {
        red = global_settings.mojang_interface_color[0];
        green = global_settings.mojang_interface_color[1];
        blue = global_settings.mojang_interface_color[2];
        alpha = global_settings.mojang_interface_color[3];
        LOG_D("    Hook! %f %f %f %f", red, green, blue, alpha)
    }
    
    GLES.glClearColor(red, green, blue, alpha);
    CHECK_GL_ERROR
}