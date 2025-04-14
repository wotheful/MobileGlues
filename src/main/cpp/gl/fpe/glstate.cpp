//
// Created by Swung 0x48 on 2025/4/11.
//
#include "types.h"
#include "transformation.h"
#include "xxhash32.h"

#define DEBUG 0

void glstate_t::send_uniforms(int program) {
    LOG()
    INIT_CHECK_GL_ERROR
    const auto& mv = fpe_uniform.transformation.matrices[matrix_idx(GL_MODELVIEW)];
    const auto& proj = fpe_uniform.transformation.matrices[matrix_idx(GL_PROJECTION)];

    LOG_D("GL_MODELVIEW: ")
    print_matrix(mv);
    LOG_D("GL_PROJECTION: ")
    print_matrix(proj);

    // TODO: detect change and only set dirty bits here
    GLES.glBindVertexArray(fpe_state.fpe_vao);
    CHECK_GL_ERROR_NO_INIT
    GLint mvmat = GLES.glGetUniformLocation(program, "ModelViewMat");
    CHECK_GL_ERROR_NO_INIT
//    GLint projmat = GLES.glGetUniformLocation(program, "ProjMat");
//    CHECK_GL_ERROR_NO_INIT
    GLint mat_id = GLES.glGetUniformLocation(program, "ModelViewProjMat");
    CHECK_GL_ERROR_NO_INIT
    const auto mat = proj * mv;
    GLES.glUniformMatrix4fv(mvmat, 1, GL_FALSE, glm::value_ptr(fpe_uniform.transformation.matrices[matrix_idx(GL_MODELVIEW)]));
    CHECK_GL_ERROR_NO_INIT
//    GLES.glUniformMatrix4fv(projmat, 1, GL_FALSE, glm::value_ptr(fpe_uniform.transformation.matrices[matrix_idx(GL_PROJECTION)]));
//    CHECK_GL_ERROR_NO_INIT
    GLES.glUniformMatrix4fv(mat_id, 1, GL_FALSE, glm::value_ptr(mat));
    CHECK_GL_ERROR_NO_INIT
    GLES.glUniform1i(GLES.glGetUniformLocation(program, "Sampler0"), 0);
    CHECK_GL_ERROR_NO_INIT

    if (fpe_state.fpe_bools.fog_enable) {
        GLint fogcolor_id = GLES.glGetUniformLocation(program, "fogParam.color");
        CHECK_GL_ERROR_NO_INIT
        LOG_D("fogcolor_id = %d", fogcolor_id)
        GLES.glUniform4fv(fogcolor_id, 1, glm::value_ptr(fpe_uniform.fog_color));
        CHECK_GL_ERROR_NO_INIT
        GLint fogdensity_id = GLES.glGetUniformLocation(program, "fogParam.density");
        CHECK_GL_ERROR_NO_INIT
        GLES.glUniform1f(fogdensity_id, fpe_uniform.fog_density);
        CHECK_GL_ERROR_NO_INIT
        GLint fogstart_id = GLES.glGetUniformLocation(program, "fogParam.start");
        CHECK_GL_ERROR_NO_INIT
        GLES.glUniform1f(fogstart_id, fpe_uniform.fog_start);
        CHECK_GL_ERROR_NO_INIT
        GLint fogend_id = GLES.glGetUniformLocation(program, "fogParam.end");
        CHECK_GL_ERROR_NO_INIT
        GLES.glUniform1f(fogend_id, fpe_uniform.fog_end);
        CHECK_GL_ERROR_NO_INIT
    }
}

uint32_t glstate_t::program_hash() {
    // TODO: Make a proper hash
    uint64_t key = g_glstate.fpe_state.vertexpointer_array.enabled_pointers;
    key |= ((g_glstate.fpe_state.fpe_bools.fog_enable & 1) << 63);
    assert(key != 0);
    return key;
}

uint32_t glstate_t::vertex_attrib_hash() {
    uint32_t ret = 0;

    auto& va = fpe_state.vertexpointer_array;

    XXHash32 hash(s_hash_seed);
    hash.add(&va.starting_pointer, sizeof(va.starting_pointer));
    for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
        hash.add(&i, sizeof(i));
        auto& attr = va.attributes[i];
//        hash.add()
    }

    uint32_t result = hash.hash();

}

program_t& glstate_t::get_or_generate_program(const uint64_t key) {
    LOG()
    if (g_glstate.fpe_programs.find(key)
        == g_glstate.fpe_programs.end()) {
        LOG_D("Generating new shader: 0x%x", key)
        fpe_shader_generator gen(g_glstate.fpe_state);
        program_t program = gen.generate_program();
        int prog_id = program.get_program();
        if (prog_id > 0)
            g_glstate.fpe_programs[key] = program;
        else {
            LOG_D("Error: FPE shader link failed!")
            // reserve key==0 as null program for failure
            return g_glstate.fpe_programs[0];
        }
    }

    auto& prog = g_glstate.fpe_programs[key];
    return prog;
}

int glstate_t::get_vao(const uint64_t key) {
    LOG()
    if (g_glstate.fpe_vaos.find(key)
        == g_glstate.fpe_vaos.end()) {
        return -1;
    }

    return g_glstate.fpe_vaos[key];
}

void glstate_t::save_vao(const uint64_t key, const GLuint vao) {
    LOG()
    g_glstate.fpe_vaos[key] = vao;
}

void glstate_t::send_vertex_attributes() {
    LOG()
    INIT_CHECK_GL_ERROR
    auto& va = g_glstate.fpe_state.vertexpointer_array;
    if (!va.dirty) return;

    va.dirty = false;

    va.normalize();

    for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
        bool enabled = ((va.enabled_pointers >> i) & 1);

        auto &vp = va.attributes[i];
        if (enabled) {
            vp.stride = va.stride;

            GLES.glVertexAttribPointer(
                    i, vp.size, vp.type, vp.normalized, vp.stride, vp.pointer);
            CHECK_GL_ERROR_NO_INIT

            GLES.glEnableVertexAttribArray(i);
            CHECK_GL_ERROR_NO_INIT

            LOG_D("attrib #%d: type = %s, size = %d, stride = %d, usage = %s, ptr = %p",
                  i, glEnumToString(vp.type), vp.size, vp.stride, glEnumToString(vp.usage), vp.pointer)
        }
        else {
            switch (vp.usage) {
                case GL_COLOR_ARRAY:
                    if (g_glstate.fpe_draw.current_data.sizes.color_size > 0) {
                        const auto& v = g_glstate.fpe_draw.current_data.color;
                        LOG_D("attrib #%d: type = %s, usage = %s, value = (%.2f, %.2f, %.2f, %.2f) (disabled)",
                              i, glEnumToString(vp.type), glEnumToString(vp.usage),
                              v[0], v[1], v[2], v[3])

                        GLES.glVertexAttrib4fv(i, glm::value_ptr(v));
                        CHECK_GL_ERROR_NO_INIT
                    }
                    break;
                case GL_NORMAL_ARRAY:
                    if (g_glstate.fpe_draw.current_data.sizes.normal_size > 0) {
                        const auto& v = g_glstate.fpe_draw.current_data.normal;
                        LOG_D("attrib #%d: type = %s, usage = %s, value = (%.2f, %.2f, %.2f, %.2f) (disabled)",
                              i, glEnumToString(vp.type), glEnumToString(vp.usage),
                              v[0], v[1], v[2], v[3])

                        GLES.glVertexAttrib4fv(i, glm::value_ptr(v));
                        CHECK_GL_ERROR_NO_INIT
                    }
                    break;
                default:
                    LOG_D("attrib #%d: (disabled)", i)
                    break;
            }

            GLES.glDisableVertexAttribArray(i);
            CHECK_GL_ERROR_NO_INIT
        }
    }
}
