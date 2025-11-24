import json
import cv2
from map_tool_config import DEFAULT_SCHEMA


# --- Auto Detect Function ---
def auto_detect_tiles(image_path):
    """Auto detect tile regions from tileset image"""
    try:
        img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
        if img is None or len(img.shape) < 3 or img.shape[2] < 4:
            return []
        alpha = img[:, :, 3]
        _, thresh = cv2.threshold(alpha, 0, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        tiles = []
        for c in contours:
            if cv2.contourArea(c) > 16:
                x, y, w, h = cv2.boundingRect(c)
                tiles.append([x, y, w, h])
        tiles.sort(key=lambda r: (r[1] // 32, r[0]))
        return tiles
    except Exception as e:
        print(f"Auto-detect error: {e}")
        return []


# --- Data Classes ---
class Tile:
    def __init__(self, tileset_idx, src_rect, pos, layer="main"):
        self.tileset_idx = tileset_idx
        self.src_rect = src_rect  # [x, y, w, h] in tileset
        self.pos = pos  # [x, y] in map
        self.layer = layer


class Collider:
    def __init__(self, collider_type, rect, data=None):
        self.type = collider_type
        self.rect = rect  # [x, y, w, h]
        self.data = data or {}


class Prop:
    def __init__(self, prop_type, tileset_idx, src_rect, pos, data=None):
        self.type = prop_type  # prop type from schema (e.g., "destructible")
        self.tileset_idx = tileset_idx
        self.src_rect = src_rect  # [x, y, w, h] in tileset
        self.pos = pos  # [x, y] in map
        self.data = data or {}


class CustomObject:
    def __init__(self, obj_type, pos, data=None):
        self.type = obj_type
        self.pos = pos  # [x, y]
        self.data = data or {}


class MapData:
    def __init__(self):
        self.name = "untitled"
        self.size = [1920, 1080]
        self.tilesets = []  # list of paths
        self.tiles = []  # list of Tile
        self.colliders = []  # list of Collider
        self.props = []  # list of Prop
        self.objects = []  # list of CustomObject
        self.schema = DEFAULT_SCHEMA.copy()

    def to_dict(self):
        return {
            "schema_version": self.schema.get("version", "1.0"),
            "map": {
                "name": self.name,
                "size": self.size,
                "tilesets": self.tilesets,
                "layers": self._tiles_by_layer(),
                "colliders": [{"type": c.type, "rect": c.rect, "data": c.data} for c in self.colliders]
            }
        }

    def _tiles_by_layer(self):
        layers = {}
        for tile in self.tiles:
            if tile.layer not in layers:
                layers[tile.layer] = []
            layers[tile.layer].append({
                "tileset": tile.tileset_idx,
                "src": tile.src_rect,
                "pos": tile.pos
            })

        result = []
        for layer_def in self.schema.get("tile_layers", []):
            name = layer_def["name"]
            result.append({
                "name": name,
                "depth": layer_def.get("depth", 0),
                "parallax": layer_def.get("parallax", 1.0),
                "tiles": layers.get(name, [])
            })
        return result

    def save(self, path):
        with open(path, 'w') as f:
            f.write('{\n')
            f.write(f'    "schema_version": "{self.schema.get("version", "1.0")}",\n')
            f.write('    "map": {\n')
            f.write(f'        "name": "{self.name}",\n')
            f.write(f'        "size": {json.dumps(self.size)},\n')
            f.write(f'        "tilesets": {json.dumps(self.tilesets)},\n')

            # Layers
            f.write('        "layers": [\n')
            layers_data = self._tiles_by_layer()
            for i, layer in enumerate(layers_data):
                comma = "," if i < len(layers_data) - 1 else ""
                f.write(f'            {{"name": "{layer["name"]}", "depth": {layer["depth"]}, "parallax": {layer["parallax"]}, "tiles": [\n')
                for j, tile in enumerate(layer["tiles"]):
                    tile_comma = "," if j < len(layer["tiles"]) - 1 else ""
                    f.write(f'                {json.dumps(tile)}{tile_comma}\n')
                f.write(f'            ]}}{comma}\n')
            f.write('        ],\n')

            # Colliders
            f.write('        "colliders": [\n')
            for i, c in enumerate(self.colliders):
                comma = "," if i < len(self.colliders) - 1 else ""
                f.write(f'            {{"type": "{c.type}", "rect": {json.dumps(c.rect)}, "data": {json.dumps(c.data)}}}{comma}\n')
            f.write('        ],\n')

            # Props
            f.write('        "props": [\n')
            for i, p in enumerate(self.props):
                comma = "," if i < len(self.props) - 1 else ""
                f.write(f'            {{"type": "{p.type}", "tileset": {p.tileset_idx}, "src": {json.dumps(p.src_rect)}, "pos": {json.dumps(p.pos)}, "data": {json.dumps(p.data)}}}{comma}\n')
            f.write('        ],\n')

            # Objects
            f.write('        "objects": [\n')
            for i, obj in enumerate(self.objects):
                comma = "," if i < len(self.objects) - 1 else ""
                f.write(f'            {{"type": "{obj.type}", "pos": {json.dumps(obj.pos)}, "data": {json.dumps(obj.data)}}}{comma}\n')
            f.write('        ]\n')

            f.write('    }\n')
            f.write('}\n')

    @staticmethod
    def load(path):
        map_data = MapData()
        with open(path, 'r') as f:
            data = json.load(f)

        map_info = data.get("map", {})
        map_data.name = map_info.get("name", "untitled")
        map_data.size = map_info.get("size", [1920, 1080])
        map_data.tilesets = map_info.get("tilesets", [])

        # Load tiles
        for layer in map_info.get("layers", []):
            layer_name = layer.get("name", "main")
            for tile_data in layer.get("tiles", []):
                tile = Tile(
                    tile_data.get("tileset", 0),
                    tile_data.get("src", [0, 0, 32, 32]),
                    tile_data.get("pos", [0, 0]),
                    layer_name
                )
                map_data.tiles.append(tile)

        # Load colliders
        for coll_data in map_info.get("colliders", []):
            collider = Collider(
                coll_data.get("type", "box"),
                coll_data.get("rect", [0, 0, 32, 32]),
                coll_data.get("data", {})
            )
            map_data.colliders.append(collider)

        # Load props
        for prop_data in map_info.get("props", []):
            prop = Prop(
                prop_data.get("type", "destructible"),
                prop_data.get("tileset", 0),
                prop_data.get("src", [0, 0, 32, 32]),
                prop_data.get("pos", [0, 0]),
                prop_data.get("data", {})
            )
            map_data.props.append(prop)

        # Load objects
        for obj_data in map_info.get("objects", []):
            obj = CustomObject(
                obj_data.get("type", "spawn_point"),
                obj_data.get("pos", [0, 0]),
                obj_data.get("data", {})
            )
            map_data.objects.append(obj)

        return map_data
