# Abstract Geometric

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: abstract](https://img.shields.io/badge/purpose-abstract-8a2be2?style=flat)](#)
[![Config: legacy](https://img.shields.io/badge/config-legacy_conf-9e9e9e?style=flat)](../../docs/configuration/legacy-format.md)

A colorful abstract composition with floating geometric shapes and soft depth.

![Screenshot placeholder: abstract scene with layered shapes](./screenshot.png)
![Layer diagram placeholder: layer order and shift multipliers](./layers.png)

## How To Use

- Run with the included legacy configuration:
  ```bash
  ./hyprlax --config examples/abstract/parallax.conf
  ```
- Optional: convert to TOML once for easier tweaking:
  ```bash
  ./hyprlax ctl convert-config examples/abstract/parallax.conf examples/abstract/hyprlax.toml --yes
  ./hyprlax --config examples/abstract/hyprlax.toml
  ```

## Dependencies

- hyprlax (built or installed)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Balancing blur and opacity to suggest depth
- Arranging background/mid/foreground parallax intensities
- Using shift multipliers to control motion strength

## Goal Of The Example

Create an eye-catching abstract wallpaper that feels deep and dreamy using parallax and blur.

## Configuration Walkthrough

- Layers (back → front):
  1) `layer0_gradient.png` — static gradient background (shift 0.0)
  2) `layer1_bg_shapes.png` — slow, blurred shapes (shift 0.2, blur 4.0)
  3) `layer2_mid_shapes.png` — moderate movement (shift 0.5, blur 2.5)
  4) `layer3_small_shapes.png` — faster details (shift 0.8, blur 1.0)
  5) `layer4_foreground.png` — sharp foreground (shift 1.2, blur 0.0)
- Global animation (legacy in `parallax.conf`):
  - `duration 2.0` — smooth transition time
  - `shift 250` — max workspace offset in pixels
  - `easing sine` — gentle acceleration curve

## Customization Tips

- Increase background blur for a dreamy atmosphere
- Tone down foreground opacity to blend layers better
- Try `easing expo` for a snappier, more modern feel

## Image Placeholders

- Add a screenshot as `examples/abstract/screenshot.png`
- Add a layer diagram as `examples/abstract/layers.png`
