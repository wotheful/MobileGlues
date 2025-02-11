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

std::vector<uint32_t> quad_to_triangle(int n) {
    int num_quads = n / 4;
    int num_indices = num_quads * 6;

    std::vector<uint32_t> indices(num_indices, 0);

    for (int i = 0; i < num_quads; i++) {
        int base_index = i * 4;

        indices[i * 6] = base_index;
        indices[i * 6 + 1] = base_index + 1;
        indices[i * 6 + 2] = base_index + 2;

        indices[i * 6 + 3] = base_index + 2;
        indices[i * 6 + 4] = base_index + 3;
        indices[i * 6 + 5] = base_index;
    }

    return indices;
}

int commit_fpe_state_on_draw(GLenum* mode, GLint* first, GLsizei* count) {
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
    LOAD_GLES_FUNC(glUseProgram)

    INIT_CHECK_GL_ERROR

    bool is_first = true;

    // All assuming tightly packed here...
    auto& vpa = g_glstate.vertexpointer_array;
    gles_glBindVertexArray(vpa.fpe_vao);
    CHECK_GL_ERROR_NO_INIT

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
                CHECK_GL_ERROR_NO_INIT

                gles_glEnableVertexAttribArray(i);
                CHECK_GL_ERROR_NO_INIT

                LOG_D("attrib #%d: type = 0x%x, size = %d, stride = %d, usage = 0x%x, ptr = 0x%x, offset = %d", i, vp.type, vp.size, vp.stride, vp.usage, vp.pointer, (long)offset)

                is_first = false;
            } else {
                gles_glDisableVertexAttribArray(i);
                CHECK_GL_ERROR_NO_INIT
            }
        }
    }

    LOG_D("VB @ 0x%x, size = %d * %d = %d", vpa.starting_pointer, *count, vpa.stride, *count * vpa.stride)

    int ret = 0;

    gles_glBindBuffer(GL_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_vbo);
    CHECK_GL_ERROR_NO_INIT
    gles_glBufferData(GL_ARRAY_BUFFER, *count * vpa.stride, vpa.starting_pointer, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR_NO_INIT

    if (*mode == GL_QUADS) {
        vpa.fpe_ibo_buffer = quad_to_triangle(*count);

        gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_ibo);
        CHECK_GL_ERROR_NO_INIT
        gles_glBufferData(GL_ELEMENT_ARRAY_BUFFER, vpa.fpe_ibo_buffer.size() * sizeof(uint32_t),
                          vpa.fpe_ibo_buffer.data(), GL_DYNAMIC_DRAW);
        CHECK_GL_ERROR_NO_INIT
        *count = vpa.fpe_ibo_buffer.size();

        *mode = GL_TRIANGLES;
        ret = 1;
    }

    gles_glUseProgram(g_glstate.fpe_program);
    CHECK_GL_ERROR_NO_INIT
//    gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_ibo);
    return ret;
}
