#ifndef _MOBILEGLUES_FPE_SHADER_H_
#define _MOBILEGLUES_FPE_SHADER_H_

#include "fpe_defines.h"

extern const char* fpeshader_signature;

const char* const* fpe_VertexShader(shaderconv_need_t* need, fpe_state_t *state);
const char* const* fpe_FragmentShader(shaderconv_need_t* need, fpe_state_t *state);

const char* const* fpe_CustomVertexShader(const char* initial, fpe_state_t* state, int default_fragment);
const char* const* fpe_CustomFragmentShader(const char* initial, fpe_state_t* state);

typedef struct {
    const char* glname;
    const char* name;
    const char* type;
    const char* prec;
    int attrib;
} builtin_attrib_t;

typedef enum {
    ARB_VERTEX = 0,
    //ARB_WEIGHT
    ARB_NORMAL = 2,
    ARB_COLOR,
    ARB_SECONDARY,
    ARB_FOGCOORD,
    // 6 and 7 are nothing specifics
    ARB_MULTITEXCOORD0 = 8,
    ARB_MULTITEXCOORD1,
    ARB_MULTITEXCOORD2,
    ARB_MULTITEXCOORD3,
    ARB_MULTITEXCOORD4,
    ARB_MULTITEXCOORD5,
    ARB_MULTITEXCOORD6,
    ARB_MULTITEXCOORD7,
    ARB_MULTITEXCOORD8,
    ARB_MULTITEXCOORD9,
    ARB_MULTITEXCOORD10,
    ARB_MULTITEXCOORD11,
    ARB_MULTITEXCOORD12,
    ARB_MULTITEXCOORD13,
    ARB_MULTITEXCOORD14,
    ARB_MULTITEXCOORD15,
    //ARB_POINTSIZE,   //this one is supported by GLES hardware
    ARB_MAX
} reserved_attrib_t;

// This map correspond tries to fit as much as possible in 8 VA
typedef enum {
    COMP_VERTEX = 0,
    COMP_FOGCOORD,
    COMP_NORMAL,
    COMP_COLOR,
    COMP_SECONDARY,
    COMP_MULTITEXCOORD0,
    COMP_MULTITEXCOORD1,
    COMP_MULTITEXCOORD2,
    COMP_MULTITEXCOORD3,
    COMP_MULTITEXCOORD4,
    COMP_MULTITEXCOORD5,
    COMP_MULTITEXCOORD6,
    COMP_MULTITEXCOORD7,
    COMP_MULTITEXCOORD8,
    COMP_MULTITEXCOORD9,
    COMP_MULTITEXCOORD10,
    COMP_MULTITEXCOORD11,
    COMP_MULTITEXCOORD12,
    COMP_MULTITEXCOORD13,
    COMP_MULTITEXCOORD14,
    COMP_MULTITEXCOORD15,
    //COMP_WEIGHT
    //COMP_POINTSIZE,   //this one is supported by GLES hardware
    COMP_MAX
} compressed_attrib_t;


typedef enum {
    MAT_MV = 0,
    MAT_MV_I,
    MAT_MV_T,
    MAT_MV_IT,
    MAT_P,
    MAT_P_I,
    MAT_P_T,
    MAT_P_IT,
    MAT_MVP,
    MAT_MVP_I,
    MAT_MVP_T,
    MAT_MVP_IT,
    MAT_T0,
    MAT_T0_I,
    MAT_T0_T,
    MAT_T0_IT,
    MAT_T1,
    MAT_T1_I,
    MAT_T1_T,
    MAT_T1_IT,
    MAT_T2,
    MAT_T2_I,
    MAT_T2_T,
    MAT_T2_IT,
    MAT_T3,
    MAT_T3_I,
    MAT_T3_T,
    MAT_T3_IT,
    MAT_T4,
    MAT_T4_I,
    MAT_T4_T,
    MAT_T4_IT,
    MAT_T5,
    MAT_T5_I,
    MAT_T5_T,
    MAT_T5_IT,
    MAT_T6,
    MAT_T6_I,
    MAT_T6_T,
    MAT_T6_IT,
    MAT_T7,
    MAT_T7_I,
    MAT_T7_T,
    MAT_T7_IT,
    MAT_T8,
    MAT_T8_I,
    MAT_T8_T,
    MAT_T8_IT,
    MAT_T9,
    MAT_T9_I,
    MAT_T9_T,
    MAT_T9_IT,
    MAT_T10,
    MAT_T10_I,
    MAT_T10_T,
    MAT_T10_IT,
    MAT_T11,
    MAT_T11_I,
    MAT_T11_T,
    MAT_T11_IT,
    MAT_T12,
    MAT_T12_I,
    MAT_T12_T,
    MAT_T12_IT,
    MAT_T13,
    MAT_T13_I,
    MAT_T13_T,
    MAT_T13_IT,
    MAT_T14,
    MAT_T14_I,
    MAT_T14_T,
    MAT_T14_IT,
    MAT_T15,
    MAT_T15_I,
    MAT_T15_T,
    MAT_T15_IT,
    MAT_N,
    MAT_MAX
} reserved_matrix_t;

const builtin_attrib_t builtin_attrib[] = {
        {"gl_Vertex", "_mg_Vertex", "vec4", "highp", ARB_VERTEX},
        {"gl_Color", "_mg_Color", "vec4", "lowp", ARB_COLOR},
        {"gl_MultiTexCoord0", "_mg_MultiTexCoord0", "vec4", "highp", ARB_MULTITEXCOORD0},
        {"gl_MultiTexCoord1", "_mg_MultiTexCoord1", "vec4", "highp", ARB_MULTITEXCOORD1},
        {"gl_MultiTexCoord2", "_mg_MultiTexCoord2", "vec4", "highp", ARB_MULTITEXCOORD2},
        {"gl_MultiTexCoord3", "_mg_MultiTexCoord3", "vec4", "highp", ARB_MULTITEXCOORD3},
        {"gl_MultiTexCoord4", "_mg_MultiTexCoord4", "vec4", "highp", ARB_MULTITEXCOORD4},
        {"gl_MultiTexCoord5", "_mg_MultiTexCoord5", "vec4", "highp", ARB_MULTITEXCOORD5},
        {"gl_MultiTexCoord6", "_mg_MultiTexCoord6", "vec4", "highp", ARB_MULTITEXCOORD6},
        {"gl_MultiTexCoord7", "_mg_MultiTexCoord7", "vec4", "highp", ARB_MULTITEXCOORD7},
        {"gl_MultiTexCoord8", "_mg_MultiTexCoord8", "vec4", "highp", ARB_MULTITEXCOORD8},
        {"gl_MultiTexCoord9", "_mg_MultiTexCoord9", "vec4", "highp", ARB_MULTITEXCOORD9},
        {"gl_MultiTexCoord10", "_mg_MultiTexCoord10", "vec4", "highp", ARB_MULTITEXCOORD10},
        {"gl_MultiTexCoord11", "_mg_MultiTexCoord11", "vec4", "highp", ARB_MULTITEXCOORD11},
        {"gl_MultiTexCoord12", "_mg_MultiTexCoord12", "vec4", "highp", ARB_MULTITEXCOORD12},
        {"gl_MultiTexCoord13", "_mg_MultiTexCoord13", "vec4", "highp", ARB_MULTITEXCOORD13},
        {"gl_MultiTexCoord14", "_mg_MultiTexCoord14", "vec4", "highp", ARB_MULTITEXCOORD14},
        {"gl_MultiTexCoord15", "_mg_MultiTexCoord15", "vec4", "highp", ARB_MULTITEXCOORD15},
        {"gl_SecondaryColor", "_mg_SecondaryColor", "vec4", "lowp", ARB_SECONDARY},
        {"gl_Normal", "_mg_Normal", "vec3", "highp", ARB_NORMAL},
        {"gl_FogCoord", "_mg_FogCoord", "float", "highp", ARB_FOGCOORD}
};

const builtin_attrib_t builtin_attrib_compressed[] = {
        {"gl_Vertex", "_mg_Vertex", "vec4", "highp", COMP_VERTEX},
        {"gl_Color", "_mg_Color", "vec4", "lowp", COMP_COLOR},
        {"gl_MultiTexCoord0", "_mg_MultiTexCoord0", "vec4", "highp", COMP_MULTITEXCOORD0},
        {"gl_MultiTexCoord1", "_mg_MultiTexCoord1", "vec4", "highp", COMP_MULTITEXCOORD1},
        {"gl_MultiTexCoord2", "_mg_MultiTexCoord2", "vec4", "highp", COMP_MULTITEXCOORD2},
        {"gl_MultiTexCoord3", "_mg_MultiTexCoord3", "vec4", "highp", COMP_MULTITEXCOORD3},
        {"gl_MultiTexCoord4", "_mg_MultiTexCoord4", "vec4", "highp", COMP_MULTITEXCOORD4},
        {"gl_MultiTexCoord5", "_mg_MultiTexCoord5", "vec4", "highp", COMP_MULTITEXCOORD5},
        {"gl_MultiTexCoord6", "_mg_MultiTexCoord6", "vec4", "highp", COMP_MULTITEXCOORD6},
        {"gl_MultiTexCoord7", "_mg_MultiTexCoord7", "vec4", "highp", COMP_MULTITEXCOORD7},
        {"gl_MultiTexCoord8", "_mg_MultiTexCoord8", "vec4", "highp", COMP_MULTITEXCOORD8},
        {"gl_MultiTexCoord9", "_mg_MultiTexCoord9", "vec4", "highp", COMP_MULTITEXCOORD9},
        {"gl_MultiTexCoord10", "_mg_MultiTexCoord10", "vec4", "highp", COMP_MULTITEXCOORD10},
        {"gl_MultiTexCoord11", "_mg_MultiTexCoord11", "vec4", "highp", COMP_MULTITEXCOORD11},
        {"gl_MultiTexCoord12", "_mg_MultiTexCoord12", "vec4", "highp", COMP_MULTITEXCOORD12},
        {"gl_MultiTexCoord13", "_mg_MultiTexCoord13", "vec4", "highp", COMP_MULTITEXCOORD13},
        {"gl_MultiTexCoord14", "_mg_MultiTexCoord14", "vec4", "highp", COMP_MULTITEXCOORD14},
        {"gl_MultiTexCoord15", "_mg_MultiTexCoord15", "vec4", "highp", COMP_MULTITEXCOORD15},
        {"gl_SecondaryColor", "_mg_SecondaryColor", "vec4", "lowp", COMP_SECONDARY},
        {"gl_Normal", "_mg_Normal", "vec3", "highp", COMP_NORMAL},
        {"gl_FogCoord", "_mg_FogCoord", "float", "highp", COMP_FOGCOORD}
};

typedef struct {
    const char* glname;
    const char* name;
    const char* type;
    int   texarray;
    reserved_matrix_t matrix;
} builtin_matrix_t;

const builtin_matrix_t builtin_matrix[] = {
        {"gl_ModelViewMatrixInverseTranspose", "_mg_ITModelViewMatrix", "mat4", 0, MAT_MV_IT},
        {"gl_ModelViewMatrixInverse", "_mg_IModelViewMatrix", "mat4", 0, MAT_MV_I},
        {"gl_ModelViewMatrixTranspose", "_mg_TModelViewMatrix", "mat4", 0, MAT_MV_T},
        {"gl_ModelViewMatrix", "_mg_ModelViewMatrix", "mat4", 0, MAT_MV},
        {"gl_ProjectionMatrixInverseTranspose", "_mg_ITProjectionMatrix", "mat4", 0, MAT_P_IT},
        {"gl_ProjectionMatrixInverse", "_mg_IProjectionMatrix", "mat4", 0, MAT_P_I},
        {"gl_ProjectionMatrixTranspose", "_mg_TProjectionMatrix", "mat4", 0, MAT_P_T},
        {"gl_ProjectionMatrix", "_mg_ProjectionMatrix", "mat4", 0, MAT_P},
        {"gl_ModelViewProjectionMatrixInverseTranspose", "_mg_ITModelViewProjectionMatrix", "mat4", 0, MAT_MVP_IT},
        {"gl_ModelViewProjectionMatrixInverse", "_mg_IModelViewProjectionMatrix", "mat4", 0, MAT_MVP_I},
        {"gl_ModelViewProjectionMatrixTranspose", "_mg_TModelViewProjectionMatrix", "mat4", 0, MAT_MVP_T},
        {"gl_ModelViewProjectionMatrix", "_mg_ModelViewProjectionMatrix", "mat4", 0, MAT_MVP},
        // non standard version to avoid useless array of Matrix Uniform (in case the compiler as issue optimising this)
        {"gl_TextureMatrix_0", "_mg_TextureMatrix_0", "mat4", 0, MAT_T0},
        {"gl_TextureMatrix_1", "_mg_TextureMatrix_1", "mat4", 0, MAT_T1},
        {"gl_TextureMatrix_2", "_mg_TextureMatrix_2", "mat4", 0, MAT_T2},
        {"gl_TextureMatrix_3", "_mg_TextureMatrix_3", "mat4", 0, MAT_T3},
        {"gl_TextureMatrix_4", "_mg_TextureMatrix_4", "mat4", 0, MAT_T4},
        {"gl_TextureMatrix_5", "_mg_TextureMatrix_5", "mat4", 0, MAT_T5},
        {"gl_TextureMatrix_6", "_mg_TextureMatrix_6", "mat4", 0, MAT_T6},
        {"gl_TextureMatrix_7", "_mg_TextureMatrix_7", "mat4", 0, MAT_T7},
        {"gl_TextureMatrix_8", "_mg_TextureMatrix_8", "mat4", 0, MAT_T8},
        {"gl_TextureMatrix_9", "_mg_TextureMatrix_9", "mat4", 0, MAT_T9},
        {"gl_TextureMatrix_10", "_mg_TextureMatrix_10", "mat4", 0, MAT_T10},
        {"gl_TextureMatrix_11", "_mg_TextureMatrix_11", "mat4", 0, MAT_T11},
        {"gl_TextureMatrix_12", "_mg_TextureMatrix_12", "mat4", 0, MAT_T12},
        {"gl_TextureMatrix_13", "_mg_TextureMatrix_13", "mat4", 0, MAT_T13},
        {"gl_TextureMatrix_14", "_mg_TextureMatrix_14", "mat4", 0, MAT_T14},
        {"gl_TextureMatrix_15", "_mg_TextureMatrix_15", "mat4", 0, MAT_T15},
        // regular texture matrix
        {"gl_TextureMatrixInverseTranspose", "_mg_ITTextureMatrix", "mat4", 1, MAT_T0_IT},
        {"gl_TextureMatrixInverse", "_mg_ITextureMatrix", "mat4", 1, MAT_T0_I},
        {"gl_TextureMatrixTranspose", "_mg_TTextureMatrix", "mat4", 1, MAT_T0_T},
        {"gl_TextureMatrix", "_mg_TextureMatrix", "mat4", 1, MAT_T0},
        {"gl_NormalMatrix", "_mg_NormalMatrix", "mat3", 0, MAT_N}
};

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
static const char* mg_MaxLightsSource =
        "#define _mg_MaxLights " STR(MAX_LIGHT) "\n";
static const char* mg_MaxClipPlanesSource =
        "#define _mg_MaxClipPlanes " STR(MAX_CLIP_PLANES) "\n";
static const char* mg_MaxTextureUnitsSource =
        "#define _mg_MaxTextureUnits " STR(MAX_TEX) "\n";
static const char* mg_MaxTextureCoordsSource =
        "#define _mg_MaxTextureCoords " STR(MAX_TEX) "\n";
#undef STR
#undef STR_HELPER

static const char* mg_LightSourceParametersSource =
        "struct gl_LightSourceParameters\n"
        "{\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "   vec4 position;\n"
        "   vec4 halfVector;\n"   //halfVector = normalize(normalize(position) + vec3(0,0,1) if vbs==FALSE)
        "   vec3 spotDirection;\n"
        "   float spotExponent;\n"
        "   float spotCutoff;\n"
        "   float spotCosCutoff;\n"
        "   float constantAttenuation;\n"
        "   float linearAttenuation;\n"
        "   float quadraticAttenuation;\n"
        "};\n"
        "uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];\n";

static const char* mg_LightModelParametersSource =
        "struct gl_LightModelParameters {\n"
        "  vec4 ambient;\n"
        "};\n"
        "uniform gl_LightModelParameters gl_LightModel;\n";

static const char* mg_MaterialParametersSource =
        "struct gl_MaterialParameters\n"
        "{\n"
        "   vec4 emission;\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "   float shininess;\n"
        "};\n"
        "uniform gl_MaterialParameters gl_FrontMaterial;\n"
        "uniform gl_MaterialParameters gl_BackMaterial;\n";

static const char* mg_LightModelProductsSource =
        "struct gl_LightModelProducts\n"
        "{\n"
        "   vec4 sceneColor;\n"
        "};\n"
        "uniform gl_LightModelProducts gl_FrontLightModelProduct;\n"
        "uniform gl_LightModelProducts gl_BackLightModelProduct;\n";

static const char* mg_LightProductsSource =
        "struct gl_LightProducts\n"
        "{\n"
        "   vec4 ambient;\n"
        "   vec4 diffuse;\n"
        "   vec4 specular;\n"
        "};\n"
        "uniform gl_LightProducts gl_FrontLightProduct[gl_MaxLights];\n"
        "uniform gl_LightProducts gl_BackLightProduct[gl_MaxLights];\n";

static const char* mg_PointSpriteSource =
        "struct gl_PointParameters\n"
        "{\n"
        "   float size;\n"
        "   float sizeMin;\n"
        "   float sizeMax;\n"
        "   float fadeThresholdSize;\n"
        "   float distanceConstantAttenuation;\n"
        "   float distanceLinearAttenuation;\n"
        "   float distanceQuadraticAttenuation;\n"
        "};\n"
        "uniform gl_PointParameters gl_Point;\n";

static const char* mg_FogParametersSource =
        "struct gl_FogParameters {\n"
        "    lowp vec4 color;\n"
        "    mediump float density;\n"
        "    mediump float start;\n"
        "    mediump float end;\n"
        "    mediump float scale;\n"   // Derived:   1.0 / (end - start)
        "};\n"
        "uniform gl_FogParameters gl_Fog;\n";
static const char* mg_FogParametersSourceHighp =
        "struct gl_FogParameters {\n"
        "    lowp vec4 color;\n"
        "    mediump float density;\n"
        "    highp   float start;\n"
        "    highp   float end;\n"
        "    highp   float scale;\n"   // Derived:   1.0 / (end - start)
        "};\n"
        "uniform gl_FogParameters gl_Fog;\n";

static const char* mg_texenvcolorSource =
        "uniform vec4 gl_TextureEnvColor[gl_MaxTextureUnits];\n";

static const char* mg_texgeneyeSource[4] = {
        "uniform vec4 gl_EyePlaneS[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneT[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneR[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_EyePlaneQ[gl_MaxTextureCoords];\n" };

static const char* mg_texgenobjSource[4] = {
        "uniform vec4 gl_ObjectPlaneS[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneT[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneR[gl_MaxTextureCoords];\n",
        "uniform vec4 gl_ObjectPlaneQ[gl_MaxTextureCoords];\n" };

static const char* mg_clipplanesSource =
        "uniform vec4  gl_ClipPlane[gl_MaxClipPlanes];\n";

static const char* mg_normalscaleSource =
        "uniform float gl_NormalScale;\n";

static const char* mg_instanceID =
        "#define GL_ARB_draw_instanced 1\n"
        "uniform int _mg_InstanceID;\n";

static const char* mg_frontColorSource =
        "varying lowp vec4 _mg_FrontColor;\n";

static const char* mg_backColorSource =
        "varying lowp vec4 _mg_BackColor;\n";

static const char* mg_frontSecondaryColorSource =
        "varying lowp vec4 _mg_FrontSecondaryColor;\n";

static const char* mg_backSecondaryColorSource =
        "varying lowp vec4 _mg_BackSecondaryColor;\n";

static const char* mg_texcoordSource =
        "varying mediump vec4 _mg_TexCoord[%d];\n";

static const char* mg_texcoordSourceAlt =
        "varying mediump vec4 _mg_TexCoord_%d;\n";

static const char* mg_fogcoordSource =
        "varying mediump float _mg_FogFragCoord;\n";

static const char* mg_ftransformSource =
        "\n"
        "highp vec4 ftransform() {\n"
        " return gl_ModelViewProjectionMatrix * gl_Vertex;\n"
        "}\n";

static const char* mg_ClipVertex =
        "vec4 mg_ClipVertex;\n";

static const char* mg_ClipVertexSource =
        "mg_ClipVertex";

static const char* mg_ClipVertex_clip =
        "\nif(any(lessThanEqual(mg_ClipVertex.xyz, vec3(-mg_ClipVertex.w)))"
        " || any(greaterThanEqual(mg_ClipVertex.xyz, vec3(mg_ClipVertex.w)))) discard;\n";

static const char* gl_TexCoordSource = "gl_TexCoord[";

static const char* gl_TexMatrixSources[] = {
        "gl_TextureMatrixInverseTranspose[",
        "gl_TextureMatrixInverse[",
        "gl_TextureMatrixTranspose[",
        "gl_TextureMatrix["
};

static const char* GLESHeader[] = {
        "#version 100\n%sprecision %s float;\nprecision %s int;\n",
        "#version 120\n%sprecision %s float;\nprecision %s int;\n",
        "#version 310 es\n#define attribute in\n#define varying out\n%sprecision %s float;\nprecision %s int;\n",
        "#version 300 es\n#define attribute in\n#define varying out\n%sprecision %s float;\nprecision %s int;\n"
};

static const char* mg_transpose =
        "mat2 mg_transpose(mat2 m) {\n"
        " return mat2(m[0][0], m[1][0],\n"
        "             m[0][1], m[1][1]);\n"
        "}\n"
        "mat3 mg_transpose(mat3 m) {\n"
        " return mat3(m[0][0], m[1][0], m[2][0],\n"
        "             m[0][1], m[1][1], m[2][1],\n"
        "             m[0][2], m[1][2], m[2][2]);\n"
        "}\n"
        "mat4 mg_transpose(mat4 m) {\n"
        " return mat4(m[0][0], m[1][0], m[2][0], m[3][0],\n"
        "             m[0][1], m[1][1], m[2][1], m[3][1],\n"
        "             m[0][2], m[1][2], m[2][2], m[3][2],\n"
        "             m[0][3], m[1][3], m[2][3], m[3][3]);\n"
        "}\n";

static const char* HackAltPow =
        "float pow(float f, int a) {\n"
        " return pow(f, float(a));\n"
        "}\n";
static const char* HackAltMax =
        "float max(float a, int b) {\n"
        " return max(a, float(b));\n"
        "}\n"
        "float max(int a, float b) {\n"
        " return max(float(a), b);\n"
        "}\n";
static const char* HackAltMin =
        "float min(float a, int b) {\n"
        " return min(a, float(b));\n"
        "}\n"
        "float min(int a, float b) {\n"
        " return min(float(a), b);\n"
        "}\n";
static const char* HackAltClamp =
        "float clamp(float f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "float clamp(float f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "float clamp(float f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec2 clamp(vec2 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec2 clamp(vec2 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec2 clamp(vec2 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec3 clamp(vec3 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec3 clamp(vec3 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec3 clamp(vec3 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n"
        "vec4 clamp(vec4 f, int a, int b) {\n"
        " return clamp(f, float(a), float(b));\n"
        "}\n"
        "vec4 clamp(vec4 f, float a, int b) {\n"
        " return clamp(f, a, float(b));\n"
        "}\n"
        "vec4 clamp(vec4 f, int a, float b) {\n"
        " return clamp(f, float(a), b);\n"
        "}\n";


static const char* HackAltMod =
        "float mod(float f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec2 mod(vec2 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec3 mod(vec3 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n"
        "vec4 mod(vec4 f, int a) {\n"
        " return mod(f, float(a));\n"
        "}\n";

static const char* texture2DLodAlt =
        "vec4 _mg_texture2DLod(sampler2D sampler, vec2 coord, float lod) {\n"
        " return texture2D(sampler, coord);\n"
        "}\n";

static const char* texture2DProjLodAlt =
        "vec4 _mg_texture2DProjLod(sampler2D sampler, vec3 coord, float lod) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n"
        "vec4 _mg_texture2DProjLod(sampler2D sampler, vec4 coord, float lod) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n";
static const char* textureCubeLodAlt =
        "vec4 _mg_textureCubeLod(samplerCube sampler, vec3 coord, float lod) {\n"
        " return textureCube(sampler, coord);\n"
        "}\n";

static const char* texture2DGradAlt =
        "vec4 _mg_texture2DGrad(sampler2D sampler, vec2 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2D(sampler, coord);\n"
        "}\n";

static const char* texture2DProjGradAlt =
        "vec4 _mg_texture2DProjGrad(sampler2D sampler, vec3 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n"
        "vec4 _mg_texture2DProjGrad(sampler2D sampler, vec4 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return texture2DProj(sampler, coord);\n"
        "}\n";
static const char* textureCubeGradAlt =
        "vec4 _mg_textureCubeGrad(samplerCube sampler, vec3 coord, vec2 dPdx, vec2 dPdy) {\n"
        " return textureCube(sampler, coord);\n"
        "}\n";


static const char* useEXTDrawBuffers =
        "#extension GL_EXT_draw_buffers : enable\n";

static const char* gl_ProgramEnv  = "gl_ProgramEnv";
static const char* gl_ProgramLocal= "gl_ProgramLocal";

static const char* gl_Samplers1D = "gl_Sampler1D_";
static const char* gl_Samplers2D = "gl_Sampler2D_";
static const char* gl_Samplers3D = "gl_Sampler3D_";
static const char* gl_SamplersCube = "gl_SamplerCube_";
static const char* mg_Samplers1D = "_mg_Sampler1D_";
static const char* mg_Samplers2D = "_mg_Sampler2D_";
static const char* mg_Samplers3D = "_mg_Sampler3D_";
static const char* mg_SamplersCube = "_mg_SamplerCube_";
static const char* mg_Samplers1D_uniform = "uniform sampler2D _mg_Sampler1D_";
static const char* mg_Samplers2D_uniform = "uniform sampler2D _mg_Sampler2D_";
static const char* mg_Samplers3D_uniform = "uniform sampler2D _mg_Sampler3D_";
static const char* mg_SamplersCube_uniform = "uniform samplerCube _mg_SamplerCube_";

static const char* gl_VertexAttrib = "gl_VertexAttrib_";
static const char* mg_VertexAttrib = "_mg_VertexAttrib_";


#endif // _MOBILEGLUES_FPE_SHADER_H_
