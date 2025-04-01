//
// Created by Swung 0x48 on 2025/4/1.
//

#include "drawing1x.h"
#include "fpe.hpp"
#include "list.h"

#define DEBUG 0

void glBegin( GLenum mode ) {
    LOG()
    LOG_D("glBegin(%s)", glEnumToString(mode))

    auto& s = g_glstate.fpe_draw;

    if (s.primitive != GL_NONE) {
        LOG_D("glBegin() nested! \nLast mode: %s", glEnumToString(s.primitive))
        return;
    }

    s.primitive = mode;
}

void glEnd() {
    LOG()

    auto& s = g_glstate.fpe_draw;
    if (s.primitive == GL_NONE) {
        LOG_E("glEnd() without effect: already ended");
        return;
    }

    // TODO: actual assembly work

    s.primitive = GL_NONE;
}

void glTexCoord2f( GLfloat s, GLfloat t ) {
    glTexCoord4f(s, t, 0, 1);
}

void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) {
    LOG()
    LOG_D("glTexCoord4f(%.2f, %.2f, %.2f, %.2f)", s, t, r, q)

    auto& state = g_glstate.fpe_draw;
    state.texcoord[0] = glm::vec4(s, t, r, q);
}


void glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {
    LOG()
    LOG_D("glVertex3f(%.2f, %.2f, %.2f)", x, y, z)
}

void glColor4f( GLfloat red, GLfloat green,
                GLfloat blue, GLfloat alpha ) {
    LOG()
    /* TODO: This can cause problem if per-vertex color is used,
     *  (using multiple glVertex*() calls in conjunction with
     *  multiple glColor4f() calls, rather than a single glColor4f()
     *  call and some gl*Pointer() call)
     *  Fix for that situation later.
    */
    LOG_D("glColor4f(%f, %f, %f, %f)", red, green, blue, alpha)

    if (!disableRecording && DisplayListManager::shouldRecord()) {
        displayListManager.record<glColor4f>({}, red, green, blue, alpha);
        if (DisplayListManager::shouldFinish())
            return;
    }

    auto& attr = g_glstate.fpe_state.vertexpointer_array.attributes[vp2idx(GL_COLOR_ARRAY)];
    auto& vpa = g_glstate.fpe_state.vertexpointer_array;
    vpa.enabled_pointers;
    if (vpa.buffer_based) {
        attr.size = 4;
        attr.usage = GL_COLOR_ARRAY;
        attr.type = GL_FLOAT;
        attr.normalized = GL_FALSE;
        attr.stride = 0;
        attr.pointer = 0;
        attr.value = glm::vec4(red, green, blue, alpha);
        attr.varies = false;
    } else {
        LOG_D("Not implemented!")
    }
}

