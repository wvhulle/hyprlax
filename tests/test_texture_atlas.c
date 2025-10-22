/*
 * test_texture_atlas.c - Comprehensive test suite for texture_atlas
 *
 * Tests the texture atlas system including:
 * - Atlas creation with various texture counts
 * - Grid layout calculation and power-of-2 sizing
 * - UV coordinate calculation accuracy
 * - Memory allocation failures
 * - Texture binding and destruction
 */

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../src/renderer/texture_atlas.h"
#include "../src/include/renderer.h"

/* Mock renderer operations for testing */
static int mock_create_texture_called = 0;
static int mock_destroy_texture_called = 0;
static texture_t *mock_last_created_texture = NULL;
static bool mock_should_fail_create = false;

static texture_t *mock_create_texture(const void *data, int width, int height,
                                     texture_format_t format) {
    mock_create_texture_called++;

    if (mock_should_fail_create) {
        return NULL;
    }

    texture_t *tex = calloc(1, sizeof(texture_t));
    if (!tex) return NULL;

    tex->id = 1000 + mock_create_texture_called;
    tex->width = width;
    tex->height = height;
    tex->format = format;

    mock_last_created_texture = tex;
    return tex;
}

static void mock_destroy_texture(texture_t *texture) {
    mock_destroy_texture_called++;
    free(texture);
}

static const renderer_ops_t mock_renderer_ops = {
    .create_texture = mock_create_texture,
    .destroy_texture = mock_destroy_texture,
    /* Other ops not needed for atlas tests */
    .init = NULL,
    .destroy = NULL,
    .begin_frame = NULL,
    .end_frame = NULL,
    .present = NULL,
    .clear = NULL,
    .draw_layer = NULL,
};

static void reset_mock_renderer(void) {
    mock_create_texture_called = 0;
    mock_destroy_texture_called = 0;
    mock_last_created_texture = NULL;
    mock_should_fail_create = false;
}

/* Test fixture setup/teardown */
static void setup(void) {
    reset_mock_renderer();
}

static void teardown(void) {
    reset_mock_renderer();
}

/* Helper to create test textures */
static texture_t **create_test_textures(int count, int width, int height) {
    texture_t **textures = calloc(count, sizeof(texture_t*));
    if (!textures) return NULL;

    for (int i = 0; i < count; i++) {
        textures[i] = calloc(1, sizeof(texture_t));
        if (!textures[i]) {
            for (int j = 0; j < i; j++) free(textures[j]);
            free(textures);
            return NULL;
        }
        textures[i]->id = 100 + i;
        textures[i]->width = width;
        textures[i]->height = height;
        textures[i]->format = TEXTURE_FORMAT_RGBA;
    }

    return textures;
}

static void free_test_textures(texture_t **textures, int count) {
    if (!textures) return;
    for (int i = 0; i < count; i++) {
        free(textures[i]);
    }
    free(textures);
}

/*****************************************************************************
 * Atlas Creation Tests
 *****************************************************************************/

START_TEST(test_atlas_create_single_texture)
{
    texture_t **textures = create_test_textures(1, 256, 256);
    ck_assert_ptr_nonnull(textures);

    texture_atlas_t *atlas = texture_atlas_create(textures, 1, &mock_renderer_ops, true);
    ck_assert_ptr_nonnull(atlas);
    ck_assert_int_eq(mock_create_texture_called, 1);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    ck_assert_int_eq(mock_destroy_texture_called, 1);

    free_test_textures(textures, 1);
}
END_TEST

START_TEST(test_atlas_create_multiple_textures)
{
    texture_t **textures = create_test_textures(4, 128, 128);
    ck_assert_ptr_nonnull(textures);

    texture_atlas_t *atlas = texture_atlas_create(textures, 4, &mock_renderer_ops, true);
    ck_assert_ptr_nonnull(atlas);
    ck_assert_int_eq(mock_create_texture_called, 1);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 4);
}
END_TEST

START_TEST(test_atlas_create_disabled)
{
    texture_t **textures = create_test_textures(2, 64, 64);
    ck_assert_ptr_nonnull(textures);

    /* Creating atlas with enabled=false should return NULL */
    texture_atlas_t *atlas = texture_atlas_create(textures, 2, &mock_renderer_ops, false);
    ck_assert_ptr_null(atlas);
    ck_assert_int_eq(mock_create_texture_called, 0);

    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_create_null_inputs)
{
    texture_t **textures = create_test_textures(2, 64, 64);

    /* NULL textures */
    texture_atlas_t *atlas = texture_atlas_create(NULL, 2, &mock_renderer_ops, true);
    ck_assert_ptr_null(atlas);

    /* Zero count */
    atlas = texture_atlas_create(textures, 0, &mock_renderer_ops, true);
    ck_assert_ptr_null(atlas);

    /* Negative count */
    atlas = texture_atlas_create(textures, -1, &mock_renderer_ops, true);
    ck_assert_ptr_null(atlas);

    /* NULL ops */
    atlas = texture_atlas_create(textures, 2, NULL, true);
    ck_assert_ptr_null(atlas);

    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_create_varying_sizes)
{
    /* Create textures of different sizes */
    texture_t **textures = calloc(3, sizeof(texture_t*));
    textures[0] = calloc(1, sizeof(texture_t));
    textures[0]->width = 64;
    textures[0]->height = 64;

    textures[1] = calloc(1, sizeof(texture_t));
    textures[1]->width = 128;
    textures[1]->height = 128;

    textures[2] = calloc(1, sizeof(texture_t));
    textures[2]->width = 32;
    textures[2]->height = 256;

    texture_atlas_t *atlas = texture_atlas_create(textures, 3, &mock_renderer_ops, true);
    ck_assert_ptr_nonnull(atlas);

    /* Atlas should be created with dimensions that fit the largest textures */
    int width, height;
    texture_atlas_get_dimensions(atlas, &width, &height);

    /* Grid size for 3 textures = ceil(sqrt(3)) = 2x2
     * Max texture size = 128x256
     * Atlas size should be next power of 2 of (2 * 128, 2 * 256) = (256, 512) */
    ck_assert_int_eq(width, 256);
    ck_assert_int_eq(height, 512);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 3);
}
END_TEST

/*****************************************************************************
 * UV Coordinate Tests
 *****************************************************************************/

START_TEST(test_atlas_uv_single_texture)
{
    texture_t **textures = create_test_textures(1, 256, 256);
    texture_atlas_t *atlas = texture_atlas_create(textures, 1, &mock_renderer_ops, true);

    float u1, v1, u2, v2;
    bool result = texture_atlas_get_uv(atlas, 0, &u1, &v1, &u2, &v2);

    ck_assert(result);
    ck_assert_float_eq_tol(u1, 0.0f, 0.001f);
    ck_assert_float_eq_tol(v1, 0.0f, 0.001f);
    /* u2,v2 depend on atlas size and texture placement */
    ck_assert(u2 > u1);
    ck_assert(v2 > v1);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 1);
}
END_TEST

START_TEST(test_atlas_uv_multiple_textures)
{
    texture_t **textures = create_test_textures(4, 64, 64);
    texture_atlas_t *atlas = texture_atlas_create(textures, 4, &mock_renderer_ops, true);

    /* Get UVs for all textures and ensure they're unique */
    float uvs[4][4]; /* [texture][u1,v1,u2,v2] */

    for (int i = 0; i < 4; i++) {
        bool result = texture_atlas_get_uv(atlas, i,
                                          &uvs[i][0], &uvs[i][1],
                                          &uvs[i][2], &uvs[i][3]);
        ck_assert(result);

        /* UV coordinates should be in [0,1] range */
        ck_assert(uvs[i][0] >= 0.0f && uvs[i][0] <= 1.0f);
        ck_assert(uvs[i][1] >= 0.0f && uvs[i][1] <= 1.0f);
        ck_assert(uvs[i][2] >= 0.0f && uvs[i][2] <= 1.0f);
        ck_assert(uvs[i][3] >= 0.0f && uvs[i][3] <= 1.0f);

        /* u2 > u1, v2 > v1 */
        ck_assert(uvs[i][2] > uvs[i][0]);
        ck_assert(uvs[i][3] > uvs[i][1]);
    }

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 4);
}
END_TEST

START_TEST(test_atlas_uv_invalid_index)
{
    texture_t **textures = create_test_textures(2, 64, 64);
    texture_atlas_t *atlas = texture_atlas_create(textures, 2, &mock_renderer_ops, true);

    float u1, v1, u2, v2;

    /* Index too high */
    bool result = texture_atlas_get_uv(atlas, 10, &u1, &v1, &u2, &v2);
    ck_assert(!result);

    /* Negative index */
    result = texture_atlas_get_uv(atlas, -1, &u1, &v1, &u2, &v2);
    ck_assert(!result);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_uv_null_atlas)
{
    float u1, v1, u2, v2;
    bool result = texture_atlas_get_uv(NULL, 0, &u1, &v1, &u2, &v2);
    ck_assert(!result);
}
END_TEST

/*****************************************************************************
 * Atlas Properties Tests
 *****************************************************************************/

START_TEST(test_atlas_get_texture)
{
    texture_t **textures = create_test_textures(2, 64, 64);
    texture_atlas_t *atlas = texture_atlas_create(textures, 2, &mock_renderer_ops, true);

    texture_t *atlas_tex = texture_atlas_get_texture(atlas);
    ck_assert_ptr_nonnull(atlas_tex);
    ck_assert_ptr_eq(atlas_tex, mock_last_created_texture);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_get_texture_null)
{
    texture_t *atlas_tex = texture_atlas_get_texture(NULL);
    ck_assert_ptr_null(atlas_tex);
}
END_TEST

START_TEST(test_atlas_is_enabled)
{
    texture_t **textures = create_test_textures(2, 64, 64);

    /* Enabled atlas */
    texture_atlas_t *atlas1 = texture_atlas_create(textures, 2, &mock_renderer_ops, true);
    ck_assert(texture_atlas_is_enabled(atlas1));

    /* NULL atlas */
    ck_assert(!texture_atlas_is_enabled(NULL));

    texture_atlas_destroy(atlas1, &mock_renderer_ops);
    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_get_dimensions)
{
    texture_t **textures = create_test_textures(1, 256, 256);
    texture_atlas_t *atlas = texture_atlas_create(textures, 1, &mock_renderer_ops, true);

    int width, height;
    texture_atlas_get_dimensions(atlas, &width, &height);

    /* Dimensions should be power of 2 */
    ck_assert((width & (width - 1)) == 0);
    ck_assert((height & (height - 1)) == 0);

    /* Should be at least as large as input texture */
    ck_assert(width >= 256);
    ck_assert(height >= 256);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 1);
}
END_TEST

START_TEST(test_atlas_get_dimensions_null)
{
    int width = 999, height = 999;
    texture_atlas_get_dimensions(NULL, &width, &height);

    ck_assert_int_eq(width, 0);
    ck_assert_int_eq(height, 0);
}
END_TEST

/*****************************************************************************
 * Power-of-2 Sizing Tests
 *****************************************************************************/

START_TEST(test_atlas_power_of_2_sizing)
{
    /* Test various input sizes to ensure output is power of 2 */
    int test_cases[][2] = {
        {64, 64},
        {100, 100},
        {255, 255},
        {256, 256},
        {257, 257},
        {512, 512},
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        texture_t **textures = create_test_textures(1, test_cases[i][0], test_cases[i][1]);
        texture_atlas_t *atlas = texture_atlas_create(textures, 1, &mock_renderer_ops, true);

        int width, height;
        texture_atlas_get_dimensions(atlas, &width, &height);

        /* Check power of 2 */
        ck_assert_msg((width & (width - 1)) == 0,
                     "Width %d is not power of 2 for input %dx%d",
                     width, test_cases[i][0], test_cases[i][1]);
        ck_assert_msg((height & (height - 1)) == 0,
                     "Height %d is not power of 2 for input %dx%d",
                     height, test_cases[i][0], test_cases[i][1]);

        texture_atlas_destroy(atlas, &mock_renderer_ops);
        free_test_textures(textures, 1);
        reset_mock_renderer();
    }
}
END_TEST

/*****************************************************************************
 * Grid Layout Tests
 *****************************************************************************/

START_TEST(test_atlas_grid_layout_4_textures)
{
    /* 4 textures should create a 2x2 grid */
    texture_t **textures = create_test_textures(4, 64, 64);
    texture_atlas_t *atlas = texture_atlas_create(textures, 4, &mock_renderer_ops, true);

    int width, height;
    texture_atlas_get_dimensions(atlas, &width, &height);

    /* Grid size = ceil(sqrt(4)) = 2
     * Atlas size = next_pow2(2 * 64) = 128 for both dimensions */
    ck_assert_int_eq(width, 128);
    ck_assert_int_eq(height, 128);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 4);
}
END_TEST

START_TEST(test_atlas_grid_layout_9_textures)
{
    /* 9 textures should create a 3x3 grid */
    texture_t **textures = create_test_textures(9, 32, 32);
    texture_atlas_t *atlas = texture_atlas_create(textures, 9, &mock_renderer_ops, true);

    int width, height;
    texture_atlas_get_dimensions(atlas, &width, &height);

    /* Grid size = ceil(sqrt(9)) = 3
     * Atlas size = next_pow2(3 * 32) = next_pow2(96) = 128 */
    ck_assert_int_eq(width, 128);
    ck_assert_int_eq(height, 128);

    texture_atlas_destroy(atlas, &mock_renderer_ops);
    free_test_textures(textures, 9);
}
END_TEST

/*****************************************************************************
 * Memory/Error Handling Tests
 *****************************************************************************/

START_TEST(test_atlas_create_failure)
{
    texture_t **textures = create_test_textures(2, 64, 64);

    /* Make texture creation fail */
    mock_should_fail_create = true;

    texture_atlas_t *atlas = texture_atlas_create(textures, 2, &mock_renderer_ops, true);

    /* Should return NULL when texture creation fails */
    ck_assert_ptr_null(atlas);

    free_test_textures(textures, 2);
}
END_TEST

START_TEST(test_atlas_destroy_null)
{
    /* Should not crash with NULL */
    texture_atlas_destroy(NULL, &mock_renderer_ops);
    ck_assert_int_eq(mock_destroy_texture_called, 0);
}
END_TEST

/*****************************************************************************
 * Test Suite Definition
 *****************************************************************************/

Suite *texture_atlas_suite(void)
{
    Suite *s;
    TCase *tc_create, *tc_uv, *tc_props, *tc_sizing, *tc_grid, *tc_error;

    s = suite_create("TextureAtlas");

    /* Atlas Creation Tests */
    tc_create = tcase_create("Creation");
    tcase_add_checked_fixture(tc_create, setup, teardown);
    tcase_add_test(tc_create, test_atlas_create_single_texture);
    tcase_add_test(tc_create, test_atlas_create_multiple_textures);
    tcase_add_test(tc_create, test_atlas_create_disabled);
    tcase_add_test(tc_create, test_atlas_create_null_inputs);
    tcase_add_test(tc_create, test_atlas_create_varying_sizes);
    suite_add_tcase(s, tc_create);

    /* UV Coordinate Tests */
    tc_uv = tcase_create("UVCoordinates");
    tcase_add_checked_fixture(tc_uv, setup, teardown);
    tcase_add_test(tc_uv, test_atlas_uv_single_texture);
    tcase_add_test(tc_uv, test_atlas_uv_multiple_textures);
    tcase_add_test(tc_uv, test_atlas_uv_invalid_index);
    tcase_add_test(tc_uv, test_atlas_uv_null_atlas);
    suite_add_tcase(s, tc_uv);

    /* Property Tests */
    tc_props = tcase_create("Properties");
    tcase_add_checked_fixture(tc_props, setup, teardown);
    tcase_add_test(tc_props, test_atlas_get_texture);
    tcase_add_test(tc_props, test_atlas_get_texture_null);
    tcase_add_test(tc_props, test_atlas_is_enabled);
    tcase_add_test(tc_props, test_atlas_get_dimensions);
    tcase_add_test(tc_props, test_atlas_get_dimensions_null);
    suite_add_tcase(s, tc_props);

    /* Power-of-2 Sizing Tests */
    tc_sizing = tcase_create("PowerOf2Sizing");
    tcase_add_checked_fixture(tc_sizing, setup, teardown);
    tcase_add_test(tc_sizing, test_atlas_power_of_2_sizing);
    suite_add_tcase(s, tc_sizing);

    /* Grid Layout Tests */
    tc_grid = tcase_create("GridLayout");
    tcase_add_checked_fixture(tc_grid, setup, teardown);
    tcase_add_test(tc_grid, test_atlas_grid_layout_4_textures);
    tcase_add_test(tc_grid, test_atlas_grid_layout_9_textures);
    suite_add_tcase(s, tc_grid);

    /* Error Handling Tests */
    tc_error = tcase_create("ErrorHandling");
    tcase_add_checked_fixture(tc_error, setup, teardown);
    tcase_add_test(tc_error, test_atlas_create_failure);
    tcase_add_test(tc_error, test_atlas_destroy_null);
    suite_add_tcase(s, tc_error);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = texture_atlas_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
