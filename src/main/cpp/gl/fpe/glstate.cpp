//
// Created by Swung 0x48 on 2025/4/11.
//
#include "types.h"
#include "transformation.h"

#define DEBUG 0

void glstate_t::send_uniforms(int program) {
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