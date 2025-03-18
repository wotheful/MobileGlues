//
// Created by Swung 0x48 on 2025/3/18.
//

#include "fpe_shadergen.h"

#define DEBUG 0

const static std::string mg_shader_header =
        "#version 300 es\n"
        "// MobileGlues FPE Shader\n";
const static std::string mg_vs_header =
        "// ** Vertex Shader **\n";
const static std::string mg_fs_header =
        "// ** Fragment Shader **\n";

std::string vp2name(GLenum vp, int index) {
    switch (vp) {
        case GL_VERTEX_ARRAY:
            return "Position";
        case GL_NORMAL_ARRAY:
            return "Normal";
        case GL_COLOR_ARRAY:
            return "Color";
        case GL_INDEX_ARRAY:
            return "Index";
        case GL_EDGE_FLAG_ARRAY:
            return "EdgeFlag";
        case GL_FOG_COORD_ARRAY:
            return "FogCoord";
        case GL_SECONDARY_COLOR_ARRAY:
            return "SecColor";
        case GL_TEXTURE_COORD_ARRAY: {
            int texidx = index - 7;
            return "UV" + std::to_string(texidx);
        }
    }
    LOG_E("ERROR: 1280")
    return "ERROR";
}

std::string type2str(GLenum type, int size) {
    if (size == 1) {
        switch (type) {
            case GL_UNSIGNED_BYTE:
            case GL_UNSIGNED_INT:
                return "uint";
            case GL_SHORT:
            case GL_INT:
                return "int";
            case GL_FLOAT:
                return "float";
            case GL_DOUBLE:
                return "double";
            default:
                return "ERROR";
        }
    } else {
        switch (type) {
            case GL_UNSIGNED_BYTE:
            case GL_UNSIGNED_INT:
                return "uvec" + std::to_string(size);
            case GL_SHORT:
            case GL_INT:
                return "ivec" + std::to_string(size);
            case GL_FLOAT:
                return "vec" + std::to_string(size);
            case GL_DOUBLE:
                return "dvec" + std::to_string(size);
            default:
                return "ERROR";
        }
    }
}

void add_vs_in(const fixed_function_state_t& state, std::string& vs) {
    auto& vpa = state.vertexpointer_array;
    for (int i = 0; i < VERTEX_POINTER_COUNT; ++i) {
        bool enabled = ((vpa.enabled_pointers >> i) & 1);

        if (enabled) {
            auto &vp = vpa.pointers[i];

            LOG_D("attrib #%d: type = 0x%x, size = %d, stride = %d, usage = 0x%x, ptr = 0x%x",
                  i, vp.type, vp.size, vp.stride, vp.usage)

            std::string name = vp2name(vp.usage, i);
            std::string type = type2str(vp.type, vp.size);

            vs += "in " + type + " " + name + ";\n";
        }
    }
}

void add_vs_out(const fixed_function_state_t& state, std::string& vs) {

}

void add_vs_uniforms(const fixed_function_state_t& state, std::string& fs) {

}

void add_fs_uniforms(const fixed_function_state_t& state, std::string& fs) {

}

std::string fpe_shadergen::vertex_shader(const fixed_function_state_t& state) {
    std::string shader;
    shader += mg_shader_header;
    shader += mg_vs_header;

    shader += "\n";
    add_vs_in(state, shader);
    shader += "\n";
    add_vs_uniforms(state, shader);
    shader += "\n";
    add_vs_out(state, shader);


    return shader;
}

std::string fpe_shadergen::fragment_shader(const fixed_function_state_t& state) {
    std::string shader;

    shader += mg_shader_header;
    shader += mg_fs_header;

    return shader;
}
