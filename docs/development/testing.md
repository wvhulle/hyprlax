# Testing Guide

Comprehensive guide for testing hyprlax functionality.

## Running Tests

### Quick Test
```bash
# Run all tests
make test

# Run tests with output
make test VERBOSE=1
```

### Specific Tests
```bash
# Run single test suite
./tests/test_animation
./tests/test_config
./tests/test_compositor

# Run with debugging
gdb ./tests/test_animation
```

### Memory Testing
```bash
# Run tests with valgrind
make memcheck

# Run specific test with valgrind
valgrind --leak-check=full ./tests/test_config
```

### Coverage Report
Coverage targets are not currently provided by the build; use external tools if needed.

## Test Structure

Tests are simple C programs. Many use the Check framework (linked via pkg-config in the Makefile). The suite covers core modules, renderer, compositor/platform adapters, IPC, and configuration loaders.

### Available Test Suites (examples)
- `tests/test_easing` – easing values and parsing
- `tests/test_animation` – animation state and timing
- `tests/test_config` / `tests/test_toml_config` – config parsing
- `tests/test_ipc` – IPC server/client operations
- `tests/test_renderer` – renderer scaffolding and shader checks
- `tests/test_platform` – platform abstraction
- `tests/test_compositor` – compositor detection/capabilities
- `tests/test_workspace_changes` – workspace events

Note: `make test` builds and runs all present `tests/test_*.c` binaries.

## Test Data

Tests create their own data where needed. No image generation scripts are required.

## Performance Tests

### Benchmark Framework
```c
// tests/bench_rendering.c
#include <time.h>

static void bench_frame_render(void) {
    struct renderer *r = create_test_renderer();
    struct layer layers[5];
    create_test_layers(layers, 5);
    
    clock_t start = clock();
    
    for (int i = 0; i < 1000; i++) {
        renderer_draw_frame(r, layers, 5);
    }
    
    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("1000 frames in %.2f seconds (%.1f FPS)\n", 
           time_spent, 1000.0 / time_spent);
}
```

### Memory Benchmarks
```bash
# Profile memory usage
valgrind --tool=massif ./hyprlax test.jpg
ms_print massif.out.*

# Check for leaks
valgrind --leak-check=full --show-leak-kinds=all ./tests/test_config
```

## Environment helpers

- `HYPRLAX_SOCKET_SUFFIX` to isolate IPC sockets during tests
- `HYPRLAX_INIT_TRACE=1` to trace init and argument/config parsing

## Test Utilities

### Helper Functions
```c
// tests/test_utils.c
void create_test_image(const char *path, int width, int height) {
    // Create dummy image file for testing
}

struct config *create_test_config(void) {
    struct config *cfg = calloc(1, sizeof(*cfg));
    cfg->fps = 60;
    cfg->duration = 1.0;
    cfg->easing = EASING_EXPO;
    return cfg;
}

void simulate_workspace_change(int from, int to) {
    // Simulate compositor workspace event
}
```

## Continuous Integration

### GitHub Actions
```yaml
# .github/workflows/test.yml
name: Tests
on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v2
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y libwayland-dev wayland-protocols
    
    - name: Build
      run: make debug
    
    - name: Run tests
      run: make test
    
    - name: Memory check
      run: make memcheck
    
    # Coverage not provided by Makefile; integrate external tooling if desired
```

## Test Checklist

### Before Commit
- [ ] All tests pass: `make test`
- [ ] No memory leaks: `make memcheck`
- [ ] New features have tests
- [ ] Edge cases tested

### Test Types to Write
- [ ] Happy path tests
- [ ] Error condition tests
- [ ] Boundary tests
- [ ] Configuration tests
- [ ] Platform compatibility tests
- [ ] Performance regression tests

## Debugging Failed Tests

### Using GDB
```bash
# Debug test failure
gdb ./tests/test_animation
(gdb) run
(gdb) bt  # Backtrace on failure
(gdb) print variable_name
```

### Add Debug Output
```c
// Temporary debug output in tests
#ifdef DEBUG_TEST
    printf("DEBUG: value=%d expected=%d\n", value, expected);
#endif
```

### Common Test Issues

#### Test Hangs
- Check for infinite loops
- Verify timeouts are set
- Check for deadlocks

#### Flaky Tests
- Remove timing dependencies
- Use mocks for external dependencies
- Ensure proper cleanup

#### Platform-Specific Failures
- Use conditional compilation
- Mock platform-specific calls
- Document platform requirements

## Writing Good Tests

### Principles
- **Fast**: Tests should run quickly
- **Independent**: Tests shouldn't depend on each other
- **Repeatable**: Same result every time
- **Self-Validating**: Clear pass/fail
- **Timely**: Write tests with code

### Best Practices
```c
// Good test
static int test_specific_behavior(void) {
    // Single behavior per test
    // Clear arrange, act, assert
    // Descriptive name
    // Minimal setup
}

// Bad test
static int test_everything(void) {
    // Tests too many things
    // Hard to understand failure
    // Complex setup
    // Order-dependent
}
```
