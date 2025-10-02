# Mountain Scene

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: nature](https://img.shields.io/badge/purpose-nature-22c55e?style=flat)](#)
[![Config: legacy](https://img.shields.io/badge/config-legacy_conf-9e9e9e?style=flat)](../../docs/configuration/legacy-format.md)

A natural landscape with layered mountains, clouds, and trees.

![Screenshot placeholder: mountain scene with parallax layers](./screenshot.png)
![Layer diagram placeholder: sky → far → clouds → mid → trees → foreground](./layers.png)

## How To Use

- Run with the included legacy configuration:
  ```bash
  ./hyprlax --config examples/mountains/parallax.conf
  ```
- Optional: convert to TOML once:
  ```bash
  ./hyprlax ctl convert-config examples/mountains/parallax.conf examples/mountains/hyprlax.toml --yes
  ./hyprlax --config examples/mountains/hyprlax.toml
  ```

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Using blur to simulate distance/fog
- Ordering layers to match real-world scale
- Choosing shift multipliers for subtle vs dramatic motion

## Goal Of The Example

Create a calm, natural parallax wallpaper with convincing depth cues.

## Configuration Walkthrough

- Layers (back → front): sky (0.0), far mountains (0.2, blur 4.0),
  clouds (0.3, opacity 0.7), mid mountains (0.5, blur 2.0),
  trees (0.8, blur 0.5), foreground (1.0)
- Global animation (legacy): `duration 1.5`, `shift 200`, `easing sine`

## Customization Tips

- Increase cloud opacity for moody weather
- Try `easing expo` for snappier transitions
- Reduce `shift` for subtle motion on large monitors

## Image Placeholders

- Add a screenshot as `examples/mountains/screenshot.png`
- Add a layer diagram as `examples/mountains/layers.png`
