//
// Created by Swung 0x48 on 2025/2/8.
//

#include "transformation.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_relational.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_float3.hpp>
#define DEBUG 0

int matrix_idx(GLenum matrix_mode) {
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

void glMatrixMode( GLenum mode ) {
    LOG()

    auto& transformation = g_glstate.transformation;

    switch (mode) {
        case GL_MODELVIEW:
        case GL_PROJECTION:
        case GL_TEXTURE:
        case GL_COLOR:
            transformation.matrix_mode = mode;
        default:
            break;
    }
}

void glLoadIdentity() {
    LOG()
    auto& transformation = g_glstate.transformation;

    transformation.matrices[matrix_idx(transformation.matrix_mode)] = glm::mat4();
}

void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val) {
    LOG()

    // TODO: precision loss?
    glOrthof(left, right, bottom, top, near_val, far_val);
}

void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    LOG()
    auto& transformation = g_glstate.transformation;

    transformation.matrices[matrix_idx(transformation.matrix_mode)] *= glm::ortho(left, right, bottom, top, zNear, zFar);
}

void glScalef( GLfloat x, GLfloat y, GLfloat z ) {
    LOG()
    auto& transformation = g_glstate.transformation;

    transformation.matrices[matrix_idx(transformation.matrix_mode)] =
            glm::scale(transformation.matrices[matrix_idx(transformation.matrix_mode)], glm::vec3(x, y, z));
}

void glTranslatef( GLfloat x, GLfloat y, GLfloat z ) {
    LOG()
    auto& transformation = g_glstate.transformation;

    transformation.matrices[matrix_idx(transformation.matrix_mode)] =
            glm::translate(transformation.matrices[matrix_idx(transformation.matrix_mode)], glm::vec3(x, y, z));
}

void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
    LOG()
    auto& transformation = g_glstate.transformation;

    transformation.matrices[matrix_idx(transformation.matrix_mode)] =
            glm::rotate(transformation.matrices[matrix_idx(transformation.matrix_mode)], angle, glm::vec3(x, y, z));
}

void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z ) {
    LOG()

    // TODO: precision loss?
    glRotatef(angle, x, y, z);
}

void glScaled(GLdouble x, GLdouble y, GLdouble z ) {
    LOG()

    // TODO: precision loss?
    glScalef(x, y, z);
}

void glTranslated(GLdouble x, GLdouble y, GLdouble z ) {
    LOG()

    // TODO: precision loss?
    glTranslatef(x, y, z);
}

void glPushMatrix( void ) {
    LOG()
    auto& transformation = g_glstate.transformation;

    auto idx = matrix_idx(transformation.matrix_mode);
    auto& mat = transformation.matrices[idx];
    transformation.matrices_stack[idx].push_back(mat);
}

void glPopMatrix( void ) {
    LOG()
    auto& transformation = g_glstate.transformation;

    auto idx = matrix_idx(transformation.matrix_mode);
    auto& mat = transformation.matrices[idx];
    mat = transformation.matrices_stack[idx].back();
    transformation.matrices_stack[idx].pop_back();
}
