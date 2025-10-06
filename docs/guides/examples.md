# Examples Gallery

A curated list of ready-to-run wallpaper examples bundled with hyprlax. Each example includes a README with usage, dependencies, and a short tutorial.

> Paths below are relative to the repository root. Replace with your local paths if installed elsewhere.

## Mountains
- Path: `examples/mountains/`
- Config: `parallax.conf` (legacy)
- Run (legacy): `./hyprlax --config examples/mountains/parallax.conf`
- TOML conversion:
  - `hyprlax ctl convert-config examples/mountains/parallax.conf examples/mountains/hyprlax.toml --yes`
  - Run (TOML): `./hyprlax --config examples/mountains/hyprlax.toml`
- Migration: See [Migration Guide](../configuration/migration-guide.md)
- Docs: [README.md](../../examples/mountains/README.md)

## City
- Path: `examples/city/`
- Config: `parallax.conf` (legacy)
- Run (legacy): `./hyprlax --config examples/city/parallax.conf`
- TOML conversion:
  - `hyprlax ctl convert-config examples/city/parallax.conf examples/city/hyprlax.toml --yes`
  - Run (TOML): `./hyprlax --config examples/city/hyprlax.toml`
- Migration: See [Migration Guide](../configuration/migration-guide.md)
- Docs: [README.md](../../examples/city/README.md)

## Abstract Geometric
- Path: `examples/abstract/`
- Config: `parallax.conf` (legacy)
- Run (legacy): `./hyprlax --config examples/abstract/parallax.conf`
- TOML conversion:
  - `hyprlax ctl convert-config examples/abstract/parallax.conf examples/abstract/hyprlax.toml --yes`
  - Run (TOML): `./hyprlax --config examples/abstract/hyprlax.toml`
- Migration: See [Migration Guide](../configuration/migration-guide.md)
- Docs: [README.md](../../examples/abstract/README.md)

## Pixel City (Pixel Art)
- Path: `examples/pixel-city/`
- Config: `parallax.toml` (preferred) or `parallax.conf`
- Run: `./hyprlax --config examples/pixel-city/parallax.toml`
- Docs: [README.md](../../examples/pixel-city/README.md)

## Pixel City (Advanced)
- Path: `examples/pixel-city-advanced/`
- Purpose: systemd/user-service packaging scaffold (use with Pixel City assets)
- Docs: [README.md](../../examples/pixel-city-advanced/README.md)

## Space
- Path: `examples/space/`
- Config: `parallax.conf` (legacy)
- Run (legacy): `./hyprlax --config examples/space/parallax.conf`
- TOML conversion:
  - `hyprlax ctl convert-config examples/space/parallax.conf examples/space/hyprlax.toml --yes`
  - Run (TOML): `./hyprlax --config examples/space/hyprlax.toml`
- Migration: See [Migration Guide](../configuration/migration-guide.md)
- Docs: [README.md](../../examples/space/README.md)

## Multi-Layer Basics
- Path: `examples/multi/`
- Config: `parallax.conf` (legacy)
- Run (legacy): `./hyprlax --config examples/multi/parallax.conf`
- TOML conversion:
  - `hyprlax ctl convert-config examples/multi/parallax.conf examples/multi/hyprlax.toml --yes`
  - Run (TOML): `./hyprlax --config examples/multi/hyprlax.toml`
- Migration: See [Migration Guide](../configuration/migration-guide.md)
- Docs: [README.md](../../examples/multi/README.md)

## Mouse Parallax (Cursor-Driven)
- Path: `examples/mouse-parallax/`
- Config: `hyprlax.toml` (TOML)
- Run: `./hyprlax --config examples/mouse-parallax/hyprlax.toml`
- Docs: [README.md](../../examples/mouse-parallax/README.md)

## Cherry Blossom (Overlay)
- Path: `examples/cherry-blossom/`
- Config: `hyprlax.toml` (TOML)
- Run: `./hyprlax --config examples/cherry-blossom/hyprlax.toml`
- Overlay: `python3 examples/cherry-blossom/time_overlay.py --once`
- Docs: [README.md](../../examples/cherry-blossom/README.md)

---

Tips
- Convert legacy configs to TOML once via:
  - `hyprlax ctl convert-config <in.conf> <out.toml> --yes`
- Examples are great templates — copy a directory and swap in your own art
- For more options, see the [TOML reference](../configuration/toml-reference.md) and [guides](./)
