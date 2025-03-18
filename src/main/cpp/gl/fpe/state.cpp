//
// Created by Swung 0x48 on 2025/2/8.
//

#include "state.h"

#define DEBUG 0

bool hijack_fpe_states(GLenum cap, bool enable, fixed_function_bool_t* bools) {
    switch (cap) {
        case GL_FOG:
            bools->fog_enable = enable;
            return true;
        default:
            break;
    }
    return false;
}

void glEnable(GLenum cap) {
    LOG()
    LOG_D("glEnable, cap = 0x%x", cap);

    if (hijack_fpe_states(cap, true, &g_glstate.fpe_state.fpe_bools))
        return;

    LOAD_GLES_FUNC(glEnable)

    gles_glEnable(cap);
}

void glDisable(GLenum cap) {
    LOG()
    LOG_D("glDisable, cap = 0x%x", cap);

    if (hijack_fpe_states(cap, false, &g_glstate.fpe_state.fpe_bools))
        return;

    LOAD_GLES_FUNC(glDisable)

    gles_glDisable(cap);
}

void glClientActiveTexture(GLenum texture) {
    LOG()
    LOG_D("glClientActiveTexture(GL_TEXTURE%d)", texture - GL_TEXTURE0)

    g_glstate.fpe_state.client_active_texture = texture;
}

void glAlphaFunc( GLenum func, GLclampf ref ) {
    LOG()
    LOG_D("glAlphaFunc(%s, %f)", glEnumToString(func), ref)

    g_glstate.fpe_state.alpha_func = func;
    g_glstate.fpe_uniform.alpha_ref = ref;
}

void glFogf( GLenum pname, GLfloat param ) {
    LOG()
    LOG_D("glFogf(%s, %f)", glEnumToString(pname), param)

    switch (pname) {
        case GL_FOG_DENSITY:
            g_glstate.fpe_uniform.fog_intensity = param;
            return;
        case GL_FOG_START:
            g_glstate.fpe_uniform.fog_start = param;
            return;
        case GL_FOG_END:
            g_glstate.fpe_uniform.fog_end = param;
            return;

        // below should not be handled here
        case GL_FOG_MODE:
        case GL_FOG_INDEX:
        case GL_FOG_COORD_SRC:
            glFogi(pname, (GLint)param);
            return;

        default:
            LOG_D("Invalid glFogf pname: %s", pname);
    }
}

void glFogi( GLenum pname, GLint param ) {
    LOG()
    LOG_D("glFogi(%s, %s)", glEnumToString(pname), glEnumToString(param))
    switch (pname) {
        case GL_FOG_MODE:
            g_glstate.fpe_state.fog_mode = param;
            break;
        case GL_FOG_INDEX:
            g_glstate.fpe_state.fog_index = param;
            break;
        case GL_FOG_COORD_SRC:
            g_glstate.fpe_state.fog_coord_src = param;
            break;

        // below should not be handled here
        case GL_FOG_DENSITY:
        case GL_FOG_START:
        case GL_FOG_END:
            glFogf(pname, (GLfloat)param);
            return;
        default:
            LOG_D("Invalid glFogi pname: %s", pname);
    }
}

void glFogfv( GLenum pname, const GLfloat *params ) {
    LOG()
    LOG_D("glFogfv(%s, [...])", glEnumToString(pname))

    switch (pname) {
        case GL_FOG_COLOR: {
            auto& fcolor = g_glstate.fpe_uniform.fog_color;
            fcolor[0] = params[0];
            fcolor[1] = params[1];
            fcolor[2] = params[2];
            fcolor[3] = params[3];
            break;
        }
    }
}

void glFogiv( GLenum pname, const GLint *params ) {
    LOG()
    LOG_D("glFogiv(%s, [...])", glEnumToString(pname))

    switch (pname) {
        case GL_FOG_COLOR: {
            auto& fcolor = g_glstate.fpe_uniform.fog_color;
            fcolor[0] = (GLfloat)params[0] / (GLfloat)INT32_MAX;
            fcolor[1] = (GLfloat)params[1] / (GLfloat)INT32_MAX;
            fcolor[2] = (GLfloat)params[2] / (GLfloat)INT32_MAX;
            fcolor[3] = (GLfloat)params[3] / (GLfloat)INT32_MAX;
            break;
        }
    }
}