//
// Created by BZLZHH on 2025/1/29.
//

#include "drawing.h"
#include "buffer.h"
#include "framebuffer.h"
#include "fpe/fpe.hpp"

#define DEBUG 0

void glMultiDrawElementsBaseVertex( GLenum mode, GLsizei *counts, GLenum type, const void * const *indices, GLsizei primcount, const GLint * basevertex) {
    LOG();
    if (primcount <= 0 || !counts || !indices) return;
    for (int i = 0; i < primcount; i++) {
        if (counts[i] > 0)
            glDrawElementsBaseVertex(mode,
                    counts[i],
                    type,
                    indices[i],
                    basevertex[i]);
    }
}


void glMultiDrawElements(GLenum mode,const GLsizei * count,GLenum type,const void * const * indices,GLsizei primcount) {
    LOG();
    if (primcount <= 0 || !count || !indices) return;
    for (GLsizei i = 0; i < primcount; ++i) {
        if (count[i] > 0) {
            glDrawElements(
                    mode,
                    count[i],
                    type,
                    indices[i]
            );
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