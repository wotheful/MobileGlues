//
// Created by Swung 0x48 on 2025/3/18.
//

#ifndef MOBILEGLUES_FPE_SHADERGEN_H
#define MOBILEGLUES_FPE_SHADERGEN_H

#include <string>
#include "types.h"

class fpe_shadergen {
    static std::string vertex_shader(const fixed_function_state_t& state);
    static std::string fragment_shader(const fixed_function_state_t& state);
};


#endif //MOBILEGLUES_FPE_SHADERGEN_H
