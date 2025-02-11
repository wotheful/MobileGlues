//
// Created by Swung 0x48 on 2025/2/8.
//

#include "fpe.hpp"

#define DEBUG 0

struct glstate_t g_glstate;

GLsizei type_size(GLenum type) {
    switch (type) {
        case GL_SHORT:
            return 2;
        case GL_INT:
            return 4;
        case GL_FLOAT:
            return 4;
        case GL_DOUBLE:
            return 8;
    }
}

void commit_fpe_state_on_draw(GLenum mode, GLint first, GLsizei count) {
    LOAD_GLES_FUNC(glGenVertexArrays)
    LOAD_GLES_FUNC(glBindVertexArray)
    LOAD_GLES_FUNC(glGenBuffers)
    LOAD_GLES_FUNC(glBindBuffer)
    LOAD_GLES_FUNC(glBufferData)
    LOAD_GLES_FUNC(glCreateShader)
    LOAD_GLES_FUNC(glShaderSource)
    LOAD_GLES_FUNC(glCompileShader)
    LOAD_GLES_FUNC(glCreateProgram)
    LOAD_GLES_FUNC(glAttachShader)
    LOAD_GLES_FUNC(glLinkProgram)
    LOAD_GLES_FUNC(glGetShaderiv)
    LOAD_GLES_FUNC(glGetShaderInfoLog)
    LOAD_GLES_FUNC(glGetProgramiv)
    LOAD_GLES_FUNC(glVertexAttribPointer)
    LOAD_GLES_FUNC(glEnableVertexAttribArray)
    LOAD_GLES_FUNC(glDisableVertexAttribArray)

    bool is_first = true;

    // All assuming tightly packed here...
    auto& vpa = g_glstate.vertexpointer_array;
    gles_glBindVertexArray(vpa.fpe_vao);

    if (vpa.dirty) {
        vpa.dirty = false;

        for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
            bool enabled = ((vpa.enabled_pointers >> i) & 1);

            if (enabled) {
                auto &vp = vpa.pointers[i];

                if (is_first) {
                    vpa.starting_pointer = vp.pointer;
                    vpa.stride = vp.stride; // TODO: stride == 0?
                }

                const void* offset = (const void*)((const char*)vp.pointer - (const char*)vpa.starting_pointer);
                gles_glVertexAttribPointer(i, vp.size, vp.type, GL_FALSE, vp.stride, offset);

                gles_glEnableVertexAttribArray(i);

                is_first = false;
            } else {
                glDisableVertexAttribArray(i);
            }
        }
    }

    gles_glBindBuffer(GL_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_vbo);
    gles_glBufferData(GL_ARRAY_BUFFER, count * vpa.stride, vpa.starting_pointer, GL_DYNAMIC_DRAW);

//    gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_ibo);

}
