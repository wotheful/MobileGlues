//
// Created by Swung 0x48 on 2025/4/15.
//
#include "gtest/gtest.h"

#include "../../gl/fpe/types.h"
#include "../../gl/fpe/vertexpointer.h"

TEST(VertexPointer, PointerNormalize) {
    struct test_struct_t {
        float pos[4];
        float normal[3];
        float uv[2];
    } ts;

    auto& vpa = g_glstate.fpe_state.vertexpointer_array;

    vpa.reset();

    glVertexPointer(4, GL_FLOAT, sizeof(test_struct_t), ts.pos);
    glNormalPointer(GL_FLOAT, sizeof(test_struct_t), ts.normal);
    glTexCoordPointer(2, GL_FLOAT, sizeof(test_struct_t), ts.uv);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    vpa.normalize();

    EXPECT_EQ((uint64_t) &ts,
              (uint64_t) vpa.starting_pointer);

    EXPECT_EQ((uint64_t) 0x83,
              (uint64_t) vpa.enabled_pointers);

    EXPECT_EQ((uint64_t) sizeof(test_struct_t),
              (uint64_t) vpa.stride);

    auto& vertex_va = vpa.attributes[vp2idx(GL_VERTEX_ARRAY)];
    EXPECT_EQ(vertex_va.size, 4);
    EXPECT_EQ(vertex_va.usage, GL_VERTEX_ARRAY);
    EXPECT_EQ(vertex_va.type, GL_FLOAT);
    EXPECT_EQ(vertex_va.stride, sizeof(test_struct_t));
    EXPECT_EQ(vertex_va.pointer, (const void*)0);

    auto& normal_va = vpa.attributes[vp2idx(GL_NORMAL_ARRAY)];
    EXPECT_EQ(normal_va.size, 3);
    EXPECT_EQ(normal_va.usage, GL_NORMAL_ARRAY);
    EXPECT_EQ(normal_va.type, GL_FLOAT);
    EXPECT_EQ(normal_va.stride, sizeof(test_struct_t));
    EXPECT_EQ(normal_va.pointer, (const void*) offsetof(test_struct_t, normal));

    auto& tex_va = vpa.attributes[vp2idx(GL_TEXTURE_COORD_ARRAY)];
    EXPECT_EQ(tex_va.size, 2);
    EXPECT_EQ(tex_va.usage, GL_TEXTURE_COORD_ARRAY);
    EXPECT_EQ(tex_va.type, GL_FLOAT);
    EXPECT_EQ(tex_va.stride, sizeof(test_struct_t));
    EXPECT_EQ(tex_va.pointer, (const void*) offsetof(test_struct_t, uv));

    // Check for idempotence
    for (int i = 0; i < 5; ++i) {
        vpa.normalize();

        EXPECT_EQ((uint64_t) &ts,
                  (uint64_t) vpa.starting_pointer);

        EXPECT_EQ((uint64_t) 0x83,
                  (uint64_t) vpa.enabled_pointers);

        EXPECT_EQ((uint64_t) sizeof(test_struct_t),
                  (uint64_t) vpa.stride);

        auto& vertex_va = vpa.attributes[vp2idx(GL_VERTEX_ARRAY)];
        EXPECT_EQ(vertex_va.size, 4);
        EXPECT_EQ(vertex_va.usage, GL_VERTEX_ARRAY);
        EXPECT_EQ(vertex_va.type, GL_FLOAT);
        EXPECT_EQ(vertex_va.stride, sizeof(test_struct_t));
        EXPECT_EQ(vertex_va.pointer, (const void*)0);

        auto& normal_va = vpa.attributes[vp2idx(GL_NORMAL_ARRAY)];
        EXPECT_EQ(normal_va.size, 3);
        EXPECT_EQ(normal_va.usage, GL_NORMAL_ARRAY);
        EXPECT_EQ(normal_va.type, GL_FLOAT);
        EXPECT_EQ(normal_va.stride, sizeof(test_struct_t));
        EXPECT_EQ(normal_va.pointer, (const void*) offsetof(test_struct_t, normal));

        auto& tex_va = vpa.attributes[vp2idx(GL_TEXTURE_COORD_ARRAY)];
        EXPECT_EQ(tex_va.size, 2);
        EXPECT_EQ(tex_va.usage, GL_TEXTURE_COORD_ARRAY);
        EXPECT_EQ(tex_va.type, GL_FLOAT);
        EXPECT_EQ(tex_va.stride, sizeof(test_struct_t));
        EXPECT_EQ(tex_va.pointer, (const void*) offsetof(test_struct_t, uv));
    }
}