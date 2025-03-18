//
// Created by Swung 0x48 on 2025/2/8.
//

#include "fpe.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fpe_shader.h"

#define DEBUG 1

#if GLOBAL_DEBUG || DEBUG
#pragma clang optimize off
#endif

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

        indices[i * 6 + 0] = base_index + 0;
        indices[i * 6 + 1] = base_index + 1;
        indices[i * 6 + 2] = base_index + 2;

        indices[i * 6 + 3] = base_index + 2;
        indices[i * 6 + 4] = base_index + 3;
        indices[i * 6 + 5] = base_index + 0;
    }

    return indices;
}

#if DEBUG || GLOBAL_DEBUG
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
#endif

bool fpe_inited = false;
int init_fpe() {
    LOG_D("Initializing fixed-function pipeline...")

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

    shaderconv_need_s vs_need = {
            .need_color = 1,
            .need_secondary = 0,
            .need_fogcoord = 0,
            .need_texcoord = 1,
            .need_notexarray = 0,
            .need_normalmatrix = 0,
            .need_mvmatrix = 1,
            .need_mvpmatrix = 0,
            .need_clean = 0,
            .need_clipvertex = 0,
            .need_texs = 0
    };

    fpe_state_t fpe_state = {0};
    memset(&fpe_state, 0, sizeof(fpe_state_t));
    fpe_state.plane = 0;
    fpe_state.fogmode = 0;
    fpe_state.fogdist = 0;
    fpe_state.fogsource = 0;
    fpe_state.fog = 0;
    fpe_state.colorsum = 0;
    fpe_state.lighting = 0;
    fpe_state.normalize = 0;
    fpe_state.rescaling = 0;
    fpe_state.alphafunc = FPE_LEQUAL;
    fpe_state.alphatest = 1;

    auto* vs = fpe_VertexShader(&vs_need, &fpe_state);
    auto* fs = fpe_FragmentShader(&vs_need, &fpe_state);
    LOG_D("fpe_VS: \n%s\n", *vs);
    LOG_D("fpe_FS: \n%s\n", *fs);

    auto es_vs = GLSLtoGLSLES(*vs, GL_VERTEX_SHADER, hardware->es_version, 100);
    LOG_D("fpe_ES_VS: \n%s\n", es_vs.c_str());
    auto es_fs = GLSLtoGLSLES(*fs, GL_FRAGMENT_SHADER, hardware->es_version, 100);
    LOG_D("fpe_ES_PS: \n%s\n", es_fs.c_str());

    g_glstate.fpe_vtx_shader_src =
            "#version 320 es\n"
            "precision highp float;\n"
            "precision highp int;\n"
            "uniform mat4 ModelViewMat;\n"
            "uniform mat4 ProjMat;\n"
            "layout (location = 0) in vec3 Pos;\n"
            "layout (location = 1) in vec3 Normal;\n"
            "layout (location = 2) in vec4 Color;\n"
            "layout (location = 7) in vec2 UV0;\n"
            "out vec4 vertexColor;\n"
            "out vec2 texCoord0;\n"
            "void main() {\n"
            "   gl_Position = ProjMat * ModelViewMat * vec4(Pos, 1.0);\n"
            "   vertexColor = Color;\n"
            "   texCoord0 = UV0;\n"
            "}\n";

    g_glstate.fpe_frag_shader_src =
            "#version 320 es\n"
            "precision highp float;\n"
            "precision highp int;\n"
            "uniform sampler2D Sampler0;\n"
            "in vec4 vertexColor;\n"
            "in vec2 texCoord0;\n"
            "out vec4 FragColor;\n"
            "\n"
            "void main() {\n"
            "   vec4 color = texture(Sampler0, texCoord0) * vertexColor;"
            "   if (color.a < 0.1) {\n"
            "       discard;\n"
            "   }\n"
            "   FragColor = color;\n"
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
    LOAD_GLES_FUNC(glVertexAttrib4fv)
    LOAD_GLES_FUNC(glEnableVertexAttribArray)
    LOAD_GLES_FUNC(glDisableVertexAttribArray)
    LOAD_GLES_FUNC(glUseProgram)
    LOAD_GLES_FUNC(glGetUniformLocation)
    LOAD_GLES_FUNC(glUniformMatrix4fv)
    LOAD_GLES_FUNC(glUniform1i)
    LOAD_GLES_FUNC(glGetIntegerv)

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


    GLint prev_vbo = 0;
    gles_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prev_vbo);
    CHECK_GL_ERROR_NO_INIT

    // Ugh...Why binding vbo is required BEFORE calling VertexAttrib* functions?
    if (prev_vbo == 0) {
        gles_glBindBuffer(GL_ARRAY_BUFFER, g_glstate.vertexpointer_array.fpe_vbo);
        CHECK_GL_ERROR_NO_INIT
    }

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
                gles_glVertexAttribPointer(i, vp.size, vp.type, vp.normalized, vp.stride, offset);
                CHECK_GL_ERROR_NO_INIT

                gles_glEnableVertexAttribArray(i);
                CHECK_GL_ERROR_NO_INIT

                LOG_D("attrib #%d: type = 0x%x, size = %d, stride = %d, usage = 0x%x, ptr = 0x%x, offset = %d", i, vp.type, vp.size, vp.stride, vp.usage, vp.pointer, (long)offset)

                is_first = false;
            } else if (vpa.pointers[i].usage == GL_COLOR_ARRAY) {
                auto &vp = vpa.pointers[i];

                LOG_D("attrib #%d: type = 0x%x, usage = 0x%x, value = (1., 1., 1., 1.)", i, vp.type, vp.usage)

                static GLfloat att[] = { 1., 1., 1., 1. };
                gles_glVertexAttrib4fv(i, att);
                CHECK_GL_ERROR_NO_INIT

                gles_glDisableVertexAttribArray(i);
                CHECK_GL_ERROR_NO_INIT
            } else {
                gles_glDisableVertexAttribArray(i);
                CHECK_GL_ERROR_NO_INIT
            }
        }
    }

    int ret = 0;

    // Making sure it is a valid pointer rather than an offset into the buffer
    if (vpa.starting_pointer != nullptr && vpa.starting_pointer > (void*)vpa.stride) {
        LOG_D("VB @ 0x%x, size = %d * %d = %d", vpa.starting_pointer, *count, vpa.stride, *count * vpa.stride)

#if DEBUG || GLOBAL_DEBUG
            //    for (int j = 0; j < *count; ++j) {
//        for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
//            bool enabled = ((vpa.enabled_pointers >> i) & 1);
//
//            if (!enabled)
//                continue;
//
//            auto &vp = vpa.pointers[i];
//
//            // const void* ptr, GLenum type, int size, int stride, int offset, int i
//            log_vtx_attrib_data(vpa.starting_pointer, vp.type, vp.size, vp.stride,
//                                (const char*)vp.pointer - (const char*)vpa.starting_pointer, j);
//
//        }
//        LOG_D("")
//    }
#endif

        LOG_D("glBufferData: size = %d, data = 0x%x -> GL_ARRAY_BUFFER (%d)", *count * vpa.stride,
              vpa.starting_pointer, g_glstate.vertexpointer_array.fpe_vbo)

        gles_glBufferData(GL_ARRAY_BUFFER, *count * vpa.stride, vpa.starting_pointer,
                          GL_DYNAMIC_DRAW);
        CHECK_GL_ERROR_NO_INIT
    } else {
        LOG_D("Using already bound VB")
    }

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

    const auto& mv = g_glstate.transformation.matrices[matrix_idx(GL_MODELVIEW)];
    const auto& proj = g_glstate.transformation.matrices[matrix_idx(GL_PROJECTION)];

    LOG_D("GL_MODELVIEW: ")
    print_matrix(mv);
    LOG_D("GL_PROJECTION: ")
    print_matrix(proj);

    gles_glUseProgram(g_glstate.fpe_program);
    CHECK_GL_ERROR_NO_INIT
    gles_glBindVertexArray(g_glstate.vertexpointer_array.fpe_vao);
    CHECK_GL_ERROR_NO_INIT
    GLint mvmat = gles_glGetUniformLocation(g_glstate.fpe_program, "ModelViewMat");
    CHECK_GL_ERROR_NO_INIT
    GLint projmat = gles_glGetUniformLocation(g_glstate.fpe_program, "ProjMat");
    CHECK_GL_ERROR_NO_INIT
    gles_glUniformMatrix4fv(mvmat, 1, GL_FALSE, glm::value_ptr(g_glstate.transformation.matrices[matrix_idx(GL_MODELVIEW)]));
    CHECK_GL_ERROR_NO_INIT
    gles_glUniformMatrix4fv(projmat, 1, GL_FALSE, glm::value_ptr(g_glstate.transformation.matrices[matrix_idx(GL_PROJECTION)]));
    CHECK_GL_ERROR_NO_INIT
    gles_glUniform1i(gles_glGetUniformLocation(g_glstate.fpe_program, "Sampler0"), 0);

    return ret;
}
