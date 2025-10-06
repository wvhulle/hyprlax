Title: Documentation Accuracy Audit – hyprlax
Date: 2025-10-06

Scope
- Audited docs under `docs/` against current code in `src/` and Makefile.
- Focus areas: configuration formats (TOML vs legacy), CLI reference, IPC commands, env vars, install/build guides, examples/guides, and testing docs.

Summary
- Found multiple inconsistencies and stale references, especially around:
  - TOML vs legacy examples (legacy syntax shown where TOML is claimed)
  - IPC property names in examples (`uv_offset.x/y` vs actual `x/y`)
  - CLI docs missing multi‑monitor options and tint extension
  - Legacy config docs listing unsupported keys (e.g., `delay`) and omitting supported ones (`vsync`, `idle_poll_rate`, `scale`)
  - Migration guide suggesting unsupported `fit` values
  - Env var docs advertising `HYPRLAX_CONFIG` (not used) and omitting several supported overrides
  - Inconsistent statements about `reload` support for legacy configs (TOML‑only in code)
  - Testing/Building docs including non‑existent files/targets or external TOML deps

Top Priority Fixes (user‑visible and correctness)
- docs/guides/multi-layer.md: Replace the “Configuration File” example (currently legacy `.conf` content) with a real TOML snippet; ensure the run command uses that TOML.
- docs/guides/ipc-overview.md and docs/reference/ipc-commands.md: Replace `uv_offset.x`/`uv_offset.y` with `x`/`y` for layer UV pan property names; clarify accepted dotted aliases for other properties only.
- docs/reference/cli.md: Add multi‑monitor options (`--primary-only`, `--monitor`, `--disable-monitor`) and the optional layer tint extension in `--layer` (`:#RRGGBB[:strength]`).
- docs/configuration/legacy-format.md: Remove `delay` from supported commands; add `vsync`, `idle_poll_rate`, and `scale` which the legacy parser supports. Update examples accordingly.
- docs/configuration/migration-guide.md: Remove unsupported `fit = "fill"` / `"scale-down"`; list only supported: `stretch`, `cover`, `contain`, `fit_width`, `fit_height`. Remove `delay` from mapping table.
- docs/guides/ipc-overview.md: Correct `reload` description (runtime reload is TOML‑only; legacy reload is refused with a conversion hint).
- docs/reference/environment-vars.md: Remove `HYPRLAX_CONFIG`; it is not used by code. Add supported overrides: `HYPRLAX_RENDER_FPS`, `HYPRLAX_PARALLAX_SHIFT_PIXELS`, `HYPRLAX_ANIMATION_DURATION`, and mention `HYPRLAX_INIT_TRACE`.

Additional Notable Issues
- docs/reference/easing-functions.md: The table under “Mathematical Basis” repeats `cubic` twice; dedupe and correct any function names to match `src/core/easing.c`.
- docs/development/building.md: Earlier sections list external TOML dev packages; later it says vendor TOML is bundled. Remove external TOML deps from the “Required Libraries” sections to avoid confusion. Also prefer `libegl1-mesa-dev` (Debian) naming, which is consistent with the Installation guide.
- docs/getting-started/installation.md: The curl one‑liner to `https://hyprlax.com/install.sh` is risky and may not be authoritative; prefer the included `install.sh` or AUR instructions. Consider removing or clearly flagging as unofficial.
- docs/reference/ipc-commands.md: Property list for `set` includes `blur_passes` and `blur_size`, but the runtime setter in `src/hyprlax_main.c` does not support them. Remove these or document as “not yet implemented”. Prefer documenting `render.accumulate` and `render.trail_strength` instead (present in CLI/TOML and code).
- docs/getting-started/compatibility.md: States Wayfire is “blocked”; code includes a Wayfire adapter (`src/compositor/wayfire.c`) and the build includes it. If still blocked due to runtime issues, add a dated note; otherwise update the matrix.
- docs/development/testing.md: Contains references to non‑existent utilities/files (e.g., `tests/test_framework.h`, `tests/generate_test_images.sh`, mock files). Align this doc with the actual Check‑based tests and Makefile targets; remove “coverage” examples unless adding a `coverage` target.

Cross‑Checks (evidence)
- Multi‑layer guide shows legacy under TOML heading: `docs/guides/multi-layer.md` → section “Method 2: Configuration File” uses `layer ...`/`duration`/`shift` text; replace with TOML.
- IPC property names: `src/ipc.c`’s `apply_layer_property` accepts `x`/`y` for UV pan; no `uv_offset.x/y` aliases. `hyprlax_ctl` help also shows `x` and `y`.
- Layer tint extension: `src/hyprlax_main.c` parses `--layer image:shift:opacity:blur:#RRGGBB[:strength]` and applies tint; CLI doc omits this capability.
- Legacy config commands supported by parser: see `src/core/config_legacy.c` – supports `layer`, `duration`, `shift`, `fps`, `vsync`, `easing`, `idle_poll_rate`, `scale`. No `delay` key.
- TOML fit modes implemented: `src/core/config_toml.c` and `fit_from_string_local()` in `src/hyprlax_main.c` support `stretch|cover|contain|fit_width|fit_height`.
- Reload behavior: `hyprlax_reload_config()` in `src/hyprlax_main.c` is TOML‑only and logs a conversion hint for legacy.
- Env overrides present in code: `HYPRLAX_RENDER_FPS`, `HYPRLAX_PARALLAX_SHIFT_PIXELS`, `HYPRLAX_ANIMATION_DURATION`, `HYPRLAX_INIT_TRACE`; plus documented ones `HYPRLAX_DEBUG`, `HYPRLAX_TRACE`, `HYPRLAX_VERBOSE`, `HYPRLAX_SOCKET_SUFFIX`.
- Blur runtime settings: No setter for `blur_passes`/`blur_size` in `hyprlax_runtime_set_property()`; remove from IPC docs.

Proposed Edits (per file)
1) docs/guides/multi-layer.md
   - Replace legacy snippet under the TOML headline with:
     ```toml
     [global]
     duration = 1.2
     shift = 250
     easing = "expo"

     [[global.layers]]
     path = "/path/to/mountains.jpg"
     shift_multiplier = 0.3
     opacity = 1.0
     blur = 3.0

     [[global.layers]]
     path = "/path/to/trees.png"
     shift_multiplier = 0.6
     opacity = 0.8
     blur = 1.5

     [[global.layers]]
     path = "/path/to/grass.png"
     shift_multiplier = 1.0
     opacity = 0.7
     ```

2) docs/guides/ipc-overview.md and docs/reference/ipc-commands.md
   - Use `x`/`y` for UV pan; remove `uv_offset.x`/`uv_offset.y` from command examples.
   - Clarify `reload` is TOML‑only; legacy paths print a conversion hint.

3) docs/reference/cli.md
   - Add multi‑monitor flags: `--primary-only`, `--monitor <name>`, `--disable-monitor <name>`.
   - Document optional tint in `--layer` (`:#RRGGBB[:strength]`).

4) docs/configuration/legacy-format.md
   - Remove `delay`; add `vsync`, `idle_poll_rate`, `scale` as supported commands.
   - Update sample and the command list accordingly.

5) docs/configuration/migration-guide.md
   - Remove unsupported `fit = "fill"`/`"scale-down"`; list supported fit modes.
   - Remove `delay` from mapping table and narrative.

6) docs/reference/environment-vars.md
   - Remove `HYPRLAX_CONFIG` examples.
   - Add `HYPRLAX_RENDER_FPS`, `HYPRLAX_PARALLAX_SHIFT_PIXELS`, `HYPRLAX_ANIMATION_DURATION`, `HYPRLAX_INIT_TRACE` with short explanations.

7) docs/reference/easing-functions.md
   - Fix duplicate `cubic` in the “Mathematical Basis” table.

8) docs/development/building.md
   - Drop external TOML dev deps; reinforce that vendor TOML is bundled.
   - Use correct Debian package names (`libegl1-mesa-dev`).

9) docs/reference/ipc-commands.md
   - Remove `blur_passes`/`blur_size` from `set` properties or annotate as “not implemented”.

10) docs/getting-started/installation.md
   - Consider removing the external curl one‑liner or mark as unofficial. Prefer `./install.sh` and AUR.

Nice‑to‑Have Cleanups
- Remove or clearly mark `src/hyprlax.c` as legacy (not built) to avoid confusion with its CLI/help that references unsupported flags.
- Ensure all guides use TOML for configuration examples unless explicitly demonstrating legacy format; cross‑link the Migration Guide where legacy appears.
- Add a short “Multi‑Monitor” subsection to CLI docs and Quick Start referencing the flags and current behavior/limitations.

Validation Checklist (post‑fix)
- TOML vs legacy examples are consistent across guides and configuration docs.
- IPC examples round‑trip with the current daemon (e.g., `modify x/y`, `set render.overflow`, `set parallax.input`).
- CLI docs enumerate all supported flags output by `--help` in `src/hyprlax_main.c`.
- Environment variables listed are actually consumed by the code; no missing or extra vars remain.
- Installation/Building docs align with Makefile targets and bundled dependencies.
- Testing guide reflects existing tests/targets and does not reference non‑existent files.

Notes
- The repo already contains a robust CLI reference at `docs/reference/cli.md` (contrary to the perception that it’s missing). Primary work is bringing it fully in sync with `src/hyprlax_main.c` and surfacing multi‑monitor + layer tint details.

