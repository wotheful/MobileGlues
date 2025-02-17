//
// Created by hanji on 2025/2/9.
//

#include "settings.h"
#include "config.h"
#include "../gl/log.h"
#include "../gl/envvars.h"
#include "gpu_utils.h"

#define DEBUG 0

struct global_settings_t global_settings;

void init_settings() {
    int success = initialized;
    if (!success) {
        success = config_refresh();
        if (!success) {
            LOG_V("Failed to load config. Use default config.")
        }
    }

    int enableANGLE = success ? config_get_int("enableANGLE") : 0;
    int enableNoError = success ? config_get_int("enableNoError") : 0;
    int enableExtGL43 = success ? config_get_int("enableExtGL43") : 0;
    int enableExtComputeShader = success ? config_get_int("enableExtComputeShader") : 0;
    int maxGlslCacheSize = success ? config_get_int("maxGlslCacheSize") * 1024 * 1024 : 0;

    if (enableANGLE < 0 || enableANGLE > 3)
        enableANGLE = 0;
    if (enableNoError < 0 || enableNoError > 3)
        enableNoError = 0;
    if (enableExtGL43 < 0 || enableExtGL43 > 1)
        enableExtGL43 = 0;
    if (enableExtComputeShader < 0 || enableExtComputeShader > 1)
        enableExtComputeShader = 0;

    // 1205
    int fclVersion = 0;
    GetEnvVarInt("FCL_VERSION_CODE", &fclVersion, 0);
    // 140000
    int zlVersion = 0;
    GetEnvVarInt("ZALITH_VERSION_CODE", &zlVersion, 0);
    // unknown
    int pgwVersion = 0;
    GetEnvVarInt("PGW_VERSION_CODE", &pgwVersion, 0);

    if (fclVersion == 0 && zlVersion == 0 && pgwVersion == 0) {
        LOG_V("Unsupported launcher detected, force using default config.")
        enableANGLE = 0;
        enableNoError = 0;
        enableExtGL43 = 0;
        enableExtComputeShader = 0;
        maxGlslCacheSize = 0;
    }

    const char* gpu = getGPUInfo();
    LOG_D("GPU: %s", gpu);

    if (enableANGLE == 1) {
        global_settings.angle = (isAdreno740(gpu) || !hasVulkan13()) ? 0 : 1;
    } else if (enableANGLE == 2 || enableANGLE == 3) {
        global_settings.angle = enableANGLE - 2;
    } else {
        int is830 = isAdreno830(gpu);
        LOG_D("Is Adreno 830? = %s", is830 ? "true" : "false")
        global_settings.angle = is830 ? 1 : 0;
    }
    if (global_settings.angle) {
        setenv("LIBGL_GLES", "libGLESv2_angle.so", 1);
        setenv("LIBGL_EGL", "libEGL_angle.so", 1);
    }

    if (enableNoError == 1 || enableNoError == 2 || enableNoError == 3) {
        global_settings.ignore_error = enableNoError - 1;
    } else {
        global_settings.ignore_error = 0;
    }

    global_settings.ext_gl43 = enableExtGL43;

    global_settings.ext_compute_shader = enableExtComputeShader;
    
    global_settings.maxGlslCacheSize = maxGlslCacheSize;
}