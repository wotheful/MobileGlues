//
// Created by BZLZHH on 2025/1/29.
//

#include "drawing.h"
#include "buffer.h"
#include "framebuffer.h"
#include "fpe/fpe.hpp"

#define DEBUG 0

void glMultiDrawElementsBaseVertex(GLenum mode, GLsizei* counts, GLenum type, const void* const* indices, GLsizei primcount, const GLint* basevertex) {
    LOG()

    LOAD_GLES_FUNC(glDrawElementsBaseVertex)

    for (GLsizei i = 0; i < primcount; ++i) {
        const GLsizei count = counts[i];
        if (count > 0) {
            gles_glDrawElementsBaseVertex(mode, count, type, indices[i], basevertex[i]);
        }
    }
}


void glMultiDrawElements(GLenum mode, const GLsizei* count, GLenum type, const void* const* indices, GLsizei primcount) {
    LOG()

    LOAD_GLES_FUNC(glDrawElements)

    for (GLsizei i = 0; i < primcount; ++i) {
        const GLsizei c = count[i];
        if (c > 0) {
            gles_glDrawElements(mode, c, type, indices[i]);
        }
    }
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    LOG()
    LOG_D("glDrawArrays(), mode = 0x%x, first = %d, count = %u", mode, first, count)

    INIT_CHECK_GL_ERROR

    CHECK_GL_ERROR_NO_INIT
    int do_draw_element = commit_fpe_state_on_draw(&mode, &first, &count);

    LOAD_GLES_FUNC(glDrawArrays)
    LOAD_GLES_FUNC(glDrawElements)


    if (do_draw_element) {
        LOG_D("Switch to glDrawElements(), mode = 0x%x, count = %u", mode, count)

        gles_glDrawElements(mode, count, GL_UNSIGNED_INT, (void *) 0);
    } else
        gles_glDrawArrays(mode, first, count);

    CHECK_GL_ERROR_NO_INIT
}


///*_Thread_local*/ static bool unexpected_error = false; // solve the crash error for ANGLE
// Why thread local here? We've never PRETEND we are thread safe.

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
    LOG_D("glDrawElements, mode: %d, count: %d, type: %d, indices: %p", mode, count, type, indices)
    LOAD_GLES_FUNC(glDrawElements)
    //LOAD_GLES_FUNC(glGetError)
    //GLenum pre_err = gles_glGetError();
    //if(pre_err != GL_NO_ERROR) {
    //    LOG_D("Skipping due to prior error: 0x%04X", pre_err)
    //    return;
    //}
    //if (!unexpected_error) {
    //    LOG_D("es_glDrawElements, mode: %d, count: %d, type: %d, indices: %p", mode, count, type, indices)
    gles_glDrawElements(mode, count, type, indices);
    CHECK_GL_ERROR
    //} else {
    //    unexpected_error = false;
    //}
}

void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) {
    LOG_D("glBindImageTexture, unit: %d, texture: %d, level: %d, layered: %d, layer: %d, access: %d, format: %d",
          unit, texture, level, layered, layer, access, format)
    LOAD_GLES_FUNC(glBindImageTexture)
    //LOAD_GLES_FUNC(glGetError)
    gles_glBindImageTexture(unit, texture, level, layered, layer, access, format);
    CHECK_GL_ERROR
    //GLenum err;
    //while((err = gles_glGetError()) != GL_NO_ERROR) {
    //    LOG_D("GL Error: 0x%04X", err)
    //    unexpected_error = true;
    //}
}

void glUniform1i(GLint location, GLint v0) {
    LOG_D("glUniform1i, location: %d, v0: %d", location, v0)
    LOAD_GLES_FUNC(glUniform1i)
    //LOAD_GLES_FUNC(glGetError)
    gles_glUniform1i(location, v0);
    CHECK_GL_ERROR
    //GLenum err;
    //while((err = gles_glGetError()) != GL_NO_ERROR) {
    //    LOG_D("GL Error: 0x%04X", err)
    //    unexpected_error = true;
    //}
}

void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
    LOG_D("glDispatchCompute, num_groups_x: %d, num_groups_y: %d, num_groups_z: %d",
          num_groups_x, num_groups_y, num_groups_z)
    LOAD_GLES_FUNC(glDispatchCompute)
    //LOAD_GLES_FUNC(glGetError)
    //GLenum pre_err = gles_glGetError();
    //if(pre_err != GL_NO_ERROR) {
    //    LOG_D("Skipping due to prior error: 0x%04X", pre_err)
    //    return;
    //}
    //if (!unexpected_error) {
    //    LOG_D("es_glDispatchCompute, num_groups_x: %d, num_groups_y: %d, num_groups_z: %d",
    //          num_groups_x, num_groups_y, num_groups_z)
    gles_glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    CHECK_GL_ERROR
    //} else {
    //    unexpected_error = false;
    //}
}