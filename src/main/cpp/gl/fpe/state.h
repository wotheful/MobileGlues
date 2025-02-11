//
// Created by Swung 0x48 on 2025/2/8.
//

#ifndef MOBILEGLUES_STATE_H
#define MOBILEGLUES_STATE_H

#include "../gl.h"
#include "../gles/loader.h"
#include "../gl/log.h"

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

GLAPI GLAPIENTRY void glEnable(GLenum cap);
GLAPI GLAPIENTRY void glDisable(GLenum cap);

#ifdef __cplusplus
}
#endif

#endif //MOBILEGLUES_STATE_H
