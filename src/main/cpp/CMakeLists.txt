cmake_minimum_required(VERSION 3.22.1)

project("mobileglues")

enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fvisibility=hidden -funwind-tables -g -D_THREAD_SAFE -fPIC -stdlib=libc++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w -g -std=gnu99 -funwind-tables -O3 -fvisibility=hidden")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")

set(CMAKE_ANDROID_STL_TYPE c++_static)

#set(CMAKE_BUILD_TYPE Release)

set(PROFILING OFF)


find_library(GLSLANG_LIB glslang PATHS ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/)

add_library(${CMAKE_PROJECT_NAME} SHARED
        init.cpp
        main.cpp
        gl/gl_stub.cpp
        gl/gl_native.cpp
        gl/gl.cpp
        gl/envvars.cpp
        gl/log.cpp
        gl/program.cpp
        gl/shader.cpp
        gl/framebuffer.cpp
        gl/texture.cpp
        gl/drawing.cpp
        gl/mg.cpp
        gl/buffer.cpp
        gl/getter.cpp
        gl/pixel.cpp
        gl/glsl/glsl_for_es.cpp
        gl/glsl/cache.cpp
        gl/glsl/preConvertedGlsl.cpp
        gl/vertexattrib.cpp
        glx/lookup.cpp
        egl/egl.c
        egl/loader.cpp
        gles/loader.cpp
        config/cJSON.c
        config/config.cpp
        config/gpu_utils.cpp
        config/settings.cpp

        gl/fpe/fpe.cpp
        gl/fpe/state.cpp
        gl/fpe/transformation.cpp
        gl/fpe/vertexpointer.cpp
        gl/fpe/fpe_shadergen.cpp
        gl/fpe/list.cpp
        gl/fpe/pointer_utils.cpp
        gl/fpe/drawing1x.cpp
        gl/fpe/vertexpointer_utils.cpp
        gl/fpe/drawstate1x.cpp
        gl/fpe/glstate.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ./includes
        ./glm
        ./3rdparty/xxhash)

target_link_libraries(${CMAKE_PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libglslang.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libspirv-cross-c-shared.so
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libshaderconv.so
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libGenericCodeGen.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libglslang-default-resource-limits.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libMachineIndependent.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libOSDependent.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-diff.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-link.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-lint.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-opt.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-reduce.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV.a
        ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPVRemapper.a
        android
        log
        EGL
)

target_include_directories(mobileglues PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        )


if (PROFILING)
        find_package(Threads)
        include_directories(${CMAKE_SOURCE_DIR}/3rdparty/perfetto/sdk)
        add_library(perfetto STATIC ${CMAKE_SOURCE_DIR}/3rdparty/perfetto/sdk/perfetto.cc)
        target_link_libraries(mobileglues perfetto ${CMAKE_THREAD_LIBS_INIT})
        target_compile_definitions(mobileglues PUBLIC PROFILING=1)
endif()

if (MOBILEGLUES_TEST)
        # Tests
        find_package(googletest REQUIRED CONFIG)
        find_package(junit-gtest REQUIRED CONFIG)

        add_library(${CMAKE_PROJECT_NAME}_tests SHARED
                        tests/sanitycheck_test.cpp
                        tests/fpe/vertexpointer_test.cpp
        )

        target_include_directories(${CMAKE_PROJECT_NAME}_tests PUBLIC
                ${CMAKE_SOURCE_DIR}/include
        )

        target_link_libraries(${CMAKE_PROJECT_NAME}_tests
                PRIVATE
                googletest::gtest
                junit-gtest::junit-gtest
                $<TARGET_OBJECTS:${CMAKE_PROJECT_NAME}>
        )
        target_link_libraries(${CMAKE_PROJECT_NAME}_tests PRIVATE
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libglslang.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libspirv-cross-c-shared.so
#                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libshaderconv.so
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libGenericCodeGen.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libglslang-default-resource-limits.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libMachineIndependent.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libOSDependent.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-diff.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-link.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-lint.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-opt.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools-reduce.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV-Tools.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPIRV.a
                ${CMAKE_SOURCE_DIR}/libraries/${ANDROID_ABI}/libSPVRemapper.a
                android
                log
                EGL
        )
endif()
