# Pixel City (Advanced)

[![Engine: hyprlax](https://img.shields.io/badge/engine-hyprlax-3fb950?style=flat)](../../README.md)
[![Compositors](https://img.shields.io/badge/compositors-hyprland%20%7C%20sway%20%7C%20wayfire%20%7C%20niri%20%7C%20river-1f6feb?style=flat)](../../docs/getting-started/compatibility.md)
[![Purpose: packaging](https://img.shields.io/badge/purpose-packaging-ef4444?style=flat)](#)
[![Config: TOML](https://img.shields.io/badge/config-TOML-4c1?style=flat)](../../docs/configuration/toml-reference.md)

This directory demonstrates advanced packaging/deployment patterns (e.g., systemd integration)
for running a hyprlax example persistently. Use together with assets/config from
`examples/pixel-city/`.

![Screenshot placeholder: pixel city running as a service](./screenshot.png)
![Diagram placeholder: service flow for hyprlax](./layers.png)

## How To Use

1) Verify the base example works:
```bash
./hyprlax --config examples/pixel-city/parallax.toml
```

2) Create a user systemd unit (example scaffold):

Create `~/.config/systemd/user/hyprlax-pixel-city.service` with content:
```ini
[Unit]
Description=Hyprlax Pixel City Wallpaper
After=graphical-session.target

[Service]
Type=simple
ExecStart=%h/Develop/hyprlax/hyprlax --config %h/Develop/hyprlax/examples/pixel-city/parallax.toml
Restart=on-failure

[Install]
WantedBy=default.target
```

Enable and start:
```bash
systemctl --user daemon-reload
systemctl --user enable --now hyprlax-pixel-city.service
```

3) Customize paths and environment as needed for your setup.

## Dependencies

- hyprlax (built or installed)
- systemd (user services)
- Wayland compositor (Hyprland, Sway, Wayfire, Niri, River, …)

## What You’ll Learn

- Running hyprlax as a long-lived user service
- Controlling hyprlax via IPC while managed by systemd
- Clean startup/shutdown integration with your desktop session

## Goal Of The Example

Demonstrate a clean, reproducible way to keep hyprlax running in the background.

## Image Placeholders

- Add a screenshot as `examples/pixel-city-advanced/screenshot.png`
- Add a diagram as `examples/pixel-city-advanced/layers.png`

