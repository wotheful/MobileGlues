//
// Created by Swung 0x48 on 2024/10/8.
//

#ifndef FOLD_CRAFT_LAUNCHER_EGL_H
#define FOLD_CRAFT_LAUNCHER_EGL_H

#include <EGL/egl.h>

typedef intptr_t EGLAttrib;

typedef __eglMustCastToProperFunctionPointerType (*EGLGETPROCADDRESSPROCP) (const char *procname);

typedef EGLint (*EGLGETERRORPROCP)(void);
typedef EGLDisplay (*EGLGETDISPLAYP)(EGLNativeDisplayType display_id);
typedef EGLBoolean (*EGLINITIALIZEPROCP)(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLBoolean (*EGLTERMINATEPROCP)(EGLDisplay dpy);
typedef const char * (*EGLQUERYSTRINGPROCP)(EGLDisplay dpy, EGLint name);
typedef EGLBoolean (*EGLGETCONFIGSPROCP)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
typedef EGLBoolean (*EGLCHOOSECONFIGPROCP)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
typedef EGLBoolean (*EGLGETCONFIGATTRIBPROCP)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);

typedef EGLSurface (*EGLCREATEWINDOWSURFACEPROCP)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
typedef EGLSurface (*EGLCREATEPBUFFERSURFACEPROCP)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
typedef EGLSurface (*EGLCREATEPIXMAPSURFACEPROCP)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
typedef EGLBoolean (*EGLDESTROYSURFACEPROCP)(EGLDisplay dpy, EGLSurface surface);
typedef EGLBoolean (*EGLQUERYSURFACEPROCP)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
typedef EGLBoolean (*EGLBINDAPIPROCP)(EGLenum api);
typedef EGLenum (*EGLQUERYAPIPROCP)(void);

typedef EGLBoolean (*EGLWAITCLIENTPROCP)(void);
typedef EGLBoolean (*EGLRELEASETHREADPROCP)(void);
typedef EGLSurface (*EGLCREATEPBUFFERFROMCLIENTBUFFERPROCP)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
typedef EGLBoolean (*EGLSURFACEATTRIBPROCP)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
typedef EGLBoolean (*EGLBINDTEXIMAGEPROCP)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
typedef EGLBoolean (*EGLRELEASETEXIMAGEPROCP)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
typedef EGLBoolean (*EGLSWAPINTERVALPROCP)(EGLDisplay dpy, EGLint interval);
typedef EGLContext (*EGLCREATECONTEXTPROCP)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
typedef EGLBoolean (*EGLDESTROYCONTEXTPROCP)(EGLDisplay dpy, EGLContext ctx);
typedef EGLBoolean (*EGLMAKECURRENTPROCP)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
typedef EGLContext (*EGLGETCURRENTCONTEXTPROCP)(void);
typedef EGLSurface (*EGLGETCURRENTSURFACEPROCP)(EGLint readdraw);
typedef EGLDisplay (*EGLGETCURRENTDISPLAYPROCP)(void);
typedef EGLDisplay (*EGLGETPLATFORMDISPLAYPROCP)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
typedef EGLBoolean (*EGLQUERYCONTEXTPROCP)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
typedef EGLBoolean (*EGLWAITGLPROCP)(void);
typedef EGLBoolean (*EGLWAITNATIVEPROCP)(EGLint engine);
typedef EGLBoolean (*EGLSWAPBUFFERSPROCP)(EGLDisplay dpy, EGLSurface surface);
typedef EGLBoolean (*EGLCOPYBUFFERSPROCP)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);


#define MAX_BOUND_BUFFERS 9
#define MAX_BOUND_BASEBUFFERS 4
#define MAX_DRAWBUFFERS 8
#define MAX_FBTARGETS 8
#define MAX_TMUS 8
#define MAX_TEXTARGETS 8

typedef struct {
    bool ready;
    GLuint indirectRenderBuffer;
} basevertex_renderer_t;

typedef struct {
    GLuint index;
    GLuint buffer;
    bool ranged;
    GLintptr  offset;
    GLintptr  size;
} basebuffer_binding_t;

typedef struct {
    GLuint color_targets[MAX_FBTARGETS];
    GLuint color_objects[MAX_FBTARGETS];
    GLuint color_levels[MAX_FBTARGETS];
    GLuint color_layers[MAX_FBTARGETS];
    GLenum virt_drawbuffers[MAX_DRAWBUFFERS];
    GLenum phys_drawbuffers[MAX_DRAWBUFFERS];
    GLsizei nbuffers;
} framebuffer_t;

typedef struct {
    bool ready;
    GLuint temp_texture;
    GLuint tempfb;
    GLuint destfb;
    void* depthData;
    GLsizei depthWidth, depthHeight;
} framebuffer_copier_t;

typedef struct {
    GLenum original_swizzle[4];
    GLboolean goofy_byte_order;
    GLboolean upload_bgra;
} texture_swizzle_track_t;

typedef struct {
    EGLContext phys_context;
    bool context_rdy;
    bool es31, es32, buffer_storage;
    GLint shader_version;
    basevertex_renderer_t basevertex;
    GLuint multidraw_element_buffer;
    framebuffer_copier_t framebuffer_copier;
    unordered_map* shader_map;
    unordered_map* program_map;
    unordered_map* framebuffer_map;
    unordered_map* texture_swztrack_map;
    unordered_map* bound_basebuffers[MAX_BOUND_BASEBUFFERS];
    int proxy_width, proxy_height, proxy_intformat, maxTextureSize;
    GLint max_drawbuffers;
    GLuint bound_buffers[MAX_BOUND_BUFFERS];
    GLuint program;
    GLuint draw_framebuffer;
    GLuint read_framebuffer;
    char* extensions_string;
    size_t nextras;
    int nextensions_es;
    char** extra_extensions_array;
} context_t;

extern thread_local context_t *current_context;

// Undocumented libmali internals, needed for ODROID Go Ultra
//NativePixmapType (*EGL_CREATE_PIXMAP_ID_MAPPINGPROCP)(void *pixmap);
//NativePixmapType (*EGL_DESTROY_PIXMAP_ID_MAPPINGPROCP)(int id);

// Undocumented libmali internals, needed for ODROID Go Ultra
//NativePixmapType (*EGL_CREATE_PIXMAP_ID_MAPPINGPROCP)(void *pixmap);
//NativePixmapType (*EGL_DESTROY_PIXMAP_ID_MAPPINGPROCP)(int id);

#endif //FOLD_CRAFT_LAUNCHER_EGL_H
