//
// Created by BZLZHH on 2025/1/28.
//

#include "getter.h"
#include "../config/settings.h"
#include "buffer.h"
#include "fpe/fpe.hpp"
#include <glm/glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

#define DEBUG 0

void glGetFloatv(GLenum pname, GLfloat *params) {
    LOG()
    LOG_D("glGetFloatv, pname: %s", glEnumToString(pname))

    switch (pname) {
        case GL_MODELVIEW_MATRIX:{
            auto* ptr = glm::value_ptr(g_glstate.fpe_uniform.transformation.matrices[matrix_idx(GL_MODELVIEW)]);
            memcpy(params, ptr, sizeof(GLfloat) * 16);
            break;
        }
        case GL_PROJECTION_MATRIX:
        {
            auto* ptr = glm::value_ptr(g_glstate.fpe_uniform.transformation.matrices[matrix_idx(GL_PROJECTION)]);
            memcpy(params, ptr, sizeof(GLfloat) * 16);
            break;
        }
        default:
            GLES.glGetFloatv(pname, params);
            LOG_D("  -> %.2f",*params)
            CHECK_GL_ERROR
    }
}

void glGetIntegerv(GLenum pname, GLint *params) {
    LOG()
    LOG_D("glGetIntegerv, pname: %s", glEnumToString(pname))
    switch (pname) {
        case GL_CONTEXT_PROFILE_MASK:
            (*params) = GL_CONTEXT_COMPATIBILITY_PROFILE_BIT;
            break;
        case GL_NUM_EXTENSIONS:
            static GLint num_extensions = -1;
            if (num_extensions == -1) {
                const GLubyte* ext_str = glGetString(GL_EXTENSIONS);
                if (ext_str) {
                    char* copy = strdup((const char*)ext_str);
                    char* token = strtok(copy, " ");
                    num_extensions = 0;
                    while (token) {
                        num_extensions++;
                        token = strtok(nullptr, " ");
                    }
                    free(copy);
                } else {
                    num_extensions = 0;
                }
            }
            (*params) = num_extensions;
            break;
        case GL_MAJOR_VERSION:
            (*params) = 4;
            break;
        case GL_MINOR_VERSION:
            (*params) = 0;
            break;
        case GL_MAX_TEXTURE_IMAGE_UNITS: {
            if (g_gles_caps.maxtex <= 0) {
                int es_params = 16;
                GLES.glGetIntegerv(pname, &es_params);
                CHECK_GL_ERROR
                g_gles_caps.maxtex = (es_params < 32) ? 32 : es_params;
            }
            (*params) = g_gles_caps.maxtex;
            break;
        }
        case GL_ARRAY_BUFFER_BINDING:
        case GL_ATOMIC_COUNTER_BUFFER_BINDING:
        case GL_COPY_READ_BUFFER_BINDING:
        case GL_COPY_WRITE_BUFFER_BINDING:
        case GL_DRAW_INDIRECT_BUFFER_BINDING:
        case GL_DISPATCH_INDIRECT_BUFFER_BINDING:
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        case GL_PIXEL_PACK_BUFFER_BINDING:
        case GL_PIXEL_UNPACK_BUFFER_BINDING:
        case GL_SHADER_STORAGE_BUFFER_BINDING:
        case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
        case GL_UNIFORM_BUFFER_BINDING:
            (*params) = (int) find_bound_buffer(pname);
            LOG_D("  -> %d",*params)
            break;
        case GL_VERTEX_ARRAY_BINDING:
            (*params) = (int) find_bound_array();
            break;
        default:
            GLES.glGetIntegerv(pname, params);
            LOG_D("  -> %d",*params)
            CHECK_GL_ERROR
    }
}

GLenum glGetError() {
    LOG()
    GLenum err = GLES.glGetError();
    // just clear gles error, no reporting
    if (err != GL_NO_ERROR) {
        // no logging without DEBUG
        LOG_W("glGetError\n -> %d", err)
        LOG_W("Now try to cheat.")
    }
    return GL_NO_ERROR;
}

static std::string es_ext;
const char* GetExtensionsList() {
    return es_ext.c_str();
}

void InitGLESBaseExtensions() {
    es_ext = "GL_ARB_fragment_program "
             "GL_ARB_vertex_buffer_object "
             "GL_ARB_vertex_array_object "
             "GL_ARB_vertex_buffer "
             "GL_EXT_vertex_array "
             "GL_ARB_ES2_compatibility "
             "GL_ARB_ES3_compatibility "
             "GL_EXT_packed_depth_stencil "
             "GL_EXT_depth_texture "
             "GL_ARB_depth_texture "
             "GL_ARB_shading_language_100 "
             "GL_ARB_imaging "
             "GL_ARB_draw_buffers_blend "
             "OpenGL11 "
             "OpenGL12 "
             "OpenGL13 "
             "OpenGL14 "
             "OpenGL15 "
             "OpenGL20 "
             "OpenGL21 "
             "OpenGL30 "
             "OpenGL31 "
             "OpenGL32 "
             "OpenGL33 "
             "OpenGL40 "
             //"OpenGL43 "
             //"ARB_compute_shader "
             "GL_ARB_get_program_binary "
             "GL_ARB_multitexture "
             "GL_ARB_shader_storage_buffer_object "
             "GL_ARB_shader_image_load_store "
             "GL_ARB_clear_texture "
             "GL_ARB_get_program_binary "
             "GL_ARB_texture_float "
             "GL_EXT_texture_filter_anisotropic "
             "GL_ARB_point_sprite "
             "GL_ARB_pixel_buffer_object "
             "GL_ARB_texture_non_power_of_two "
             "GL_ARB_vertex_buffer_object "
             "GL_EXT_framebuffer_object "
             "GL_ARB_framebuffer_object "
             "GL_EXT_framebuffer_multisample_blit_scaled "
             "GL_EXT_framebuffer_blit_layers "
             "GL_EXT_framebuffer_blit "
             "GL_ARB_occlusion_query "
             "GL_ARB_program_interface_query "
             "GL_ARB_texture_rectangle "
             "GL_ARB_multisample "
             "GL_EXT_framebuffer_multisample "
             "GL_ARB_uniform_buffer_object "
             "GL_ARB_shader_objects "
             "GL_ARB_vertex_shader "
             "GL_ARB_fragment_shader "
             "GL_EXT_separate_shader_objects "
             "GL_ARB_separate_shader_objects ";
}

void AppendExtension(const char* ext) {
    es_ext += ext;
    es_ext += ' ';
}

const char* getBeforeThirdSpace(const char* str) {
    static char buffer[256];
    int spaceCount = 0;
    const char* start = str;
    while (*str) {
        if (*str == ' ') {
            spaceCount++;
            if (spaceCount == 3) break;
        }
        str++;
    }
    long len = str - start;
    if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
    strncpy(buffer, start, len);
    buffer[len] = '\0';
    return buffer;
}

const char* getGpuName() {
    const char *gpuName = (const char *) GLES.glGetString(GL_RENDERER);

    if (!gpuName) {
        return "<unknown>";
    }

    if (strncmp(gpuName, "ANGLE", 5) == 0) {
        std::string gpuStr(gpuName);

        size_t firstParen = gpuStr.find('(');
        size_t secondParen = gpuStr.find('(', firstParen + 1);
        size_t lastParen = gpuStr.rfind('(');

        std::string gpu = gpuStr.substr(secondParen + 1, lastParen - secondParen - 2);

        size_t vulkanStart = gpuStr.find("Vulkan ");
        size_t vulkanEnd = gpuStr.find(' ', vulkanStart + 7);
        std::string vulkanVersion = gpuStr.substr(vulkanStart + 7, vulkanEnd - (vulkanStart + 7));

        std::string formattedGpuName = gpu + " | ANGLE | Vulkan " + vulkanVersion;

        char* result = new char[formattedGpuName.size() + 1];
        std::strcpy(result, formattedGpuName.c_str());
        return result;
    }

    return gpuName;
}

void set_es_version() {
    const char* ESVersion = getBeforeThirdSpace((const char*)GLES.glGetString(GL_VERSION));
    int major, minor;

    if (sscanf(ESVersion, "OpenGL ES %d.%d", &major, &minor) == 2) {
        hardware->es_version = major * 100 + minor * 10;
    } else {
        hardware->es_version = 320;
    }
    LOG_I("OpenGL ES Version: %s (%d)", ESVersion, hardware->es_version)
    if (hardware->es_version < 300) {
        LOG_I("OpenGL ES version is lower than 3.0! This version is not supported!")
    }
}

const char* getGLESName() {
    char* ESVersion = (char*)GLES.glGetString(GL_VERSION);
    return getBeforeThirdSpace(ESVersion);
}

static std::string rendererString;
static std::string versionString;
const GLubyte * glGetString( GLenum name ) {
    LOG()
    /* Feature in the Future
     * Advanced OpenGL driver: NV renderer.
    switch (name) {
        case GL_VENDOR:
            return (const GLubyte *) "NVIDIA Corporation";
        case GL_VERSION:
            return (const GLubyte *) "4.6.0 NVIDIA 572.16";
        case GL_RENDERER:
            return (const GLubyte *) "NVIDIA GeForce RTX 5090/PCIe/SSE2";
        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *) "4.50 MobileGlues with glslang and SPIRV-Cross";
        case GL_EXTENSIONS:
            return (const GLubyte *) GetExtensionsList();
    }
    */
    switch (name) {
        case GL_VENDOR: {
            if(versionString.empty()) {
                std::string vendor = "Swung0x48, BZLZHH, Tungsten";
#if defined(VERSION_TYPE) && (VERSION_TYPE == VERSION_ALPHA)
                vendor += " | §c§l内测版本, 严禁任何外传!§r";
#endif
                versionString = vendor;
            }
            return (const GLubyte *)versionString.c_str();
        }
        case GL_VERSION: {
            static std::string versionCache;
            if (versionCache.empty()) {
                versionCache = "4.0.0 MobileGlues ";
                versionCache += std::to_string(MAJOR) + "."
                                +  std::to_string(MINOR) + "."
                                +  std::to_string(REVISION);
#if PATCH != 0
                versionCache += "." + std::to_string(PATCH);
#endif
#if defined(VERSION_TYPE)
#if VERSION_TYPE == VERSION_ALPHA
                versionCache += " | §4§l如果您在公开平台看到这一提示, 则发布者已违规!§r";
#elif VERSION_TYPE == VERSION_DEVELOPMENT
                versionCache += ".Dev";
#endif
#endif
                versionCache += VERSION_SUFFIX;
            }
            return (const GLubyte *)versionCache.c_str();
        }

        case GL_RENDERER:
        {
            if (rendererString == std::string("")) {
                const char* gpuName = getGpuName();
                const char* glesName = getGLESName();
                rendererString = std::string(gpuName) + " | " + std::string(glesName);
            }
            return (const GLubyte *)rendererString.c_str();
        }
        case GL_SHADING_LANGUAGE_VERSION:
            return (const GLubyte *) "4.50 MobileGlues with glslang and SPIRV-Cross";
        case GL_EXTENSIONS:
            return (const GLubyte *) GetExtensionsList();
        default:
            return GLES.glGetString(name);
    }
}

const GLubyte * glGetStringi(GLenum name, GLuint index) {
    LOG()
    typedef struct {
        GLenum name;
        const char** parts;
        GLuint count;
    } StringCache;
    static StringCache caches[] = {
            {GL_EXTENSIONS, nullptr, 0},
            {GL_VENDOR, nullptr, 0},
            {GL_VERSION, nullptr, 0},
            {GL_SHADING_LANGUAGE_VERSION, nullptr, 0}
    };
    static int initialized = 0;
    if (!initialized) {
        for (auto & cache : caches) {
            GLenum target = cache.name;
            const GLubyte* str = nullptr;
            const char* delimiter = " ";

            switch (target) {
                case GL_VENDOR:
                    str = (const GLubyte*)"Swung0x48, BZLZHH, Tungsten";
                    delimiter = ", ";
                    break;
                case GL_VERSION:
                    str = (const GLubyte*)"4.0.0 MobileGlues";
                    delimiter = " .";
                    break;
                case GL_SHADING_LANGUAGE_VERSION:
                    str = (const GLubyte*)"4.50 MobileGlues with glslang and SPIRV-Cross";
                    break;
                case GL_EXTENSIONS:
                    str = glGetString(GL_EXTENSIONS);
                    break;
                default:
                    return GLES.glGetStringi(name, index);
            }

            if (!str) continue;

            char* copy = strdup((const char*)str);
            char* token = strtok(copy, delimiter);
            while (token) {
                cache.parts = (const char**)realloc(cache.parts, (cache.count + 1) * sizeof(char*));
                cache.parts[cache.count++] = strdup(token);
                token = strtok(nullptr, delimiter);
            }
            free(copy);
        }
        initialized = 1;
    }

    for (auto & cache : caches) {
        if (cache.name == name) {
            if (index >= cache.count) {
                return nullptr;
            }
            return (const GLubyte*)cache.parts[index];
        }
    }

    return nullptr;
}

void glGetQueryObjectiv(GLuint id, GLenum pname, GLint* params) {
    LOG()
    GLES.glGetQueryObjectivEXT(id, pname, params);
    CHECK_GL_ERROR
}

void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64* params) {
    LOG()
    GLES.glGetQueryObjecti64vEXT(id, pname, params);
    CHECK_GL_ERROR
}
