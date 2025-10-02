# Multi-Layer Basics

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: tutorial](https://img.shields.io/badge/purpose-tutorial-10b981?style=flat)](#)
[![Config: legacy](https://img.shields.io/badge/config-legacy_conf-9e9e9e?style=flat)](../../docs/configuration/legacy-format.md)

Simple three-layer scene to illustrate core parallax concepts.

![Screenshot placeholder: basic multi-layer parallax](./screenshot.png)
![Layer diagram placeholder: sky → mountains → foreground](./layers.png)

## How To Use

```bash
./hyprlax --config examples/multi/parallax.conf
```

Optional: convert to TOML for further tweaking.

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Minimal setup for multi-layer parallax
- Picking shift multipliers across background/mid/foreground
- Global animation controls: `duration`, `shift`, `easing`, `fps`

## Goal Of The Example

Provide a compact, readable starting point for new configurations.

## Configuration Walkthrough

- `layer ./sky-day.png 0.1 1.0 4.0` — slow sky, heavy blur
- `layer ./mountain.png 0.25 1.0 2.0` — mid layer, medium blur
- `layer ./foreground.png 0.4 1.0 0.0` — foreground, sharp
- Global (legacy): `duration 4`, `shift 200`, `easing expo`, `fps 60`

## Image Placeholders

- Add a screenshot as `examples/multi/screenshot.png`
- Add a layer diagram as `examples/multi/layers.png`

