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
    GLenum normalized;
    GLsizei stride;
    const void *pointer;
};

#define VERTEX_POINTER_COUNT (8 + 16)
struct vertex_pointer_array_t {
    // Fixed-function VAO
    // Reserve a vao purely for fpe, so that
    // it won't mess up with other states in
    // programmable pipeline.
    GLuint fpe_vao = 0;

    GLuint fpe_vbo = 0;

    GLuint fpe_ibo = 0;

    std::vector<uint32_t> fpe_ibo_buffer;

    const void* starting_pointer = NULL;
    GLsizei stride = 0;

    struct vertexpointer_t pointers[VERTEX_POINTER_COUNT];
    uint32_t enabled_pointers = 0;
    bool dirty = false;
};

struct fixed_function_bool_t { // glEnable/glDisable
    bool fog_enable = false; // GL_FOG
};

struct fixed_function_state_t {
    GLenum client_active_texture = GL_TEXTURE0; // glClientActiveTexture, specifies active texcood
    GLenum alpha_func = GL_ALWAYS; // glAlphaFunc
    GLenum fog_mode = GL_EXP; // glFogi(GL_FOG_MODE)
    GLint fog_index = 0; // glFogi(GL_FOG_INDEX)
    GLenum fog_coord_src = GL_FRAGMENT_DEPTH; // glFogi(GL_FOG_COORD_SRC)

    struct fixed_function_bool_t fpe_bools;
};

struct fixed_function_uniform_t {
    // glAlphaFunc
    GLclampf alpha_ref = 0.0f;

    // glFogf
    GLfloat fog_intensity = 1.f;
    GLfloat fog_start = 0.f;
    GLfloat fog_end = 1.f;
    // glFogfv/glFogiv
    GLfloat fog_color[4] = { 0., 0., 0., 0. };
};

struct glstate_t {
    struct transformation_t transformation;

    struct fixed_function_state_t fpe_state;
    struct fixed_function_uniform_t fpe_uniform;

    struct vertex_pointer_array_t vertexpointer_array;
    GLuint fpe_vtx_shader = 0;
    GLuint fpe_frag_shader = 0;
    GLuint fpe_program = 0;
    const char* fpe_vtx_shader_src;
    const char* fpe_frag_shader_src;

    static glstate_t& get_instance();
};

#endif //MOBILEGLUES_TYPES_H
