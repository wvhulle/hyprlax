# Mouse Parallax (Cursor-Driven)

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: cursor_parallax](https://img.shields.io/badge/purpose-cursor__parallax-16a34a?style=flat)](#)
[![Config: TOML](https://img.shields.io/badge/config-TOML-4c1?style=flat)](../../docs/configuration/toml-reference.md)

Foreground flower with a blurred background driven by smooth mouse motion.

![Screenshot placeholder: flower foreground over blurred background](./screenshot.png)
![Layer diagram placeholder: bg (slow) → fg (fast)](./layers.png)

## How To Use

```bash
./hyprlax --config examples/mouse-parallax/hyprlax.toml
```

Tips:
- Move the cursor to see subtle parallax.
- Workspace animation is disabled here (`duration = 0.0`).

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Cursor-driven parallax (`parallax.mode = "cursor"`)
- Blending sources and clamping offsets
- Per-layer axis multipliers for vertical stability

## Goal Of The Example

Create a tasteful focus-follow effect with minimal motion and good readability.

## Configuration Walkthrough

- Global:
  - `parallax.mode = "cursor"`
  - `parallax.sources.cursor.weight = 1.0`, `workspace.weight = 0.0`
  - `parallax.max_offset_px` to clamp motion (avoid large vertical shifts)
- Layers:
  - Background: slight `shift_multiplier` (x=0.50, y=0.25), blur for bokeh
  - Foreground: `shift_multiplier` (x=1.0, y=1.0), sharp

## Customization Tips

- Invert axes with `[global.parallax.invert.cursor]` for a different feel
- Reduce `sensitivity_*` if movement feels too strong
- Adjust `margin_px`/`align` for subject framing

## Image Placeholders

- Add a screenshot as `examples/mouse-parallax/screenshot.png`
- Add a layer diagram as `examples/mouse-parallax/layers.png`

