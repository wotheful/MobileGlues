#include "fpe_shader.h"

#include <stdio.h>
#include <string.h>

#include "string_utils.h"
//#include "init.h"
#include "fpe_defines.h"
#include "../gles/loader.h"

#define DEBUG 1
#if GLOBAL_DEBUG || DEBUG
#pragma clang optimize off
#endif

const char* fpeshader_signature = "// MobileGlues FPE shader generated\n";

static char* shad = NULL;
static int shad_cap = 0;

static int comments = 1;

//                           2D   Rectangle    3D   CubeMap  Stream
const char* texvecsize[] = {"vec4", "vec2", "vec2", "vec3", "vec2"};
const char* texxyzsize[] = {"stpq", "st",    "st",  "stp",   "st"};
//                          2D              Rectangle      3D          CubeMap          Stream
const char* texname[] = {"texture2DProj", "texture2D", "texture2D", "textureCube", "textureStreamIMG"};    // textureRectange and 3D are emulated with 2D
const char* texnoproj[] = {"texture2D", "texture2D", "texture2D", "textureCube", "textureStreamIMG"};    // textureRectange and 3D are emulated with 2D
const char* texsampler[] = {"sampler2D", "sampler2D", "sampler2D", "samplerCube", "samplerStreamIMG"};
int texnsize[] = {2, 2, 3, 3, 2};
const char texcoordname[] = {'s', 't', 'r', 'q'};
const char texcoordNAME[] = {'S', 'T', 'R', 'Q'};
const char texcoordxy[] = {'x', 'y', 'z', 'w'};

const char* mg_alphaRefSource = "uniform float _mg_AlphaRef;\n";

const char* fpe_texenvSrc(int src, int tmu, int twosided) {
    static char buff[200];
    switch(src) {
        case FPE_SRC_TEXTURE:
            sprintf(buff, "texColor%d", tmu);
            break;
        case FPE_SRC_TEXTURE0:
        case FPE_SRC_TEXTURE1:
        case FPE_SRC_TEXTURE2:
        case FPE_SRC_TEXTURE3:
        case FPE_SRC_TEXTURE4:
        case FPE_SRC_TEXTURE5:
        case FPE_SRC_TEXTURE6:
        case FPE_SRC_TEXTURE7:
        case FPE_SRC_TEXTURE8:
        case FPE_SRC_TEXTURE9:
        case FPE_SRC_TEXTURE10:
        case FPE_SRC_TEXTURE11:
        case FPE_SRC_TEXTURE12:
        case FPE_SRC_TEXTURE13:
        case FPE_SRC_TEXTURE14:
        case FPE_SRC_TEXTURE15:
            sprintf(buff, "texColor%d", src-FPE_SRC_TEXTURE0);  // should check if texture is enabled
            break;
        case FPE_SRC_CONSTANT:
            sprintf(buff, "_mg_TextureEnvColor_%d", tmu);
            break;
        case FPE_SRC_PRIMARY_COLOR:
            sprintf(buff, "%s", twosided?"((gl_FrontFacing)?Color:BackColor)":"Color");
            break;
        case FPE_SRC_PREVIOUS:
            sprintf(buff, "fColor");
            break;
        case FPE_SRC_ONE:
            sprintf(buff, "vec4(1.0)");
            break;
        case FPE_SRC_ZERO:
            sprintf(buff, "vec4(0.0)");
            break;
        case FPE_SRC_SECONDARY_COLOR:
            sprintf(buff, "%s", twosided?"((gl_FrontFacing)?SecColor:SecBackColor)":"SecColor");
            break;
    }
    return buff;
}

int fpe_texenvSecondary(fpe_state_t* state) {
    // check if one of the texenv need secondary color...
    for (int i=0; i<g_gles_caps.maxtex; i++) {
        int t = state->texture[i].textype;
        if(t) {
            int texenv = state->texenv[i].texenv;
            if(texenv==FPE_COMBINE) {
                int combine_rgb = state->texcombine[i]&0xf;
                int src_r[4];
                src_r[0] = state->texenv[i].texsrcrgb0;
                src_r[1] = state->texenv[i].texsrcrgb1;
                src_r[2] = state->texenv[i].texsrcrgb2;
                src_r[3] = state->texenv[i].texsrcrgb3;
                if(combine_rgb==FPE_CR_DOT3_RGBA) {
                        src_r[2] = -1;
                } else {
                    if(combine_rgb==FPE_CR_REPLACE) {
                        src_r[1] = src_r[2] = -1;
                    } else if (combine_rgb>=FPE_CR_MOD_ADD) {
                        // need all 3
                    } else if (combine_rgb!=FPE_CR_INTERPOLATE) {
                        src_r[2] = -1;
                    }
                }
                if(src_r[0]==FPE_SRC_SECONDARY_COLOR || src_r[1]==FPE_SRC_SECONDARY_COLOR || src_r[2]==FPE_SRC_SECONDARY_COLOR)
                    return 1;
            }
        }
    }
    return 0;   
}

char* fpe_packed64(uint64_t x, int s, int k) {
    static char buff[8][65];
    static int idx = 0;

    idx&=7;
    uint64_t mask = (1L<<k)-1L;

    const char *hex = "0123456789ABCDEF";

    int j=s/k;
    buff[idx][j] = '\0';
    for (int i=0; i<s; i+=k) {
        buff[idx][--j] = hex[(x&mask)];
        x>>=k;
    }
    return buff[idx++];
}
char* fpe_packed(int x, int s, int k) {
    static char buff[8][33];
    static int idx = 0;

    idx&=7;
    int mask = (1<<k)-1;

    const char *hex = "0123456789ABCDEF";

    int j=s/k;
    buff[idx][j] = '\0';
    for (int i=0; i<s; i+=k) {
        buff[idx][--j] = hex[(x&mask)];
        x>>=k;
    }
    return buff[idx++];
}
char* fpe_binary(int x, int s) {
    return fpe_packed(x, s, 1);
}

char gl_VA[MAX_VATTRIB][32] = {0};
char mg_VA[MAX_VATTRIB][32] = {0};


void PreConvert() {
    if(gl_VA[0][0]=='\0') {
        for (int i=0; i<MAX_VATTRIB; ++i) {
            sprintf(gl_VA[i], "%s%d", gl_VertexAttrib, i);
            sprintf(mg_VA[i], "%s%d", mg_VertexAttrib, i);
        }
    }
}

char* ConvertShader(const char* pEntry, int isVertex, shaderconv_need_t *need)
{
#define ShadAppend(S) Tmp = mg_append(Tmp, &tmpsize, S)

    PreConvert();
    int fpeShader = (strstr(pEntry, fpeshader_signature)!=NULL)?1:0;
    int maskbefore = 4|(isVertex?1:2);
    int maskafter = 8|(isVertex?1:2);
#if DEBUG || GLOBAL_DEBUG
        printf("Shader source%s:\n%s\n", fpeShader?" (FPEShader generated)":"", pEntry);
#endif
    int comments = DEBUG || GLOBAL_DEBUG;

    char* pBuffer = (char*)pEntry;

    int version120 = 0;
    char* versionString = NULL;
//    if(!fpeShader) {
//        extensions_t exts;  // dummy...
//        exts.cap = exts.size = 0;
//        exts.ext = NULL;
//        // hacks
//        char* pHacked = ShaderHacks(pBuffer);
//        // preproc first
//        pBuffer = preproc(pHacked, comments, globals4es.shadernogles, &exts, &versionString);
//        if(pHacked!=pEntry && pHacked!=pBuffer)
//            free(pHacked);
//        // now comment all line starting with precision...
//        if(strstr(pBuffer, "\nprecision")) {
//            int sz = strlen(pBuffer);
//            pBuffer = mg_inplace_replace(pBuffer, &sz, "\nprecision", "\n//precision");
//        }
//        // should do something with the extension list...
//        if(exts.ext)
//            free(exts.ext);
//    }

    static shaderconv_need_t dummy_need = {0};
    if(!need) {
        need = &dummy_need;
        need->need_texcoord = -1;
        need->need_clean = 1; // no hack, this is a dummy need structure
    }
    int notexarray = g_gles_caps.notexarray || need->need_notexarray || fpeShader;

    //const char* GLESUseFragHighp = "#extension GL_OES_fragment_precision_high : enable\n"; // does this one is needed?
    char GLESFullHeader[512];
    int wanthighp = !fpeShader;
    if(wanthighp && !g_gles_caps.highp) wanthighp = 0;
    int versionHeader = 0;
#if 0
    // support for higher glsl require much more work
  // around some keyword
  // like in/out that depend on the shader beeing vertex or fragment
  // and a few other little things...
  if(versionString && strcmp(versionString, "120")==0)
     version120 = 1;
  if(version120) {
    if(hardext.glsl120) versionHeader = 1;
    else if(hardext.glsl310es) versionHeader = 2;
    else if(hardext.glsl300es) { versionHeader = 3; /* location on uniform not supported ! */ }
    /* else no location or in / out are supported */
  }
#endif
    //sprintf(GLESFullHeader, GLESHeader, (wanthighp && g_gles_caps.highp==1 && !isVertex)?GLESUseFragHighp:"", (wanthighp)?"highp":"mediump", (wanthighp)?"highp":"mediump");
    sprintf(GLESFullHeader, GLESHeader[versionHeader], "", (wanthighp)?"highp":"mediump", (wanthighp)?"highp":"mediump");

    int tmpsize = strlen(pBuffer)*2+strlen(GLESFullHeader)+100;
    char* Tmp = (char*)calloc(1, tmpsize);
    strcpy(Tmp, pBuffer);

    // and now change the version header, and add default precision
    char* newptr;
    newptr=strstr(Tmp, "#version");
    if (!newptr) {
        Tmp = mg_inplace_insert(Tmp, GLESFullHeader, Tmp, &tmpsize);
    } else {
        while(*newptr!=0x0a) newptr++;
        newptr++;
        memmove(Tmp, newptr, strlen(newptr)+1);
        Tmp = mg_inplace_insert(Tmp, GLESFullHeader, Tmp, &tmpsize);
    }
    int headline = 3;
    // move all "#extension in header zone"
    while (strstr(Tmp, "#extension") && strstr(Tmp, "#extension")>mg_getline(Tmp, headline-2)) {
        char* ext = strstr(Tmp, "#extension");
        size_t l = (uintptr_t)strstr(ext, "\n")-(uintptr_t)ext + sizeof("\n");
#ifndef _MSC_VER
        char e[l];
#else
        char* e = _alloca(l);
#endif
        memset(e, 0, l);
        strncpy(e, ext, l-1);
        Tmp = mg_inplace_replace_simple(Tmp, &tmpsize, e, "");
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline-2), e, Tmp, &tmpsize);
        ++headline;
    }
    // check if gl_FragDepth is used
//    int fragdepth = (strstr(pBuffer, "gl_FragDepth"))?1:0;
//    const char* GLESUseFragDepth = "#extension GL_EXT_frag_depth : enable\n";
//    const char* GLESFakeFragDepth = "mediump float fakeFragDepth = 0.0;\n";
//    if (fragdepth) {
//        /* If #extension is used, it should be placed before the second line of the header. */
//        if(hardext.fragdepth)
//            Tmp = mg_inplace_insert(mg_getline(Tmp, 1), GLESUseFragDepth, Tmp, &tmpsize);
//        else
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline-1), GLESFakeFragDepth, Tmp, &tmpsize);
//        headline++;
//    }
    int threed_texture = (strstr(pBuffer, "sampler3D"))?1:0;
    const char* GLESUse3DTexture = "#extension GL_OES_texture_3D : enable\n";
    if (threed_texture) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 1), GLESUse3DTexture, Tmp, &tmpsize);
    }
//    int derivatives = (strstr(pBuffer, "dFdx(") || strstr(pBuffer, "dFdy(") || strstr(pBuffer, "fwidth("))?1:0;
//    const char* GLESUseDerivative = "#extension GL_OES_standard_derivatives : enable\n";
//    // complete fake value... A better thing should be use....
//    const char* GLESFakeDerivative = "float dFdx(float p) {return 0.0001;}\nvec2 dFdx(vec2 p) {return vec2(0.0001);}\nvec3 dFdx(vec3 p) {return vec3(0.0001);}\n"
//                                     "float dFdy(float p) {return 0.0001;}\nvec2 dFdy(vec2 p) {return vec2(0.0001);}\nvec3 dFdy(vec3 p) {return vec3(0.0001);}\n"
//                                     "float fwidth(float p) {return abs(dFdx(p))+abs(dFdy(p));}\nvec2 fwidth(vec2 p) {return abs(dFdx(p))+abs(dFdy(p));}\n"
//                                     "vec3 fwidth(vec3 p) {return abs(dFdx(p))+abs(dFdy(p));}\n";
//    if (derivatives) {
//        /* If #extension is used, it should be placed before the second line of the header. */
//        if(hardext.derivatives)
//            Tmp = mg_inplace_insert(mg_getline(Tmp, 1), GLESUseDerivative, Tmp, &tmpsize);
//        else
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline-1), GLESFakeDerivative, Tmp, &tmpsize);
//        headline++;
//    }
    // check if draw_buffers may be used (no fallback here :( )
    if(/*hardext.maxdrawbuffers>1 &&*/ strstr(pBuffer, "gl_FragData[")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 1), useEXTDrawBuffers, Tmp, &tmpsize);
    }
    // if some functions are used, add some int/float alternative
//    if(!fpeShader && !globals4es.nointovlhack) {
//        if(strstr(Tmp, "pow(") || strstr(Tmp, "pow (")) {
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), HackAltPow, Tmp, &tmpsize);
//        }
//        if(strstr(Tmp, "max(") || strstr(Tmp, "max (")) {
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), HackAltMax, Tmp, &tmpsize);
//        }
//        if(strstr(Tmp, "min(") || strstr(Tmp, "min (")) {
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), HackAltMin, Tmp, &tmpsize);
//        }
//        if(strstr(Tmp, "clamp(") || strstr(Tmp, "clamp (")) {
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), HackAltClamp, Tmp, &tmpsize);
//        }
//        if(strstr(Tmp, "mod(") || strstr(Tmp, "mod (")) {
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), HackAltMod, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && hardext.shaderlod &&
//       (mg_find_string(Tmp, "texture2DLod") || mg_find_string(Tmp, "texture2DProjLod")
//        || mg_find_string(Tmp, "textureCubeLod")
//        || mg_find_string(Tmp, "texture2DGradARB") || mg_find_string(Tmp, "texture2DProjGradARB")|| mg_find_string(Tmp, "textureCubeGradARB")
//       )) {
//        const char* GLESUseShaderLod = "#extension GL_EXT_shader_texture_lod : enable\n";
//        Tmp = mg_inplace_insert(mg_getline(Tmp, 1), GLESUseShaderLod, Tmp, &tmpsize);
//    }
//    if(!isVertex && (mg_find_string(Tmp, "texture2DLod"))) {
//        if(hardext.shaderlod) {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DLod", "texture2DLodEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DLod", "_mg_texture2DLod");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), texture2DLodAlt, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && (mg_find_string(Tmp, "texture2DProjLod"))) {
//        if(hardext.shaderlod) {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DProjLod", "texture2DProjLodEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DProjLod", "_mg_texture2DProjLod");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), texture2DProjLodAlt, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && (mg_find_string(Tmp, "textureCubeLod"))) {
//        if(hardext.shaderlod) {
//            if(!hardext.cubelod)
//                Tmp = mg_inplace_replace(Tmp, &tmpsize, "textureCubeLod", "textureCubeLodEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "textureCubeLod", "_mg_textureCubeLod");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), textureCubeLodAlt, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && (mg_find_string(Tmp, "texture2DGradARB"))) {
//        if(hardext.shaderlod) {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DGradARB", "texture2DGradEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DGradARB", "_mg_texture2DGrad");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), texture2DGradAlt, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && (mg_find_string(Tmp, "texture2DProjGradARB"))) {
//        if(hardext.shaderlod) {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DProjGradARB", "texture2DProjGradEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "texture2DProjGradARB", "_mg_texture2DProjGrad");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), texture2DProjGradAlt, Tmp, &tmpsize);
//        }
//    }
//    if(!isVertex && (mg_find_string(Tmp, "textureCubeGradARB"))) {
//        if(hardext.shaderlod) {
//            if(!hardext.cubelod)
//                Tmp = mg_inplace_replace(Tmp, &tmpsize, "textureCubeGradARB", "textureCubeGradEXT");
//        } else {
//            Tmp = mg_inplace_replace(Tmp, &tmpsize, "textureCubeGradARB", "_mg_textureCubeGrad");
//            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), textureCubeGradAlt, Tmp, &tmpsize);
//        }
//    }

    // Some drivers have troubles with "\\\r\n" or "\\\n" sequences on preprocessor macros
    newptr = Tmp;
    while (*newptr!=0x00) {
        if (*newptr == '\\') {
            if (*(newptr+1) == '\r' && *(newptr+2) == '\n')
                memmove(newptr, newptr+3, strlen(newptr+3)+1);
            else if (*(newptr+1) == '\n')
                memmove(newptr, newptr+2, strlen(newptr+2)+1);
        }

        newptr++;
    }

    // now check to remove trailling "f" after float, as it's not supported too
    newptr = Tmp;
    // simple state machine...
    int state = 0;
    while (*newptr!=0x00) {
        switch(state) {
            case 0:
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 1;  // integer part
                else if (*newptr == '.')
                    state = 2;  // fractional part
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else
                    state = 3; // something else
                break;
            case 1: // integer part
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 1;  // integer part
                else if (*newptr == '.')
                    state = 2;  // fractional part
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else  if (*newptr == 'f' ) {
                    // remove that f
                    memmove(newptr, newptr+1, strlen(newptr+1)+1);
                    newptr--;
                } else
                    state = 3;
                break;
            case 2: // fractionnal part
                if ((*newptr >= '0') && (*newptr <= '9'))
                    state = 2;
                else if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else  if (*newptr == 'f' ) {
                    // remove that f
                    memmove(newptr, newptr+1, strlen(newptr+1)+1);
                    newptr--;
                } else
                    state = 3;
                break;
            case 3:
                if ((*newptr==' ') || (*newptr==0x0d) || (*newptr==0x0a) || (*newptr=='-') || (*newptr=='+') || (*newptr=='*') || (*newptr=='/') || (*newptr=='(') || (*newptr==')' || (*newptr=='>') || (*newptr=='<')))
                    state = 0; // separator
                else
                    state = 3;
                break;
        }
        newptr++;
    }
    // Frag depth already in core es 3
//    Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FragDepth", (hardext.fragdepth)?"gl_FragDepthEXT":"fakeFragDepth");
    // builtin attribs
    if(isVertex) {
        // check for ftransform function
        if(strstr(Tmp, "ftransform(")) {
            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_ftransformSource, Tmp, &tmpsize);
            // don't increment headline count, as all variying and attributes should be created before
        }
        // check for builtin OpenGL attributes...
        int n = sizeof(builtin_attrib)/sizeof(builtin_attrib_t);
        for (int i=0; i<n; i++) {
            if(strstr(Tmp, builtin_attrib[i].glname)) {
                // ok, this attribute is used
                // replace gl_name by _mg_ one
                Tmp = mg_inplace_replace(Tmp, &tmpsize, builtin_attrib[i].glname, builtin_attrib[i].name);
                // insert a declaration of it
                char def[100];
                sprintf(def, "attribute %s %s %s;\n", builtin_attrib[i].prec, builtin_attrib[i].type, builtin_attrib[i].name);
                Tmp = mg_inplace_insert(mg_getline(Tmp, headline++), def, Tmp, &tmpsize);
            }
        }
        if(strstr(Tmp, gl_VertexAttrib)) {
            // Generic VA from Old Programs
            for (int i=0; i<MAX_VATTRIB; ++i) {
                char A[100];
                if(mg_find_string(Tmp, gl_VA[i])) {
                    sprintf(A, "attribute highp vec4 %s%d;\n", mg_VertexAttrib, i);
                    Tmp = mg_inplace_replace(Tmp, &tmpsize, gl_VA[i], mg_VA[i]);
                    Tmp = mg_inplace_insert(mg_getline(Tmp, headline++), A, Tmp, &tmpsize);
                }
            }
        }
    }
    // builtin varying
    int nvarying = 0;
    if(strstr(Tmp, "gl_Color") || need->need_color) {
        if(need->need_color<1) need->need_color = 1;
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_Color", (need->need_color==1)?"gl_FrontColor":"(gl_FrontFacing?gl_FrontColor:gl_BackColor)");
    }
    if(strstr(Tmp, "gl_FrontColor") || need->need_color) {
        if(need->need_color<1) need->need_color = 1;
        nvarying+=1;
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_frontColorSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_frontColorSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FrontColor", "_mg_FrontColor");
    }
    if(strstr(Tmp, "gl_BackColor") || (need->need_color==2)) {
        need->need_color = 2;
        nvarying+=1;
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_backColorSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_backColorSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_BackColor", "_mg_BackColor");
    }
    if(strstr(Tmp, "gl_SecondaryColor") || need->need_secondary) {
        if(need->need_secondary<1) need->need_secondary = 1;
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_SecondaryColor", (need->need_secondary==1)?"gl_FrontSecondaryColor":"(gl_FrontFacing?gl_FrontSecondaryColor:gl_BackSecondaryColor)");
    }
    if(strstr(Tmp, "gl_FrontSecondaryColor") || need->need_secondary) {
        if(need->need_secondary<1) need->need_secondary = 1;
        nvarying+=1;
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_frontSecondaryColorSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_frontSecondaryColorSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FrontSecondaryColor", "_mg_FrontSecondaryColor");
    }
    if(strstr(Tmp, "gl_BackSecondaryColor") || (need->need_secondary==2)) {
        need->need_secondary = 2;
        nvarying+=1;
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_backSecondaryColorSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_backSecondaryColorSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_BackSecondaryColor", "_mg_BackSecondaryColor");
    }
    if(strstr(Tmp, "gl_FogFragCoord") || need->need_fogcoord) {
        need->need_fogcoord = 1;
        nvarying+=1;
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_fogcoordSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_fogcoordSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FogFragCoord", "_mg_FogFragCoord");
    }
    // Get the max_texunit and the calc notexarray
    if(strstr(Tmp, "gl_TexCoord") || need->need_texcoord!=-1) {
        int ntex = need->need_texcoord;
        // Try to determine max gl_TexCoord used
        char* p = Tmp;
        int notexarray_ok = 1;
        while((p=strstr(p, gl_TexCoordSource))) {
            p+=strlen(gl_TexCoordSource);
            if(*p>='0' && *p<='9') {
                int n = (*p) - '0';
                if(p[1]>='0' && p[1]<='9')
                    n = n*10 + (p[1] - '0');
                if (ntex<n) ntex = n;
            } else
                notexarray_ok=0;
        }
        // if failed to determine, take max...
        if (ntex==-1) ntex = g_gles_caps.maxtex;
        // check constraint, and switch to notexarray if needed
        if (!notexarray && ntex+nvarying>g_gles_caps.maxvarying && !need->need_clean && notexarray_ok) {
            notexarray = 1;
            need->need_notexarray = 1;
        }
        // prefer notexarray...
        if(!isVertex && notexarray_ok && !need->need_clean) {
            notexarray = 1;
            need->need_notexarray = 1;
        }
        // check constaints
        if (!notexarray && ntex+nvarying>g_gles_caps.maxvarying) ntex = g_gles_caps.maxvarying - nvarying;
        need->need_texcoord = ntex;
        char d[100];
        if(notexarray) {
            for (int k=0; k<ntex+1; k++) {
                char d2[100];
                sprintf(d2, "gl_TexCoord[%d]", k);
                if(strstr(Tmp, d2)) {
                    sprintf(d, mg_texcoordSourceAlt, k);
                    Tmp = mg_inplace_insert(mg_getline(Tmp, headline), d, Tmp, &tmpsize);
                    headline+=mg_countline(d);
                    sprintf(d, "_mg_TexCoord_%d", k);
                    Tmp = mg_inplace_replace(Tmp, &tmpsize, d2, d);
                }
                // check if texture is there
                sprintf(d2, "_mg_TexCoord_%d", k);
                if(strstr(Tmp, d2))
                    need->need_texs |= (1<<k);
            }
        } else {
            sprintf(d, mg_texcoordSource, ntex+1);
            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), d, Tmp, &tmpsize);
            headline+=mg_countline(d);
            Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_TexCoord", "_mg_TexCoord");
            // set textures as all ntex used
            for (int k=0; k<ntex+1; k++)
                need->need_texs |= (1<<k);
        }
    }

    // builtin matrices work
    {
        if(strstr(Tmp, "transpose(") || strstr(Tmp, "transpose ") || strstr(Tmp, "transpose\t")) {
            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_transpose, Tmp, &tmpsize);
            mg_inplace_replace(Tmp, &tmpsize, "transpose", "mg_transpose");
            // don't increment headline count, as all variying and attributes should be created before
        }
        // check for builtin matrix uniform...
        {
            // first check number of texture matrices used
            int ntex = -1;
            // Try to determine max Texture matrice used, for each transposed inverse or regular...
            for(int i=0; i<4; ++i) {
                char* p = Tmp;
                while((p=strstr(p, gl_TexMatrixSources[i]))) {
                    p+=strlen(gl_TexMatrixSources[i]);
                    if(*p>='0' && *p<='9') {
                        int n = 0;
                        while(*p>='0' && *p<='9')
                            n = n*10 + (*(p++) - '0');

                        if (ntex<n) ntex = n;
                    }
                }
            }

            // if failed to determine, take max...
            if (ntex==-1) ntex = need->need_texcoord; else ++ntex;
            // change gl_TextureMatrix[X] to gl_TextureMatrix_X if possible
            int change_textmat = notexarray;
            if(!change_textmat) {
                change_textmat = 1;
                char* p = Tmp;
                while(change_textmat && (p=strstr(p, "gl_TextureMatrix["))) {
                    p += strlen("gl_TextureMatrix[");
                    while((*p)>='0' && (*p)<='9') ++p;
                    if((*p)!=']')
                        change_textmat = 0;
                }
            }
            if(change_textmat) {
                for (int k=0; k<ntex+1; k++) {
                    char d[100];
                    char d2[100];
                    sprintf(d2, "gl_TextureMatrix[%d]", k);
                    if(strstr(Tmp, d2)) {
                        sprintf(d, "gl_TextureMatrix_%d", k);
                        Tmp = mg_inplace_replace(Tmp, &tmpsize, d2, d);
                    }
                }
            }

            int n = sizeof(builtin_matrix)/sizeof(builtin_matrix_t);
            for (int i=0; i<n; i++) {
                if(strstr(Tmp, builtin_matrix[i].glname)) {
                    // ok, this matrix is used
                    // replace gl_name by _mg_ one
                    Tmp = mg_inplace_replace(Tmp, &tmpsize, builtin_matrix[i].glname, builtin_matrix[i].name);
                    // insert a declaration of it
                    char def[100];
                    int ishighp = (isVertex || g_gles_caps.highp)?1:0;
                    if(builtin_matrix[i].matrix == MAT_N) {
                        if(need->need_normalmatrix && !g_gles_caps.highp)
                            ishighp = 0;
                        if(!g_gles_caps.highp && !isVertex)
                            need->need_normalmatrix = 1;
                    }
                    if(builtin_matrix[i].matrix == MAT_MV) {
                        if(need->need_mvmatrix && !g_gles_caps.highp)
                            ishighp = 0;
                        if(!g_gles_caps.highp && !isVertex)
                            need->need_mvmatrix = 1;
                    }
                    if(builtin_matrix[i].matrix == MAT_MVP) {
                        if(need->need_mvpmatrix && !g_gles_caps.highp)
                            ishighp = 0;
                        if(!g_gles_caps.highp && !isVertex)
                            need->need_mvpmatrix = 1;
                    }
                    if(builtin_matrix[i].texarray)
                        sprintf(def, "uniform %s%s %s[%d];\n", (ishighp)?"highp ":"mediump ", builtin_matrix[i].type, builtin_matrix[i].name, ntex);
                    else
                        sprintf(def, "uniform %s%s %s;\n", (ishighp)?"highp ":"mediump ", builtin_matrix[i].type, builtin_matrix[i].name);
                    Tmp = mg_inplace_insert(mg_getline(Tmp, headline++), def, Tmp, &tmpsize);
                }
            }
        }
    }
    // Handling of gl_LightSource[x].halfVector => normalize(gl_LightSource[x].position - gl_Vertex), but what if in the FragShader ?
/*  if(strstr(Tmp, "halfVector"))
  {
    char *p = Tmp;
    while((p=strstr(p, "gl_LightSource["))) {
      char *p2 = strchr(p, ']');
      if (p2 && !strncmp(p2, "].halfVector", strlen("].halfVector"))) {
        // found an occurrence, lets change
        char p3[500];
        strncpy(p3,p, (p2-p)+1); p3[(p2-p)+1]='\0';
        char p4[500], p5[500];
        sprintf(p4, "%s.halfVector", p3);
        sprintf(p5, "normalize(normalize(%s.position.xyz) + vec3(0., 0., 1.))", p3);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, p4, p5);
        p = Tmp;
      } else
        ++p;
    }
  }*/
    // cleaning up the "centroid" keyword...
    if(strstr(Tmp, "centroid"))
    {
        char *p = Tmp;
        while((p=strstr(p, "centroid"))!=NULL)
        {
            if(p[8]==' ' || p[8]=='\t') { // what next...
                const char* p2 = mg_get_next_str(p+8);
                if(strcmp(p2, "uniform")==0 || strcmp(p2, "varying")==0) {
                    memset(p, ' ', 8);  // erase the keyword...
                }
            }
            p+=8;
        }
    }

    // check for builtin OpenGL gl_LightSource & friends
    if(strstr(Tmp, "gl_LightSourceParameters") || strstr(Tmp, "gl_LightSource"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_LightSourceParametersSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_LightSourceParametersSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightSourceParameters", "_mg_LightSourceParameters");
    }
    if(strstr(Tmp, "gl_LightModelParameters") || strstr(Tmp, "gl_LightModel"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_LightModelParametersSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_LightModelParametersSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightModelParameters", "_mg_LightModelParameters");
    }
    if(strstr(Tmp, "gl_LightModelProducts") || strstr(Tmp, "gl_FrontLightModelProduct") || strstr(Tmp, "gl_BackLightModelProduct"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_LightModelProductsSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_LightModelProductsSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightModelProducts", "_mg_LightModelProducts");
    }
    if(strstr(Tmp, "gl_LightProducts") || strstr(Tmp, "gl_FrontLightProduct") || strstr(Tmp, "gl_BackLightProduct"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_LightProductsSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_LightProductsSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightProducts", "_mg_LightProducts");
    }
    if(strstr(Tmp, "gl_MaterialParameters ") || (strstr(Tmp, "gl_FrontMaterial")) || strstr(Tmp, "gl_BackMaterial"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_MaterialParametersSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_MaterialParametersSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_MaterialParameters", "_mg_MaterialParameters");
    }
    if(strstr(Tmp, "gl_LightSource")) {
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightSource", "_mg_LightSource");
    }
    if(strstr(Tmp, "gl_LightModel"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_LightModel", "_mg_LightModel");
    if(strstr(Tmp, "gl_FrontLightModelProduct"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FrontLightModelProduct", "_mg_FrontLightModelProduct");
    if(strstr(Tmp, "gl_BackLightModelProduct"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_BackLightModelProduct", "_mg_BackLightModelProduct");
    if(strstr(Tmp, "gl_FrontLightProduct"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FrontLightProduct", "_mg_FrontLightProduct");
    if(strstr(Tmp, "gl_BackLightProduct"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_BackLightProduct", "_mg_BackLightProduct");
    if(strstr(Tmp, "gl_FrontMaterial"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FrontMaterial", "_mg_FrontMaterial");
    if(strstr(Tmp, "gl_BackMaterial"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_BackMaterial", "_mg_BackMaterial");
    if(strstr(Tmp, "gl_MaxLights"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_MaxLightsSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_MaxLightsSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_MaxLights", "_mg_MaxLights");
    }
    if(strstr(Tmp, "gl_NormalScale")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_normalscaleSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_normalscaleSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_NormalScale", "_mg_NormalScale");
    }
    if(strstr(Tmp, "gl_InstanceID") || strstr(Tmp, "gl_InstanceIDARB")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_instanceID, Tmp, &tmpsize);
        headline+=mg_countline(mg_instanceID);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_InstanceIDARB", "_mg_InstanceID");
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_InstanceID", "_mg_InstanceID");
    }
    if(strstr(Tmp, "gl_ClipPlane")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_clipplanesSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_clipplanesSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ClipPlane", "_mg_ClipPlane");
    }
    if(strstr(Tmp, "gl_MaxClipPlanes")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_MaxClipPlanesSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_MaxClipPlanesSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_MaxClipPlanes", "_mg_MaxClipPlanes");
    }

    if(strstr(Tmp, "gl_PointParameters") || strstr(Tmp, "gl_Point"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_PointSpriteSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_PointSpriteSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_PointParameters", "_mg_PointParameters");
    }
    if(strstr(Tmp, "gl_Point"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_Point", "_mg_Point");
    if(strstr(Tmp, "gl_FogParameters") || strstr(Tmp, "gl_Fog"))
    {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), g_gles_caps.highp?mg_FogParametersSourceHighp:mg_FogParametersSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_FogParametersSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_FogParameters", "_mg_FogParameters");
    }
    if(strstr(Tmp, "gl_Fog"))
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_Fog", "_mg_Fog");
    if(strstr(Tmp, "gl_TextureEnvColor")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texenvcolorSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_texenvcolorSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_TextureEnvColor", "_mg_TextureEnvColor");
    }
    if(strstr(Tmp, "gl_EyePlaneS")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgeneyeSource[0], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgeneyeSource[0]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_EyePlaneS", "_mg_EyePlaneS");
    }
    if(strstr(Tmp, "gl_EyePlaneT")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgeneyeSource[1], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgeneyeSource[1]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_EyePlaneT", "_mg_EyePlaneT");
    }
    if(strstr(Tmp, "gl_EyePlaneR")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgeneyeSource[2], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgeneyeSource[2]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_EyePlaneR", "_mg_EyePlaneR");
    }
    if(strstr(Tmp, "gl_EyePlaneQ")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgeneyeSource[3], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgeneyeSource[3]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_EyePlaneQ", "_mg_EyePlaneQ");
    }
    if(strstr(Tmp, "gl_ObjectPlaneS")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgenobjSource[0], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgenobjSource[0]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ObjectPlaneS", "_mg_ObjectPlaneS");
    }
    if(strstr(Tmp, "gl_ObjectPlaneT")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgenobjSource[1], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgenobjSource[1]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ObjectPlaneT", "_mg_ObjectPlaneT");
    }
    if(strstr(Tmp, "gl_ObjectPlaneR")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgenobjSource[2], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgenobjSource[2]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ObjectPlaneR", "_mg_ObjectPlaneR");
    }
    if(strstr(Tmp, "gl_ObjectPlaneQ")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), mg_texgenobjSource[3], Tmp, &tmpsize);
        headline+=mg_countline(mg_texgenobjSource[3]);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ObjectPlaneQ", "_mg_ObjectPlaneQ");
    }

    if(strstr(Tmp, "gl_MaxTextureUnits")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_MaxTextureUnitsSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_MaxTextureUnitsSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_MaxTextureUnits", "_mg_MaxTextureUnits");
    }
    if(strstr(Tmp, "gl_MaxTextureCoords")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_MaxTextureCoordsSource, Tmp, &tmpsize);
        headline+=mg_countline(mg_MaxTextureCoordsSource);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_MaxTextureCoords", "_mg_MaxTextureCoords");
    }
    if(strstr(Tmp, "gl_ClipVertex")) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_ClipVertex, Tmp, &tmpsize);
        headline+=mg_countline(mg_ClipVertex);
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "gl_ClipVertex", mg_ClipVertexSource);
        need->need_clipvertex = 1;
    } else if(isVertex && need && need->need_clipvertex) {
        Tmp = mg_inplace_insert(mg_getline(Tmp, 2), mg_ClipVertex, Tmp, &tmpsize);
        headline+=mg_countline(mg_ClipVertex);
        char *p = strchr(mg_find_string_nc(Tmp, "main"), '{'); // find the openning curly bracket of main
        if(p) {
            // add regular clipping at start of main
            Tmp = mg_inplace_insert(p+1, mg_ClipVertex_clip, Tmp, &tmpsize);
        }
    }
    //oldprogram uniforms...
    if(mg_find_string(Tmp, gl_ProgramEnv)) {
        // check if array can be removed
        int maxind = -1;
        int noarray_ok = 1;
        char* p = Tmp;
        while(noarray_ok && (p=mg_find_string_nc(p, gl_ProgramEnv))) {
            p+=strlen(gl_ProgramEnv);
            if(*p=='[') {
                ++p;
                if(*p>='0' && *p<='9') {
                    int n = (*p) - '0';
                    if(p[1]>='0' && p[1]<='9')
                        n = n*10 + (p[1] - '0');
                    if (maxind<n) maxind = n;
                } else
                    noarray_ok=0;
            } else
                noarray_ok=0;
        }
        if(noarray_ok) {
            // ok, so change array to single...
            char F[60], T[60], U[300];
            for(int i=0; i<=maxind; ++i) {
                sprintf(F, "%s[%d]", gl_ProgramEnv, i);
                sprintf(T, "_mg_%s_ProgramEnv_%d", isVertex?"Vertex":"Fragment", i);
                Tmp = mg_inplace_replace(Tmp, &tmpsize, F, T);
                if(mg_find_string(Tmp, T)) {
                    // add the uniform declaration if needed
                    sprintf(U, "uniform vec4 %s;\n", T);
                    Tmp = mg_inplace_insert(mg_getline(Tmp, headline), U, Tmp, &tmpsize);
                    headline += 1;
                }
            }
        } else {
            // need the full array...
            char T[60], U[300];
            sprintf(T, "_mg_%s_ProgramEnv", isVertex?"Vertex":"Fragment");
            sprintf(U, "uniform vec4 %s[%d];\n", T, isVertex?MAX_VTX_PROG_ENV_PARAMS:MAX_FRG_PROG_ENV_PARAMS);
            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), U, Tmp, &tmpsize);
            headline += 1;
            Tmp = mg_inplace_replace(Tmp, &tmpsize, gl_ProgramEnv, T);
        }
    }
    if(mg_find_string(Tmp, gl_ProgramLocal)) {
        // check if array can be removed
        int maxind = -1;
        int noarray_ok = 1;
        char* p = Tmp;
        while(noarray_ok && (p=mg_find_string_nc(p, gl_ProgramLocal))) {
            p+=strlen(gl_ProgramLocal);
            if(*p=='[') {
                ++p;
                if(*p>='0' && *p<='9') {
                    int n = (*p) - '0';
                    if(p[1]>='0' && p[1]<='9')
                        n = n*10 + (p[1] - '0');
                    if (maxind<n) maxind = n;
                } else
                    noarray_ok=0;
            } else
                noarray_ok=0;
        }
        if(noarray_ok) {
            // ok, so change array to single...
            char F[60], T[60], U[300];
            for(int i=0; i<=maxind; ++i) {
                sprintf(F, "%s[%d]", gl_ProgramLocal, i);
                sprintf(T, "_mg_%s_ProgramLocal_%d", isVertex?"Vertex":"Fragment", i);
                Tmp = mg_inplace_replace(Tmp, &tmpsize, F, T);
                if(mg_find_string(Tmp, T)) {
                    // add the uniform declaration if needed
                    sprintf(U, "uniform vec4 %s;\n", T);
                    Tmp = mg_inplace_insert(mg_getline(Tmp, headline), U, Tmp, &tmpsize);
                    headline += 1;
                }
            }
        } else {
            // need the full array...
            char T[60], U[300];
            sprintf(T, "_mg_%s_ProgramLocal", isVertex?"Vertex":"Fragment");
            sprintf(U, "uniform vec4 %s[%d];\n", T, isVertex?MAX_VTX_PROG_LOC_PARAMS:MAX_FRG_PROG_LOC_PARAMS);
            Tmp = mg_inplace_insert(mg_getline(Tmp, headline), U, Tmp, &tmpsize);
            headline += 1;
            Tmp = mg_inplace_replace(Tmp, &tmpsize, gl_ProgramLocal, T);
        }
    }
#define GO(A) \
  if(strstr(Tmp, gl_Samplers ## A)) {                                   \
    char S[60], D[60], U[60];                                           \
    for(int i=0; i<MAX_TEX; ++i) {                                      \
      sprintf(S, "%s%d", gl_Samplers ## A, i);                          \
      if(mg_find_string(Tmp, S)) {                                          \
        sprintf(D, "%s%d", mg_Samplers ## A, i);                     \
        sprintf(U, "%s%d;\n", mg_Samplers ## A ## _uniform, i);      \
        Tmp = mg_inplace_replace(Tmp, &tmpsize, S, D);                      \
        Tmp = mg_inplace_insert(mg_getline(Tmp, headline), U, Tmp, &tmpsize);  \
        headline += 1;                                                  \
      }                                                                 \
    }                                                                   \
  }
    GO(1D)
    GO(2D)
    GO(3D)
    GO(Cube)
#undef GO

    // non-square matrix handling
    // the square one first
    if(strstr(Tmp, "mat2x2")) {
        // better to use #define ?
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "mat2x2", "mat2");
    }
    if(strstr(Tmp, "mat3x3")) {
        // better to use #define ?
        Tmp = mg_inplace_replace(Tmp, &tmpsize, "mat3x3", "mat3");
    }

    // finish
#if DEBUG || GLOBAL_DEBUG
    printf("New Shader source:\n%s\n", Tmp);
#endif
    // clean preproc'd source
    if(versionString != NULL)
        free(versionString);
    if(pEntry!=pBuffer)
        free(pBuffer);
    return Tmp;
#undef ShadAppend()
}


const char* const* fpe_VertexShader(shaderconv_need_t* need, fpe_state_t *state) {
#define ShadAppend(S) shad = mg_append(shad, &shad_cap, S)
    // vertex is first called, so 1st time init is only here
    if(!shad_cap) shad_cap = 1024;
    if(!shad) shad = (char*)malloc(shad_cap);
    // state can be NULL, so provide a 0 default
    fpe_state_t default_state = {0};
    int is_default = !!need;
    if(!state) state = &default_state;
    int lighting = state->lighting;
    int twosided = state->twosided && lighting;
    if(need && ((need->need_color>1) || (need->need_secondary>1)))
        twosided = 1;
    int light_separate = state->light_separate && lighting;
    int secondary = (state->colorsum && !(lighting && light_separate)) || fpe_texenvSecondary(state) || (need && need->need_secondary);
    int fog = state->fog || (need && need->need_fogcoord);
    int fogsource = state->fogsource;
    int fogdist = state->fogdist;
    int fogmode = state->fogmode;
    int color_material = state->color_material && lighting;
    int point = state->point;
    int pointsprite = state->pointsprite;
    int headers = 0;
    int planes = state->plane;
    char buff[1024];
    int need_vertex = 0;
    int need_eyeplane[MAX_TEX][4] = {0};
    int need_objplane[MAX_TEX][4] = {0};
    int need_adjust[MAX_TEX] = {0};
    int need_lightproduct[2][MAX_LIGHT] = {0};
    int cm_front_nullexp = state->cm_front_nullexp;
    int cm_back_nullexp = state->cm_back_nullexp;
    int texgens = 0;
    int texmats = 0;
    const char* fogp = "highp";

    for (int i=0; i < g_gles_caps.maxtex; ++i) {
        if(state->texgen[i].texgen_s || state->texgen[i].texgen_t || state->texgen[i].texgen_r || state->texgen[i].texgen_q)
            texgens = 1;
        if(state->texture[i].texmat)
            texmats = 1;
    }

    strcpy(shad, fpeshader_signature);

//    comments = globals4es.comments;
//    DBG(comments=1-comments;)   // When DEBUG is activated, the effect of LIBGL_COMMENTS is reversed
    comments = DEBUG || GLOBAL_DEBUG;

    if(comments) {
        sprintf(buff, "// ** Vertex Shader **\n// lighting=%d (twosided=%d, separate=%d, color_material=%d)\n// secondary=%d, planes=%s\n// point=%d%s\n",
            lighting, twosided, light_separate, color_material, secondary, fpe_binary(planes, 6), point, need?" with need":"");
        ShadAppend(buff);
        headers+=mg_countline(buff);
        if(need) {
            sprintf(buff, "// need: color=%d, texs=%s, fogcoord=%d\n", need->need_color, fpe_binary(need->need_texs, 16), need->need_fogcoord);
            ShadAppend(buff);
            headers+=mg_countline(buff);
        }
    }
    if(!is_default) {
        ShadAppend("varying vec4 Color;\n");  // might be unused...
        headers++;
    }
    if(planes) {
        for (int i=0; i<g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                sprintf(buff, "uniform highp vec4 _mg_ClipPlane_%d;\n", i);
                ShadAppend(buff);
                ++headers;
                sprintf(buff, "varying mediump float clippedvertex_%d;\n", i);
                ShadAppend(buff);
                ++headers;
            }
        }
    }
    if(lighting) {
        sprintf(buff, 
            "struct _mg_FPELightSourceParameters1\n"
            "{\n"
            "%s"
            "   highp vec4 specular;\n"
            "   highp vec4 position;\n"
            "   highp vec3 spotDirection;\n"
            "   highp float spotExponent;\n"
            "   highp float spotCosCutoff;\n"
            "   highp float constantAttenuation;\n"
            "   highp float linearAttenuation;\n"
            "   highp float quadraticAttenuation;\n"
            "};\n", 
            (color_material)?
            "   highp vec4 ambient;\n"
            "   highp vec4 diffuse;\n"
            : ""
            );
        ShadAppend(buff);
        headers += mg_countline(buff);
        sprintf(buff, 
            "struct _mg_FPELightSourceParameters0\n"
            "{\n"
            "%s"
            "   highp vec4 specular;\n"
            "   highp vec4 position;\n"
            "   highp vec3 spotDirection;\n"
            "   highp float spotExponent;\n"
            "   highp float spotCosCutoff;\n"
            "};\n", 
            (color_material)?
            "   highp vec4 ambient;\n"
            "   highp vec4 diffuse;\n"
            : ""
            );
        ShadAppend(buff);
        headers += mg_countline(buff);

        sprintf(buff,
                "struct _mg_LightProducts\n"
                "{\n"
                "   highp vec4 ambient;\n"
                "   highp vec4 diffuse;\n"
                "   highp vec4 specular;\n"
                "};\n"                
        );
        ShadAppend(buff);
        headers += mg_countline(buff);

        if(!(cm_front_nullexp && color_material)) {
            ShadAppend("uniform highp float _mg_FrontMaterial_shininess;\n");
            headers++;
        }
        if(twosided && !(cm_back_nullexp && color_material)) {
            ShadAppend("uniform highp float _mg_BackMaterial_shininess;\n");
            headers++;
        }
        if(!(color_material && (state->cm_front_mode==FPE_CM_DIFFUSE || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE))) {
            ShadAppend("uniform highp float _mg_FrontMaterial_alpha;\n");
            headers++;
            if(twosided) {
                ShadAppend("uniform highp float _mg_BackMaterial_alpha;\n");
                headers++;
            }
        }
        for(int i=0; i<g_gles_caps.maxlights; i++) {
            if(state->light&(1<<i)) {
                sprintf(buff, "uniform _mg_FPELightSourceParameters%d _mg_LightSource_%d;\n", (state->light_direction>>i&1)?1:0, i);
                ShadAppend(buff);
                headers++;

                sprintf(buff, "uniform _mg_LightProducts _mg_FrontLightProduct_%d;\n", i);
                ShadAppend(buff);
                headers++;

                if(twosided) {
                    sprintf(buff, "uniform _mg_LightProducts _mg_BackLightProduct_%d;\n", i);
                    ShadAppend(buff);
                    headers++;
                }
            }
        }
    }
    if(!is_default) {
        if(twosided) {
            ShadAppend("varying vec4 BackColor;\n");
            headers++;
        }
        if(light_separate || secondary) {
            ShadAppend("varying vec4 SecColor;\n");
            headers++;
            if(twosided) {
                ShadAppend("varying vec4 SecBackColor;\n");
                headers++;
            }
        }
    }
    if(fog) {
        #if 0   // vertex fog
        ShadAppend("varying mediump float FogF;\n");
        headers++;
        if(fogsource==FPE_FOG_SRC_DEPTH && need_vertex<1)
            need_vertex = 1;
        #else   // pixel fog
        if(fogsource==FPE_FOG_SRC_COORD)
            sprintf(buff, "varying %s float FogSrc;\n", fogp);
        else {
            if(need_vertex<1)
                need_vertex = 1;
            switch(fogdist) {
                case FPE_FOG_DIST_RADIAL: sprintf(buff, "varying %s vec3 FogSrc;\n", fogp); break;
                case FPE_FOG_DIST_PLANE: sprintf(buff, "varying %s float FogSrc;\n", fogp); break;
                default: sprintf(buff, "varying %s float FogSrc;\n", fogp);
            }
        }
        ShadAppend(buff);
        #endif
    }
    // textures coordinates
    for (int i=0; i<g_gles_caps.maxtex; i++) {
        int t = state->texture[i].textype;
        if(need)
            t = (need->need_texs&(1<<i))?1:0;
        if(t) {
            sprintf(buff, "varying %s _mg_TexCoord_%d;\n", texvecsize[t-1], i);
            ShadAppend(buff);
            headers++;
            if(state->texture[i].texmat) {
                sprintf(buff, "uniform highp mat4 _mg_TextureMatrix_%d;\n", i);
                ShadAppend(buff);
                headers++;
            }
        }
    }
    // let's start
    ShadAppend("\nvoid main() {\n");
    int need_normal = 0;
    int normal_line = mg_countline(shad) - headers;
    if(planes) {
        for (int i=0; i<g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                sprintf(buff, "clippedvertex_%d = dot(vertex, _mg_ClipPlane_%d);\n", i, i);
                ShadAppend(buff);
            }
        }
        if(!need_vertex)
            need_vertex  = 1;
        //ShadAppend("gl_Position = clipvertex;\n");
    }
    ShadAppend("gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n");
    // initial Color / lighting calculation
    if(!lighting) {
        if(is_default && need) {
            if(need->need_color>=1)
                ShadAppend("gl_FrontColor = gl_Color;\n");
            if(need->need_color==2)
                ShadAppend("gl_BackColor = gl_Color;\n");
            if(need->need_secondary>=1)
                ShadAppend("gl_FrontSecondaryColor = gl_SecondaryColor;\n");
            if(need->need_secondary==2)
                ShadAppend("gl_BackSecondaryColor = gl_SecondaryColor;\n");
        } else {
            if(!need || (need && need->need_color))
                ShadAppend("Color = gl_Color;\n");
            if(secondary) {
                ShadAppend("SecColor = gl_SecondaryColor;\n");
            }
        }
    } else {
        if(comments) {
            sprintf(buff, "// ColorMaterial On/Off=%d Front = %d Back = %d\n", color_material, state->cm_front_mode, state->cm_back_mode);
            ShadAppend(buff);
        }
        if(is_default && need) {
            ShadAppend("vec4 Color;\n");
            if(twosided)
                ShadAppend("vec4 BackColor\n");
            if(secondary)
                ShadAppend("vec4 SecColor\n;");
            if(secondary && twosided)
                ShadAppend("vec4 SecBackColor\n");
        }
        // material emission
        char fm_emission[60], fm_ambient[60], fm_diffuse[60], fm_specular[60];
        char bm_emission[60], bm_ambient[60], bm_diffuse[60], bm_specular[60];
        sprintf(fm_emission, "%s", (color_material && state->cm_front_mode==FPE_CM_EMISSION)?"gl_Color":"gl_FrontMaterial.emission");
        sprintf(fm_ambient, "%s", (color_material && (state->cm_front_mode==FPE_CM_AMBIENT || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color":"gl_FrontMaterial.ambient");
        sprintf(fm_diffuse, "%s", (color_material && (state->cm_front_mode==FPE_CM_DIFFUSE || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color.xyz * _mg_LightSource_":"_mg_FrontLightProduct_");
        sprintf(fm_specular, "%s", (color_material && state->cm_front_mode==FPE_CM_SPECULAR)?"gl_Color.xyz * _mg_LightSource_":"_mg_FrontLightProduct_");
        if(twosided) {
            sprintf(bm_emission, "%s", (color_material && state->cm_back_mode==FPE_CM_EMISSION)?"gl_Color":"gl_BackMaterial.emission");
            sprintf(bm_ambient, "%s", (color_material && (state->cm_back_mode==FPE_CM_AMBIENT || state->cm_back_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color":"gl_BackMaterial.ambient");
            sprintf(bm_diffuse, "%s", (color_material && (state->cm_back_mode==FPE_CM_DIFFUSE || state->cm_back_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color.xyz * _mg_LightSource_":"_mg_BackLightProduct_");
            sprintf(bm_specular, "%s", (color_material && state->cm_back_mode==FPE_CM_SPECULAR)?"gl_Color.xyz * _mg_LightSource_":"_mg_BackLightProduct_");
        }

        if(color_material && 
            (state->cm_front_mode==FPE_CM_EMISSION 
            || state->cm_front_mode==FPE_CM_AMBIENT
            || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE
            || (twosided && 
                (state->cm_back_mode==FPE_CM_EMISSION || state->cm_back_mode==FPE_CM_AMBIENT || state->cm_back_mode==FPE_CM_AMBIENTDIFFUSE)))) 
        {
            sprintf(buff, "Color = %s;\n", fm_emission);
            ShadAppend(buff);
            if(twosided) {
                sprintf(buff, "BackColor = %s;\n", bm_emission);
                ShadAppend(buff);
            }
            
            sprintf(buff, "Color += %s*gl_LightModel.ambient;\n", fm_ambient);
            ShadAppend(buff);
            if(twosided) {
                sprintf(buff, "BackColor += %s*gl_LightModel.ambient;\n", bm_ambient);
                ShadAppend(buff);
            }
        } else {
            ShadAppend("Color = gl_FrontLightModelProduct.sceneColor;\n");
            if(twosided) {
                ShadAppend("BackColor = gl_BackLightModelProduct.sceneColor;\n");
            }
        }
        if(light_separate) {
            ShadAppend("SecColor=vec4(0.);\n");
            if(twosided)
                ShadAppend("SecBackColor=vec4(0.);\n");
        }
        ShadAppend("highp float att;\n");
        ShadAppend("highp float spot;\n");
        ShadAppend("highp vec3 VP;\n");
        ShadAppend("highp float lVP;\n");
        ShadAppend("highp float nVP;\n");
        ShadAppend("highp vec3 aa,dd,ss;\n");
        ShadAppend("highp vec3 hi;\n");
        if(twosided)
            ShadAppend("highp vec3 back_aa,back_dd,back_ss;\n");
        need_normal = 1;
        for(int i=0; i<g_gles_caps.maxlights; i++) {
            if(state->light&(1<<i)) {
                if(comments) {
                    sprintf(buff, "// light %d on, light_direction=%d, light_cutoff180=%d\n", i, (state->light_direction>>i&1), (state->light_cutoff180>>i&1));
                    ShadAppend(buff);
                }
                // enabled light i
                // att depend on light position w
                if((state->light_direction>>i&1)==0) { // flag is 1 if light is has w!=0
                    ShadAppend("att = 1.0;\n");
                    sprintf(buff, "VP = normalize(_mg_LightSource_%d.position.xyz);\n", i);
                    ShadAppend(buff);
                } else {
                    sprintf(buff, "VP = _mg_LightSource_%d.position.xyz - vertex.xyz;\n", i);
                    ShadAppend(buff);
                    ShadAppend("lVP = length(VP);\n");
                    sprintf(buff, "att = 1.0/(_mg_LightSource_%d.constantAttenuation + lVP*(_mg_LightSource_%d.linearAttenuation + _mg_LightSource_%d.quadraticAttenuation * lVP));\n", i, i, i);
                    ShadAppend(buff);
                    ShadAppend("VP = normalize(VP);\n");
                    if(!need_vertex) need_vertex=1;
                }
                // spot depend on spotlight cutoff angle
                if((state->light_cutoff180>>i&1)==0) {
                    //ShadAppend("spot = 1.0;\n");
                } else {
                    /*if((state->light_direction>>i&1)==0) {
                        sprintf(buff, "spot = max(dot(-normalize(vertex.xyz), _mg_LightSource_%d.spotDirection), 0.);\n", i);
                        if(!need_vertex) need_vertex=1;
                    } else*/ {
                        sprintf(buff, "spot = max(dot(-VP, _mg_LightSource_%d.spotDirection), 0.);\n", i);
                    }
                    ShadAppend(buff);
                    sprintf(buff, "if(spot<_mg_LightSource_%d.spotCosCutoff) spot=0.0; else spot=pow(spot, _mg_LightSource_%d.spotExponent);\n", i, i);
                    ShadAppend(buff);
                    ShadAppend("att *= spot;\n");
                }
                if(color_material && (state->cm_front_mode==FPE_CM_AMBIENT || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE)) {
                    sprintf(buff, "aa = %s.xyz * _mg_LightSource_%d.ambient.xyz;\n", fm_ambient, i);
                    ShadAppend(buff);
                } else {
                    sprintf(buff, "aa = _mg_FrontLightProduct_%d.ambient.xyz;\n", i);
                    ShadAppend(buff);
                    need_lightproduct[0][i] = 1;
                }
                if(twosided) {
                    if(color_material && (state->cm_back_mode==FPE_CM_AMBIENT || state->cm_back_mode==FPE_CM_AMBIENTDIFFUSE)) {
                        sprintf(buff, "back_aa = %s.xyz * _mg_LightSource_%d.ambient.xyz;\n", bm_ambient, i);
                        ShadAppend(buff);
                    } else {
                        sprintf(buff, "back_aa = _mg_BackLightProduct_%d.ambient.xyz;\n", i);
                        ShadAppend(buff);
                        need_lightproduct[1][i] = 1;                     
                    }                        
                }
                sprintf(buff, "nVP = dot(normal, VP);\n");
                ShadAppend(buff);
                sprintf(buff, "dd = (nVP>0.)?(nVP * %s%d.diffuse.xyz):vec3(0.);\n", fm_diffuse, i);
                ShadAppend(buff);
                need_lightproduct[0][i] = 1;
                if(twosided) {
                    sprintf(buff, "back_dd = (nVP<0.)?(-nVP * %s%d.diffuse.xyz):vec3(0.);\n", bm_diffuse, i);
                    ShadAppend(buff);
                    need_lightproduct[1][i] = 1;
                }
                if(state->light_localviewer) {
                    ShadAppend("hi = normalize(VP + normalize(-vertex.xyz));\n");
                    if(!need_vertex) need_vertex=1;
                } else {
                    ShadAppend("hi = normalize(VP + vec3(0., 0., 1.));\n");
                }
                ShadAppend("lVP = dot(normal, hi);\n");
                if(cm_front_nullexp)
                    sprintf(buff, "ss = (nVP>0. && lVP>0.)?(pow(lVP, %s)*%s%d.specular.xyz):vec3(0.);\n", (color_material)?"gl_FrontMaterial.shininess":"_mg_FrontMaterial_shininess", fm_specular, i);
                else
                    sprintf(buff, "ss = (nVP>0. && lVP>0.)?(%s%d.specular.xyz):vec3(0.);\n", fm_specular, i);
                ShadAppend(buff);
                if(twosided) {
                    if(state->cm_back_nullexp)    // 1, exp is not null
                        sprintf(buff, "back_ss = (nVP<0. && lVP<0.)?(pow(-lVP, %s)*%s%d.specular.xyz):vec3(0.);\n", (color_material)?"gl_BackMaterial.shininess":"_mg_BackMaterial_shininess", bm_specular, i);
                    else
                        sprintf(buff, "back_ss = (nVP<0. && lVP<0.)?(%s%d.specular.xyz):vec3(0.);\n", bm_specular, i);
                    ShadAppend(buff);
                }
                if(state->light_separate) {
                    ShadAppend("Color.rgb += att*(aa+dd);\n");
                    ShadAppend("SecColor.rgb += att*(ss);\n");
                    if(twosided) {
                        ShadAppend("BackColor.rgb += att*(back_aa+back_dd);\n");
                        ShadAppend("SecBackColor.rgb += att*(back_ss);\n");
                    }
                } else {
                    ShadAppend("Color.rgb += att*(aa+dd+ss);\n");
                    if(twosided)
                        ShadAppend("BackColor.rgb += att*(back_aa+back_dd+back_ss);\n");
                }
                if(comments) {
                    sprintf(buff, "// end of light %d\n", i);
                    ShadAppend(buff);
                }
            }
        }
        sprintf(buff, "Color.a = %s;\n", (color_material && (state->cm_front_mode==FPE_CM_DIFFUSE || state->cm_front_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color.a":"_mg_FrontMaterial_alpha");
        ShadAppend(buff);
        ShadAppend("Color.rgb = clamp(Color.rgb, 0., 1.);\n");
        if(twosided) {
            sprintf(buff, "BackColor.a = %s;\n", (color_material && (state->cm_back_mode==FPE_CM_DIFFUSE || state->cm_back_mode==FPE_CM_AMBIENTDIFFUSE))?"gl_Color.a":"_mg_BackMaterial_alpha");
            ShadAppend("BackColor.rgb = clamp(BackColor.rgb, 0., 1.);\n");
            ShadAppend(buff);
        }
        if(state->light_separate) {
            ShadAppend("SecColor.rgb = clamp(SecColor.rgb, 0., 1.);\n");
            if(twosided) {
                ShadAppend("SecBackColor.rgb = clamp(SecBackColor.rgb, 0., 1.);\n");
            }
        }
        if(is_default && need) {
            if(need->need_color>0)
                ShadAppend("gl_FrontColor = Color;\n");
            if(need->need_color>1)
                ShadAppend("gl_BackColor = BackColor;\n");
            if(need->need_secondary>0)
                ShadAppend("gl_FrontSecondaryColor = SecColor;\n");
            if(need->need_secondary>1)
                ShadAppend("gl_BackSecondaryColor = SecBackColor;\n");
        }
    }
    // calculate texture coordinates
    if(comments)
        ShadAppend("// texturing\n");
    if(texgens)
        ShadAppend("vec4 tmp_tcoor;\n");
    int spheremap = 0;
    int reflectmap = 0;
    if(texmats)
        ShadAppend("vec4 tmp_tex;\n");
    for (int i=0; i<g_gles_caps.maxtex; i++) {
        int t = state->texture[i].textype;
        if(need && (need->need_texs&(1<<i)) && t==0)
            t = 1;
        if(need && !(need->need_texs&(1<<i)))
            t = 0;
        int mat = state->texture[i].texmat;
        int adjust = state->texture[i].texadjust;
        int tg[4];
        tg[0] = state->texgen[i].texgen_s;
        tg[1] = state->texgen[i].texgen_t;
        tg[2] = state->texgen[i].texgen_r;
        tg[3] = state->texgen[i].texgen_q;        
        if(t) {
            int ntc = texnsize[t-1];
            if(comments) {
                sprintf(buff, "// texture %d active: %X %s %s\n", i, t, mat?"with matrix":"", adjust?"npot adjusted":"");
                ShadAppend(buff);
            }
            char texcoord[50];
            if (tg[0] || tg[1] || tg[2] || tg[3]) {
                // One or more texgen is active
                if(tg[0]) tg[0] = state->texgen[i].texgen_s_mode; else tg[0] = FPE_TG_NONE;
                if(tg[1]) tg[1] = state->texgen[i].texgen_t_mode; else tg[1] = FPE_TG_NONE;
                if(tg[2]) tg[2] = state->texgen[i].texgen_r_mode; else tg[2] = FPE_TG_NONE;
                if(tg[3]) tg[3] = state->texgen[i].texgen_q_mode; else tg[3] = FPE_TG_NONE;
                if(comments) {
                    sprintf(buff, "//  texgen %d / %d / %d / %d\n", tg[0], tg[1], tg[2], tg[3]);
                    ShadAppend(buff);
                }
                sprintf(texcoord, "tmp_tcoor");
                ShadAppend("tmp_tcoor=vec4(0., 0., 0., 1.);\n");
                if(mat) {
                    ntc = 4;
                }
                for (int j=0; j<ntc; j++) {
                    if(tg[j]==FPE_TG_NORMALMAP) {
                        need_normal=1;
                        if(j==0 && tg[j+1]==FPE_TG_NORMALMAP) {
                            sprintf(buff, "tmp_tcoor.%c%c=normal.%c%c;\n", texcoordxy[j], texcoordxy[j+1], texcoordxy[j], texcoordxy[j+1]);
                            ++j;
                        } else
                            sprintf(buff, "tmp_tcoor.%c=normal.%c;\n", texcoordxy[j], texcoordxy[j]);
                    } else if(tg[j]==FPE_TG_SPHEREMAP) {
                        if(!spheremap) {
                            spheremap = 1;
                            if(!need_vertex) need_vertex=1;
                            need_normal = 1;
                            ShadAppend("vec3 tmpsphere = reflect(normalize(vertex.xyz), normal);\n");
                            ShadAppend("tmpsphere.z+=1.0;\n");
                            if(j==0 && tg[j+1]==FPE_TG_SPHEREMAP)
                                sprintf(buff, "tmp_tcoor.xy = tmpsphere.xy*(0.5*inversesqrt(dot(tmpsphere, tmpsphere))) + vec2(0.5);");
                            else
                                ShadAppend("tmpsphere.xy = tmpsphere.xy*(0.5*inversesqrt(dot(tmpsphere, tmpsphere))) + vec2(0.5);");
                        }
                        if(j==0 && tg[j+1]==FPE_TG_SPHEREMAP)
                            ++j;
                        else
                            sprintf(buff, "tmp_tcoor.%c=tmpsphere.%c;\n", texcoordxy[j], texcoordxy[j]);
                    } else if(tg[j]==FPE_TG_OBJLINEAR) {
                        sprintf(buff, "tmp_tcoor.%c=dot(gl_Vertex, _mg_ObjectPlane%c_%d);\n", texcoordxy[j], texcoordNAME[j], i);
                        need_objplane[i][j] = 1;
                    } else if(tg[j]==FPE_TG_EYELINEAR) {
                        sprintf(buff, "tmp_tcoor.%c=dot(vertex, _mg_EyePlane%c_%d);\n", texcoordxy[j], texcoordNAME[j], i);
                        need_eyeplane[i][j] = 1;
                        if(!need_vertex) need_vertex=1;
                    } else if(tg[j]==FPE_TG_REFLECMAP) {
                        if(!reflectmap) {
                            reflectmap = 1;
                            if(!need_vertex) need_vertex=1;
                            need_normal = 1;
                            if(j==0 && tg[j+1]==FPE_TG_REFLECMAP && tg[j+2]==FPE_TG_REFLECMAP)
                                sprintf(buff, "tmp_tcoor.xyz = reflect(normalize(vertex.xyz), normal);\n");
                            else
                                ShadAppend("vec3 tmpreflect = reflect(normalize(vertex.xyz), normal);\n");
                        }
                        if(j==0 && tg[j+1]==FPE_TG_REFLECMAP && tg[j+2]==FPE_TG_REFLECMAP)
                            j+=2;
                        else
                            sprintf(buff, "tmp_tcoor.%c=tmpreflect.%c;\n", texcoordxy[j], texcoordxy[j]);
                    } else if(tg[j]==FPE_TG_NONE) {
                        sprintf(buff, "tmp_tcoor.%c=gl_MultiTexCoord%d.%c;\n", texcoordxy[j], i, texcoordxy[j]);
                    }
                    ShadAppend(buff);
                }
            } else {
                sprintf(texcoord, "gl_MultiTexCoord%d", i);
            }
            const char* text_tmp = texcoord;
            static const char* tmp_tex = "tmp_tex";
            if(mat) {
                text_tmp = tmp_tex;
                sprintf(buff, "%s = (_mg_TextureMatrix_%d * %s);\n", text_tmp, i, texcoord);
                ShadAppend(buff);
            }
            if(t==FPE_TEX_STRM) {
                sprintf(buff, "_mg_TexCoord_%d = %s.%s / %s.q;\n", i, text_tmp, texxyzsize[t-1], text_tmp);
            } else {
                sprintf(buff, "_mg_TexCoord_%d = %s.%s;\n", i, text_tmp, texxyzsize[t-1]);
            }
            ShadAppend(buff);
            if(adjust) {
                need_adjust[i] = 1;
                sprintf(buff, "_mg_TexCoord_%d.xy *= _mg_TexAdjust_%d;\n", i, i);    // to avoid error on Cube map... but will that work anyway?
                ShadAppend(buff);
            }
        }
    }
    // point sprite special case
    if(point) {
        if(!need_vertex)
            need_vertex = 1;
        ShadAppend("float ps_d = length(vertex);\n");
        sprintf(buff, "gl_PointSize = clamp(gl_Point.size*inversesqrt(gl_Point.distanceConstantAttenuation + ps_d*(gl_Point.distanceLinearAttenuation + ps_d*gl_Point.distanceQuadraticAttenuation)), gl_Point.sizeMin, gl_Point.sizeMax);\n");
        ShadAppend(buff);
    }
    // insert normal, vertex and eye/obj planes if needed
    if(need_vertex) {
        buff[0] = '\0';
        if(need_vertex==1)
            strcat(buff, "vec4 ");
        strcat(buff, "vertex = gl_ModelViewMatrix * gl_Vertex;\n");
        shad = mg_inplace_insert(mg_getline(shad, normal_line + headers), buff, shad, &shad_cap);
        normal_line += mg_countline(buff);
    }
    if(need_normal) {
#if 0
        if(state->rescaling)
            strcpy(buff, "vec3 normal = gl_NormalScale*(gl_NormalMatrix * gl_Normal);\n");
        else
            strcpy(buff, "vec3 normal = gl_NormalMatrix * gl_Normal;\n");
        if(state->normalize)
            strcat(buff, "normal = normalize(normal);\n");
#else
// Implementions may choose to normalize for rescale...
        if(state->rescaling || state->normalize)
            strcpy(buff, "vec3 normal = normalize(gl_NormalMatrix * gl_Normal);\n");
        else
            //strcpy(buff, "vec3 normal = (vec4(gl_Normal, (gl_Vertex.w==0.0)?0.0:(-dot(gl_Normal, gl_Vertex.xyz)/gl_Vertex.w))*gl_ModelViewMatrixInverse).xyz;\n");
            strcpy(buff, "vec3 normal = gl_NormalMatrix * gl_Normal;\n");
#endif
        shad = mg_inplace_insert(mg_getline(shad, normal_line + headers), buff, shad, &shad_cap);
    }
    buff[0] = '\0';
    for (int i=0; i<MAX_TEX; i++) {
        char tmp[100];
        for (int j=0; j<4; j++) {
            if(need_objplane[i][j]) {
                sprintf(tmp, "uniform vec4 _mg_ObjectPlane%c_%d;\n", texcoordNAME[j], i);
                strcat(buff, tmp);
            }
            if(need_eyeplane[i][j]) {
                sprintf(tmp, "uniform vec4 _mg_EyePlane%c_%d;\n", texcoordNAME[j], i);
                strcat(buff, tmp);
            }
        }
        if(need_adjust[i]) {
            sprintf(tmp, "uniform vec2 _mg_TexAdjust_%d;\n", i);
            strcat(buff, tmp);
        }
    }
    if(buff[0]!='\0') {
        shad = mg_inplace_insert(mg_getline(shad, headers), buff, shad, &shad_cap);
        headers += mg_countline(buff);
    }
    if(fog) {
        if(comments) {
            sprintf(buff, "// Fog On: mode=%X, source=%X distance=%X\n", fogmode, fogsource, fogdist);
            ShadAppend(buff);
        }
        #if 0    // vertex fog
        char fogsrc[50];
        if(fogsource==FPE_FOG_SRC_COORD)
            strcpy(fogsrc, "gl_FogCoord");
        else switch(fogdist) {
            case FPE_FOG_DIST_RADIAL: strcpy(fogsrc, "length(vertex.xyz)"); break;
            case FPE_FOG_DIST_PLANE: strcpy(fogsrc, "vertex.z"); break;
            default: strcpy(fogsrc, "abs(vertex.z)");
        }
        sprintf(buff, "float fog_c = %s;\n", fogsrc);
        ShadAppend(buff);
        switch(fogmode) {
            case FPE_FOG_EXP:
                ShadAppend("FogF = clamp(exp(-gl_Fog.density * fog_c), 0., 1.);\n");
                break;
            case FPE_FOG_EXP2:
                ShadAppend("FogF = clamp(exp(-(gl_Fog.density * fog_c)*(gl_Fog.density * fog_c)), 0., 1.);\n");
                break;
            case FPE_FOG_LINEAR:
                ShadAppend("FogF = clamp((gl_Fog.end - fog_c) * gl_Fog.scale, 0., 1.);\n");
                break;
        }
        #else   // pixel fog
        if(fogsource==FPE_FOG_SRC_COORD)
            sprintf(buff, "FogSrc = gl_FogCoord;\n");
        else switch(fogdist) {
            case FPE_FOG_DIST_RADIAL: sprintf(buff, "FogSrc = vertex.xyz;\n"); break;
            case FPE_FOG_DIST_PLANE:
            default: sprintf(buff, "FogSrc = vertex.z;\n");
        }
        ShadAppend(buff);
        #endif
    }
        

    ShadAppend("}\n");

    LOG_D("FPE Shader: \n%s\n", shad)

    return (const char* const*)&shad;
}

const char* const* fpe_FragmentShader(shaderconv_need_t* need, fpe_state_t *state) {
    // state can be NULL, so provide a 0 default
    fpe_state_t default_state = {0};
    int is_default = !need;
    if(!state) state = &default_state;
    int headers = 0;
    int lighting = state->lighting;
    int twosided = state->twosided && lighting;
    int light_separate = state->light_separate && lighting;
    int secondary = is_default?((state->colorsum && !(lighting && light_separate)) || fpe_texenvSecondary(state)):need->need_secondary;
    int alpha_test = state->alphatest;
    int alpha_func = state->alphafunc;
    int fog = is_default?state->fog:need->need_fogcoord;
    int fogsource = state->fogsource;
    int fogmode = state->fogmode;
    int fogdist = state->fogdist;
    int planes = state->plane;
    int point = state->point;
    int pointsprite = state->pointsprite;
    int pointsprite_coord = state->pointsprite_coord;
    int pointsprite_upper = state->pointsprite_upper;
    int shaderblend = state->blend_enable;
    int texenv_combine = 0;
    int texturing = 0;
    char buff[1024];
    const char* fogp = "highp";


    strcpy(shad, fpeshader_signature);

    // check texture streaming and texturing
    {
        int need_stream = 0;
        for (int i=0; i<g_gles_caps.maxtex; i++) {
            const int t = state->texture[i].textype;
            if(t==FPE_TEX_STRM)
                need_stream = 1;
            if(t)
                ++texturing;
        }
        if(need_stream)
            ShadAppend("#extension GL_IMG_texture_stream2 : enable\n");
    }

    if(shaderblend)
        ShadAppend("#extension GL_ARM_shader_framebuffer_fetch : enable\n");
    
    if(comments) {
        sprintf(buff, "// ** Fragment Shader **\n// lighting=%d, alpha=%d, secondary=%d, planes=%s, texturing=%d point=%d\n", lighting, alpha_test, secondary, fpe_binary(planes, 6), texturing, point);
        ShadAppend(buff);
        headers+=mg_countline(buff);
    }
    ShadAppend("varying vec4 Color;\n");
    headers++;
    if(twosided) {
        ShadAppend("varying vec4 BackColor;\n");
        headers++;
    }
    if(light_separate || secondary) {
        ShadAppend("varying vec4 SecColor;\n");
        headers++;
        if(twosided) {
            ShadAppend("varying vec4 SecBackColor;\n");
            headers++;
        }
    }
    if(fog) {
        #if 0   // vertex fog
        ShadAppend("varying mediump float FogF;\n");
        headers++;
        #else   // pixel fog
        if(fogsource==FPE_FOG_SRC_COORD) {
            sprintf(buff, "varying %s float FogSrc;\n", fogp);
        } else switch(fogdist) {
            case FPE_FOG_DIST_RADIAL: sprintf(buff, "varying %s vec3 FogSrc;\n", fogp); break;
            case FPE_FOG_DIST_PLANE: sprintf(buff, "varying %s float FogSrc;\n", fogp); break;
            default: sprintf(buff, "varying %s float FogSrc;\n", fogp);
        }
        ShadAppend(buff);
        #endif
    }
    if(planes) {
        //ShadAppend("varying vec4 clipvertex;\n");
        for (int i=0; i<g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                sprintf(buff, "varying mediump float clippedvertex_%d;\n", i);
                ShadAppend(buff);
                headers++;
            }
        }
    }
    // textures coordinates
    for (int i=0; i<g_gles_caps.maxtex; i++) {
        int t = state->texture[i].textype;
        if(point && !pointsprite) t=0;
        if(!is_default)
            if(t && !need->need_texs&(1<<i))
                t = 0;
        if(t) {
            sprintf(buff, "varying %s _mg_TexCoord_%d;\n", texvecsize[t-1], i);
            ShadAppend(buff);
            sprintf(buff, "uniform %s _mg_TexSampler_%d;\n", texsampler[t-1], i);
            ShadAppend(buff);
            headers++;

            int texenv = state->texenv[i].texenv;
            if (texenv>=FPE_COMBINE) {
                int n = 1+texenv-FPE_COMBINE;
                if(n>texenv_combine) texenv_combine=n;
                if(state->texenv[i].texrgbscale) {
                    sprintf(buff, "uniform float _mg_TexEnvRGBScale_%d;\n", i);
                    ShadAppend(buff);
                    headers++;
                }
                if(state->texenv[i].texalphascale) {
                    sprintf(buff, "uniform float _mg_TexEnvAlphaScale_%d;\n", i);
                    ShadAppend(buff);
                    headers++;
                }
            }
        }
    }
    if(alpha_test && alpha_func>FPE_NEVER) {
        ShadAppend(mg_alphaRefSource);
        headers++;
    } 

    ShadAppend("void main() {\n");

    //*** Clip Planes (it's probably not the best idea to do that here...)
    if(planes) {
        ShadAppend("if((");
        int k=0;
        for (int i=0; i<g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                //sprintf(buff, "%smin(0., dot(clipvertex, gl_ClipPlane[%d]))", k?"+":"",  i);
                sprintf(buff, "%smin(0., clippedvertex_%d)", k?"+":"",  i);
                ShadAppend(buff);
                k=1;
            }
        }
        ShadAppend(")<0.) discard;\n");
    }

    //*** initial color
    sprintf(buff, "vec4 fColor = %s;\n", twosided?"(gl_FrontFacing)?Color:BackColor":"Color");
    ShadAppend(buff);

    //*** apply textures
    if(texturing && (!point || pointsprite) ) {
        // fetch textures first
        for (int i=0; i<g_gles_caps.maxtex; i++) {
            int t = state->texture[i].textype;
            if(t) {
                if(point && pointsprite && pointsprite_coord) {
                    if(pointsprite_upper)
                        sprintf(buff, "vec4 texColor%d = %s(_mg_TexSampler_%d, vec2(gl_PointCoord.x, 1.-gl_PointCoord.y));\n", i, texnoproj[t-1], i);
                    else
                        sprintf(buff, "vec4 texColor%d = %s(_mg_TexSampler_%d, gl_PointCoord);\n", i, texnoproj[t-1], i);
                } else
                    sprintf(buff, "vec4 texColor%d = %s(_mg_TexSampler_%d, _mg_TexCoord_%d);\n", i, texname[t-1], i, i);
                ShadAppend(buff);
            }
        }

        // TexEnv stuff
        if(texenv_combine>0) {
            if(texenv_combine==2)
                ShadAppend("vec4 Arg0, Arg1, Arg2, Arg3;\n");
            else
                ShadAppend("vec4 Arg0, Arg1, Arg2;\n");
        }
        // fetch textures first
        for (int i=0; i<g_gles_caps.maxtex; i++) {
            int t = state->texture[i].textype;

            if(t) {
                int texenv = state->texenv[i].texenv;
                int texformat = state->texture[i].texformat;
                if(comments) {
                    sprintf(buff, "// Texture %d active: %X, texenv=%X, format=%X\n", i, t, texenv, texformat);
                    ShadAppend(buff);
                }
                int needclamp = 1;
                switch (texenv) {
                    case FPE_MODULATE:
                        if(texformat==FPE_TEX_RGB || texformat==FPE_TEX_LUM) {
                            sprintf(buff, "fColor.rgb *= texColor%d.rgb;\n", i);
                            ShadAppend(buff);
                        } else if(texformat==FPE_TEX_ALPHA) {
                            sprintf(buff, "fColor.a *= texColor%d.a;\n", i);
                            ShadAppend(buff);
                        } else {
                            sprintf(buff, "fColor *= texColor%d;\n", i);
                            ShadAppend(buff);
                        }
                        needclamp = 0;
                        break;
                    case FPE_ADD:
                        if(texformat!=FPE_TEX_ALPHA) {
                            sprintf(buff, "fColor.rgb += texColor%d.rgb;\n", i);
                            ShadAppend(buff);
                        }
                        if(texformat==FPE_TEX_INTENSITY || texformat==FPE_TEX_DEPTH)
                            sprintf(buff, "fColor.a += texColor%d.a;\n", i);
                        else
                            sprintf(buff, "fColor.a *= texColor%d.a;\n", i);
                        ShadAppend(buff);
                        break;
                    case FPE_DECAL:
                        sprintf(buff, "fColor.rgb = mix(fColor.rgb, texColor%d.rgb, texColor%d.a);\n", i, i);
                        ShadAppend(buff);
                        needclamp = 0;
                        break;
                    case FPE_BLEND:
                        // create the Uniform for TexEnv Constant color
                        sprintf(buff, "uniform lowp vec4 _mg_TextureEnvColor_%d;\n", i);
                        shad = mg_inplace_insert(mg_getline(shad, headers), buff, shad, &shad_cap);
                        headers+=mg_countline(buff);
                        needclamp=0;
                        if(texformat!=FPE_TEX_ALPHA) {
                            sprintf(buff, "fColor.rgb = mix(fColor.rgb, _mg_TextureEnvColor_%d.rgb, texColor%d.rgb);\n", i, i);
                            ShadAppend(buff);
                        }
                        switch(texformat) {
                            case FPE_TEX_LUM:
                            case FPE_TEX_RGB:
                                // no change in alpha channel
                                break;
                            case FPE_TEX_INTENSITY:
                            case FPE_TEX_DEPTH:
                                sprintf(buff, "fColor.a = mix(fColor.a, _mg_TextureEnvColor_%d.a, texColor%d.a);\n", i, i);
                                ShadAppend(buff);
                                break;
                            default:
                                sprintf(buff, "fColor.a *= texColor%d.a;\n", i);
                                ShadAppend(buff);
                        }
                        ShadAppend(buff);
                        break;
                    case FPE_REPLACE:
                        if(texformat==FPE_TEX_RGB || texformat==FPE_TEX_LUM) {
                            sprintf(buff, "fColor.rgb = texColor%d.rgb;\n", i);
                            ShadAppend(buff);
                        } else if(texformat==FPE_TEX_ALPHA) {
                            sprintf(buff, "fColor.a = texColor%d.a;\n", i);
                            ShadAppend(buff);
                        } else {
                            sprintf(buff, "fColor = texColor%d;\n", i);
                            ShadAppend(buff);
                        }
                        needclamp = 0;
                        break;
                    case FPE_COMBINE:
                    case FPE_COMBINE4:
                        {
                            int constant = 0;
                            // parse the combine state
                            int combine_rgb = state->texcombine[i]&0xf;
                            int combine_alpha = (state->texcombine[i]>>4)&0xf;
                            int src_r[4], op_r[4];
                            int src_a[4], op_a[4];
                                src_a[0] = state->texenv[i].texsrcalpha0;
                                op_a[0] = state->texenv[i].texopalpha0;
                                src_r[0] = state->texenv[i].texsrcrgb0;
                                op_r[0] = state->texenv[i].texoprgb0;
                                src_a[1] = state->texenv[i].texsrcalpha1;
                                op_a[1] = state->texenv[i].texopalpha1;
                                src_r[1] = state->texenv[i].texsrcrgb1;
                                op_r[1] = state->texenv[i].texoprgb1;
                                src_a[2] = state->texenv[i].texsrcalpha2;
                                op_a[2] = state->texenv[i].texopalpha2;
                                src_r[2] = state->texenv[i].texsrcrgb2;
                                op_r[2] = state->texenv[i].texoprgb2;
                                src_a[3] = state->texenv[i].texsrcalpha3;
                                op_a[3] = state->texenv[i].texopalpha3;
                                src_r[3] = state->texenv[i].texsrcrgb3;
                                op_r[3] = state->texenv[i].texoprgb3;
                            if(combine_rgb==FPE_CR_DOT3_RGBA) {
                                    src_a[0] = src_a[1] = src_a[2] = -1;
                                    op_a[0] = op_a[1] = op_a[2] = -1;
                                    src_r[2] = op_r[2] = -1;
                                    src_r[3] = op_r[3] = -1;
                                    src_a[3] = op_a[3] = -1;
                            } else {
                                if(combine_alpha==FPE_CR_REPLACE) {
                                    src_a[1] = src_a[2] = src_a[3] = -1;
                                    op_a[1] = op_a[2] = op_a[3] = -1;
                                } else if (combine_alpha>=FPE_CR_MOD_ADD || combine_alpha==FPE_CR_INTERPOLATE) {
                                    // need all 3
                                    src_a[3] = -1; op_a[3] = -1;
                                } else {
                                    if(texenv==FPE_COMBINE4 && (combine_alpha==FPE_CR_ADD || combine_alpha==FPE_CR_ADD_SIGNED)) {
                                        // need all 4
                                    } else {
                                        src_a[2] = src_a[3] = -1;
                                        op_a[2] = op_a[3] = -1;
                                    }
                                }
                                if(combine_rgb==FPE_CR_REPLACE) {
                                    src_r[1] = src_r[2] = src_r[3] = -1;
                                    op_r[1] = op_r[2] = op_r[3] = -1;
                                } else if (combine_rgb>=FPE_CR_MOD_ADD || combine_rgb==FPE_CR_INTERPOLATE) {
                                    // need all 3
                                    src_r[3] = -1; op_r[3] = -1;
                                } else {
                                    if(texenv==FPE_COMBINE4 && (combine_rgb==FPE_CR_ADD || combine_rgb==FPE_CR_ADD_SIGNED)) {
                                        // need all 4
                                    } else {
                                        src_r[2] = src_r[3] = -1;
                                        op_r[2] = op_r[3] = -1;
                                    }
                                }
                            }
                            // is texture constants needed ?
                            for (int j=0; j<4; j++) {
                                if (src_a[j]==FPE_SRC_CONSTANT || src_r[j]==FPE_SRC_CONSTANT)
                                    constant=1;
                            }
                            if(comments) {
                                sprintf(buff, " //  Combine RGB: fct=%d, Src/Op: 0=%d/%d 1=%d/%d 2=%d/%d 3=%d/%d\n", combine_rgb, src_r[0], op_r[0], src_r[1], op_r[1], src_r[2], op_r[2], src_r[3], op_r[3]);
                                ShadAppend(buff);
                                sprintf(buff, " //  Combine Alpha: fct=%d, Src/Op: 0=%d/%d 1=%d/%d 2=%d/%d 3=%d/%d\n", combine_alpha, src_a[0], op_a[0], src_a[1], op_a[1], src_a[2], op_a[2], src_a[3], op_a[3]);
                                ShadAppend(buff);
                            }
                            if(constant) {
                                // yep, create the Uniform
                                sprintf(buff, "uniform lowp vec4 _mg_TextureEnvColor_%d;\n", i);
                                shad = mg_inplace_insert(mg_getline(shad, headers), buff, shad, &shad_cap);
                                headers+=mg_countline(buff);                            
                            }
                            for (int j=0; j<4; j++) {
                                if(src_r[j]==src_a[j] && op_r[j]==FPE_OP_SRCCOLOR && op_a[j]==FPE_OP_ALPHA) {
                                    sprintf(buff, "Arg%d = %s;\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                    ShadAppend(buff);
                                } else if(src_r[j]==src_a[j] && op_r[j]==FPE_OP_MINUSCOLOR && op_a[j]==FPE_OP_MINUSALPHA) {
                                    sprintf(buff, "Arg%d = vec4(1.) - %s;\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                    ShadAppend(buff);
                                } else {
                                    if(op_r[j]!=-1)
                                    switch(op_r[j]) {
                                        case FPE_OP_SRCCOLOR:
                                            sprintf(buff, "Arg%d.rgb = %s.rgb;\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                        case FPE_OP_MINUSCOLOR:
                                            sprintf(buff, "Arg%d.rgb = vec3(1.) - %s.rgb;\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                        case FPE_OP_ALPHA:
                                            sprintf(buff, "Arg%d.rgb = vec3(%s.a);\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                        case FPE_OP_MINUSALPHA:
                                            sprintf(buff, "Arg%d.rgb = vec3(1. - %s.a);\n", j, fpe_texenvSrc(src_r[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                    }
                                    if(op_a[j]!=-1)
                                    switch(op_a[j]) {
                                        case FPE_OP_ALPHA:
                                            sprintf(buff, "Arg%d.a = %s.a;\n", j, fpe_texenvSrc(src_a[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                        case FPE_OP_MINUSALPHA:
                                            sprintf(buff, "Arg%d.a = 1. - %s.a;\n", j, fpe_texenvSrc(src_a[j], i, twosided));
                                            ShadAppend(buff);
                                            break;
                                    }
                                }
                            }
                            if(combine_rgb!=FPE_CR_DOT3_RGBA && combine_rgb!=FPE_CR_DOT3_RGB && combine_rgb==combine_alpha) {
                                switch(combine_alpha) {
                                    case FPE_CR_REPLACE:
                                        ShadAppend("fColor = Arg0;\n");
                                        break;
                                    case FPE_CR_MODULATE:
                                        ShadAppend("fColor = Arg0 * Arg1;\n");
                                        break;
                                    case FPE_CR_ADD:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor = Arg0*Arg1 + Arg2*Arg3;\n");
                                        else
                                            ShadAppend("fColor = Arg0 + Arg1;\n");
                                        break;
                                    case FPE_CR_ADD_SIGNED:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor = Arg0*Arg1 + Arg2*Arg3 - vec4(0.5);\n");
                                        else
                                            ShadAppend("fColor = Arg0 + Arg1 - vec4(0.5);\n");
                                        break;
                                    case FPE_CR_INTERPOLATE:
                                        ShadAppend("fColor = Arg0*Arg2 + Arg1*(vec4(1.)-Arg2);\n");
                                        break;
                                    case FPE_CR_SUBTRACT:
                                        ShadAppend("fColor = Arg0 - Arg1;\n");
                                        break;
                                    case FPE_CR_MOD_ADD:
                                        ShadAppend("fColor = Arg0*Arg2 + Arg1;\n");
                                        break;
                                    case FPE_CR_MOD_ADD_SIGNED:
                                        ShadAppend("fColor = Arg0*Arg2 + Arg1 - vec4(0.5);\n");
                                        break;
                                    case FPE_CR_MOD_SUB:
                                        ShadAppend("fColor = Arg0*Arg2 - Arg1;\n");
                                        break;
                                }
                            } else {
                                switch(combine_rgb) {
                                    case FPE_CR_REPLACE:
                                        ShadAppend("fColor.rgb = Arg0.rgb;\n");
                                        break;
                                    case FPE_CR_MODULATE:
                                        ShadAppend("fColor.rgb = Arg0.rgb * Arg1.rgb;\n");
                                        break;
                                    case FPE_CR_ADD:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor.rgb = Arg0.rgb*Arg1.rgb + Arg2.rgb*Arg3.rgb;\n");
                                        else
                                            ShadAppend("fColor.rgb = Arg0.rgb + Arg1.rgb;\n");
                                        break;
                                    case FPE_CR_ADD_SIGNED:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor.rgb = Arg0.rgb*Arg1.rgb + Arg2.rgb*Arg3.rgb - vec3(0.5);\n");
                                        else
                                            ShadAppend("fColor.rgb = Arg0.rgb + Arg1.rgb - vec3(0.5);\n");
                                        break;
                                    case FPE_CR_INTERPOLATE:
                                        ShadAppend("fColor.rgb = Arg0.rgb*Arg2.rgb + Arg1.rgb*(vec3(1.)-Arg2.rgb);\n");
                                        break;
                                    case FPE_CR_SUBTRACT:
                                        ShadAppend("fColor.rgb = Arg0.rgb - Arg1.rgb;\n");
                                        break;
                                    case FPE_CR_DOT3_RGB:
                                        ShadAppend("fColor.rgb = vec3(4.*((Arg0.r-0.5)*(Arg1.r-0.5)+(Arg0.g-0.5)*(Arg1.g-0.5)+(Arg0.b-0.5)*(Arg1.b-0.5)));\n");
                                        break;
                                    case FPE_CR_DOT3_RGBA:
                                        ShadAppend("fColor = vec4(4.*((Arg0.r-0.5)*(Arg1.r-0.5)+(Arg0.g-0.5)*(Arg1.g-0.5)+(Arg0.b-0.5)*(Arg1.b-0.5)));\n");
                                        break;
                                    case FPE_CR_MOD_ADD:
                                        ShadAppend("fColor.rgb = Arg0.rgb*Arg2.rgb + Arg1.rgb;\n");
                                        break;
                                    case FPE_CR_MOD_ADD_SIGNED:
                                        ShadAppend("fColor.rgb = Arg0.rgb*Arg2.rgb + Arg1.rgb - vec3(0.5);\n");
                                        break;
                                    case FPE_CR_MOD_SUB:
                                        ShadAppend("fColor.rgb = Arg0.rgb*Arg2.rgb - Arg1.rgb;\n");
                                        break;
                                }
                                if(combine_rgb!=FPE_CR_DOT3_RGBA) 
                                switch(combine_alpha) {
                                    case FPE_CR_REPLACE:
                                        ShadAppend("fColor.a = Arg0.a;\n");
                                        break;
                                    case FPE_CR_MODULATE:
                                        ShadAppend("fColor.a = Arg0.a * Arg1.a;\n");
                                        break;
                                    case FPE_CR_ADD:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor.a = Arg0.a*Arg1.a + Arg2.a*Arg3.a;\n");
                                        else
                                            ShadAppend("fColor.a = Arg0.a + Arg1.a;\n");
                                        break;
                                    case FPE_CR_ADD_SIGNED:
                                        if(texenv==FPE_COMBINE4)
                                            ShadAppend("fColor.a = Arg0.a*Arg1.a + Arg2.a*Arg3.a - 0.5;\n");
                                        else
                                            ShadAppend("fColor.a = Arg0.a + Arg1.a - 0.5;\n");
                                        break;
                                    case FPE_CR_INTERPOLATE:
                                        ShadAppend("fColor.a = Arg0.a*Arg2.a + Arg1.a*(1.-Arg2.a);\n");
                                        break;
                                    case FPE_CR_SUBTRACT:
                                        ShadAppend("fColor.a = Arg0.a - Arg1.a;\n");
                                        break;
                                    case FPE_CR_MOD_ADD:
                                        ShadAppend("fColor.a = Arg0.a*Arg2.a + Arg1.a;\n");
                                        break;
                                    case FPE_CR_MOD_ADD_SIGNED:
                                        ShadAppend("fColor.a = Arg0.a*Arg2.a + Arg1.a - 0.5;\n");
                                        break;
                                    case FPE_CR_MOD_SUB:
                                        ShadAppend("fColor.a = Arg0.a*Arg2.a - Arg1.a;\n");
                                        break;
                                }
                            }
                            if((state->texenv[i].texrgbscale) && (state->texenv[i].texalphascale)) {
                                sprintf(buff, "fColor *= _mg_TexEnvRGBScale_%d;\n", i);
                                ShadAppend(buff);
                            } else {
                                if(state->texenv[i].texrgbscale) {
                                    sprintf(buff, "fColor.rgb *= _mg_TexEnvRGBScale_%d;\n", i);
                                    ShadAppend(buff);
                                }
                                if(state->texenv[i].texalphascale) {
                                    sprintf(buff, "fColor.a *= _mg_TexEnvAlphaScale_%d;\n", i);
                                    ShadAppend(buff);
                                }
                            }
                        }
                        break;
                }
                if(needclamp)
                    ShadAppend("fColor = clamp(fColor, 0., 1.);\n");
            }
        }
    }
    //*** Alpha Test
    if(alpha_test) {
        if(comments) {
            sprintf(buff, "// Alpha Test, fct=%X\n", alpha_func);
            ShadAppend(buff);
        }
        if(alpha_func==FPE_ALWAYS) {
            // nothing here...
        } else if (alpha_func==FPE_NEVER) {
            ShadAppend("discard;\n"); // Never pass...
        } else {
            // FPE_LESS FPE_EQUAL FPE_LEQUAL FPE_GREATER FPE_NOTEQUAL FPE_GEQUAL
            // but need to negate the operator
            const char* alpha_test_op[] = {">=","!=",">","<=","==","<"}; 
            sprintf(buff, "if (floor(fColor.a*255.) %s _mg_AlphaRef) discard;\n", alpha_test_op[alpha_func-FPE_LESS]);
            ShadAppend(buff);
        }
    }

    //*** Add secondary color
    if(light_separate || secondary) {
        if(comments) {
            sprintf(buff, "// Add Secondary color (%s %s)\n", light_separate?"light":"", secondary?"secondary":"");
            ShadAppend(buff);
        }
        sprintf(buff, "fColor.rgb += (%s).rgb;\n", twosided?"(gl_FrontFacing)?SecColor:SecBackColor":"SecColor");
        ShadAppend(buff);
        ShadAppend("fColor.rgb = clamp(fColor.rgb, 0., 1.);\n");
    }

    //*** Fog
    if(fog) {
        if(comments) {
            sprintf(buff, "// Fog On: mode=%X, source=%X\n", fogmode, fogsource);
            ShadAppend(buff);
        }
        #if 0   // vertex fog
        ShadAppend("fColor.rgb = mix(gl_Fog.color.rgb, fColor.rgb, FogF);\n");
        #else   // pixel fog
        char fogsrc[50];
        if(fogsource==FPE_FOG_SRC_COORD)
            strcpy(fogsrc, "FogSrc");
        else switch(fogdist) {
            case FPE_FOG_DIST_RADIAL: strcpy(fogsrc, "length(FogSrc)"); break;
            case FPE_FOG_DIST_PLANE: strcpy(fogsrc, "FogSrc"); break;
            default: strcpy(fogsrc, "abs(FogSrc)");
        }
        sprintf(buff, "%s float fog_c = %s;\n", fogp, fogsrc);
        ShadAppend(buff);
        switch(fogmode) {
            case FPE_FOG_EXP:
                sprintf(buff, "%s float FogF = clamp(exp(-gl_Fog.density * fog_c), 0., 1.);\n", fogp);
                break;
            case FPE_FOG_EXP2:
                sprintf(buff, "%s float FogF = clamp(exp(-(gl_Fog.density * fog_c)*(gl_Fog.density * fog_c)), 0., 1.);\n", fogp);
                break;
            case FPE_FOG_LINEAR:
                sprintf(buff, "%s float FogF = clamp((gl_Fog.end - fog_c) %s, 0., 1.);\n", fogp, "* gl_Fog.scale");
                break;
        }
        ShadAppend(buff);
        ShadAppend("fColor.rgb = mix(gl_Fog.color.rgb, fColor.rgb, FogF);\n");
        #endif
    }
    //Blend (probably not needed)
//    if(shaderblend) {
//        if(comments) {
//            sprintf(buff, "//Blend: src=%d/%d, dst=%d/%d, eq=%d/%d\n", state->blendsrcrgb, state->blendsrcalpha, state->blenddstrgb, state->blenddstalpha, state->blendeqrgb, state->blendeqalpha);
//            ShadAppend(buff);
//        }
//        const char* frgcolor = "fColor";
//        const char* dstcolor = "gl_LastFragColorARM";
//        for(int i=0; i<2; ++i) {
//            const char* blend = i?"dstblend":"srcblend";
//            int blendrgb= i?state->blenddstrgb:state->blendsrcrgb;
//            int blendalpha = i?state->blenddstalpha:state->blendsrcalpha;
//            if(blendrgb==blendalpha) {
//                int need_vec4 = 0;
//                switch(blendrgb) {
//                    case FPE_BLEND_ZERO:
//                        sprintf(buff, " %s = 0.0;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE:
//                        sprintf(buff, " %s = 1.0;\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = %s;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = vec4(1.0)-%s;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = %s;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = vec4(1.0)-%s;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA:
//                        sprintf(buff, " %s = %s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
//                        sprintf(buff, " %s = 1.0 - %s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_ALPHA:
//                        sprintf(buff, " %s = %s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_ALPHA:
//                        sprintf(buff, " %s = 1.0 - %s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_CONSTANT_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = _mg_BlendColor;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
//                        need_vec4 = 1;
//                        sprintf(buff, " %s = vec4(1.0)-_mg_BlendColor;\n", blend);
//                        break;
//                    case FPE_BLEND_CONSTANT_ALPHA:
//                        sprintf(buff, " %s = _mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
//                        sprintf(buff, " %s = 1.0 - _mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA_SATURATE:
//                        sprintf(buff, " %s = min(%s.a, 1.0-%s.a);\n", blend, frgcolor, dstcolor);
//                        break;
//                }
//                char buff2[100];
//                if(need_vec4)
//                    sprintf(buff2, "lowp vec4 %s;\n", blend);
//                else
//                    sprintf(buff2, "lowp float %s;\n", blend);
//                ShadAppend(buff2);
//                ShadAppend(buff);
//            } else {
//                sprintf(buff, "lowp vec4 %s;\n", blend);
//                ShadAppend(buff);
//                switch(blendrgb) {
//                    case FPE_BLEND_ZERO:
//                        sprintf(buff, " %s.rgb = vec3(0.0);\n", blend);
//                        break;
//                    case FPE_BLEND_ONE:
//                        sprintf(buff, " %s.rgb = vec3(1.0);\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_COLOR:
//                        sprintf(buff, " %s.rgb = %s.rgb;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_COLOR:
//                        sprintf(buff, " %s.rgb = vec3(1.0)-%s.rgb;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_COLOR:
//                        sprintf(buff, " %s.rgb = %s.rgb;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_COLOR:
//                        sprintf(buff, " %s.rgb = vec3(1.0)-%s.rgb;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(%s.a);\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(1.0 - %s.a);\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(%s.a);\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(1.0 - %s.a);\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_CONSTANT_COLOR:
//                        sprintf(buff, " %s.rgb = _mg_BlendColor.rgb;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
//                        sprintf(buff, " %s.rgb = vec3(1.0)-_mg_BlendColor.rgb;\n", blend);
//                        break;
//                    case FPE_BLEND_CONSTANT_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(_mg_BlendColor.a);\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
//                        sprintf(buff, " %s.rgb = vec3(1.0 - _mg_BlendColor.a);\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA_SATURATE:
//                        sprintf(buff, " %s.rgb = vec3(min(%s.a, 1.0-%s.a));\n", blend, frgcolor, dstcolor);
//                        break;
//                }
//                ShadAppend(buff);
//                switch(blendalpha) {
//                    case FPE_BLEND_ZERO:
//                        sprintf(buff, " %s.a = 0.0;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE:
//                        sprintf(buff, " %s.a = 1.0;\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_COLOR:
//                        sprintf(buff, " %s.a = %s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_COLOR:
//                        sprintf(buff, " %s.a = 1.0-%s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_COLOR:
//                        sprintf(buff, " %s.a = %s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_COLOR:
//                        sprintf(buff, " %s.a = 1.0-%s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA:
//                        sprintf(buff, " %s.a = %s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
//                        sprintf(buff, " %s.a = 1.0 - %s.a;\n", blend, frgcolor);
//                        break;
//                    case FPE_BLEND_DST_ALPHA:
//                        sprintf(buff, " %s.a = %s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_DST_ALPHA:
//                        sprintf(buff, " %s.a = 1.0 - %s.a;\n", blend, dstcolor);
//                        break;
//                    case FPE_BLEND_CONSTANT_COLOR:
//                        sprintf(buff, " %s.a = _mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
//                        sprintf(buff, " %s.a = 1.0-_mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_CONSTANT_ALPHA:
//                        sprintf(buff, " %s.a = _mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
//                        sprintf(buff, " %s.a = 1.0 - _mg_BlendColor.a;\n", blend);
//                        break;
//                    case FPE_BLEND_SRC_ALPHA_SATURATE:
//                        sprintf(buff, " %s.a = min(%s.a, 1.0-%s.a);\n", blend, frgcolor, dstcolor);
//                        break;
//                }
//                ShadAppend(buff);
//            }
//        }
//        if(state->blendeqrgb==state->blendeqalpha)
//        {
//            switch(state->blendeqrgb) {
//                case FPE_BLENDEQ_FUNC_ADD:
//                    sprintf(buff, "%s = srcblend*%s + dstblend*%s;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_SUBTRACT:
//                    sprintf(buff, "%s = srcblend*%s - dstblend*%s;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
//                    sprintf(buff, "%s = dstblend*%s - srcblend*%s;\n", frgcolor, dstcolor, frgcolor);
//                    break;
//                case FPE_BLENDEQ_MIN:
//                    sprintf(buff, "%s = min(%s ,%s);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_MAX:
//                    sprintf(buff, "%s = max(%s ,%s);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//            }
//            ShadAppend(buff);
//        } else {
//            switch(state->blendeqrgb) {
//                case FPE_BLENDEQ_FUNC_ADD:
//                    sprintf(buff, "%s.rgb = srcblend.rgb*%s.rgb + dstblend.rgb*%s.rgb;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_SUBTRACT:
//                    sprintf(buff, "%s.rgb = srcblend.rgb*%s.rgb - dstblend.rgb*%s.rgb;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
//                    sprintf(buff, "%s.rgb = dstblend.rgb*%s.rgb - srcblend.rgb*%s.rgb;\n", frgcolor, dstcolor, frgcolor);
//                    break;
//                case FPE_BLENDEQ_MIN:
//                    sprintf(buff, "%s.rgb = min(%s.rgb ,%s.rgb);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_MAX:
//                    sprintf(buff, "%s.rgb = max(%s.rgb ,%s.rgb);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//            }
//            ShadAppend(buff);
//            switch(state->blendeqalpha) {
//                case FPE_BLENDEQ_FUNC_ADD:
//                    sprintf(buff, "%s.a = srcblend.a*%s.a + dstblend.a*%s.a;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_SUBTRACT:
//                    sprintf(buff, "%s.a = srcblend.a*%s.a - dstblend.a*%s.a;\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
//                    sprintf(buff, "%s.a = dstblend.a*%s.a - srcblend.a*%s.a;\n", frgcolor, dstcolor, frgcolor);
//                    break;
//                case FPE_BLENDEQ_MIN:
//                    sprintf(buff, "%s.a = min(%s.a ,%s.a);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//                case FPE_BLENDEQ_MAX:
//                    sprintf(buff, "%s.a = max(%s.a ,%s.a);\n", frgcolor, frgcolor, dstcolor);
//                    break;
//            }
//            ShadAppend(buff);
//        }
//    }

    //done
    ShadAppend("gl_FragColor = fColor;\n");
    ShadAppend("}");

    LOG_D("FPE Shader: \n%s\n", shad)

    return (const char* const*)&shad;
}

const char* const* fpe_CustomVertexShader(const char* initial, fpe_state_t* state, int default_fragment)
{
    int planes = state->plane;
    char buff[1024];
    if(!shad_cap) shad_cap = 1024;
    if(!shad) shad = (char*)malloc(shad_cap);
    int headline = mg_getline_for(initial, "main");
    if(headline) --headline;

    strcpy(shad, "");
    ShadAppend(initial);

    int color = default_fragment?(strstr(initial, "_mg_Color")?0:1):0;   // need to add a simple color variant?
if(default_fragment) printf("fpe_CustomVertexShader(%p, %p, %d)\n%s\ncolor=%d\n", initial, state, default_fragment, initial, color);
    // add some uniform and varying
    if(planes) {
        for (int i=0; i< g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                sprintf(buff, "uniform highp vec4 _mg_ClipPlane_%d;\n", i);
                ShadAppend(buff);
                ++headline;
                sprintf(buff, "varying mediump float clippedvertex_%d;\n", i);
                ShadAppend(buff);
                ++headline;
            }
        }
    }
    if(color) {
        sprintf(buff, "attribute lowp vec4 _mg_Color;\n");
        ShadAppend(buff);
        ++headline;
        sprintf(buff, "varying lowp vec4 Color;\n");
        ShadAppend(buff);
        ++headline;
    }
    // wrap main if needed
    if(planes || color) {
        // wrap real main...
        shad = mg_inplace_replace(shad, &shad_cap, "main", "_mg_main");
    }

    // let's start
    if(strstr(shad, "_mg_main")) {
        ShadAppend("\nvoid main() {\n");
        if(color) {
            sprintf(buff, "Color = _mg_Color;\n");
        }
        ShadAppend("_mg_main();\n");
        if(planes) {
            int clipvertex = 0;
            if(strstr(shad, "mg_ClipVertex"))
                clipvertex = 1;
            for (int i=0; i<g_gles_caps.maxplanes; i++) {
                if((planes>>i)&1) {
                    sprintf(buff, "clippedvertex_%d = dot(%s, _mg_ClipPlane_%d);\n", i, clipvertex?"mg_ClipVertex":"gl_ModelViewMatrix * gl_Vertex", i);
                    ShadAppend(buff);
                }
            }
        }
        ShadAppend("}");
    }

    return (const char* const*)&shad;
}
const char* const* fpe_CustomFragmentShader(const char* initial, fpe_state_t* state)
{
    // the shader is unconverted yet!
    if(!shad_cap) shad_cap = 1024;
    if(!shad) shad = (char*)malloc(shad_cap);

    int planes = state->plane;
    int alpha_test = state->alphatest;
    int alpha_func = state->alphafunc;
    int shaderblend = state->blend_enable;
    char buff[1024];
    int headline = mg_getline_for(initial, "main");
    if(headline) --headline;

    strcpy(shad, "");
    if(shaderblend) {
        ShadAppend("#extension GL_ARM_shader_framebuffer_fetch : enable\n");
    }
    ShadAppend(initial);

    // add some varying
    if(planes) {
        for (int i=0; i<g_gles_caps.maxplanes; i++) {
            if((planes>>i)&1) {
                sprintf(buff, "varying mediump float clippedvertex_%d;\n", i);
                ShadAppend(buff);
            }
        }
    }
    if(shaderblend && (
        (state->blendsrcrgb>=FPE_BLEND_CONSTANT_COLOR && state->blendsrcrgb<=FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA)
     || (state->blenddstrgb>=FPE_BLEND_CONSTANT_COLOR && state->blenddstrgb<=FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA)
     || (state->blendsrcalpha>=FPE_BLEND_CONSTANT_COLOR && state->blendsrcalpha<=FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA)
     || (state->blenddstalpha>=FPE_BLEND_CONSTANT_COLOR && state->blenddstalpha<=FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA)
    )) {
        sprintf(buff, "uniform mediump vec4 _mg_BlendColor;\n");
        ShadAppend(buff);
    }
    int is_fragcolor = (strstr(shad, "gl_FragColor")!=NULL)?1:0;
    if(alpha_test || planes || shaderblend) {
        // wrap real main...
        shad = mg_inplace_replace(shad, &shad_cap, "main", "_mg_main");
        if(is_fragcolor) {
            int l_main = mg_getline_for(shad, mg_prev_str(shad, strstr(shad, "_mg_main"))) - 1;
            shad = mg_inplace_insert(mg_getline(shad, l_main), "lowp vec4 _mg_FragColor;\n", shad, &shad_cap);
            shad = mg_inplace_replace(shad, &shad_cap, "gl_FragColor", "_mg_FragColor");
        }
    }
    if(strstr(shad, "_mg_main")) {
        ShadAppend("void main() {\n");
        ShadAppend(" _mg_main();\n");
        //*** Plane Culling
        if(planes) {
            ShadAppend(" if((");
            int k=0;
            for (int i=0; i<g_gles_caps.maxplanes; i++) {
                if((planes>>i)&1) {
                    //sprintf(buff, "%smin(0., dot(clipvertex, gl_ClipPlane[%d]))", k?"+":"",  i);
                    sprintf(buff, "%smin(0., clippedvertex_%d)", k?"+":"",  i);
                    ShadAppend(buff);
                    k=1;
                }
            }
            ShadAppend(")<0.) discard;\n");
        }

        //*** Alpha Test
        if(alpha_test) {
            if(alpha_test && alpha_func>FPE_NEVER) {
                shad = mg_inplace_insert(mg_getline(shad, headline), mg_alphaRefSource, shad, &shad_cap);
                headline+=mg_countline(mg_alphaRefSource);
            } 
            if(comments) {
                sprintf(buff, "// Alpha Test, fct=%X\n", alpha_func);
                ShadAppend(buff);
            }
            if(alpha_func==FPE_ALWAYS) {
                // nothing here...
            } else if (alpha_func==FPE_NEVER) {
                ShadAppend("discard;\n"); // Never pass...
            } else {
                // FPE_LESS FPE_EQUAL FPE_LEQUAL FPE_GREATER FPE_NOTEQUAL FPE_GEQUAL
                // but need to negate the operator
                const char* alpha_test_op[] = {">=","!=",">","<=","==","<"}; 
                sprintf(buff, " if (floor(%s.a*255.) %s _mg_AlphaRef) discard;\n", is_fragcolor?"_mg_FragColor":"gl_FragData[0]", alpha_test_op[alpha_func-FPE_LESS]);
                ShadAppend(buff);
            }
        }

        //*** Blend in Shader
        if(shaderblend) {
            const char* frgcolor = is_fragcolor?"_mg_FragColor":"gl_FragData[0]";
            const char* dstcolor = "gl_LastFragColorARM";
            
            for(int i=0; i<2; ++i) {
                const char* blend = i?"dstblend":"srcblend";
                int blendrgb= i?state->blenddstrgb:state->blendsrcrgb;
                int blendalpha = i?state->blenddstalpha:state->blendsrcalpha;
                if(blendrgb==blendalpha) {
                    int need_vec4 = 0;
                    switch(blendrgb) {
                        case FPE_BLEND_ZERO:
                            sprintf(buff, " %s = 0.0;\n", blend);
                            break;
                        case FPE_BLEND_ONE:
                            sprintf(buff, " %s = 1.0;\n", blend);
                            break;
                        case FPE_BLEND_SRC_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = %s;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = vec4(1.0)-%s;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = %s;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = vec4(1.0)-%s;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_SRC_ALPHA:
                            sprintf(buff, " %s = %s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
                            sprintf(buff, " %s = 1.0 - %s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_ALPHA:
                            sprintf(buff, " %s = %s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_ALPHA:
                            sprintf(buff, " %s = 1.0 - %s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_CONSTANT_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = _mg_BlendColor;\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
                            need_vec4 = 1;
                            sprintf(buff, " %s = vec4(1.0)-_mg_BlendColor;\n", blend);
                            break;
                        case FPE_BLEND_CONSTANT_ALPHA:
                            sprintf(buff, " %s = _mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
                            sprintf(buff, " %s = 1.0 - _mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_SRC_ALPHA_SATURATE:
                            sprintf(buff, " %s = min(%s.a, 1.0-%s.a);\n", blend, frgcolor, dstcolor);
                            break;
                    }
                    char buff2[100];
                    if(need_vec4)
                        sprintf(buff2, "lowp vec4 %s;\n", blend);
                    else
                        sprintf(buff2, "lowp float %s;\n", blend);
                    ShadAppend(buff2);
                    ShadAppend(buff);
                } else {
                    sprintf(buff, "lowp vec4 %s;\n", blend);
                    ShadAppend(buff);
                    switch(blendrgb) {
                        case FPE_BLEND_ZERO:
                            sprintf(buff, " %s.rgb = vec3(0.0);\n", blend);
                            break;
                        case FPE_BLEND_ONE:
                            sprintf(buff, " %s.rgb = vec3(1.0);\n", blend);
                            break;
                        case FPE_BLEND_SRC_COLOR:
                            sprintf(buff, " %s.rgb = %s.rgb;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_COLOR:
                            sprintf(buff, " %s.rgb = vec3(1.0)-%s.rgb;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_COLOR:
                            sprintf(buff, " %s.rgb = %s.rgb;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_COLOR:
                            sprintf(buff, " %s.rgb = vec3(1.0)-%s.rgb;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_SRC_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(%s.a);\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(1.0 - %s.a);\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(%s.a);\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(1.0 - %s.a);\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_CONSTANT_COLOR:
                            sprintf(buff, " %s.rgb = _mg_BlendColor.rgb;\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
                            sprintf(buff, " %s.rgb = vec3(1.0)-_mg_BlendColor.rgb;\n", blend);
                            break;
                        case FPE_BLEND_CONSTANT_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(_mg_BlendColor.a);\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
                            sprintf(buff, " %s.rgb = vec3(1.0 - _mg_BlendColor.a);\n", blend);
                            break;
                        case FPE_BLEND_SRC_ALPHA_SATURATE:
                            sprintf(buff, " %s.rgb = vec3(min(%s.a, 1.0-%s.a));\n", blend, frgcolor, dstcolor);
                            break;
                    }
                    ShadAppend(buff);
                    switch(blendalpha) {
                        case FPE_BLEND_ZERO:
                            sprintf(buff, " %s.a = 0.0;\n", blend);
                            break;
                        case FPE_BLEND_ONE:
                            sprintf(buff, " %s.a = 1.0;\n", blend);
                            break;
                        case FPE_BLEND_SRC_COLOR:
                            sprintf(buff, " %s.a = %s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_COLOR:
                            sprintf(buff, " %s.a = 1.0-%s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_COLOR:
                            sprintf(buff, " %s.a = %s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_COLOR:
                            sprintf(buff, " %s.a = 1.0-%s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_SRC_ALPHA:
                            sprintf(buff, " %s.a = %s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_SRC_ALPHA:
                            sprintf(buff, " %s.a = 1.0 - %s.a;\n", blend, frgcolor);
                            break;
                        case FPE_BLEND_DST_ALPHA:
                            sprintf(buff, " %s.a = %s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_ONE_MINUS_DST_ALPHA:
                            sprintf(buff, " %s.a = 1.0 - %s.a;\n", blend, dstcolor);
                            break;
                        case FPE_BLEND_CONSTANT_COLOR:
                            sprintf(buff, " %s.a = _mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_COLOR:
                            sprintf(buff, " %s.a = 1.0-_mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_CONSTANT_ALPHA:
                            sprintf(buff, " %s.a = _mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_ONE_MINUS_CONSTANT_ALPHA:
                            sprintf(buff, " %s.a = 1.0 - _mg_BlendColor.a;\n", blend);
                            break;
                        case FPE_BLEND_SRC_ALPHA_SATURATE:
                            sprintf(buff, " %s.a = min(%s.a, 1.0-%s.a);\n", blend, frgcolor, dstcolor);
                            break;
                    }
                    ShadAppend(buff);
                }
            }
            if(state->blendeqrgb==state->blendeqalpha)
            {
                switch(state->blendeqrgb) {
                    case FPE_BLENDEQ_FUNC_ADD:
                        sprintf(buff, "%s = srcblend*%s + dstblend*%s;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_SUBTRACT:
                        sprintf(buff, "%s = srcblend*%s - dstblend*%s;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
                        sprintf(buff, "%s = dstblend*%s - srcblend*%s;\n", frgcolor, dstcolor, frgcolor);
                        break;
                    case FPE_BLENDEQ_MIN:
                        sprintf(buff, "%s = min(%s ,%s);\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_MAX:
                        sprintf(buff, "%s = max(%s ,%s);\n", frgcolor, frgcolor, dstcolor);
                        break;
                }
                ShadAppend(buff);
            } else {
                switch(state->blendeqrgb) {
                    case FPE_BLENDEQ_FUNC_ADD:
                        sprintf(buff, "%s.rgb = srcblend.rgb*%s.rgb + dstblend.rgb*%s.rgb;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_SUBTRACT:
                        sprintf(buff, "%s.rgb = srcblend.rgb*%s.rgb - dstblend.rgb*%s.rgb;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
                        sprintf(buff, "%s.rgb = dstblend.rgb*%s.rgb - srcblend.rgb*%s.rgb;\n", frgcolor, dstcolor, frgcolor);
                        break;
                    case FPE_BLENDEQ_MIN:
                        sprintf(buff, "%s.rgb = min(%s.rgb ,%s.rgb);\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_MAX:
                        sprintf(buff, "%s.rgb = max(%s.rgb ,%s.rgb);\n", frgcolor, frgcolor, dstcolor);
                        break;
                }
                ShadAppend(buff);
                switch(state->blendeqalpha) {
                    case FPE_BLENDEQ_FUNC_ADD:
                        sprintf(buff, "%s.a = srcblend.a*%s.a + dstblend.a*%s.a;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_SUBTRACT:
                        sprintf(buff, "%s.a = srcblend.a*%s.a - dstblend.a*%s.a;\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_FUNC_REVERSE_SUBTRACT:
                        sprintf(buff, "%s.a = dstblend.a*%s.a - srcblend.a*%s.a;\n", frgcolor, dstcolor, frgcolor);
                        break;
                    case FPE_BLENDEQ_MIN:
                        sprintf(buff, "%s.a = min(%s.a ,%s.a);\n", frgcolor, frgcolor, dstcolor);
                        break;
                    case FPE_BLENDEQ_MAX:
                        sprintf(buff, "%s.a = max(%s.a ,%s.a);\n", frgcolor, frgcolor, dstcolor);
                        break;
                }
                ShadAppend(buff);
            }
        }
        if((alpha_test || planes || shaderblend) && is_fragcolor)
            ShadAppend("gl_FragColor = _mg_FragColor;\n");

        ShadAppend("}");
    }

    return (const char* const*)&shad;
}

#ifdef mg_COMPILE_FOR_USE_IN_SHARED_LIB
void fpe_shader_reset_internals() {
	if(shad) {
		free(shad);
		shad=NULL;
	}
	shad_cap=0;
	comments=1;
}
#endif
