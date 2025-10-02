# City Skyline

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: cityscape](https://img.shields.io/badge/purpose-cityscape-0ea5e9?style=flat)](#)
[![Config: legacy](https://img.shields.io/badge/config-legacy_conf-9e9e9e?style=flat)](../../docs/configuration/legacy-format.md)

An urban nightscape with layered buildings, stars, and atmospheric depth.

![Screenshot placeholder: city skyline with parallax layers](./screenshot.png)
![Layer diagram placeholder: far/mid/near skyline, street](./layers.png)

## How To Use

- Run with the included legacy configuration:
  ```bash
  ./hyprlax --config examples/city/parallax.conf
  ```
- Optional: convert to TOML for easier tweaking:
  ```bash
  ./hyprlax ctl convert-config examples/city/parallax.conf examples/city/hyprlax.toml --yes
  ./hyprlax --config examples/city/hyprlax.toml
  ```

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Using blur to simulate atmospheric perspective
- Layer ordering for convincing depth
- Picking shift multipliers that match real-world scale

## Goal Of The Example

Deliver a cinematic city night scene with multiple skylines that move at different speeds.

## Configuration Walkthrough

- Layers (back → front):
  1) `layer0_sky.png` — static night sky (0.0)
  2) `layer1_stars.png` — very slow drift (0.1)
  3) `layer2_far_skyline.png` — heavy blur, slow (0.3)
  4) `layer3_mid_skyline.png` — moderate blur, medium (0.5)
  5) `layer4_near_skyline.png` — slight blur, faster (0.8)
  6) `layer5_street.png` — sharp, normal speed (1.0)
- Global animation (legacy): `duration 1.2`, `shift 180`, `easing expo`

## Customization Tips

- Increase star layer opacity for clearer skies
- Adjust skyline blur to taste based on your display size
- Try `duration 1.5` for a slower, moodier transition

## Image Placeholders

- Add a screenshot as `examples/city/screenshot.png`
- Add a layer diagram as `examples/city/layers.png`
