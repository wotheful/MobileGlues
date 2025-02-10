//
// Created by Swung 0x48 on 2025/2/8.
//

#include "transformation.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
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

    // TODO: precision loss?
    glOrthof(left, right, bottom, top, near_val, far_val);
//    g_glstate.matrices[g_glstate.matrix_idx()] = glm::ortho(left, right, bottom, top, near_val, far_val);
}

void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] *= glm::ortho(left, right, bottom, top, zNear, zFar);
}

void glScalef( GLfloat x, GLfloat y, GLfloat z ) {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] =
            glm::scale(g_glstate.matrices[g_glstate.matrix_idx()], glm::vec3(x, y, z));
}

void glTranslatef( GLfloat x, GLfloat y, GLfloat z ) {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] =
            glm::translate(g_glstate.matrices[g_glstate.matrix_idx()], glm::vec3(x, y, z));
}

void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
    LOG()

    g_glstate.matrices[g_glstate.matrix_idx()] =
            glm::rotate(g_glstate.matrices[g_glstate.matrix_idx()], angle, glm::vec3(x, y, z));
}

void glPushMatrix( void ) {
    LOG()

    auto idx = g_glstate.matrix_idx();
    auto& mat = g_glstate.matrices[idx];
    g_glstate.matrices_stack[idx].push_back(mat);
}

void glPopMatrix( void ) {
    LOG()
    auto idx = g_glstate.matrix_idx();
    auto& mat = g_glstate.matrices[idx];
    mat = g_glstate.matrices_stack[idx].back();
    g_glstate.matrices_stack[idx].pop_back();
}
