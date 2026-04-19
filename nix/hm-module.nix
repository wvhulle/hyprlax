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
              type = lib.types.nullOr (
                lib.types.either lib.types.str (lib.types.listOf lib.types.str)
              );
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
      (lib.mapAttrs (
        _: v:
        if lib.isAttrs v then filterNulls v else v
      ))
      (lib.filterAttrs (_: v: !(lib.isAttrs v && v == { })))
    ];

  mkConfigFile =
    layers:
    let
      settings = filterNulls cfg.settings;
      # Separate layers from the rest of settings
      settingsWithoutLayers = builtins.removeAttrs settings [ "layers" ];
      global = settingsWithoutLayers // { inherit layers; };
    in
    tomlFormat.generate "hyprlax.toml" { inherit global; };

  lightLayers =
    lib.optionals (cfg.wallpaper != null) [ (mkWallpaperLayer cfg.wallpaper) ]
    ++ cfg.settings.layers;

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

    services.dual-scheme-app.apps = lib.mkIf hasDarkMode {
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
