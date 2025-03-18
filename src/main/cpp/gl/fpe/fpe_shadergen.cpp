//
// Created by Swung 0x48 on 2025/3/18.
//

#include "fpe_shadergen.h"

const static std::string mg_shader_header =
        "#version 300 es"
        "// MobileGlues FPE Shader";
const static std::string mg_vs_header =
        "// ** Vertex Shader **";
const static std::string mg_fs_header =
        "// ** Fragment Shader **";

void add_vs_uniforms(const fixed_function_state_t& state, std::string& vs) {
}

void add_fs_uniforms(const fixed_function_state_t& state, std::string& ps) {

}

std::string fpe_shadergen::vertex_shader(const fixed_function_state_t& state) {
    std::string shader;
    shader += mg_shader_header;
    shader += mg_vs_header;


    return shader;
}

std::string fpe_shadergen::fragment_shader(const fixed_function_state_t& state) {
    std::string shader;

    shader += mg_shader_header;
    shader += mg_fs_header;

    return shader;
}
