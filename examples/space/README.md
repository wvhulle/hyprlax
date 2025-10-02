# Deep Space Parallax

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: space](https://img.shields.io/badge/purpose-space-6366f1?style=flat)](#)
[![Config: legacy](https://img.shields.io/badge/config-legacy_conf-9e9e9e?style=flat)](../../docs/configuration/legacy-format.md)

Layered starfields and nebulae with strong depth cues.

![Screenshot placeholder: deep space starfields with parallax](./screenshot.png)
![Layer diagram placeholder: bkgd_0..7 ordered back → front](./layers.png)

## How To Use

```bash
./hyprlax --config examples/space/parallax.conf
```

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Multi-layer depth with similar visual elements
- Using heavier blur on distant backgrounds
- Keeping frame time low with many layers

## Goal Of The Example

Create a dramatic parallax starfield ideal for ultrawide/4K desktops.

## Configuration Walkthrough

- Eight `bkgd_*.png` layers with increasing `shift_multiplier` 0.1 … 0.8
- Strong blur on most layers to emphasize distance
- Global (legacy): `duration 4`, `shift 200`, `easing expo`, `fps 60`

## Image Placeholders

- Add a screenshot as `examples/space/screenshot.png`
- Add a layer diagram as `examples/space/layers.png`

