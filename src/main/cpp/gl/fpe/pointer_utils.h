//
// Created by hanji on 2025/3/29.
//

#ifndef MOBILEGLUES_PLUGIN_POINTER_UTILS_H
#define MOBILEGLUES_PLUGIN_POINTER_UTILS_H

#include "../gl.h"
#include "../log.h"

#define DEBUG 0

class PointerUtils {
public:
    static int type_to_bytes(GLenum type);
    static int pname_to_count(GLenum pname);
    static void* copy_pointer(const void* src, size_t size, int count);
};

#endif //MOBILEGLUES_PLUGIN_POINTER_UTILS_H
