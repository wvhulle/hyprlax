/*
 * test_input_manager.c - Comprehensive test suite for input_manager
 *
 * Tests the input management system including:
 * - Input source selection parsing and weight calculation
 * - Multi-source input blending (workspace + cursor + window)
 * - Monitor cache management
 * - Provider registration and lifecycle
 * - Weight normalization edge cases
 */

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../src/core/input/input_manager.h"
#include "../src/include/core.h"
#include "../src/include/defaults.h"

/* Mock monitor instance for testing - must match real struct layout */
typedef struct monitor_instance {
    char name[64];                    /* Must be first to match real struct */
    uint32_t id;
    /* Other fields not needed for these tests */
    int width;
    int height;
    float scale;
} monitor_instance_t;

/* Mock hyprlax context for testing */
typedef struct hyprlax_context {
    config_t config;
} hyprlax_context_t;

/* Mock provider for testing */
static int mock_provider_init_called = 0;
static int mock_provider_destroy_called = 0;
static int mock_provider_tick_called = 0;
static bool mock_provider_should_fail = false;
static input_sample_t mock_provider_sample = {0.0f, 0.0f, false};

static int mock_provider_init(struct hyprlax_context *ctx, void **state) {
    mock_provider_init_called++;
    if (mock_provider_should_fail) {
        return HYPRLAX_ERROR_INVALID_ARGS;
    }
    *state = malloc(sizeof(int));
    if (!*state) return HYPRLAX_ERROR_NO_MEMORY;
    *(int*)*state = 42;
    return HYPRLAX_SUCCESS;
}

static void mock_provider_destroy(void *state) {
    mock_provider_destroy_called++;
    free(state);
}

static bool mock_provider_tick(void *state, const struct monitor_instance *monitor,
                               double now, input_sample_t *out) {
    mock_provider_tick_called++;
    *out = mock_provider_sample;
    return mock_provider_sample.valid;
}

static const input_provider_ops_t mock_provider_ops = {
    .name = "mock",
    .init = mock_provider_init,
    .destroy = mock_provider_destroy,
    .on_config = NULL,
    .start = NULL,
    .stop = NULL,
    .tick = mock_provider_tick,
};

static void reset_mock_provider(void) {
    mock_provider_init_called = 0;
    mock_provider_destroy_called = 0;
    mock_provider_tick_called = 0;
    mock_provider_should_fail = false;
    mock_provider_sample.x = 0.0f;
    mock_provider_sample.y = 0.0f;
    mock_provider_sample.valid = false;
}

/* Test fixture setup/teardown */
static void setup(void) {
    input_clear_provider_registry();
    reset_mock_provider();
}

static void teardown(void) {
    input_clear_provider_registry();
}

/*****************************************************************************
 * Input Source Selection Tests
 *****************************************************************************/

START_TEST(test_input_source_selection_init)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    for (int i = 0; i < INPUT_MAX; i++) {
        ck_assert(!selection.seen[i]);
        ck_assert(!selection.explicit_weight[i]);
        ck_assert_float_eq(selection.weights[i], 0.0f);
    }
    ck_assert(!selection.modified);
}
END_TEST

START_TEST(test_input_source_selection_single_source)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    int result = input_source_selection_add_spec(&selection, "workspace");
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert(selection.seen[INPUT_WORKSPACE]);
    ck_assert(!selection.explicit_weight[INPUT_WORKSPACE]);
    ck_assert(selection.modified);
}
END_TEST

START_TEST(test_input_source_selection_with_weight)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    int result = input_source_selection_add_spec(&selection, "workspace:0.7");
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert(selection.seen[INPUT_WORKSPACE]);
    ck_assert(selection.explicit_weight[INPUT_WORKSPACE]);
    ck_assert_float_eq_tol(selection.weights[INPUT_WORKSPACE], 0.7f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_multiple_sources)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    int result = input_source_selection_add_spec(&selection, "workspace:0.7,cursor:0.3");
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert(selection.seen[INPUT_WORKSPACE]);
    ck_assert(selection.seen[INPUT_CURSOR]);
    ck_assert_float_eq_tol(selection.weights[INPUT_WORKSPACE], 0.7f, 0.001f);
    ck_assert_float_eq_tol(selection.weights[INPUT_CURSOR], 0.3f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_whitespace_handling)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    /* Test with various whitespace patterns */
    int result = input_source_selection_add_spec(&selection, "  workspace : 0.5 , cursor : 0.5  ");
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert(selection.seen[INPUT_WORKSPACE]);
    ck_assert(selection.seen[INPUT_CURSOR]);
    ck_assert_float_eq_tol(selection.weights[INPUT_WORKSPACE], 0.5f, 0.001f);
    ck_assert_float_eq_tol(selection.weights[INPUT_CURSOR], 0.5f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_unknown_source)
{
    input_source_selection_t selection;
    input_source_selection_init(&selection);

    /* Should succeed but log warning for unknown source */
    int result = input_source_selection_add_spec(&selection, "unknown:0.5,workspace:0.5");
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert(selection.seen[INPUT_WORKSPACE]);
}
END_TEST

START_TEST(test_input_source_selection_commit_hybrid_defaults)
{
    input_source_selection_t selection;
    config_t config = {0};

    input_source_selection_init(&selection);
    input_source_selection_add_spec(&selection, "workspace,cursor");
    input_source_selection_commit(&selection, &config);

    /* Should apply hybrid defaults when both specified without weights */
    ck_assert_float_eq_tol(config.parallax_workspace_weight,
                          HYPRLAX_DEFAULT_HYBRID_WORKSPACE_WEIGHT, 0.001f);
    ck_assert_float_eq_tol(config.parallax_cursor_weight,
                          HYPRLAX_DEFAULT_HYBRID_CURSOR_WEIGHT, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_commit_explicit_weights)
{
    input_source_selection_t selection;
    config_t config = {0};

    input_source_selection_init(&selection);
    input_source_selection_add_spec(&selection, "workspace:0.6,cursor:0.4");
    input_source_selection_commit(&selection, &config);

    ck_assert_float_eq_tol(config.parallax_workspace_weight, 0.6f, 0.001f);
    ck_assert_float_eq_tol(config.parallax_cursor_weight, 0.4f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_commit_weight_distribution)
{
    input_source_selection_t selection;
    config_t config = {0};

    input_source_selection_init(&selection);
    /* workspace explicit, cursor/window should split remaining */
    input_source_selection_add_spec(&selection, "workspace:0.5,cursor,window");
    input_source_selection_commit(&selection, &config);

    ck_assert_float_eq_tol(config.parallax_workspace_weight, 0.5f, 0.001f);
    /* Remaining 0.5 split between cursor and window = 0.25 each */
    ck_assert_float_eq_tol(config.parallax_cursor_weight, 0.25f, 0.001f);
    ck_assert_float_eq_tol(config.parallax_window_weight, 0.25f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_commit_over_weight)
{
    input_source_selection_t selection;
    config_t config = {0};

    input_source_selection_init(&selection);
    /* Sum > 1.0, should handle gracefully */
    input_source_selection_add_spec(&selection, "workspace:0.8,cursor:0.5");
    input_source_selection_commit(&selection, &config);

    /* Should clamp/normalize */
    ck_assert_float_eq_tol(config.parallax_workspace_weight, 0.8f, 0.001f);
    ck_assert_float_eq_tol(config.parallax_cursor_weight, 0.5f, 0.001f);
}
END_TEST

START_TEST(test_input_source_selection_null_handling)
{
    input_source_selection_t selection;

    /* NULL selection should not crash */
    input_source_selection_init(NULL);

    int result = input_source_selection_add_spec(NULL, "workspace");
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);

    result = input_source_selection_add_spec(&selection, NULL);
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);
}
END_TEST

/*****************************************************************************
 * Provider Registration Tests
 *****************************************************************************/

START_TEST(test_provider_registration)
{
    int result = input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
}
END_TEST

START_TEST(test_provider_registration_null)
{
    int result = input_register_provider(NULL, INPUT_WORKSPACE);
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);
}
END_TEST

START_TEST(test_provider_registration_invalid_id)
{
    int result = input_register_provider(&mock_provider_ops, INPUT_MAX);
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);

    result = input_register_provider(&mock_provider_ops, -1);
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);
}
END_TEST

/*****************************************************************************
 * Input Manager Initialization Tests
 *****************************************************************************/

START_TEST(test_input_manager_init)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 0.7f,
        .parallax_cursor_weight = 0.3f,
        .parallax_window_weight = 0.0f,
    };

    int result = input_manager_init(&ctx, &manager, &config);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert_ptr_eq(manager.ctx, &ctx);
    ck_assert_ptr_eq(manager.config, &config);
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 0.7f, 0.001f);
    ck_assert_float_eq_tol(manager.weights[INPUT_CURSOR], 0.3f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_init_null)
{
    int result = input_manager_init(NULL, NULL, NULL);
    ck_assert_int_eq(result, HYPRLAX_ERROR_INVALID_ARGS);
}
END_TEST

START_TEST(test_input_manager_init_zero_weights_fallback)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 0.0f,
        .parallax_cursor_weight = 0.0f,
        .parallax_window_weight = 0.0f,
    };

    int result = input_manager_init(&ctx, &manager, &config);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);

    /* Should default to workspace-only when all weights are zero */
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 1.0f, 0.001f);
    ck_assert_float_eq_tol(manager.weights[INPUT_CURSOR], 0.0f, 0.001f);
    ck_assert_float_eq_tol(manager.weights[INPUT_WINDOW], 0.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_init_with_provider)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 1.0f,
    };

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);

    int result = input_manager_init(&ctx, &manager, &config);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert_int_eq(mock_provider_init_called, 1);

    input_manager_destroy(&manager);
    ck_assert_int_eq(mock_provider_destroy_called, 1);
}
END_TEST

START_TEST(test_input_manager_enabled_mask)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 0.5f,
        .parallax_cursor_weight = 0.5f,
        .parallax_window_weight = 0.0f,
    };

    input_manager_init(&ctx, &manager, &config);

    ck_assert(manager.enabled_mask & (1u << INPUT_WORKSPACE));
    ck_assert(manager.enabled_mask & (1u << INPUT_CURSOR));
    ck_assert(!(manager.enabled_mask & (1u << INPUT_WINDOW)));

    input_manager_destroy(&manager);
}
END_TEST

/*****************************************************************************
 * Monitor Cache Tests
 *****************************************************************************/

START_TEST(test_input_manager_cache_reset)
{
    input_manager_t manager = {0};

    /* Set some cache data */
    manager.monitor_cache[0].occupied = true;
    manager.monitor_cache[0].monitor_id = 123;
    manager.monitor_cache[0].composite.x = 100.0f;

    input_manager_reset_cache(&manager);

    ck_assert(!manager.monitor_cache[0].occupied);
    ck_assert_float_eq(manager.monitor_cache[0].composite.x, 0.0f);
}
END_TEST

START_TEST(test_input_manager_tick_no_providers)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {.parallax_workspace_weight = 1.0f};
    monitor_instance_t monitor = {.id = 1, .width = 1920, .height = 1080};

    input_manager_init(&ctx, &manager, &config);

    float out_x = 999.0f, out_y = 999.0f;
    bool result = input_manager_tick(&manager, &monitor, 1.0, &out_x, &out_y);

    ck_assert(!result);
    ck_assert_float_eq(out_x, 0.0f);
    ck_assert_float_eq(out_y, 0.0f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_tick_with_valid_sample)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 1.0f,
        .parallax_max_offset_x = 200.0f,
        .parallax_max_offset_y = 200.0f,
    };
    monitor_instance_t monitor = {.id = 1, .width = 1920, .height = 1080};

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    mock_provider_sample.x = 100.0f;
    mock_provider_sample.y = 50.0f;
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);

    float out_x = 0.0f, out_y = 0.0f;
    bool result = input_manager_tick(&manager, &monitor, 1.0, &out_x, &out_y);

    ck_assert(result);
    ck_assert_float_eq_tol(out_x, 100.0f, 0.001f);
    ck_assert_float_eq_tol(out_y, 50.0f, 0.001f);
    ck_assert_int_eq(mock_provider_tick_called, 1);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_tick_offset_clamping)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 1.0f,
        .parallax_max_offset_x = 50.0f,
        .parallax_max_offset_y = 30.0f,
    };
    monitor_instance_t monitor = {.id = 1, .width = 1920, .height = 1080};

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    mock_provider_sample.x = 200.0f;  /* Exceeds max */
    mock_provider_sample.y = -100.0f; /* Exceeds max negative */
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);

    float out_x = 0.0f, out_y = 0.0f;
    input_manager_tick(&manager, &monitor, 1.0, &out_x, &out_y);

    /* Should be clamped */
    ck_assert_float_eq_tol(out_x, 50.0f, 0.001f);
    ck_assert_float_eq_tol(out_y, -30.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_tick_multi_source_blending)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 0.5f,
        .parallax_cursor_weight = 0.5f,
        .parallax_max_offset_x = 1000.0f,
        .parallax_max_offset_y = 1000.0f,
    };
    monitor_instance_t monitor = {.id = 1, .width = 1920, .height = 1080};

    /* Register two providers */
    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    input_register_provider(&mock_provider_ops, INPUT_CURSOR);

    mock_provider_sample.x = 100.0f;
    mock_provider_sample.y = 100.0f;
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);

    float out_x = 0.0f, out_y = 0.0f;
    input_manager_tick(&manager, &monitor, 1.0, &out_x, &out_y);

    /* Both providers return same sample, weighted 0.5 each = 100.0 */
    ck_assert_float_eq_tol(out_x, 100.0f, 0.001f);
    ck_assert_float_eq_tol(out_y, 100.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_get_cache)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 1.0f,
        .parallax_max_offset_x = 1000.0f,
        .parallax_max_offset_y = 1000.0f,
    };
    monitor_instance_t monitor = {.id = 42, .width = 1920, .height = 1080};

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    mock_provider_sample.x = 123.0f;
    mock_provider_sample.y = 456.0f;
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);
    input_manager_tick(&manager, &monitor, 1.0, NULL, NULL);

    const input_monitor_cache_entry_t *cache = input_manager_get_cache(&manager, &monitor);
    ck_assert_ptr_nonnull(cache);
    ck_assert(cache->occupied);
    ck_assert_int_eq(cache->monitor_id, 42);
    ck_assert_float_eq_tol(cache->composite.x, 123.0f, 0.001f);
    ck_assert_float_eq_tol(cache->composite.y, 456.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_last_source)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {.parallax_workspace_weight = 1.0f};
    monitor_instance_t monitor = {.id = 1, .width = 1920, .height = 1080};

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    mock_provider_sample.x = 99.0f;
    mock_provider_sample.y = 88.0f;
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);
    input_manager_tick(&manager, &monitor, 1.0, NULL, NULL);

    input_sample_t sample;
    bool found = input_manager_last_source(&manager, &monitor, INPUT_WORKSPACE, &sample);

    ck_assert(found);
    ck_assert(sample.valid);
    ck_assert_float_eq_tol(sample.x, 99.0f, 0.001f);
    ck_assert_float_eq_tol(sample.y, 88.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_cache_multiple_monitors)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {
        .parallax_workspace_weight = 1.0f,
        .parallax_max_offset_x = 1000.0f,
        .parallax_max_offset_y = 1000.0f,
    };

    monitor_instance_t mon1 = {.id = 1, .width = 1920, .height = 1080};
    monitor_instance_t mon2 = {.id = 2, .width = 2560, .height = 1440};

    input_register_provider(&mock_provider_ops, INPUT_WORKSPACE);
    mock_provider_sample.valid = true;

    input_manager_init(&ctx, &manager, &config);

    /* Tick monitor 1 */
    mock_provider_sample.x = 100.0f;
    mock_provider_sample.y = 100.0f;
    input_manager_tick(&manager, &mon1, 1.0, NULL, NULL);

    /* Save cache1 values before next tick */
    const input_monitor_cache_entry_t *cache1_before = input_manager_get_cache(&manager, &mon1);
    ck_assert_ptr_nonnull(cache1_before);
    float cache1_x = cache1_before->composite.x;
    float cache1_y = cache1_before->composite.y;

    /* Tick monitor 2 */
    mock_provider_sample.x = 200.0f;
    mock_provider_sample.y = 200.0f;
    input_manager_tick(&manager, &mon2, 1.0, NULL, NULL);

    /* Verify both caches are independent */
    const input_monitor_cache_entry_t *cache1 = input_manager_get_cache(&manager, &mon1);
    const input_monitor_cache_entry_t *cache2 = input_manager_get_cache(&manager, &mon2);

    ck_assert_ptr_nonnull(cache1);
    ck_assert_ptr_nonnull(cache2);

    /* Monitor 1's cache should still have its original values */
    ck_assert_float_eq_tol(cache1->composite.x, cache1_x, 0.001f);
    ck_assert_float_eq_tol(cache1->composite.y, cache1_y, 0.001f);

    /* Monitor 2's cache should have new values */
    ck_assert_float_eq_tol(cache2->composite.x, 200.0f, 0.001f);
    ck_assert_float_eq_tol(cache2->composite.y, 200.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_set_enabled)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {0};

    input_manager_init(&ctx, &manager, &config);

    /* Enable workspace input with weight 0.8 */
    int result = input_manager_set_enabled(&manager, INPUT_WORKSPACE, true, 0.8f);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 0.8f, 0.001f);
    ck_assert(manager.enabled_mask & (1u << INPUT_WORKSPACE));

    /* Disable it */
    result = input_manager_set_enabled(&manager, INPUT_WORKSPACE, false, 0.0f);
    ck_assert_int_eq(result, HYPRLAX_SUCCESS);
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 0.0f, 0.001f);
    ck_assert(!(manager.enabled_mask & (1u << INPUT_WORKSPACE)));

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_set_enabled_weight_clamping)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config = {0};

    input_manager_init(&ctx, &manager, &config);

    /* Weight > 1.0 should be clamped */
    input_manager_set_enabled(&manager, INPUT_CURSOR, true, 1.5f);
    ck_assert_float_eq_tol(manager.weights[INPUT_CURSOR], 1.0f, 0.001f);

    /* Weight < 0.0 should be clamped */
    input_manager_set_enabled(&manager, INPUT_WINDOW, true, -0.5f);
    ck_assert_float_eq_tol(manager.weights[INPUT_WINDOW], 0.0f, 0.001f);

    input_manager_destroy(&manager);
}
END_TEST

START_TEST(test_input_manager_apply_config)
{
    hyprlax_context_t ctx = {0};
    input_manager_t manager = {0};
    config_t config1 = {.parallax_workspace_weight = 0.5f};
    config_t config2 = {.parallax_workspace_weight = 0.9f};

    input_manager_init(&ctx, &manager, &config1);
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 0.5f, 0.001f);

    /* Apply new config */
    input_manager_apply_config(&manager, &config2);
    ck_assert_float_eq_tol(manager.weights[INPUT_WORKSPACE], 0.9f, 0.001f);
    ck_assert_ptr_eq(manager.config, &config2);

    input_manager_destroy(&manager);
}
END_TEST

/*****************************************************************************
 * Test Suite Definition
 *****************************************************************************/

Suite *input_manager_suite(void)
{
    Suite *s;
    TCase *tc_selection, *tc_provider, *tc_manager, *tc_cache;

    s = suite_create("InputManager");

    /* Input Source Selection Tests */
    tc_selection = tcase_create("SourceSelection");
    tcase_add_checked_fixture(tc_selection, setup, teardown);
    tcase_add_test(tc_selection, test_input_source_selection_init);
    tcase_add_test(tc_selection, test_input_source_selection_single_source);
    tcase_add_test(tc_selection, test_input_source_selection_with_weight);
    tcase_add_test(tc_selection, test_input_source_selection_multiple_sources);
    tcase_add_test(tc_selection, test_input_source_selection_whitespace_handling);
    tcase_add_test(tc_selection, test_input_source_selection_unknown_source);
    tcase_add_test(tc_selection, test_input_source_selection_commit_hybrid_defaults);
    tcase_add_test(tc_selection, test_input_source_selection_commit_explicit_weights);
    tcase_add_test(tc_selection, test_input_source_selection_commit_weight_distribution);
    tcase_add_test(tc_selection, test_input_source_selection_commit_over_weight);
    tcase_add_test(tc_selection, test_input_source_selection_null_handling);
    suite_add_tcase(s, tc_selection);

    /* Provider Registration Tests */
    tc_provider = tcase_create("ProviderRegistration");
    tcase_add_checked_fixture(tc_provider, setup, teardown);
    tcase_add_test(tc_provider, test_provider_registration);
    tcase_add_test(tc_provider, test_provider_registration_null);
    tcase_add_test(tc_provider, test_provider_registration_invalid_id);
    suite_add_tcase(s, tc_provider);

    /* Manager Initialization Tests */
    tc_manager = tcase_create("ManagerInit");
    tcase_add_checked_fixture(tc_manager, setup, teardown);
    tcase_add_test(tc_manager, test_input_manager_init);
    tcase_add_test(tc_manager, test_input_manager_init_null);
    tcase_add_test(tc_manager, test_input_manager_init_zero_weights_fallback);
    tcase_add_test(tc_manager, test_input_manager_init_with_provider);
    tcase_add_test(tc_manager, test_input_manager_enabled_mask);
    tcase_add_test(tc_manager, test_input_manager_set_enabled);
    tcase_add_test(tc_manager, test_input_manager_set_enabled_weight_clamping);
    tcase_add_test(tc_manager, test_input_manager_apply_config);
    suite_add_tcase(s, tc_manager);

    /* Cache and Tick Tests */
    tc_cache = tcase_create("CacheAndTick");
    tcase_add_checked_fixture(tc_cache, setup, teardown);
    tcase_add_test(tc_cache, test_input_manager_cache_reset);
    tcase_add_test(tc_cache, test_input_manager_tick_no_providers);
    tcase_add_test(tc_cache, test_input_manager_tick_with_valid_sample);
    tcase_add_test(tc_cache, test_input_manager_tick_offset_clamping);
    tcase_add_test(tc_cache, test_input_manager_tick_multi_source_blending);
    tcase_add_test(tc_cache, test_input_manager_get_cache);
    tcase_add_test(tc_cache, test_input_manager_last_source);
    tcase_add_test(tc_cache, test_input_manager_cache_multiple_monitors);
    suite_add_tcase(s, tc_cache);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = input_manager_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
