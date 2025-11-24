import os
import json

# --- Tool Config ---
def load_tool_config():
    """Load tool configuration from tool_config.json"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    config_path = os.path.join(script_dir, "tool_config.json")
    default_config = {
        "project_root": os.path.dirname(script_dir),
        "resources_folder": "Resources",
        "maps_folder": "Maps",
        "animations_folder": "Animations"
    }
    if os.path.exists(config_path):
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
                if "project_root" in config:
                    config["project_root"] = config["project_root"].replace("\\", "/")
                return config
        except Exception as e:
            print(f"Warning: Failed to load config: {e}")
    return default_config

def get_relative_path(absolute_path, project_root):
    """Get relative path from project root"""
    abs_path = os.path.abspath(absolute_path).replace("\\", "/")
    proj_root = os.path.abspath(project_root).replace("\\", "/")
    if abs_path.startswith(proj_root):
        rel_path = abs_path[len(proj_root):].lstrip("/")
        return rel_path
    return os.path.basename(absolute_path)

def resolve_relative_path(relative_path, project_root):
    """Resolve relative path to absolute path using project root"""
    if os.path.isabs(relative_path):
        return relative_path
    return os.path.join(project_root, relative_path).replace("\\", "/")

TOOL_CONFIG = load_tool_config()

# --- Configuration ---
WINDOW_WIDTH = 1400
WINDOW_HEIGHT = 900
BACKGROUND_COLOR = (40, 40, 45)
PANEL_COLOR = (50, 50, 55)
PANEL_BORDER_COLOR = (70, 70, 75)
TEXT_COLOR = (220, 220, 220)
HIGHLIGHT_COLOR = (100, 150, 255)
GRID_COLOR = (60, 60, 65)
GRID_SIZE = 32

# Panel sizes
LEFT_PANEL_WIDTH = 250
RIGHT_PANEL_WIDTH = 280
TOP_BAR_HEIGHT = 30
BOTTOM_BAR_HEIGHT = 25

# --- Default Schema ---
DEFAULT_SCHEMA = {
    "version": "1.0",
    "collider_types": {
        "box": {
            "display_name": "Box Collider",
            "color": [0, 255, 0],
            "fields": [
                {"name": "layer", "type": "enum", "options": ["ground", "wall", "platform"], "default": "ground"},
                {"name": "one_way", "type": "bool", "default": False}
            ]
        },
        "slope_left": {
            "display_name": "Slope Left",
            "color": [255, 0, 255],
            "fields": [
                {"name": "angle", "type": "int", "min": 15, "max": 60, "default": 45}
            ]
        },
        "slope_right": {
            "display_name": "Slope Right",
            "color": [255, 100, 255],
            "fields": [
                {"name": "angle", "type": "int", "min": 15, "max": 60, "default": 45}
            ]
        },
        "trigger": {
            "display_name": "Trigger Zone",
            "color": [255, 255, 0],
            "fields": [
                {"name": "event_id", "type": "string", "default": ""},
                {"name": "once", "type": "bool", "default": True}
            ]
        },
        "damage": {
            "display_name": "Damage Zone",
            "color": [255, 0, 0],
            "fields": [
                {"name": "damage", "type": "int", "min": 0, "default": 10},
                {"name": "element", "type": "enum", "options": ["none", "fire", "ice", "poison"], "default": "none"}
            ]
        }
    },
    "tile_layers": [
        {"name": "bg_far", "depth": -20, "parallax": 0.3},
        {"name": "bg_near", "depth": -10, "parallax": 0.6},
        {"name": "main", "depth": 0, "parallax": 1.0},
        {"name": "fg", "depth": 10, "parallax": 1.0}
    ]
}
