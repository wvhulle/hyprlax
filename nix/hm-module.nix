# Home-manager module for hyprlax — parallax wallpaper daemon
# https://github.com/sandwichfarm/hyprlax
{
  config,
  lib,
  pkgs,
  ...
}:

let
  cfg = config.services.hyprlax;

  tomlFormat = pkgs.formats.toml { };

  easingType = lib.types.enum [
    "linear"
    "quad"
    "cubic"
    "quart"
    "quint"
    "sine"
    "expo"
    "circ"
    "elastic"
    "back"
    "bounce"
    "snap"
  ];

  fitType = lib.types.enum [
    "stretch"
    "cover"
    "contain"
    "fit_width"
    "fit_height"
  ];

  overflowType = lib.types.enum [
    "repeat_edge"
    "clamp"
    "repeat"
    "tile"
    "repeat_x"
    "repeat_y"
    "none"
    "off"
  ];

  layerSubmodule = lib.types.submodule {
    freeformType = tomlFormat.type;
    options = {
      path = lib.mkOption {
        type = lib.types.str;
        description = "Path to the image file for this layer.";
      };
      shift_multiplier = lib.mkOption {
        type = lib.types.either lib.types.float (lib.types.attrsOf lib.types.float);
        default = 1.0;
        description = "Parallax speed multiplier. Float for uniform, or { x, y } for per-axis.";
      };
      scale = lib.mkOption {
        type = lib.types.float;
        default = 1.3;
        description = "Per-layer content scale. Values above 1.0 provide extra image area for the parallax effect.";
      };
      opacity = lib.mkOption {
        type = lib.types.float;
        default = 1.0;
        description = "Layer opacity (0.0–1.0).";
      };
      blur = lib.mkOption {
        type = lib.types.float;
        default = 0.0;
        description = "Blur amount.";
      };
      fit = lib.mkOption {
        type = fitType;
        default = "cover";
        description = "Content fit mode.";
      };
    };
  };

  mkWallpaperLayer = path: {
    inherit path;
    fit = "cover";
    shift_multiplier = 1.0;
    opacity = 1.0;
    blur = 0.0;
  };

  # Curated multi-layer scene presets, translated from the upstream example
  # configs shipped alongside this module in examples/<scene>/. Each value is
  # shaped like the `settings` submodule (a layer list plus global keys) and is
  # fed into services.hyprlax.settings at mkDefault priority, so any explicit
  # settings.* the user sets still overrides the preset.
  # https://github.com/sandwichfarm/hyprlax/tree/master/examples
  examplesRoot = ../examples;

  mkLayer = path: shift_multiplier: opacity: blur: {
    inherit
      path
      shift_multiplier
      opacity
      blur
      ;
  };

  sceneLayer = scene: file: mkLayer "${examplesRoot}/${scene}/${file}";

  # Multi-layer parallax packs downloaded from OpenGameArt, fetched at build
  # time rather than vendored into the repo. Each is referenced only by its own
  # preset, so Nix realises the download lazily — only when that preset is
  # selected. Attribution is required by the licenses below; cite it if you
  # redistribute the images themselves.
  forestPack = pkgs.fetchzip {
    # "Large forest background" by Julien Jorge — CC-BY-SA 3.0, 5120x2880.
    # https://opengameart.org/content/large-forest-background
    url = "https://opengameart.org/sites/default/files/forest-background.zip";
    hash = "sha256-2Fw6Gd3HP/pE4tqlES7wXm5VV4yBDTJJ5/+W32Li7aQ=";
    stripRoot = true;
  };

  horizonPack = pkgs.fetchzip {
    # "Horizontal 2D Backgrounds" by CraftPix.net — OGA-BY 3.0, 1920x1080.
    # Four scenes (game_background_1..4), each split into layers/.
    # https://opengameart.org/content/horizontal-2d-backgrounds
    url = "https://opengameart.org/sites/default/files/Horizontal-2D-BG-PNG.zip";
    hash = "sha256-qq/cvCm/KEVooZOdVoMF2src3XI/pDRy2Fhb/vyDsRc=";
    stripRoot = false;
  };

  seaviewPack = pkgs.fetchzip {
    # "Background Seaview Parallax" by tigitalart — CC0, native 3840x2160,
    # horizontally seamless. https://opengameart.org/content/background-seaview-parallax
    url = "https://opengameart.org/sites/default/files/seaview_background.zip";
    hash = "sha256-grqNzkRYcuL7SgG4f1kiVy3Qwea0eBrW55mQNAl5Bb4=";
    stripRoot = true;
  };

  # Layer from a fetched pack: opaque, sharp (opacity 1.0, blur 0.0); scale/fit
  # fall back to the module defaults (1.3 overscan, cover) which keep edges off
  # screen across workspaces.
  packLayer =
    pack: file: sm:
    mkLayer "${pack}/${file}" sm 1.0 0.0;

  # Like packLayer, but for horizontally seamless art: tile on X (overflow
  # repeat_x) at native scale so it stays pixel-perfect and never shows an edge,
  # however far it shifts.
  seamlessLayer =
    pack: file: sm:
    (packLayer pack file sm)
    // {
      scale = 1.0;
      overflow = "repeat_x";
    };

  presets = {
    mountains = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      shift = 200;
      layers = [
        (sceneLayer "mountains" "layer0_sky.png" 0.0 1.0 0.0)
        (sceneLayer "mountains" "layer1_far_mountains.png" 0.2 1.0 4.0)
        (sceneLayer "mountains" "layer2_clouds.png" 0.3 0.7 2.0)
        (sceneLayer "mountains" "layer3_mid_mountains.png" 0.5 1.0 2.0)
        (sceneLayer "mountains" "layer4_trees.png" 0.8 1.0 0.5)
        (sceneLayer "mountains" "layer5_foreground.png" 1.0 1.0 0.0)
      ];
    };

    city = {
      animation = {
        duration = 1.2;
        easing = "expo";
      };
      shift = 180;
      layers = [
        (sceneLayer "city" "layer0_sky.png" 0.0 1.0 0.0)
        (sceneLayer "city" "layer1_stars.png" 0.1 0.8 0.0)
        (sceneLayer "city" "layer2_far_skyline.png" 0.3 1.0 3.5)
        (sceneLayer "city" "layer3_mid_skyline.png" 0.5 1.0 2.0)
        (sceneLayer "city" "layer4_near_skyline.png" 0.8 1.0 0.8)
        (sceneLayer "city" "layer5_street.png" 1.0 1.0 0.0)
      ];
    };

    abstract = {
      animation = {
        duration = 2.0;
        easing = "sine";
      };
      shift = 250;
      layers = [
        (sceneLayer "abstract" "layer0_gradient.png" 0.0 1.0 0.0)
        (sceneLayer "abstract" "layer1_bg_shapes.png" 0.2 1.0 4.0)
        (sceneLayer "abstract" "layer2_mid_shapes.png" 0.5 0.8 2.5)
        (sceneLayer "abstract" "layer3_small_shapes.png" 0.8 0.7 1.0)
        (sceneLayer "abstract" "layer4_foreground.png" 1.2 0.6 0.0)
      ];
    };

    space = {
      animation = {
        duration = 4.0;
        easing = "expo";
      };
      shift = 200;
      fps = 60;
      layers = [
        (sceneLayer "space" "bkgd_0.png" 0.1 1.0 4.0)
        (sceneLayer "space" "bkgd_1.png" 0.2 1.0 4.0)
        (sceneLayer "space" "bkgd_2.png" 0.3 1.0 4.0)
        (sceneLayer "space" "bkgd_3.png" 0.4 1.0 4.0)
        (sceneLayer "space" "bkgd_4.png" 0.5 1.0 4.0)
        (sceneLayer "space" "bkgd_5.png" 0.6 1.0 4.0)
        (sceneLayer "space" "bkgd_6.png" 0.7 1.0 4.0)
        (sceneLayer "space" "bkgd_7.png" 0.8 1.0 1.0)
      ];
    };

    multi = {
      animation = {
        duration = 4.0;
        easing = "expo";
      };
      shift = 200;
      fps = 60;
      layers = [
        (sceneLayer "multi" "sky-day.png" 0.1 1.0 4.0)
        (sceneLayer "multi" "mountain.png" 0.25 1.0 2.0)
        (sceneLayer "multi" "foreground.png" 0.4 1.0 0.0)
      ];
    };

    pixel-city = {
      fps = 144;
      vsync = false;
      animation = {
        duration = 4.0;
        easing = "expo";
      };
      render = {
        tile = {
          x = true;
          y = false;
        };
        content_scale = 1.0;
      };
      parallax = {
        input = "workspace";
        shift_percent = 5.0;
      };
      # Pixel art tiles at content_scale 1.0 — override the module's default
      # 1.3 overscan per layer so the tiling stays pixel-aligned (matching
      # examples/pixel-city/parallax.toml render.content_scale = 1.0).
      layers = map (l: l // { scale = 1.0; }) [
        (sceneLayer "pixel-city" "1.png" 0.1 1.0 0.0)
        (sceneLayer "pixel-city" "2.png" 0.2 1.0 2.0)
        (sceneLayer "pixel-city" "3.png" 0.3 1.0 1.1)
        (sceneLayer "pixel-city" "4.png" 0.4 1.0 0.3)
        (sceneLayer "pixel-city" "5.png" 0.5 1.0 0.0)
        (sceneLayer "pixel-city" "6.png" 1.0 1.0 0.0)
      ];
    };

    # --- Downloaded OpenGameArt scenes (high-res; fetched lazily) ---

    # "Background Seaview Parallax" by tigitalart, CC0, native 3840x2160 (4K),
    # horizontally seamless. https://opengameart.org/content/background-seaview-parallax
    seaview = {
      animation = {
        duration = 1.2;
        easing = "expo";
      };
      parallax.shift_percent = 1.5;
      layers = [
        (seamlessLayer seaviewPack "seaview_sky.png" 0.0)
        (seamlessLayer seaviewPack "seaview_clouds.png" 0.1)
        (seamlessLayer seaviewPack "seaview_hills.png" 0.25)
        (seamlessLayer seaviewPack "seaview_sea.png" 0.45)
        (seamlessLayer seaviewPack "seaview_foreground.png" 1.0)
      ];
    };

    # "Large forest background" by Julien Jorge, CC-BY-SA 3.0, 5120x2880 (5K).
    # https://opengameart.org/content/large-forest-background
    forest = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      parallax.shift_percent = 1.0;
      layers = [
        (packLayer forestPack "far-background.png" 0.2)
        (packLayer forestPack "near-background.png" 1.0)
      ];
    };

    # The four "Horizontal 2D Backgrounds" scenes by CraftPix.net, OGA-BY 3.0,
    # 1920x1080 flat-vector art. https://opengameart.org/content/horizontal-2d-backgrounds
    lake = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      parallax.shift_percent = 1.0;
      layers = [
        (packLayer horizonPack "game_background_1/layers/sky.png" 0.0)
        (packLayer horizonPack "game_background_1/layers/clouds_1.png" 0.1)
        (packLayer horizonPack "game_background_1/layers/clouds_2.png" 0.15)
        (packLayer horizonPack "game_background_1/layers/clouds_3.png" 0.2)
        (packLayer horizonPack "game_background_1/layers/clouds_4.png" 0.25)
        (packLayer horizonPack "game_background_1/layers/rocks_1.png" 0.6)
        (packLayer horizonPack "game_background_1/layers/rocks_2.png" 1.0)
      ];
    };

    hills = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      parallax.shift_percent = 1.0;
      layers = [
        (packLayer horizonPack "game_background_2/layers/sky.png" 0.0)
        (packLayer horizonPack "game_background_2/layers/clouds_1.png" 0.1)
        (packLayer horizonPack "game_background_2/layers/clouds_2.png" 0.15)
        (packLayer horizonPack "game_background_2/layers/clouds_3.png" 0.2)
        (packLayer horizonPack "game_background_2/layers/birds.png" 0.3)
        (packLayer horizonPack "game_background_2/layers/rocks_1.png" 0.5)
        (packLayer horizonPack "game_background_2/layers/rocks_2.png" 0.65)
        (packLayer horizonPack "game_background_2/layers/rocks_3.png" 0.8)
        (packLayer horizonPack "game_background_2/layers/pines.png" 1.0)
      ];
    };

    night-forest = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      parallax.shift_percent = 1.0;
      layers = [
        (packLayer horizonPack "game_background_3/layers/sky.png" 0.0)
        (packLayer horizonPack "game_background_3/layers/clouds_1.png" 0.1)
        (packLayer horizonPack "game_background_3/layers/clouds_2.png" 0.15)
        (packLayer horizonPack "game_background_3/layers/rocks.png" 0.4)
        (packLayer horizonPack "game_background_3/layers/ground_1.png" 0.6)
        (packLayer horizonPack "game_background_3/layers/ground_2.png" 0.75)
        (packLayer horizonPack "game_background_3/layers/ground_3.png" 0.9)
        (packLayer horizonPack "game_background_3/layers/plant.png" 1.0)
      ];
    };

    waterfall = {
      animation = {
        duration = 1.5;
        easing = "sine";
      };
      parallax.shift_percent = 1.0;
      layers = [
        (packLayer horizonPack "game_background_4/layers/sky.png" 0.0)
        (packLayer horizonPack "game_background_4/layers/clouds_1.png" 0.1)
        (packLayer horizonPack "game_background_4/layers/clouds_2.png" 0.15)
        (packLayer horizonPack "game_background_4/layers/rocks.png" 0.5)
        (packLayer horizonPack "game_background_4/layers/ground.png" 1.0)
      ];
    };
  };

  settingsSubmodule = lib.types.submodule {
    freeformType = tomlFormat.type;
    options = {
      fps = lib.mkOption {
        type = lib.types.nullOr (lib.types.ints.between 30 240);
        default = null;
        description = "Target frame rate (30–240). Null uses hyprlax default (60).";
      };
      vsync = lib.mkOption {
        type = lib.types.nullOr lib.types.bool;
        default = null;
        description = "Enable vertical sync.";
      };
      debug = lib.mkOption {
        type = lib.types.nullOr lib.types.bool;
        default = null;
        description = "Enable debug output.";
      };
      scale = lib.mkOption {
        type = lib.types.nullOr lib.types.float;
        default = null;
        description = "Global content scale factor. Per-layer scale overrides this.";
      };
      idle_poll_rate = lib.mkOption {
        type = lib.types.nullOr lib.types.float;
        default = null;
        description = "Polling rate when idle in Hz (0.1–10.0).";
      };
      animation = lib.mkOption {
        type = lib.types.submodule {
          freeformType = tomlFormat.type;
          options = {
            duration = lib.mkOption {
              type = lib.types.nullOr lib.types.float;
              default = null;
              description = "Animation duration in seconds.";
            };
            easing = lib.mkOption {
              type = lib.types.nullOr easingType;
              default = null;
              description = "Animation easing function.";
            };
          };
        };
        default = { };
        description = "Animation settings.";
      };
      parallax = lib.mkOption {
        type = lib.types.submodule {
          freeformType = tomlFormat.type;
          options = {
            shift_percent = lib.mkOption {
              type = lib.types.nullOr lib.types.float;
              default = null;
              description = "Parallax shift as percentage of screen width (0–100).";
            };
            input = lib.mkOption {
              type = lib.types.nullOr (lib.types.either lib.types.str (lib.types.listOf lib.types.str));
              default = null;
              description = ''
                Parallax input sources. A string like "workspace" or "workspace,cursor:0.3",
                or a list like [ "workspace" "cursor:0.3" ].
              '';
            };
          };
        };
        default = { };
        description = "Parallax behavior settings.";
      };
      input = lib.mkOption {
        type = lib.types.submodule {
          freeformType = tomlFormat.type;
        };
        default = { };
        description = "Input source configuration (cursor sensitivity, deadzone, etc.).";
      };
      render = lib.mkOption {
        type = lib.types.submodule {
          freeformType = tomlFormat.type;
          options = {
            overflow = lib.mkOption {
              type = lib.types.nullOr overflowType;
              default = null;
              description = "Texture overflow/wrapping mode.";
            };
            accumulate = lib.mkOption {
              type = lib.types.nullOr lib.types.bool;
              default = null;
              description = "Enable motion trail accumulation.";
            };
          };
        };
        default = { };
        description = "Rendering settings.";
      };
      layers = lib.mkOption {
        type = lib.types.listOf layerSubmodule;
        default = [ ];
        description = "Layer definitions rendered back to front.";
      };
    };
  };

  # Remove null values recursively so they don't appear in TOML output
  filterNulls =
    attrs:
    lib.pipe attrs [
      (lib.filterAttrs (_: v: v != null))
      (lib.mapAttrs (_: v: if lib.isAttrs v then filterNulls v else v))
      (lib.filterAttrs (_: v: !(lib.isAttrs v && v == { })))
    ];

  mkConfigFile =
    layers:
    let
      settings = filterNulls cfg.settings;
      # Separate layers from the rest of settings
      settingsWithoutLayers = builtins.removeAttrs settings [ "layers" ];
      global = settingsWithoutLayers // {
        inherit layers;
      };
    in
    tomlFormat.generate "hyprlax.toml" { inherit global; };

  lightLayers =
    lib.optionals (cfg.wallpaper != null) [ (mkWallpaperLayer cfg.wallpaper) ] ++ cfg.settings.layers;

  darkLayersFinal =
    lib.optionals (cfg.darkWallpaper != null) [ (mkWallpaperLayer cfg.darkWallpaper) ]
    ++ (if cfg.darkLayers != null then cfg.darkLayers else [ ]);

  hasDarkMode = cfg.darkWallpaper != null || cfg.darkLayers != null;

  configFile = mkConfigFile lightLayers;

  configPath =
    if hasDarkMode then "${config.xdg.configHome}/hyprlax/hyprlax.toml" else "${configFile}";

  execStart =
    "${lib.getExe cfg.package}"
    + " --config ${configPath}"
    + lib.optionalString (cfg.compositor != null) " --compositor ${cfg.compositor}";
in
{
  options.services.hyprlax = {
    enable = lib.mkEnableOption "hyprlax parallax wallpaper daemon";

    package = lib.mkPackageOption pkgs "hyprlax" { };

    compositor = lib.mkOption {
      type = lib.types.nullOr (
        lib.types.enum [
          "auto"
          "hyprland"
          "sway"
          "niri"
          "river"
          "generic"
        ]
      );
      default = null;
      description = "Compositor backend. Null lets hyprlax auto-detect.";
    };

    preset = lib.mkOption {
      type = lib.types.nullOr (lib.types.enum (lib.attrNames presets));
      default = null;
      example = "mountains";
      description = ''
        Bundled multi-layer scene. Supplies layers and animation defaults from
        the matching examples/<scene>/ config at mkDefault priority, so any
        explicit settings.* (including settings.layers) overrides them. A set
        wallpaper still prepends one cover layer on top of the scene.
      '';
    };

    wallpaper = lib.mkOption {
      type = lib.types.nullOr lib.types.str;
      default = null;
      description = ''
        Path to a wallpaper image. Generates a single layer with cover fit,
        prepended to any layers defined in settings.layers.
      '';
    };

    darkWallpaper = lib.mkOption {
      type = lib.types.nullOr lib.types.str;
      default = null;
      description = ''
        Dark-mode wallpaper image. When set, darkman switches between
        light and dark hyprlax configs at runtime.
      '';
    };

    darkLayers = lib.mkOption {
      type = lib.types.nullOr (lib.types.listOf layerSubmodule);
      default = null;
      description = "Dark-mode layer overrides. Used instead of settings.layers in dark config.";
    };

    settings = lib.mkOption {
      type = settingsSubmodule;
      default = { };
      description = ''
        Hyprlax configuration. Maps directly to the [global] section of hyprlax.toml.
        Any key supported by hyprlax can be set here via the freeform type.
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    services.hyprlax.settings = lib.mkIf (cfg.preset != null) (lib.mkDefault presets.${cfg.preset});

    home.packages = [ cfg.package ];

    xdg.configFile = lib.mkMerge [
      (lib.mkIf (!hasDarkMode) {
        "hyprlax/hyprlax.toml".source = configFile;
      })
      (lib.mkIf hasDarkMode {
        "hyprlax/hyprlax-light.toml".source = mkConfigFile lightLayers;
        "hyprlax/hyprlax-dark.toml".source = mkConfigFile darkLayersFinal;
      })
    ];

    colorScheme.apps = lib.mkIf hasDarkMode {
      hyprlax = {
        dark = "dark";
        light = "light";
        configFile = "hyprlax/hyprlax.toml";
        postSwitch = "systemctl --user restart hyprlax.service";
        activation = true;
      };
    };

    systemd.user.services.hyprlax = {
      Unit = {
        Description = "hyprlax parallax wallpaper daemon";
        After = [ "graphical-session.target" ];
        PartOf = [ "graphical-session.target" ];
        X-Restart-Triggers =
          if hasDarkMode then
            [
              "${mkConfigFile lightLayers}"
              "${mkConfigFile darkLayersFinal}"
            ]
          else
            [ "${configFile}" ];
      };
      Service = {
        Type = "simple";
        ExecStart = execStart;
        Restart = "on-failure";
        RestartSec = 3;
      };
      Install = {
        WantedBy = [ "graphical-session.target" ];
      };
    };
  };
}
