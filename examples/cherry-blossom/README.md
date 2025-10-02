# Cherry Blossom (Workspace + Time Overlay)

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: dynamic_overlay](https://img.shields.io/badge/purpose-dynamic__overlay-ec4899?style=flat)](#)
[![Config: TOML](https://img.shields.io/badge/config-TOML-4c1?style=flat)](../../docs/configuration/toml-reference.md)

Elegant cherry blossom wallpaper with an optional dynamic clock overlay added at runtime.

![Screenshot placeholder: cherry blossom with time overlay](./screenshot.png)
![Layer diagram placeholder: background → bloom → overlay (clock)](./layers.png)

## How To Use

1) Start hyprlax with the provided TOML config:
```bash
./hyprlax --config examples/cherry-blossom/hyprlax.toml
```

2) Add the time overlay (requires Python + Pillow):
```bash
cd examples/cherry-blossom
python3 time_overlay.py --font-size 120 --position center --scale 11.0 --verbose
```

3) Optional: run once from cron (updates once per minute). Add to crontab:
```cron
*/1 * * * * /path/to/hyprlax/examples/cherry-blossom/time_overlay_cron.sh
```

Notes:
- The overlay is added via `hyprlax ctl add` and refreshed via `ctl modify`.
- Use `--only-image` to just generate the PNG without IPC calls.

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)
- For overlay: Python 3, Pillow (`pip install Pillow`), optional cron

## What You’ll Learn

- Using the runtime IPC to add/modify layers dynamically
- Generating transparent overlays positioned on the canvas
- Combining static layers with a live-updating overlay

## Goal Of The Example

Blend a serene blossom scene with a readable, dynamic time display.

## Configuration Walkthrough

- Base wallpaper uses `hyprlax.toml` (workspace-driven parallax)
- `time_overlay.py` creates `time_overlay.png` at the screen resolution, then:
  - Adds a new layer via `hyprlax ctl add <image> z=5 opacity=1.0 shift_multiplier=0.0`
  - Updates the image path to refresh the layer without flicker
- Important vars (in script): `FONT_SIZE`, `FONT_SCALE`, `LAYER_Z`, `LAYER_SHIFT_MULTIPLIER`

## Customization Tips

- Increase `LAYER_SHIFT_MULTIPLIER` to give the overlay subtle parallax
- Adjust `--position` (`bottom-right`, `top-right`, `center`, `bottom-left`)
- Use a local TTF for crisper type on your system

## Image Placeholders

- Add a screenshot as `examples/cherry-blossom/screenshot.png`
- Add a layer diagram as `examples/cherry-blossom/layers.png`

