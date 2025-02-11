//
// Created by Swung 0x48 on 2025/2/11.
//

#ifndef MOBILEGLUES_TYPES_H
#define MOBILEGLUES_TYPES_H

#include "../gl.h"
#include "../gles/loader.h"
#include "../gl/log.h"
#include <glm/glm.hpp>

struct transformation_t {
    glm::mat4 matrices[4];
    std::vector<glm::mat4> matrices_stack[4];
    GLenum matrix_mode = GL_MODELVIEW;
};

struct vertexpointer_t {
    GLint size;
    GLenum usage;
    GLenum type;
    GLsizei stride;
    const void *pointer;
};

#define VERTEX_POINTER_COUNT 8
struct vertex_pointer_array_t {
    // Fixed-function VAO
    // Reserve a vao purely for fpe, so that
    // it won't mess up with other states in
    // programmable pipeline.
    GLuint fpe_vao = 0;

    GLuint fpe_ibo = 0;

    struct vertexpointer_t pointers[VERTEX_POINTER_COUNT];
    uint32_t enabled_pointers = 0;
    bool dirty = false;
};



struct fixed_state_t {

};

struct glstate_t {
    struct transformation_t transformation;
    struct fixed_state_t state;
    struct vertex_pointer_array_t vertexpointer_array;
};

#endif //MOBILEGLUES_TYPES_H
