// Test suite for GIF animation functionality
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../src/vendor/gifdec.h"

// Test GIF decoder basic functionality
START_TEST(test_gif_decoder_init)
{
    // Test that we can handle NULL paths
    gd_GIF *gif = gd_open_gif(NULL);
    ck_assert_ptr_null(gif);

    // Test that we can handle nonexistent files
    gif = gd_open_gif("/nonexistent/path/to/file.gif");
    ck_assert_ptr_null(gif);
}
END_TEST

// Test creating a minimal valid GIF for testing
START_TEST(test_gif_create_minimal)
{
    // Create a minimal 1x1 single-frame GIF for testing
    const char *test_gif = "/tmp/test_minimal.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // GIF89a header
    unsigned char gif_data[] = {
        // Header
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,  // "GIF89a"
        // Logical Screen Descriptor
        0x01, 0x00,  // width = 1
        0x01, 0x00,  // height = 1
        0x80,        // GCT flag, color resolution, sort flag
        0x00,        // background color index
        0x00,        // pixel aspect ratio
        // Global Color Table (2 colors)
        0x00, 0x00, 0x00,  // color 0: black
        0xFF, 0xFF, 0xFF,  // color 1: white
        // Image Descriptor
        0x2C,        // Image separator
        0x00, 0x00,  // left
        0x00, 0x00,  // top
        0x01, 0x00,  // width
        0x01, 0x00,  // height
        0x00,        // packed fields
        // Image Data
        0x02,        // LZW minimum code size
        0x02,        // data sub-block size
        0x4C, 0x01,  // image data
        0x00,        // block terminator
        // Trailer
        0x3B         // GIF trailer
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    // Try to load the GIF
    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);
    ck_assert_int_eq(gif->width, 1);
    ck_assert_int_eq(gif->height, 1);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test frame counting
START_TEST(test_gif_frame_count)
{
    // Create a test GIF with known frame count
    const char *test_gif = "/tmp/test_frames.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // Multi-frame GIF (2 frames)
    unsigned char gif_data[] = {
        // Header
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,  // "GIF89a"
        // Logical Screen Descriptor
        0x02, 0x00,  // width = 2
        0x02, 0x00,  // height = 2
        0xF0,        // GCT flag
        0x00,        // background
        0x00,        // aspect ratio
        // Global Color Table (2 colors)
        0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF,
        // Graphics Control Extension for frame 1
        0x21, 0xF9, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x00,
        // Image Descriptor frame 1
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        // Image Data frame 1
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        // Graphics Control Extension for frame 2
        0x21, 0xF9, 0x04, 0x00, 0x14, 0x00, 0x00, 0x00,
        // Image Descriptor frame 2
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        // Image Data frame 2
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        // Trailer
        0x3B
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    // Count frames
    int frame_count = 0;
    while (gd_get_frame(gif)) {
        frame_count++;
    }

    ck_assert_int_eq(frame_count, 2);

    // Test rewind
    gd_rewind(gif);
    frame_count = 0;
    while (gd_get_frame(gif)) {
        frame_count++;
    }
    ck_assert_int_eq(frame_count, 2);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test GIF frame delays
START_TEST(test_gif_frame_delays)
{
    const char *test_gif = "/tmp/test_delays.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // GIF with specific delay (10 = 100ms)
    unsigned char gif_data[] = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
        0x02, 0x00, 0x02, 0x00, 0xF0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
        // Graphics Control Extension with delay = 10 (0x0A = 100ms)
        0x21, 0xF9, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x00,
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        0x3B
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    int got_frame = gd_get_frame(gif);
    ck_assert_int_eq(got_frame, 1);

    // Check delay value (should be 10 centiseconds = 100ms)
    ck_assert_int_eq(gif->gce.delay, 10);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test GIF transparency flag
START_TEST(test_gif_transparency)
{
    const char *test_gif = "/tmp/test_transparency.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // GIF with transparency flag set
    unsigned char gif_data[] = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
        0x02, 0x00, 0x02, 0x00, 0xF0, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
        // Graphics Control Extension with transparency (flag = 0x01, tindex = 0)
        0x21, 0xF9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        0x3B
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    gd_get_frame(gif);

    // Check transparency flag
    ck_assert_int_eq(gif->gce.transparency, 1);
    ck_assert_int_eq(gif->gce.tindex, 0);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test empty GIF (no frames) - should be handled gracefully
START_TEST(test_gif_empty)
{
    const char *test_gif = "/tmp/test_empty.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // GIF with no image data, just header and trailer
    unsigned char gif_data[] = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
        0x01, 0x00, 0x01, 0x00, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
        0x3B  // Trailer (no frames)
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    // Should not be able to get any frames
    int frame_count = 0;
    while (gd_get_frame(gif)) {
        frame_count++;
    }

    ck_assert_int_eq(frame_count, 0);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test GIF render functionality
START_TEST(test_gif_render_frame)
{
    const char *test_gif = "/tmp/test_render.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // Simple 2x2 GIF
    unsigned char gif_data[] = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
        0x02, 0x00, 0x02, 0x00, 0xF0, 0x00, 0x00,
        0xFF, 0x00, 0x00,  // Red
        0x00, 0xFF, 0x00,  // Green
        0x21, 0xF9, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x00,
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        0x3B
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    int got_frame = gd_get_frame(gif);
    ck_assert_int_eq(got_frame, 1);

    // Allocate buffer for RGB data
    uint8_t *buffer = malloc(gif->width * gif->height * 3);
    ck_assert_ptr_nonnull(buffer);

    // Render the frame
    gd_render_frame(gif, buffer);

    // Buffer should contain some data (not all zeros)
    int has_data = 0;
    for (int i = 0; i < gif->width * gif->height * 3; i++) {
        if (buffer[i] != 0) {
            has_data = 1;
            break;
        }
    }
    ck_assert_int_eq(has_data, 1);

    free(buffer);
    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Test GIF dimensions
START_TEST(test_gif_dimensions)
{
    const char *test_gif = "/tmp/test_dims.gif";
    FILE *f = fopen(test_gif, "wb");
    ck_assert_ptr_nonnull(f);

    // GIF with specific dimensions (10x20)
    unsigned char gif_data[] = {
        0x47, 0x49, 0x46, 0x38, 0x39, 0x61,
        0x0A, 0x00,  // width = 10
        0x14, 0x00,  // height = 20
        0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
        0x2C, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x14, 0x00, 0x00,
        0x02, 0x04, 0x84, 0x8F, 0xA9, 0xCB, 0x00,
        0x3B
    };

    fwrite(gif_data, 1, sizeof(gif_data), f);
    fclose(f);

    gd_GIF *gif = gd_open_gif(test_gif);
    ck_assert_ptr_nonnull(gif);

    ck_assert_int_eq(gif->width, 10);
    ck_assert_int_eq(gif->height, 20);

    gd_close_gif(gif);
    unlink(test_gif);
}
END_TEST

// Create test suite
Suite *gif_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("GIF");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_gif_decoder_init);
    tcase_add_test(tc_core, test_gif_create_minimal);
    tcase_add_test(tc_core, test_gif_frame_count);
    tcase_add_test(tc_core, test_gif_frame_delays);
    tcase_add_test(tc_core, test_gif_transparency);
    tcase_add_test(tc_core, test_gif_empty);
    tcase_add_test(tc_core, test_gif_render_frame);
    tcase_add_test(tc_core, test_gif_dimensions);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = gif_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
