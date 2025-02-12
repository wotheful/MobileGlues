//
// Created by Swung 0x48 on 2025/2/8.
//

#include "fpe.hpp"

#define DEBUG 0

glstate_t& glstate_t::get_instance() {
    static glstate_t s_glstate;
    return s_glstate;
}

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

void log_vtx_attrib_data(const void* ptr, GLenum type, int size, int stride, int offset, int idx) {
    const char* p = (const char*) ptr + idx * stride + offset;
    switch (type) {
        case GL_FLOAT: {
            LOG_D_N("(GL_FLOAT): (")
            const GLfloat *p_data = (const GLfloat *) p;
            for (int i = 0; i < size; ++i) {
                LOG_D_N("%.2f, ", p_data[i]);
            }
            LOG_D_N(") ")
            break;
        }
        case GL_UNSIGNED_BYTE: {
            LOG_D_N("(GL_UNSIGNED_BYTE): (")
            const GLubyte *p_data = (const GLubyte *) p;
            for (int i = 0; i < size; ++i) {
                LOG_D_N("%hhu, ", p_data[i]);
            }
            LOG_D_N(") ")
        }
    }
}

bool fpe_inited = false;
int init_fpe() {
    LOG_I("Initializing fixed-function pipeline...")

    char compile_info[1024];

    INIT_CHECK_GL_ERROR

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

    g_glstate.fpe_vtx_shader_src =
            "#version 320 es\n"
            "precision highp float;\n"
            "precision highp int;\n"
            "layout (location = 0) in vec3 vPos;\n"
            "layout (location = 2) in vec4 vColor;\n"
            "layout (location = 4) in vec2 vTexCoord;\n"
            "out vec4 fColor;\n"
            "out vec2 fTexCoord;\n"
            "void main() {\n"
            "   gl_Position = vec4(vPos, 1.0);\n"
            "   fColor = vColor;\n"
            "   fTexCoord = vTexCoord;\n"
            "}\n";

    g_glstate.fpe_frag_shader_src =
            "#version 320 es\n"
            "precision highp float;\n"
            "precision highp int;\n"
            "in vec4 fColor;\n"
            "in vec2 fTexCoord;"
            "out vec4 FragColor;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    FragColor = vec4(fTexCoord, 1., 1.);\n"
            "}";

    g_glstate.fpe_vtx_shader = gles_glCreateShader(GL_VERTEX_SHADER);
    CHECK_GL_ERROR_NO_INIT
    gles_glShaderSource(g_glstate.fpe_vtx_shader, 1, &g_glstate.fpe_vtx_shader_src, NULL);
    CHECK_GL_ERROR_NO_INIT
    gles_glCompileShader(g_glstate.fpe_vtx_shader);
    CHECK_GL_ERROR_NO_INIT
    int success = 0;
    gles_glGetShaderiv(g_glstate.fpe_vtx_shader, GL_COMPILE_STATUS, &success);
    CHECK_GL_ERROR_NO_INIT
    if (!success) {
        gles_glGetShaderInfoLog(g_glstate.fpe_vtx_shader, 1024, NULL, compile_info);
        CHECK_GL_ERROR_NO_INIT
        LOG_E("fpe vertex shader compile error: %s", compile_info);
        return -1;
    }

    g_glstate.fpe_frag_shader = gles_glCreateShader(GL_FRAGMENT_SHADER);
    CHECK_GL_ERROR_NO_INIT
    gles_glShaderSource(g_glstate.fpe_frag_shader, 1, &g_glstate.fpe_frag_shader_src, NULL);
    CHECK_GL_ERROR_NO_INIT

    gles_glCompileShader(g_glstate.fpe_frag_shader);
    CHECK_GL_ERROR_NO_INIT
    gles_glGetShaderiv(g_glstate.fpe_frag_shader, GL_COMPILE_STATUS, &success);
    CHECK_GL_ERROR_NO_INIT
    if (!success) {
        gles_glGetShaderInfoLog(g_glstate.fpe_frag_shader, 1024, NULL, compile_info);
        CHECK_GL_ERROR_NO_INIT
        LOG_E("fpe fragment shader compile error: %s", compile_info);
        return -1;
    }


    g_glstate.fpe_program = gles_glCreateProgram();
    CHECK_GL_ERROR_NO_INIT
    gles_glAttachShader(g_glstate.fpe_program, g_glstate.fpe_vtx_shader);
    CHECK_GL_ERROR_NO_INIT
    gles_glAttachShader(g_glstate.fpe_program, g_glstate.fpe_frag_shader);
    CHECK_GL_ERROR_NO_INIT
    gles_glLinkProgram(g_glstate.fpe_program);
    CHECK_GL_ERROR_NO_INIT
    gles_glGetProgramiv(g_glstate.fpe_program, GL_LINK_STATUS, &success);
    CHECK_GL_ERROR_NO_INIT
    if(!success) {
        glGetProgramInfoLog(g_glstate.fpe_program, 1024, NULL, compile_info);
        CHECK_GL_ERROR_NO_INIT
        LOG_E("fpe program link error: %s", compile_info);
        return -1;
    }

    gles_glGenVertexArrays(1, &g_glstate.vertexpointer_array.fpe_vao);
    CHECK_GL_ERROR_NO_INIT
    gles_glGenBuffers(1, &g_glstate.vertexpointer_array.fpe_vbo);
    CHECK_GL_ERROR_NO_INIT
    gles_glGenBuffers(1, &g_glstate.vertexpointer_array.fpe_ibo);
    CHECK_GL_ERROR_NO_INIT

    LOG_D("fpe_vao: %d", g_glstate.vertexpointer_array.fpe_vao)
    LOG_D("fpe_vbo: %d", g_glstate.vertexpointer_array.fpe_vbo)
    LOG_D("fpe_ibo: %d", g_glstate.vertexpointer_array.fpe_ibo)

    gles_glBindVertexArray(g_glstate.vertexpointer_array.fpe_vao);
    CHECK_GL_ERROR_NO_INIT
    gles_glBindVertexArray(0);
    CHECK_GL_ERROR_NO_INIT

    return 0;
}


int commit_fpe_state_on_draw(GLenum* mode, GLint* first, GLsizei* count) {
    LOG()

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

    if (!fpe_inited) {
        if (init_fpe() != 0)
            abort();
        else
            fpe_inited = true;
    }

    bool is_first = true;

    // All assuming tightly packed here...
    auto& vpa = g_glstate.vertexpointer_array;
//    gles_glGenVertexArrays(1, &vpa.fpe_vao);
    LOG_D("fpe_vao: %d", g_glstate.vertexpointer_array.fpe_vao)
    gles_glBindVertexArray(vpa.fpe_vao);
    CHECK_GL_ERROR_NO_INIT

    gles_glBindBuffer(GL_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_vbo);
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

#if DEBUG || GLOBAL_DEBUG
    for (int j = 0; j < *count; ++j) {
        for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
            bool enabled = ((vpa.enabled_pointers >> i) & 1);

            if (!enabled)
                continue;

            auto &vp = vpa.pointers[i];

            // const void* ptr, GLenum type, int size, int stride, int offset, int i
            log_vtx_attrib_data(vpa.starting_pointer, vp.type, vp.size, vp.stride,
                                (const char*)vp.pointer - (const char*)vpa.starting_pointer, j);

        }
        LOG_D("")
    }
#endif

    int ret = 0;

    LOG_D("glBufferData: size = %d, data = 0x%x -> GL_ARRAY_BUFFER (%d)", *count * vpa.stride, vpa.starting_pointer, g_glstate.vertexpointer_array.fpe_vbo)

    gles_glBufferData(GL_ARRAY_BUFFER, *count * vpa.stride, vpa.starting_pointer, GL_DYNAMIC_DRAW);
    CHECK_GL_ERROR_NO_INIT

    if (*mode == GL_QUADS) {
        vpa.fpe_ibo_buffer = quad_to_triangle(*count);

        LOG_D("glBufferData: size = %d, data = 0x%x -> GL_ELEMENT_ARRAY_BUFFER (%d)",
              vpa.fpe_ibo_buffer.size() * sizeof(uint32_t), vpa.fpe_ibo_buffer.data(), g_glstate.vertexpointer_array.fpe_ibo)

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
    gles_glBindVertexArray(g_glstate.vertexpointer_array.fpe_vao);
    CHECK_GL_ERROR_NO_INIT
//    gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_ibo);
    return ret;
}
