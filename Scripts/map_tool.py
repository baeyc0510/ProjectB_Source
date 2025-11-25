import pygame
import json
import os

from map_tool_config import (
    TOOL_CONFIG, get_relative_path, resolve_relative_path,
    WINDOW_WIDTH, WINDOW_HEIGHT, BACKGROUND_COLOR, PANEL_COLOR,
    PANEL_BORDER_COLOR, TEXT_COLOR, HIGHLIGHT_COLOR, GRID_COLOR, GRID_SIZE,
    LEFT_PANEL_WIDTH, RIGHT_PANEL_WIDTH, TOP_BAR_HEIGHT, BOTTOM_BAR_HEIGHT
)
from map_tool_ui import Button, InputField, Dropdown, Checkbox
from map_tool_data import Tile, Collider, Prop, CustomObject, MapData, auto_detect_tiles


def get_pivot_offset(pivot_mode, width, height):
    """Calculate pixel offset from pivot mode string"""
    # Default to top-left (no offset)
    offset_x, offset_y = 0, 0

    if "center" in pivot_mode and "bottom" not in pivot_mode and "top" not in pivot_mode:
        # Pure "center"
        offset_x = width // 2
        offset_y = height // 2
    else:
        # Handle horizontal
        if "left" in pivot_mode:
            offset_x = 0
        elif "right" in pivot_mode:
            offset_x = width
        else:  # center (default for horizontal)
            offset_x = width // 2

        # Handle vertical
        if "top" in pivot_mode:
            offset_y = 0
        elif "bottom" in pivot_mode:
            offset_y = height
        else:  # middle
            offset_y = height // 2

    return offset_x, offset_y


# --- Main Editor ---
class MapEditor:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT), pygame.RESIZABLE)
        pygame.display.set_caption("Map Tool")

        self.font = pygame.font.Font(None, 20)
        self.font_small = pygame.font.Font(None, 16)
        self.clock = pygame.time.Clock()
        self.running = True

        # Map data
        self.map_data = MapData()
        self.map_path = None

        # Auto-load schema from map_schema.json if exists
        script_dir = os.path.dirname(os.path.abspath(__file__))
        schema_path = os.path.join(script_dir, "map_schema.json")
        if os.path.exists(schema_path):
            try:
                with open(schema_path, 'r', encoding='utf-8') as f:
                    self.map_data.schema = json.load(f)
            except Exception as e:
                print(f"Warning: Failed to load schema: {e}")

        # View state
        self.cam_x, self.cam_y = 0, 0
        self.zoom = 1.0
        self.is_panning = False
        self.pan_start = None

        # Tool state
        self.current_tool = "select"  # select, tile, collider, prop, object
        self.current_layer = "main"
        self.current_collider_type = "box"
        self.current_prop_type = "destructible"
        self.current_object_type = "spawn_point"

        # Selection
        self.selected_tiles = []
        self.selected_collider = None
        self.selected_prop = None
        self.selected_object = None
        self.selection_rect = None
        self.is_drawing = False
        self.draw_start = None

        # Tileset
        self.tileset_images = []
        self.tileset_paths = []
        self.tileset_zoom = 1.0
        self.tileset_cam_x = 0
        self.tileset_cam_y = 0
        self.tileset_panning = False
        self.tileset_pan_start = None
        self.selected_tile_src = None  # [x, y, w, h]
        self.tile_selection_start = None
        self.tile_selection_end = None
        self.is_selecting_tile = False
        self.detected_tile_boxes = []  # Auto-detected tile regions

        # Animation state for animated props
        self.loaded_animation = None  # Loaded animation JSON data
        self.animation_image = None  # Loaded animation sprite sheet
        self.animation_path = None  # Path to animation JSON file
        self.animation_frames = []  # List of frame data from animation
        self.animation_time = 0  # Animation timer for preview

        # UI state
        self.show_grid = True
        self.show_colliders = True
        self.show_type_hints = False
        self.snap_to_grid = True
        self.grid_size = GRID_SIZE  # Adjustable grid size

        # Notification
        self.notification = None
        self.notification_time = 0

        # Property editor fields (dynamic)
        self.property_fields = {}
        self.active_input = None

        self._create_ui()

        # Auto-load schema from same directory
        self._auto_load_schema()

    def _auto_load_schema(self):
        """Auto-load map_schema.json from script directory"""
        script_dir = os.path.dirname(os.path.abspath(__file__))
        schema_path = os.path.join(script_dir, "map_schema.json")
        if os.path.exists(schema_path):
            try:
                with open(schema_path, 'r', encoding='utf-8') as f:
                    schema = json.load(f)
                if "collider_types" in schema and "tile_layers" in schema:
                    self.map_data.schema = schema
                    # Set default collider type
                    if self.current_collider_type not in schema["collider_types"]:
                        self.current_collider_type = list(schema["collider_types"].keys())[0]
                    # Set default layer
                    layer_names = [l["name"] for l in schema["tile_layers"]]
                    if self.current_layer not in layer_names:
                        self.current_layer = layer_names[0] if layer_names else "main"
                    print(f"Loaded schema: {schema_path}")
            except Exception as e:
                print(f"Failed to load schema: {e}")

    def _create_ui(self):
        # Tool buttons
        self.tool_buttons = []
        tools = [("Select", "select"), ("Tile", "tile"), ("Collider", "collider"), ("Prop", "prop"), ("Object", "object")]
        for i, (name, tool) in enumerate(tools):
            btn = Button(10 + i * 70, 5, 65, 22, name, self.font_small, lambda t=tool: self._set_tool(t))
            self.tool_buttons.append((btn, tool))

        # Layer buttons
        self.layer_buttons = []

        # Collider type buttons
        self.collider_buttons = []

    def _set_tool(self, tool):
        self.current_tool = tool
        # Don't clear selection when switching tools - preserve selection for property editing

    def _get_canvas_rect(self):
        w, h = self.screen.get_size()
        return pygame.Rect(LEFT_PANEL_WIDTH, TOP_BAR_HEIGHT,
                          w - LEFT_PANEL_WIDTH - RIGHT_PANEL_WIDTH,
                          h - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)

    def _screen_to_world(self, pos):
        canvas = self._get_canvas_rect()
        x = (pos[0] - canvas.x - self.cam_x) / self.zoom
        y = (pos[1] - canvas.y - self.cam_y) / self.zoom
        return (x, y)

    def _world_to_screen(self, pos):
        canvas = self._get_canvas_rect()
        x = pos[0] * self.zoom + self.cam_x + canvas.x
        y = pos[1] * self.zoom + self.cam_y + canvas.y
        return (x, y)

    def _snap(self, pos):
        if self.snap_to_grid:
            return (round(pos[0] / self.grid_size) * self.grid_size,
                    round(pos[1] / self.grid_size) * self.grid_size)
        return pos

    def load_tileset(self, path):
        try:
            img = pygame.image.load(path).convert_alpha()
            self.tileset_images.append(img)
            self.tileset_paths.append(path)
            # Store relative path from project root for saving, ensuring it's a .bmp
            project_root = TOOL_CONFIG.get("project_root", "")
            rel_path = get_relative_path(path, project_root)
            bmp_rel_path = os.path.splitext(rel_path)[0] + ".bmp"
            
            # Avoid adding duplicate paths
            if bmp_rel_path not in self.map_data.tilesets:
                self.map_data.tilesets.append(bmp_rel_path)
            
            # Find the index for the tile
            self.tile_tileset_idx = self.map_data.tilesets.index(bmp_rel_path)

            # Auto-detect tile regions
            self.detected_tile_boxes = auto_detect_tiles(path)
            # Reset tileset view
            self.tileset_zoom = 1.0
            self.tileset_cam_x = 0
            self.tileset_cam_y = 0
            self._notify(f"Loaded: {os.path.basename(path)} ({len(self.detected_tile_boxes)} tiles detected)")
        except Exception as e:
            self._notify(f"Error loading tileset: {e}")

    def _notify(self, msg):
        self.notification = msg
        self.notification_time = pygame.time.get_ticks()

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False

            elif event.type == pygame.VIDEORESIZE:
                self.screen = pygame.display.set_mode((event.w, event.h), pygame.RESIZABLE)

            elif event.type == pygame.KEYDOWN:
                self._handle_keydown(event)

            elif event.type == pygame.MOUSEBUTTONDOWN:
                self._handle_mousedown(event)

            elif event.type == pygame.MOUSEBUTTONUP:
                self._handle_mouseup(event)

            elif event.type == pygame.MOUSEMOTION:
                self._handle_mousemotion(event)

            elif event.type == pygame.MOUSEWHEEL:
                self._handle_mousewheel(event)

            # Handle tileset panel events first
            if self._handle_tileset_event(event):
                continue

            # UI elements
            for btn, tool in self.tool_buttons:
                btn.active = (self.current_tool == tool)
                btn.handle_event(event)

    def _handle_keydown(self, event):
        # Handle property input first
        if self._handle_property_key(event):
            return

        if event.key == pygame.K_ESCAPE:
            self.running = False
        elif event.key == pygame.K_g:
            self.show_grid = not self.show_grid
        elif event.key == pygame.K_c:
            self.show_colliders = not self.show_colliders
        elif event.key == pygame.K_h:
            self.show_type_hints = not self.show_type_hints
            self._notify(f"Type hints: {'ON' if self.show_type_hints else 'OFF'}")
        elif event.key == pygame.K_s and event.mod & pygame.KMOD_CTRL:
            self._save_map()
        elif event.key == pygame.K_o and event.mod & pygame.KMOD_CTRL:
            self._open_file_dialog()
        elif event.key == pygame.K_t:
            self._open_tileset_dialog()
        elif event.key == pygame.K_F2:
            self._load_schema_dialog()
        elif event.key == pygame.K_DELETE:
            self._delete_selected()
        elif event.key == pygame.K_1:
            self.current_layer = "bg_far"
        elif event.key == pygame.K_2:
            self.current_layer = "bg_near"
        elif event.key == pygame.K_3:
            self.current_layer = "main"
        elif event.key == pygame.K_4:
            self.current_layer = "fg"
        # Grid size adjustment
        elif event.key == pygame.K_LEFTBRACKET:  # [
            self.grid_size = max(4, self.grid_size // 2)
            self._notify(f"Grid: {self.grid_size}px")
        elif event.key == pygame.K_RIGHTBRACKET:  # ]
            self.grid_size = min(256, self.grid_size * 2)
            self._notify(f"Grid: {self.grid_size}px")
        elif event.key == pygame.K_n:  # Toggle snap
            self.snap_to_grid = not self.snap_to_grid
            self._notify(f"Snap: {'ON' if self.snap_to_grid else 'OFF'}")
        elif event.key == pygame.K_a:  # Load/clear animation for animated prop
            if event.mod & pygame.KMOD_SHIFT:
                # Clear loaded animation
                self._clear_animation()
            else:
                self._open_animation_dialog()

    def _handle_mousedown(self, event):
        canvas = self._get_canvas_rect()
        mouse_pos = pygame.mouse.get_pos()
        w = self.screen.get_width()

        # Right click - pan
        if event.button == 3:
            if canvas.collidepoint(mouse_pos):
                self.is_panning = True
                self.pan_start = mouse_pos

        # Left click
        elif event.button == 1:
            # Check if in right panel (properties)
            right_panel = pygame.Rect(w - RIGHT_PANEL_WIDTH, TOP_BAR_HEIGHT, RIGHT_PANEL_WIDTH,
                                      self.screen.get_height() - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)
            if right_panel.collidepoint(mouse_pos):
                self._handle_property_click(mouse_pos)
                return

            # Check if in canvas
            elif canvas.collidepoint(mouse_pos):
                world_pos = self._screen_to_world(mouse_pos)
                snapped = self._snap(world_pos)

                if self.current_tool == "select":
                    self._select_at(world_pos)

                elif self.current_tool == "tile":
                    if self.selected_tile_src:
                        self._place_tile(snapped)

                elif self.current_tool == "collider":
                    self.is_drawing = True
                    self.draw_start = snapped
                    self.selection_rect = None

                elif self.current_tool == "prop":
                    # Place prop if we have a tile selected OR animation loaded
                    if self.selected_tile_src or (self.loaded_animation and self.animation_frames):
                        self._place_prop(snapped)

                elif self.current_tool == "object":
                    self._place_object(snapped)

    def _handle_mouseup(self, event):
        if event.button == 3:
            self.is_panning = False
            self.pan_start = None

        elif event.button == 1:
            if self.is_drawing and self.draw_start:
                mouse_pos = pygame.mouse.get_pos()
                world_pos = self._screen_to_world(mouse_pos)
                snapped = self._snap(world_pos)

                x = min(self.draw_start[0], snapped[0])
                y = min(self.draw_start[1], snapped[1])
                w = abs(snapped[0] - self.draw_start[0])
                h = abs(snapped[1] - self.draw_start[1])

                if w > 0 and h > 0:
                    # Create default data from schema
                    default_data = {}
                    schema_type = self.map_data.schema["collider_types"].get(self.current_collider_type, {})
                    for field in schema_type.get("fields", []):
                        default_data[field["name"]] = field.get("default", "")

                    collider = Collider(self.current_collider_type, [int(x), int(y), int(w), int(h)], default_data)
                    self.map_data.colliders.append(collider)
                    self.selected_collider = collider

                self.is_drawing = False
                self.draw_start = None

            self.tile_selection_start = None

    def _handle_mousemotion(self, event):
        if self.is_panning and self.pan_start:
            dx = event.pos[0] - self.pan_start[0]
            dy = event.pos[1] - self.pan_start[1]
            self.cam_x += dx
            self.cam_y += dy
            self.pan_start = event.pos

    def _handle_mousewheel(self, event):
        canvas = self._get_canvas_rect()
        mouse_pos = pygame.mouse.get_pos()

        # Zoom in canvas (tileset zoom is handled in _handle_tileset_event)
        if canvas.collidepoint(mouse_pos):
            old_zoom = self.zoom
            self.zoom *= 1.1 if event.y > 0 else 0.9
            self.zoom = max(0.1, min(5.0, self.zoom))

            # Zoom towards mouse
            factor = self.zoom / old_zoom
            rel_x = mouse_pos[0] - canvas.x - self.cam_x
            rel_y = mouse_pos[1] - canvas.y - self.cam_y
            self.cam_x = mouse_pos[0] - canvas.x - rel_x * factor
            self.cam_y = mouse_pos[1] - canvas.y - rel_y * factor

    def _get_tileset_panel_rect(self):
        """Get the tileset panel rectangle - dynamic based on content"""
        h = self.screen.get_height()
        # Calculate top position based on layers and conditional type sections
        num_layers = len(self.map_data.schema.get("tile_layers", []))
        # Layers: 25 title + 18*n + 20 spacing
        top_offset = TOP_BAR_HEIGHT + 10 + 25 + (num_layers * 18) + 20

        # Add space for collider types only in collider mode
        if self.current_tool == "collider":
            num_colliders = len(self.map_data.schema.get("collider_types", {}))
            top_offset += 25 + (num_colliders * 22) + 10

        # Add space for object types only in object mode
        if self.current_tool == "object":
            num_objects = len(self.map_data.schema.get("custom_objects", {}))
            top_offset += 25 + (num_objects * 22) + 10

        top_offset += 20  # tileset title space
        return pygame.Rect(5, top_offset, LEFT_PANEL_WIDTH - 10,
                          h - top_offset - BOTTOM_BAR_HEIGHT - 5)

    def _tileset_screen_to_image(self, pos, panel_rect):
        """Convert screen position to tileset image coordinates"""
        rel_x = (pos[0] - panel_rect.x - self.tileset_cam_x) / self.tileset_zoom
        rel_y = (pos[1] - panel_rect.y - self.tileset_cam_y) / self.tileset_zoom
        return (rel_x, rel_y)

    def _handle_tileset_event(self, event):
        """Handle all tileset panel events"""
        panel_rect = self._get_tileset_panel_rect()
        mouse_pos = pygame.mouse.get_pos()

        if not panel_rect.collidepoint(mouse_pos) and event.type not in [pygame.MOUSEBUTTONUP, pygame.MOUSEMOTION]:
            return False

        if not self.tileset_images:
            return False

        # Mouse wheel - zoom
        if event.type == pygame.MOUSEWHEEL and panel_rect.collidepoint(mouse_pos):
            old_zoom = self.tileset_zoom
            self.tileset_zoom *= 1.15 if event.y > 0 else 0.85
            self.tileset_zoom = max(0.1, min(5.0, self.tileset_zoom))

            # Zoom towards mouse
            factor = self.tileset_zoom / old_zoom
            rel_x = mouse_pos[0] - panel_rect.x - self.tileset_cam_x
            rel_y = mouse_pos[1] - panel_rect.y - self.tileset_cam_y
            self.tileset_cam_x = mouse_pos[0] - panel_rect.x - rel_x * factor
            self.tileset_cam_y = mouse_pos[1] - panel_rect.y - rel_y * factor
            return True

        # Right-click - start panning
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 3 and panel_rect.collidepoint(mouse_pos):
            self.tileset_panning = True
            self.tileset_pan_start = mouse_pos
            return True

        # Right-click release - stop panning
        if event.type == pygame.MOUSEBUTTONUP and event.button == 3:
            self.tileset_panning = False
            self.tileset_pan_start = None
            return True

        # Mouse motion - panning or selecting
        if event.type == pygame.MOUSEMOTION:
            if self.tileset_panning and self.tileset_pan_start:
                dx = mouse_pos[0] - self.tileset_pan_start[0]
                dy = mouse_pos[1] - self.tileset_pan_start[1]
                self.tileset_cam_x += dx
                self.tileset_cam_y += dy
                self.tileset_pan_start = mouse_pos
                return True
            elif self.is_selecting_tile and self.tile_selection_start:
                self.tile_selection_end = self._tileset_screen_to_image(mouse_pos, panel_rect)
                return True

        # Left-click - start tile selection or select detected tile
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1 and panel_rect.collidepoint(mouse_pos):
            img_pos = self._tileset_screen_to_image(mouse_pos, panel_rect)

            # Check if clicking on a detected tile box
            for box in self.detected_tile_boxes:
                box_rect = pygame.Rect(box)
                if box_rect.collidepoint(img_pos):
                    self.selected_tile_src = box.copy()
                    # Clear animation mode when selecting a tile
                    self.loaded_animation = None
                    self.animation_image = None
                    self.animation_frames = []
                    # Don't force tool change - just select the tile
                    return True

            # Start free-form selection
            self.is_selecting_tile = True
            self.tile_selection_start = img_pos
            self.tile_selection_end = img_pos
            return True

        # Left-click release - finish tile selection
        if event.type == pygame.MOUSEBUTTONUP and event.button == 1:
            if self.is_selecting_tile and self.tile_selection_start and self.tile_selection_end:
                x1, y1 = self.tile_selection_start
                x2, y2 = self.tile_selection_end
                x = int(min(x1, x2))
                y = int(min(y1, y2))
                w = int(abs(x2 - x1))
                h = int(abs(y2 - y1))

                if w > 2 and h > 2:
                    self.selected_tile_src = [x, y, w, h]
                    # Clear animation mode when selecting a tile
                    self.loaded_animation = None
                    self.animation_image = None
                    self.animation_frames = []

            self.is_selecting_tile = False
            self.tile_selection_start = None
            self.tile_selection_end = None
            return True

        return False

    def _handle_property_click(self, pos):
        """Handle clicks on property panel fields"""
        # Determine target (object, prop, or collider)
        target = None
        if self.selected_object:
            target = self.selected_object
        elif self.selected_prop:
            target = self.selected_prop
        elif self.selected_collider:
            target = self.selected_collider

        if not target:
            return False

        for field_name, field_info in self.property_fields.items():
            rect = field_info.get("rect")
            if not rect:
                continue

            field_type = field_info.get("type")

            if field_type == "prop_type":
                # Handle prop type dropdown
                options = field_info.get("options", [])
                if field_info.get("expanded"):
                    for i, opt in enumerate(options):
                        opt_rect = pygame.Rect(rect.x, rect.y + (i + 1) * 20, rect.width, 18)
                        if opt_rect.collidepoint(pos):
                            self._change_prop_type(opt)
                            field_info["expanded"] = False
                            return True
                    field_info["expanded"] = False
                elif rect.collidepoint(pos):
                    for fn, fi in self.property_fields.items():
                        if fi.get("type") in ("enum", "prop_type", "object_type"):
                            fi["expanded"] = False
                    field_info["expanded"] = True
                    return True

            elif field_type == "object_type":
                # Handle object type dropdown
                options = field_info.get("options", [])
                if field_info.get("expanded"):
                    for i, opt in enumerate(options):
                        opt_rect = pygame.Rect(rect.x, rect.y + (i + 1) * 20, rect.width, 18)
                        if opt_rect.collidepoint(pos):
                            self._change_object_type(opt)
                            field_info["expanded"] = False
                            return True
                    field_info["expanded"] = False
                elif rect.collidepoint(pos):
                    for fn, fi in self.property_fields.items():
                        if fi.get("type") in ("enum", "prop_type", "object_type"):
                            fi["expanded"] = False
                    field_info["expanded"] = True
                    return True

            elif field_type == "enum":
                options = field_info.get("options", [])
                if field_info.get("expanded"):
                    # Check option clicks
                    for i, opt in enumerate(options):
                        opt_rect = pygame.Rect(rect.x, rect.y + (i + 1) * 20, rect.width, 18)
                        if opt_rect.collidepoint(pos):
                            target.data[field_name] = opt
                            field_info["expanded"] = False
                            return True
                    field_info["expanded"] = False
                elif rect.collidepoint(pos):
                    # Close other dropdowns
                    for fn, fi in self.property_fields.items():
                        if fi.get("type") in ("enum", "prop_type", "object_type"):
                            fi["expanded"] = False
                    field_info["expanded"] = True
                    return True

            elif field_type == "bool":
                if rect.collidepoint(pos):
                    current = target.data.get(field_name, False)
                    target.data[field_name] = not current
                    return True

            elif field_type == "prop_pivot":
                # Handle prop pivot dropdown
                options = field_info.get("options", [])
                if field_info.get("expanded"):
                    for i, opt in enumerate(options):
                        opt_rect = pygame.Rect(rect.x, rect.y + (i + 1) * 20, rect.width, 18)
                        if opt_rect.collidepoint(pos):
                            self.selected_prop.data["pivot"] = opt
                            field_info["expanded"] = False
                            return True
                    field_info["expanded"] = False
                elif rect.collidepoint(pos):
                    for fn, fi in self.property_fields.items():
                        if fi.get("type") in ("enum", "prop_type", "object_type", "prop_pivot", "prop_collider_type"):
                            fi["expanded"] = False
                    field_info["expanded"] = True
                    return True

            elif field_type == "prop_has_collider":
                if rect.collidepoint(pos):
                    current = self.selected_prop.data.get("has_collider", False)
                    self.selected_prop.data["has_collider"] = not current
                    # Set sensible default offset when enabling collider
                    if not current:  # Was False, now True
                        coll_w = self.selected_prop.data.get("collider_width", 32)
                        coll_h = self.selected_prop.data.get("collider_height", 32)
                        # Center horizontally, position above pivot (for bottom-center pivot)
                        self.selected_prop.data["collider_offset_x"] = -coll_w // 2
                        self.selected_prop.data["collider_offset_y"] = -coll_h
                    return True

            elif field_type == "prop_collider_type":
                # Handle prop collider type dropdown
                options = field_info.get("options", [])
                if field_info.get("expanded"):
                    for i, opt in enumerate(options):
                        opt_rect = pygame.Rect(rect.x, rect.y + (i + 1) * 20, rect.width, 18)
                        if opt_rect.collidepoint(pos):
                            self.selected_prop.data["collider_type"] = opt
                            field_info["expanded"] = False
                            return True
                    field_info["expanded"] = False
                elif rect.collidepoint(pos):
                    for fn, fi in self.property_fields.items():
                        if fi.get("type") in ("enum", "prop_type", "object_type", "prop_pivot", "prop_collider_type"):
                            fi["expanded"] = False
                    field_info["expanded"] = True
                    return True

            elif field_type in ("prop_collider_size", "prop_collider_offset"):
                if rect.collidepoint(pos):
                    self.active_input = field_name
                    return True

            else:  # string, int, float
                if rect.collidepoint(pos):
                    self.active_input = field_name
                    return True

        self.active_input = None
        return False

    def _handle_property_key(self, event):
        """Handle keyboard input for active property field"""
        # Determine target (object, prop, or collider)
        target = None
        if self.selected_object:
            target = self.selected_object
        elif self.selected_prop:
            target = self.selected_prop
        elif self.selected_collider:
            target = self.selected_collider

        if not self.active_input or not target:
            return False

        field_name = self.active_input
        field_info = self.property_fields.get(field_name, {})
        field_type = field_info.get("type", "string")

        # Handle special prop collider fields
        if field_type in ("prop_collider_size", "prop_collider_offset"):
            actual_field = field_info.get("field", field_name)
            current_value = str(self.selected_prop.data.get(actual_field, ""))

            if event.key == pygame.K_BACKSPACE:
                self.selected_prop.data[actual_field] = current_value[:-1]
                return True
            elif event.key == pygame.K_RETURN:
                val = self.selected_prop.data.get(actual_field, "")
                try:
                    self.selected_prop.data[actual_field] = int(val)
                except:
                    self.selected_prop.data[actual_field] = 0
                self.active_input = None
                return True
            elif event.unicode.isprintable():
                self.selected_prop.data[actual_field] = current_value + event.unicode
                return True
            return False

        # Normal field handling
        current_value = str(target.data.get(field_name, ""))

        if event.key == pygame.K_BACKSPACE:
            target.data[field_name] = current_value[:-1]
            return True
        elif event.key == pygame.K_RETURN:
            # Convert type if needed
            val = target.data.get(field_name, "")
            try:
                if field_type == "int":
                    target.data[field_name] = int(val)
                elif field_type == "float":
                    target.data[field_name] = float(val)
            except:
                pass
            self.active_input = None
            return True
        elif event.unicode.isprintable():
            target.data[field_name] = current_value + event.unicode
            return True

        return False

    def _select_at(self, world_pos):
        # Try to select object first (they are usually smaller and on top)
        for obj in reversed(self.map_data.objects):
            # Objects are point-based, use a hit area
            hit_size = 24
            hit_rect = pygame.Rect(obj.pos[0] - hit_size // 2, obj.pos[1] - hit_size // 2, hit_size, hit_size)
            if hit_rect.collidepoint(world_pos):
                self.selected_object = obj
                self.selected_collider = None
                self.selected_prop = None
                self.selected_tiles = []
                return

        # Try to select prop
        for prop in reversed(self.map_data.props):
            # Get pivot offset for correct hit detection
            if prop.tileset_idx == -1 and hasattr(prop, 'animation_data'):
                # Animated prop - use animation's pivot
                pivot_mode = prop.animation_data.get("pivot", "bottom-center")
                frames = prop.animation_data.get("frames", [])
                if frames:
                    first_frame = frames[0]
                    if isinstance(first_frame, list):
                        w, h = first_frame[2], first_frame[3]
                    else:
                        w, h = first_frame.get("rect", [0, 0, 32, 32])[2:4]
                else:
                    w, h = 32, 32
            else:
                # Regular prop - use prop data's pivot
                pivot_mode = prop.data.get("pivot", "top-left")
                w, h = prop.src_rect[2], prop.src_rect[3]

            pivot_x, pivot_y = get_pivot_offset(pivot_mode, w, h)
            rect = pygame.Rect(prop.pos[0] - pivot_x, prop.pos[1] - pivot_y, w, h)
            if rect.collidepoint(world_pos):
                self.selected_prop = prop
                self.selected_collider = None
                self.selected_object = None
                self.selected_tiles = []
                return

        # Try to select collider
        for collider in reversed(self.map_data.colliders):
            rect = pygame.Rect(collider.rect)
            if rect.collidepoint(world_pos):
                self.selected_collider = collider
                self.selected_object = None
                self.selected_prop = None
                self.selected_tiles = []
                return

        # Try to select tile
        for tile in reversed(self.map_data.tiles):
            rect = pygame.Rect(tile.pos[0], tile.pos[1], tile.src_rect[2], tile.src_rect[3])
            if rect.collidepoint(world_pos):
                self.selected_tiles = [tile]
                self.selected_collider = None
                self.selected_prop = None
                self.selected_object = None
                return

        self.selected_tiles = []
        self.selected_collider = None
        self.selected_prop = None
        self.selected_object = None

    def _place_tile(self, pos):
        if not self.tileset_images or not self.selected_tile_src:
            return

        tile = Tile(self.tile_tileset_idx, self.selected_tile_src.copy(), [int(pos[0]), int(pos[1])], self.current_layer)
        self.map_data.tiles.append(tile)

    def _place_prop(self, pos):
        """Place a prop at the given position with sprite from tileset or animation"""
        # Check if we have an animation loaded (for animated props)
        if self.loaded_animation and self.animation_frames:
            self._place_animated_prop(pos)
            return

        # Regular prop placement from tileset
        if not self.tileset_images or not self.selected_tile_src:
            return

        # Get default data from schema
        default_data = {}
        schema_prop = self.map_data.schema.get("prop_types", {}).get(self.current_prop_type, {})
        for field in schema_prop.get("fields", []):
            default_data[field["name"]] = field.get("default", "")

        prop = Prop(
            self.current_prop_type,
            self.tile_tileset_idx,  # tileset_idx
            self.selected_tile_src.copy(),
            [int(pos[0]), int(pos[1])],
            default_data
        )
        self.map_data.props.append(prop)
        self.selected_prop = prop
        self.selected_collider = None
        self.selected_object = None
        self.selected_tiles = []

    def _place_animated_prop(self, pos):
        """Place an animated prop using loaded animation data"""
        if not self.animation_frames:
            return

        # Get first frame for size reference (frames can be [x,y,w,h] or {"rect": [x,y,w,h]})
        first_frame = self.animation_frames[0]
        if isinstance(first_frame, list):
            frame_rect = first_frame
        else:
            frame_rect = first_frame.get("rect", [0, 0, 32, 32])
        src_rect = [frame_rect[0], frame_rect[1], frame_rect[2], frame_rect[3]]

        # Get default data from schema
        default_data = {}
        schema_prop = self.map_data.schema.get("prop_types", {}).get("decoration", {})
        for field in schema_prop.get("fields", []):
            default_data[field["name"]] = field.get("default", "")

        # Set animated flag and animation file (relative path from project root)
        default_data["animated"] = True
        project_root = TOOL_CONFIG.get("project_root", "")
        default_data["animation_file"] = get_relative_path(self.animation_path, project_root)
        # Apply pivot mode from animation JSON
        default_data["pivot"] = self.loaded_animation.get("pivot", "bottom-center")

        # Store animation data directly in prop for editor rendering
        prop = Prop(
            "decoration",
            -1,  # Special tileset_idx for animated props
            src_rect,
            [int(pos[0]), int(pos[1])],
            default_data
        )
        # Store animation reference for editor preview
        prop.animation_data = self.loaded_animation
        prop.animation_image = self.animation_image

        self.map_data.props.append(prop)
        self.selected_prop = prop
        self.selected_collider = None
        self.selected_object = None
        self.selected_tiles = []

    def _change_prop_type(self, new_type):
        """Change the type of the selected prop and update its data fields"""
        if not self.selected_prop:
            return

        old_type = self.selected_prop.type
        self.selected_prop.type = new_type

        # Reset data to new type's defaults
        new_data = {}
        schema_prop = self.map_data.schema.get("prop_types", {}).get(new_type, {})
        for field in schema_prop.get("fields", []):
            field_name = field["name"]
            # Keep old value if field exists in both types
            if field_name in self.selected_prop.data:
                new_data[field_name] = self.selected_prop.data[field_name]
            else:
                new_data[field_name] = field.get("default", "")
        self.selected_prop.data = new_data

    def _change_object_type(self, new_type):
        """Change the type of the selected object and update its data fields"""
        if not self.selected_object:
            return

        old_type = self.selected_object.type
        self.selected_object.type = new_type

        # Reset data to new type's defaults
        new_data = {}
        schema_obj = self.map_data.schema.get("custom_objects", {}).get(new_type, {})
        for field in schema_obj.get("fields", []):
            field_name = field["name"]
            # Keep old value if field exists in both types
            if field_name in self.selected_object.data:
                new_data[field_name] = self.selected_object.data[field_name]
            else:
                new_data[field_name] = field.get("default", "")
        self.selected_object.data = new_data

    def _place_object(self, pos):
        """Place a custom object at the given position"""
        # Get default data from schema
        default_data = {}
        schema_obj = self.map_data.schema.get("custom_objects", {}).get(self.current_object_type, {})
        for field in schema_obj.get("fields", []):
            default_data[field["name"]] = field.get("default", "")

        obj = CustomObject(self.current_object_type, [int(pos[0]), int(pos[1])], default_data)
        self.map_data.objects.append(obj)
        self.selected_object = obj
        self.selected_collider = None
        self.selected_prop = None
        self.selected_tiles = []

    def _delete_selected(self):
        if self.selected_object:
            self.map_data.objects.remove(self.selected_object)
            self.selected_object = None
        if self.selected_prop:
            self.map_data.props.remove(self.selected_prop)
            self.selected_prop = None
        if self.selected_collider:
            self.map_data.colliders.remove(self.selected_collider)
            self.selected_collider = None
        for tile in self.selected_tiles:
            if tile in self.map_data.tiles:
                self.map_data.tiles.remove(tile)
        self.selected_tiles = []

    def _save_map(self):
        import tkinter as tk
        from tkinter import filedialog

        # Determine default directory from config
        project_root = TOOL_CONFIG.get("project_root", "")
        resources_folder = TOOL_CONFIG.get("resources_folder", "Resources")
        maps_folder = TOOL_CONFIG.get("maps_folder", "Maps")
        default_dir = os.path.join(project_root, resources_folder, maps_folder)

        # Create directory if it doesn't exist
        if not os.path.exists(default_dir):
            os.makedirs(default_dir, exist_ok=True)

        # Default filename
        default_filename = self.map_data.name + ".json" if self.map_data.name != "untitled" else "map_output.json"

        root = tk.Tk()
        root.withdraw()
        path = filedialog.asksaveasfilename(
            title="Save Map File",
            initialdir=default_dir,
            initialfile=default_filename,
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        root.destroy()

        if path:
            self.map_path = path
            # Update map name from filename
            self.map_data.name = os.path.splitext(os.path.basename(path))[0]
            self.map_data.save(self.map_path)
            self._notify(f"Saved to {self.map_path}")

    def _open_file_dialog(self):
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk()
        root.withdraw()
        path = filedialog.askopenfilename(
            title="Open Map File",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        root.destroy()
        if path:
            self._load_map(path)

    def _load_map(self, path):
        try:
            self.map_data = MapData.load(path)
            self.map_path = path

            # Clear selection states
            self.selected_object = None
            self.selected_prop = None
            self.selected_collider = None
            self.selected_tiles = []
            self.property_fields.clear()

            # Try to auto-load schema from map_schema.json in same directory as script
            script_dir = os.path.dirname(os.path.abspath(__file__))
            schema_path = os.path.join(script_dir, "map_schema.json")
            if os.path.exists(schema_path):
                try:
                    with open(schema_path, 'r', encoding='utf-8') as f:
                        self.map_data.schema = json.load(f)
                except Exception as e:
                    print(f"Warning: Failed to load schema: {e}")

            # Load tilesets referenced in the map
            self.tileset_images = []
            self.tileset_paths = []
            project_root = TOOL_CONFIG.get("project_root", "")
            for tileset_path in self.map_data.tilesets:
                try:
                    # Resolve relative path to absolute using project root
                    abs_tileset_path = resolve_relative_path(tileset_path, project_root)
                    img = pygame.image.load(abs_tileset_path).convert_alpha()
                    self.tileset_images.append(img)
                    self.tileset_paths.append(abs_tileset_path)
                except Exception as e:
                    self._notify(f"Failed to load tileset: {tileset_path}")

            # Auto-detect tiles if we have a tileset
            if self.tileset_paths:
                self.detected_tile_boxes = auto_detect_tiles(self.tileset_paths[0])

            self._notify(f"Loaded: {os.path.basename(path)}")
        except Exception as e:
            self._notify(f"Error loading map: {e}")

    def _open_tileset_dialog(self):
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk()
        root.withdraw()
        path = filedialog.askopenfilename(
            title="Select Tileset",
            filetypes=[("Image files", "*.bmp *.png"), ("All files", "*.*")]
        )
        root.destroy()
        if path:
            self.load_tileset(path)

    def _open_animation_dialog(self):
        """Open file dialog to load an animation JSON file"""
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk()
        root.withdraw()
        path = filedialog.askopenfilename(
            title="Select Animation JSON",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        root.destroy()
        if path:
            self._load_animation(path)

    def _load_animation(self, path):
        """Load animation JSON and its sprite sheet image"""
        try:
            with open(path, 'r', encoding='utf-8') as f:
                anim_data = json.load(f)

            # Get the image path from animation JSON (can be relative to project root)
            image_filename = anim_data.get("image", "")
            if not image_filename:
                self._notify("Animation JSON missing 'image' field")
                return

            # Try to resolve image path using project root first
            project_root = TOOL_CONFIG.get("project_root", "")
            image_path = resolve_relative_path(image_filename, project_root)

            # Fallback: try same directory as JSON
            if not os.path.exists(image_path):
                anim_dir = os.path.dirname(path)
                image_path = os.path.join(anim_dir, os.path.basename(image_filename))

            if not os.path.exists(image_path):
                self._notify(f"Image not found: {image_filename}")
                return

            # Load the image
            anim_image = pygame.image.load(image_path).convert_alpha()

            # Store animation data
            self.loaded_animation = anim_data
            self.animation_image = anim_image
            self.animation_path = path
            self.animation_frames = anim_data.get("frames", [])

            # Switch to prop mode with decoration type for animated props
            self.current_tool = "prop"
            self.current_prop_type = "decoration"

            frame_count = len(self.animation_frames)
            self._notify(f"Animation loaded: {frame_count} frames")

        except Exception as e:
            self._notify(f"Error loading animation: {e}")

    def _clear_animation(self):
        """Clear loaded animation and return to normal prop mode"""
        self.loaded_animation = None
        self.animation_image = None
        self.animation_path = None
        self.animation_frames = []
        self._notify("Animation cleared")

    def _load_schema_dialog(self):
        import tkinter as tk
        from tkinter import filedialog
        root = tk.Tk()
        root.withdraw()
        path = filedialog.askopenfilename(
            title="Load Schema",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        root.destroy()
        if path:
            self._load_schema(path)

    def _load_schema(self, path):
        try:
            with open(path, 'r', encoding='utf-8') as f:
                schema = json.load(f)

            # Validate schema has required keys
            if "collider_types" not in schema:
                self._notify("Invalid schema: missing collider_types")
                return
            if "tile_layers" not in schema:
                self._notify("Invalid schema: missing tile_layers")
                return

            self.map_data.schema = schema

            # Reset current collider type if not in new schema
            if self.current_collider_type not in schema["collider_types"]:
                self.current_collider_type = list(schema["collider_types"].keys())[0]

            # Reset current layer if not in new schema
            layer_names = [l["name"] for l in schema["tile_layers"]]
            if self.current_layer not in layer_names:
                self.current_layer = layer_names[0] if layer_names else "main"

            self._notify(f"Schema loaded: {os.path.basename(path)}")
        except Exception as e:
            self._notify(f"Error loading schema: {e}")

    def update(self):
        pass

    def draw(self):
        self.screen.fill(BACKGROUND_COLOR)

        canvas = self._get_canvas_rect()

        # Draw canvas background
        pygame.draw.rect(self.screen, (30, 30, 35), canvas)

        # Set clipping for canvas
        self.screen.set_clip(canvas)

        # Draw grid
        if self.show_grid:
            self._draw_grid(canvas)

        # Draw tiles
        self._draw_tiles(canvas)

        # Draw props
        self._draw_props(canvas)

        # Draw colliders
        if self.show_colliders:
            self._draw_colliders(canvas)

        # Draw custom objects
        self._draw_objects(canvas)

        # Draw type hints overlay
        if self.show_type_hints:
            self._draw_type_hints(canvas)

        # Draw current drawing rect (for colliders)
        if self.is_drawing and self.draw_start:
            mouse_pos = pygame.mouse.get_pos()
            world_pos = self._screen_to_world(mouse_pos)
            snapped = self._snap(world_pos)

            x = min(self.draw_start[0], snapped[0])
            y = min(self.draw_start[1], snapped[1])
            w = abs(snapped[0] - self.draw_start[0])
            h = abs(snapped[1] - self.draw_start[1])

            screen_pos = self._world_to_screen((x, y))
            draw_rect = pygame.Rect(screen_pos[0], screen_pos[1], w * self.zoom, h * self.zoom)

            color = self.map_data.schema["collider_types"].get(self.current_collider_type, {}).get("color", [255, 255, 0])
            pygame.draw.rect(self.screen, color, draw_rect, 2)

        # Draw tile preview (semi-transparent) when tile tool is active
        if self.current_tool == "tile" and self.selected_tile_src and self.tileset_images:
            mouse_pos = pygame.mouse.get_pos()
            if canvas.collidepoint(mouse_pos):
                world_pos = self._screen_to_world(mouse_pos)
                snapped = self._snap(world_pos)
                screen_pos = self._world_to_screen(snapped)

                src_rect = pygame.Rect(self.selected_tile_src)
                tileset = self.tileset_images[0]

                # Get the tile subsurface
                try:
                    subsurface = tileset.subsurface(src_rect)
                    scaled_w = int(src_rect.width * self.zoom)
                    scaled_h = int(src_rect.height * self.zoom)
                    if scaled_w > 0 and scaled_h > 0:
                        scaled = pygame.transform.scale(subsurface, (scaled_w, scaled_h))
                        # Make semi-transparent
                        scaled.set_alpha(128)
                        self.screen.blit(scaled, screen_pos)
                        # Draw outline
                        preview_rect = pygame.Rect(screen_pos[0], screen_pos[1], scaled_w, scaled_h)
                        pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, preview_rect, 2)
                except:
                    pass

        # Draw animation preview (semi-transparent) when prop tool is active with loaded animation
        if self.current_tool == "prop" and self.loaded_animation and self.animation_frames and self.animation_image:
            mouse_pos = pygame.mouse.get_pos()
            if canvas.collidepoint(mouse_pos):
                world_pos = self._screen_to_world(mouse_pos)
                snapped = self._snap(world_pos)
                screen_pos = self._world_to_screen(snapped)

                # Get first frame rect (frames can be [x,y,w,h] or {"rect": [x,y,w,h]})
                first_frame = self.animation_frames[0]
                if isinstance(first_frame, list):
                    frame_rect = first_frame
                else:
                    frame_rect = first_frame.get("rect", [0, 0, 32, 32])
                src_rect = pygame.Rect(frame_rect[0], frame_rect[1], frame_rect[2], frame_rect[3])

                # Get pivot offset
                pivot_mode = self.loaded_animation.get("pivot", "bottom-center")
                pivot_x, pivot_y = get_pivot_offset(pivot_mode, src_rect.width, src_rect.height)

                # Get the animation subsurface
                try:
                    subsurface = self.animation_image.subsurface(src_rect)
                    scaled_w = int(src_rect.width * self.zoom)
                    scaled_h = int(src_rect.height * self.zoom)
                    if scaled_w > 0 and scaled_h > 0:
                        scaled = pygame.transform.scale(subsurface, (scaled_w, scaled_h))
                        # Make semi-transparent
                        scaled.set_alpha(128)
                        # Apply pivot offset (scaled)
                        draw_x = screen_pos[0] - int(pivot_x * self.zoom)
                        draw_y = screen_pos[1] - int(pivot_y * self.zoom)
                        self.screen.blit(scaled, (draw_x, draw_y))
                        # Draw outline with animation indicator color
                        preview_rect = pygame.Rect(draw_x, draw_y, scaled_w, scaled_h)
                        pygame.draw.rect(self.screen, (255, 200, 50), preview_rect, 2)
                        # Draw pivot point indicator
                        pygame.draw.circle(self.screen, (255, 0, 0), (int(screen_pos[0]), int(screen_pos[1])), 4)
                except:
                    pass

        # Reset clipping
        self.screen.set_clip(None)

        # Draw UI
        self._draw_left_panel()
        self._draw_right_panel()
        self._draw_top_bar()
        self._draw_bottom_bar()

        # Draw notification
        if self.notification:
            elapsed = pygame.time.get_ticks() - self.notification_time
            if elapsed < 2000:
                alpha = max(0, 255 - int(255 * elapsed / 2000))
                notif_surf = self.font.render(self.notification, True, (100, 255, 100))
                self.screen.blit(notif_surf, (canvas.centerx - notif_surf.get_width() // 2, canvas.y + 10))
            else:
                self.notification = None

        pygame.display.flip()

    def _draw_grid(self, canvas):
        # Calculate visible grid range
        gs = self.grid_size
        start_x = int(-self.cam_x / self.zoom / gs) - 1
        end_x = int((-self.cam_x + canvas.width) / self.zoom / gs) + 1
        start_y = int(-self.cam_y / self.zoom / gs) - 1
        end_y = int((-self.cam_y + canvas.height) / self.zoom / gs) + 1

        for gx in range(start_x, end_x + 1):
            screen_x = gx * gs * self.zoom + self.cam_x + canvas.x
            if canvas.left <= screen_x <= canvas.right:
                pygame.draw.line(self.screen, GRID_COLOR, (screen_x, canvas.top), (screen_x, canvas.bottom))

        for gy in range(start_y, end_y + 1):
            screen_y = gy * gs * self.zoom + self.cam_y + canvas.y
            if canvas.top <= screen_y <= canvas.bottom:
                pygame.draw.line(self.screen, GRID_COLOR, (canvas.left, screen_y), (canvas.right, screen_y))

    def _draw_tiles(self, canvas):
        for tile in self.map_data.tiles:
            if tile.tileset_idx < len(self.tileset_images):
                tileset = self.tileset_images[tile.tileset_idx]
                src_rect = pygame.Rect(tile.src_rect)

                screen_pos = self._world_to_screen(tile.pos)
                dest_rect = pygame.Rect(screen_pos[0], screen_pos[1],
                                       src_rect.width * self.zoom, src_rect.height * self.zoom)

                if dest_rect.colliderect(canvas):
                    subsurface = tileset.subsurface(src_rect)
                    scaled = pygame.transform.scale(subsurface, (int(dest_rect.width), int(dest_rect.height)))
                    self.screen.blit(scaled, dest_rect.topleft)

                    # Highlight selected
                    if tile in self.selected_tiles:
                        pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, dest_rect, 2)

    def _draw_props(self, canvas):
        """Draw props with colored outline based on type"""
        for prop in self.map_data.props:
            # Check if this is an animated prop
            if prop.tileset_idx == -1 and hasattr(prop, 'animation_image'):
                self._draw_animated_prop(prop, canvas)
                continue

            if prop.tileset_idx >= 0 and prop.tileset_idx < len(self.tileset_images):
                tileset = self.tileset_images[prop.tileset_idx]
                src_rect = pygame.Rect(prop.src_rect)

                # Apply pivot offset from prop data
                pivot_mode = prop.data.get("pivot", "top-left")
                pivot_x, pivot_y = get_pivot_offset(pivot_mode, src_rect.width, src_rect.height)

                screen_pos = self._world_to_screen(prop.pos)
                draw_x = screen_pos[0] - int(pivot_x * self.zoom)
                draw_y = screen_pos[1] - int(pivot_y * self.zoom)
                dest_rect = pygame.Rect(draw_x, draw_y,
                                       src_rect.width * self.zoom, src_rect.height * self.zoom)

                if dest_rect.colliderect(canvas):
                    subsurface = tileset.subsurface(src_rect)
                    scaled = pygame.transform.scale(subsurface, (int(dest_rect.width), int(dest_rect.height)))
                    self.screen.blit(scaled, dest_rect.topleft)

                    # Determine which rect to draw border around
                    if prop.data.get("has_collider", False):
                        # Show ONLY collider rect - use fixed defaults (32x32)
                        try:
                            coll_w = int(prop.data.get("collider_width", 32))
                            coll_h = int(prop.data.get("collider_height", 32))
                            coll_off_x = int(prop.data.get("collider_offset_x", 0))
                            coll_off_y = int(prop.data.get("collider_offset_y", 0))
                        except (TypeError, ValueError):
                            coll_w, coll_h = 32, 32
                            coll_off_x, coll_off_y = 0, 0
                        coll_type = prop.data.get("collider_type", "box")
                        # Collider rect relative to prop's pivot position
                        collider_screen_x = screen_pos[0] + int(coll_off_x * self.zoom)
                        collider_screen_y = screen_pos[1] + int(coll_off_y * self.zoom)
                        collider_rect = pygame.Rect(collider_screen_x, collider_screen_y,
                                                   int(coll_w * self.zoom), int(coll_h * self.zoom))
                        # Get collider color from schema
                        schema_coll = self.map_data.schema.get("collider_types", {}).get(coll_type, {})
                        color = tuple(schema_coll.get("color", [0, 255, 0]))
                        # Only draw collider border (green), even when selected
                        pygame.draw.rect(self.screen, color, collider_rect, 2)
                    else:
                        # Get color from schema for prop type
                        schema_prop = self.map_data.schema.get("prop_types", {}).get(prop.type, {})
                        color = tuple(schema_prop.get("color", [255, 255, 255]))
                        # Highlight selected with white, otherwise with type color
                        if prop == self.selected_prop:
                            pygame.draw.rect(self.screen, (255, 255, 255), dest_rect, 3)
                        else:
                            pygame.draw.rect(self.screen, color, dest_rect, 2)

    def _draw_animated_prop(self, prop, canvas):
        """Draw an animated prop with animation preview"""
        if not hasattr(prop, 'animation_data') or not hasattr(prop, 'animation_image'):
            return

        frames = prop.animation_data.get("frames", [])
        if not frames:
            return

        # Calculate current frame based on time
        frame_duration = 100  # Default 100ms per frame
        current_time = pygame.time.get_ticks()
        total_duration = len(frames) * frame_duration
        frame_idx = (current_time % total_duration) // frame_duration
        frame_idx = min(frame_idx, len(frames) - 1)

        frame_data = frames[frame_idx]
        if isinstance(frame_data, list):
            frame_rect = frame_data
        else:
            frame_rect = frame_data.get("rect", [0, 0, 32, 32])
        src_rect = pygame.Rect(frame_rect[0], frame_rect[1], frame_rect[2], frame_rect[3])

        # Apply pivot offset
        pivot_mode = prop.animation_data.get("pivot", "bottom-center")
        pivot_x, pivot_y = get_pivot_offset(pivot_mode, src_rect.width, src_rect.height)

        screen_pos = self._world_to_screen(prop.pos)
        draw_x = screen_pos[0] - int(pivot_x * self.zoom)
        draw_y = screen_pos[1] - int(pivot_y * self.zoom)
        dest_rect = pygame.Rect(draw_x, draw_y,
                               src_rect.width * self.zoom, src_rect.height * self.zoom)

        if dest_rect.colliderect(canvas):
            subsurface = prop.animation_image.subsurface(src_rect)
            scaled = pygame.transform.scale(subsurface, (int(dest_rect.width), int(dest_rect.height)))
            self.screen.blit(scaled, dest_rect.topleft)

            # Determine which rect to draw border around
            if prop.data.get("has_collider", False):
                # Show ONLY collider rect - use fixed defaults (32x32)
                try:
                    coll_w = int(prop.data.get("collider_width", 32))
                    coll_h = int(prop.data.get("collider_height", 32))
                    coll_off_x = int(prop.data.get("collider_offset_x", 0))
                    coll_off_y = int(prop.data.get("collider_offset_y", 0))
                except (TypeError, ValueError):
                    coll_w, coll_h = 32, 32
                    coll_off_x, coll_off_y = 0, 0
                coll_type = prop.data.get("collider_type", "box")
                # Collider rect relative to prop's pivot position
                collider_screen_x = screen_pos[0] + int(coll_off_x * self.zoom)
                collider_screen_y = screen_pos[1] + int(coll_off_y * self.zoom)
                collider_rect = pygame.Rect(collider_screen_x, collider_screen_y,
                                           int(coll_w * self.zoom), int(coll_h * self.zoom))
                # Get collider color from schema
                schema_coll = self.map_data.schema.get("collider_types", {}).get(coll_type, {})
                color = tuple(schema_coll.get("color", [0, 255, 0]))
                # Only draw collider border (green), even when selected
                pygame.draw.rect(self.screen, color, collider_rect, 2)
            else:
                # Get color from schema for decoration type
                schema_prop = self.map_data.schema.get("prop_types", {}).get("decoration", {})
                color = tuple(schema_prop.get("color", [150, 150, 150]))
                # Highlight selected with white, otherwise with type color
                if prop == self.selected_prop:
                    pygame.draw.rect(self.screen, (255, 255, 255), dest_rect, 3)
                else:
                    pygame.draw.rect(self.screen, color, dest_rect, 2)

    def _draw_colliders(self, canvas):
        for collider in self.map_data.colliders:
            screen_pos = self._world_to_screen(collider.rect[:2])
            w = collider.rect[2] * self.zoom
            h = collider.rect[3] * self.zoom
            draw_rect = pygame.Rect(screen_pos[0], screen_pos[1], w, h)

            if draw_rect.colliderect(canvas):
                schema_type = self.map_data.schema["collider_types"].get(collider.type, {})
                color = tuple(schema_type.get("color", [0, 255, 0]))
                border_width = 3 if collider == self.selected_collider else 2

                # Draw slope as triangle
                if "slope" in collider.type:
                    if "left" in collider.type:
                        # Triangle: bottom-left, bottom-right, top-right
                        points = [draw_rect.bottomleft, draw_rect.bottomright, draw_rect.topright]
                    else:
                        # Triangle: bottom-left, bottom-right, top-left
                        points = [draw_rect.bottomleft, draw_rect.bottomright, draw_rect.topleft]

                    # Fill with transparency
                    s = pygame.Surface((int(w), int(h)), pygame.SRCALPHA)
                    local_points = [(p[0] - draw_rect.x, p[1] - draw_rect.y) for p in points]
                    pygame.draw.polygon(s, (*color, 60), local_points)
                    self.screen.blit(s, draw_rect.topleft)

                    # Border
                    pygame.draw.polygon(self.screen, color, points, border_width)
                else:
                    # Regular rectangle for other colliders
                    s = pygame.Surface((int(w), int(h)), pygame.SRCALPHA)
                    s.fill((*color, 60))
                    self.screen.blit(s, draw_rect.topleft)

                    pygame.draw.rect(self.screen, color, draw_rect, border_width)

    def _draw_objects(self, canvas):
        """Draw custom objects on the canvas"""
        obj_size = int(24 * self.zoom)  # Base size for object markers

        for obj in self.map_data.objects:
            screen_pos = self._world_to_screen(obj.pos)
            center = (int(screen_pos[0]), int(screen_pos[1]))

            # Skip if not visible
            if not canvas.collidepoint(center):
                continue

            # Get color from schema
            schema_obj = self.map_data.schema.get("custom_objects", {}).get(obj.type, {})
            color = tuple(schema_obj.get("color", [255, 255, 255]))
            display_name = schema_obj.get("display_name", obj.type)

            is_selected = obj == self.selected_object

            # Draw marker based on object type
            if "spawn" in obj.type:
                # Draw arrow/triangle for spawn points
                half = max(8, obj_size // 2)
                points = [
                    (center[0], center[1] - half),
                    (center[0] - half, center[1] + half),
                    (center[0] + half, center[1] + half)
                ]
                pygame.draw.polygon(self.screen, color, points)
                if is_selected:
                    pygame.draw.polygon(self.screen, (255, 255, 255), points, 2)
            elif "checkpoint" in obj.type:
                # Draw flag-like shape for checkpoints
                half = max(8, obj_size // 2)
                # Pole
                pygame.draw.line(self.screen, color, (center[0], center[1] - half), (center[0], center[1] + half), 3)
                # Flag
                flag_points = [
                    (center[0], center[1] - half),
                    (center[0] + half, center[1] - half // 2),
                    (center[0], center[1])
                ]
                pygame.draw.polygon(self.screen, color, flag_points)
                if is_selected:
                    pygame.draw.rect(self.screen, (255, 255, 255),
                                   pygame.Rect(center[0] - half, center[1] - half, half * 2, half * 2), 2)
            elif "door" in obj.type:
                # Draw rectangle for doors
                half = max(8, obj_size // 2)
                door_rect = pygame.Rect(center[0] - half // 2, center[1] - half, half, half * 2)
                pygame.draw.rect(self.screen, color, door_rect)
                pygame.draw.rect(self.screen, (255, 255, 255) if is_selected else (100, 100, 100), door_rect, 2)
            else:
                # Default: draw circle
                radius = max(6, obj_size // 2)
                pygame.draw.circle(self.screen, color, center, radius)
                if is_selected:
                    pygame.draw.circle(self.screen, (255, 255, 255), center, radius + 2, 2)

            # Draw label
            label = self.font_small.render(display_name[:10], True, color)
            self.screen.blit(label, (center[0] - label.get_width() // 2, center[1] + max(10, obj_size // 2) + 2))

    def _draw_type_hints(self, canvas):
        """Draw type hints overlay for colliders, props, and objects"""
        # Draw collider type hints
        for collider in self.map_data.colliders:
            screen_pos = self._world_to_screen((collider.rect[0], collider.rect[1]))
            draw_rect = pygame.Rect(
                screen_pos[0], screen_pos[1],
                collider.rect[2] * self.zoom, collider.rect[3] * self.zoom
            )
            if not draw_rect.colliderect(canvas):
                continue

            schema_type = self.map_data.schema["collider_types"].get(collider.type, {})
            display_name = schema_type.get("display_name", collider.type)
            color = tuple(schema_type.get("color", [255, 255, 255]))

            # Draw type label with background
            label = self.font_small.render(display_name, True, color)
            label_x = draw_rect.centerx - label.get_width() // 2
            label_y = draw_rect.centery - label.get_height() // 2

            # Semi-transparent background
            bg_rect = pygame.Rect(label_x - 2, label_y - 1, label.get_width() + 4, label.get_height() + 2)
            bg_surf = pygame.Surface((bg_rect.width, bg_rect.height), pygame.SRCALPHA)
            bg_surf.fill((0, 0, 0, 160))
            self.screen.blit(bg_surf, bg_rect.topleft)
            self.screen.blit(label, (label_x, label_y))

        # Draw prop type hints
        for prop in self.map_data.props:
            screen_pos = self._world_to_screen(prop.pos)
            prop_rect = pygame.Rect(
                screen_pos[0], screen_pos[1],
                prop.src_rect[2] * self.zoom, prop.src_rect[3] * self.zoom
            )
            if not prop_rect.colliderect(canvas):
                continue

            schema_prop = self.map_data.schema.get("prop_types", {}).get(prop.type, {})
            display_name = schema_prop.get("display_name", prop.type)
            color = tuple(schema_prop.get("color", [255, 255, 255]))

            # Draw type label with background
            label = self.font_small.render(display_name, True, color)
            label_x = prop_rect.centerx - label.get_width() // 2
            label_y = prop_rect.centery - label.get_height() // 2

            bg_rect = pygame.Rect(label_x - 2, label_y - 1, label.get_width() + 4, label.get_height() + 2)
            bg_surf = pygame.Surface((bg_rect.width, bg_rect.height), pygame.SRCALPHA)
            bg_surf.fill((0, 0, 0, 160))
            self.screen.blit(bg_surf, bg_rect.topleft)
            self.screen.blit(label, (label_x, label_y))

        # Draw object type hints (objects already show labels, so just add type background)
        for obj in self.map_data.objects:
            screen_pos = self._world_to_screen(obj.pos)
            center = (int(screen_pos[0]), int(screen_pos[1]))
            if not canvas.collidepoint(center):
                continue

            schema_obj = self.map_data.schema.get("custom_objects", {}).get(obj.type, {})
            display_name = schema_obj.get("display_name", obj.type)
            color = tuple(schema_obj.get("color", [255, 255, 255]))

            # Additional type info label at top
            label = self.font_small.render(f"[{display_name}]", True, color)
            label_x = center[0] - label.get_width() // 2
            label_y = center[1] - 35

            bg_rect = pygame.Rect(label_x - 2, label_y - 1, label.get_width() + 4, label.get_height() + 2)
            bg_surf = pygame.Surface((bg_rect.width, bg_rect.height), pygame.SRCALPHA)
            bg_surf.fill((0, 0, 0, 160))
            self.screen.blit(bg_surf, bg_rect.topleft)
            self.screen.blit(label, (label_x, label_y))

    def _draw_left_panel(self):
        h = self.screen.get_height()
        panel_rect = pygame.Rect(0, TOP_BAR_HEIGHT, LEFT_PANEL_WIDTH, h - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)
        pygame.draw.rect(self.screen, PANEL_COLOR, panel_rect)
        pygame.draw.line(self.screen, PANEL_BORDER_COLOR, (LEFT_PANEL_WIDTH, TOP_BAR_HEIGHT), (LEFT_PANEL_WIDTH, h - BOTTOM_BAR_HEIGHT))

        y = TOP_BAR_HEIGHT + 10

        # Layers section
        title = self.font.render("Layers (1-4)", True, TEXT_COLOR)
        self.screen.blit(title, (10, y))
        y += 25

        for layer in self.map_data.schema.get("tile_layers", []):
            name = layer["name"]
            color = HIGHLIGHT_COLOR if self.current_layer == name else TEXT_COLOR
            layer_text = self.font_small.render(f"  {name}", True, color)
            self.screen.blit(layer_text, (10, y))
            y += 18

        y += 20

        # Collider types section (only show in collider mode)
        if self.current_tool == "collider":
            title = self.font.render("Collider Types", True, TEXT_COLOR)
            self.screen.blit(title, (10, y))
            y += 25

            for ctype, cdata in self.map_data.schema.get("collider_types", {}).items():
                color = HIGHLIGHT_COLOR if self.current_collider_type == ctype else TEXT_COLOR
                type_rect = pygame.Rect(10, y, LEFT_PANEL_WIDTH - 20, 20)

                if type_rect.collidepoint(pygame.mouse.get_pos()):
                    pygame.draw.rect(self.screen, (60, 60, 70), type_rect)
                    if pygame.mouse.get_pressed()[0]:
                        self.current_collider_type = ctype

                # Color indicator
                indicator_color = tuple(cdata.get("color", [255, 255, 255]))
                pygame.draw.rect(self.screen, indicator_color, (15, y + 3, 12, 12))

                type_text = self.font_small.render(cdata.get("display_name", ctype), True, color)
                self.screen.blit(type_text, (32, y + 2))
                y += 22

            y += 10

        # Object types section (only show in object mode)
        if self.current_tool == "object":
            title = self.font.render("Object Types", True, TEXT_COLOR)
            self.screen.blit(title, (10, y))
            y += 25

            for otype, odata in self.map_data.schema.get("custom_objects", {}).items():
                color = HIGHLIGHT_COLOR if self.current_object_type == otype else TEXT_COLOR
                type_rect = pygame.Rect(10, y, LEFT_PANEL_WIDTH - 20, 20)

                if type_rect.collidepoint(pygame.mouse.get_pos()):
                    pygame.draw.rect(self.screen, (60, 60, 70), type_rect)
                    if pygame.mouse.get_pressed()[0]:
                        self.current_object_type = otype

                # Color indicator
                indicator_color = tuple(odata.get("color", [255, 255, 255]))
                pygame.draw.rect(self.screen, indicator_color, (15, y + 3, 12, 12))

                type_text = self.font_small.render(odata.get("display_name", otype), True, color)
                self.screen.blit(type_text, (32, y + 2))
                y += 22

            y += 10

        # Tileset section
        title = self.font.render("Tileset (T)", True, TEXT_COLOR)
        self.screen.blit(title, (10, y))

        tileset_rect = self._get_tileset_panel_rect()
        pygame.draw.rect(self.screen, (35, 35, 40), tileset_rect)

        if self.tileset_images:
            img = self.tileset_images[0]
            img_w, img_h = img.get_size()

            # Clip to tileset panel
            self.screen.set_clip(tileset_rect)

            # Calculate scaled image size and position
            scaled_w = int(img_w * self.tileset_zoom)
            scaled_h = int(img_h * self.tileset_zoom)
            draw_x = tileset_rect.x + self.tileset_cam_x
            draw_y = tileset_rect.y + self.tileset_cam_y

            # Draw scaled tileset
            if scaled_w > 0 and scaled_h > 0:
                scaled_img = pygame.transform.scale(img, (scaled_w, scaled_h))
                self.screen.blit(scaled_img, (draw_x, draw_y))

            # Draw detected tile boxes
            for box in self.detected_tile_boxes:
                box_screen = pygame.Rect(
                    draw_x + box[0] * self.tileset_zoom,
                    draw_y + box[1] * self.tileset_zoom,
                    box[2] * self.tileset_zoom,
                    box[3] * self.tileset_zoom
                )
                pygame.draw.rect(self.screen, (100, 200, 100), box_screen, 1)

            # Draw current selection
            if self.selected_tile_src:
                sel_screen = pygame.Rect(
                    draw_x + self.selected_tile_src[0] * self.tileset_zoom,
                    draw_y + self.selected_tile_src[1] * self.tileset_zoom,
                    self.selected_tile_src[2] * self.tileset_zoom,
                    self.selected_tile_src[3] * self.tileset_zoom
                )
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, sel_screen, 3)

            # Draw selection in progress
            if self.is_selecting_tile and self.tile_selection_start and self.tile_selection_end:
                x1, y1 = self.tile_selection_start
                x2, y2 = self.tile_selection_end
                sel_rect = pygame.Rect(
                    draw_x + min(x1, x2) * self.tileset_zoom,
                    draw_y + min(y1, y2) * self.tileset_zoom,
                    abs(x2 - x1) * self.tileset_zoom,
                    abs(y2 - y1) * self.tileset_zoom
                )
                pygame.draw.rect(self.screen, (255, 255, 0), sel_rect, 2)

            self.screen.set_clip(None)

            # Zoom info
            zoom_text = self.font_small.render(f"Zoom: {int(self.tileset_zoom * 100)}%", True, (150, 150, 150))
            self.screen.blit(zoom_text, (tileset_rect.x + 5, tileset_rect.bottom - 18))
        else:
            no_tile_text = self.font_small.render("No tileset loaded", True, (100, 100, 100))
            self.screen.blit(no_tile_text, (tileset_rect.centerx - no_tile_text.get_width() // 2, tileset_rect.centery))

    def _draw_right_panel(self):
        w, h = self.screen.get_size()
        panel_rect = pygame.Rect(w - RIGHT_PANEL_WIDTH, TOP_BAR_HEIGHT, RIGHT_PANEL_WIDTH, h - TOP_BAR_HEIGHT - BOTTOM_BAR_HEIGHT)
        pygame.draw.rect(self.screen, PANEL_COLOR, panel_rect)
        pygame.draw.line(self.screen, PANEL_BORDER_COLOR, (panel_rect.x, TOP_BAR_HEIGHT), (panel_rect.x, h - BOTTOM_BAR_HEIGHT))

        y = TOP_BAR_HEIGHT + 10

        title = self.font.render("Properties", True, TEXT_COLOR)
        self.screen.blit(title, (panel_rect.x + 10, y))
        y += 30

        # Clear property fields for fresh rebuild each frame
        old_expanded = {k: v.get("expanded", False) for k, v in self.property_fields.items()}
        self.property_fields.clear()

        # Store expanded dropdowns to draw at the end (on top of everything)
        pending_dropdowns = []

        if self.selected_object:
            # Object type (dropdown)
            custom_objects = self.map_data.schema.get("custom_objects", {})
            type_options = list(custom_objects.keys())
            schema_obj = custom_objects.get(self.selected_object.type, {})

            label_text = self.font_small.render("Type:", True, (180, 180, 180))
            self.screen.blit(label_text, (panel_rect.x + 10, y))

            field_x = panel_rect.x + 50
            field_w = panel_rect.width - 60
            field_rect = pygame.Rect(field_x, y, field_w, 18)
            pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
            pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
            display_name = schema_obj.get('display_name', self.selected_object.type)
            val_text = self.font_small.render(display_name, True, TEXT_COLOR)
            self.screen.blit(val_text, (field_x + 5, y + 2))

            self.property_fields["__object_type__"] = {"rect": field_rect, "type": "object_type", "options": type_options, "expanded": old_expanded.get("__object_type__", False)}

            if self.property_fields["__object_type__"].get("expanded"):
                # Store for later drawing on top
                display_options = [(opt, custom_objects.get(opt, {}).get("display_name", opt)) for opt in type_options]
                pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": display_options})
            y += 22

            # Position
            pos_text = self.font_small.render(f"Pos: {self.selected_object.pos}", True, TEXT_COLOR)
            self.screen.blit(pos_text, (panel_rect.x + 10, y))
            y += 25

            # Fields from schema - editable
            for field in schema_obj.get("fields", []):
                field_name = field["name"]
                field_type = field["type"]
                current_value = self.selected_object.data.get(field_name, field.get("default", ""))

                label_text = self.font_small.render(f"{field_name}:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 10, y))

                # Draw editable field based on type
                field_x = panel_rect.x + 90
                field_w = panel_rect.width - 100
                field_rect = pygame.Rect(field_x, y, field_w, 18)

                if field_type == "enum":
                    options = field.get("options", [])
                    pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    self.property_fields[field_name] = {"rect": field_rect, "type": "enum", "options": options, "expanded": old_expanded.get(field_name, False), "target": "object"}

                    if self.property_fields[field_name].get("expanded"):
                        # Store for later drawing on top
                        pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": [(o, o) for o in options]})

                elif field_type == "bool":
                    check_rect = pygame.Rect(field_x, y + 2, 14, 14)
                    pygame.draw.rect(self.screen, (60, 60, 70), check_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, check_rect, 1)
                    if current_value:
                        inner = check_rect.inflate(-4, -4)
                        pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, inner)
                    self.property_fields[field_name] = {"rect": check_rect, "type": "bool", "target": "object"}

                else:
                    is_active = self.active_input == field_name
                    bg_color = (70, 70, 90) if is_active else (60, 60, 70)
                    pygame.draw.rect(self.screen, bg_color, field_rect)
                    pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active else PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    self.property_fields[field_name] = {"rect": field_rect, "type": field_type, "target": "object"}

                y += 22

        elif self.selected_prop:
            # Prop type (dropdown)
            prop_types = self.map_data.schema.get("prop_types", {})
            type_options = list(prop_types.keys())
            schema_prop = prop_types.get(self.selected_prop.type, {})

            label_text = self.font_small.render("Type:", True, (180, 180, 180))
            self.screen.blit(label_text, (panel_rect.x + 10, y))

            field_x = panel_rect.x + 50
            field_w = panel_rect.width - 60
            field_rect = pygame.Rect(field_x, y, field_w, 18)
            pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
            pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
            display_name = schema_prop.get('display_name', self.selected_prop.type)
            val_text = self.font_small.render(display_name, True, TEXT_COLOR)
            self.screen.blit(val_text, (field_x + 5, y + 2))

            self.property_fields["__prop_type__"] = {"rect": field_rect, "type": "prop_type", "options": type_options, "expanded": old_expanded.get("__prop_type__", False)}

            if self.property_fields["__prop_type__"].get("expanded"):
                # Store for later drawing on top
                display_options = [(opt, prop_types.get(opt, {}).get("display_name", opt)) for opt in type_options]
                pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": display_options})
            y += 22

            # Position
            pos_text = self.font_small.render(f"Pos: {self.selected_prop.pos}", True, TEXT_COLOR)
            self.screen.blit(pos_text, (panel_rect.x + 10, y))
            y += 20

            # Src rect
            src_text = self.font_small.render(f"Src: {self.selected_prop.src_rect}", True, TEXT_COLOR)
            self.screen.blit(src_text, (panel_rect.x + 10, y))
            y += 25

            # Fields from schema - editable
            for field in schema_prop.get("fields", []):
                field_name = field["name"]
                field_type = field["type"]
                current_value = self.selected_prop.data.get(field_name, field.get("default", ""))

                label_text = self.font_small.render(f"{field_name}:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 10, y))

                # Draw editable field based on type
                field_x = panel_rect.x + 90
                field_w = panel_rect.width - 100
                field_rect = pygame.Rect(field_x, y, field_w, 18)

                if field_type == "enum":
                    options = field.get("options", [])
                    pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    self.property_fields[field_name] = {"rect": field_rect, "type": "enum", "options": options, "expanded": old_expanded.get(field_name, False), "target": "prop"}

                    if self.property_fields[field_name].get("expanded"):
                        # Store for later drawing on top
                        pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": [(o, o) for o in options]})

                elif field_type == "bool":
                    check_rect = pygame.Rect(field_x, y + 2, 14, 14)
                    pygame.draw.rect(self.screen, (60, 60, 70), check_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, check_rect, 1)
                    if current_value:
                        inner = check_rect.inflate(-4, -4)
                        pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, inner)
                    self.property_fields[field_name] = {"rect": check_rect, "type": "bool", "target": "prop"}

                else:
                    is_active = self.active_input == field_name
                    bg_color = (70, 70, 90) if is_active else (60, 60, 70)
                    pygame.draw.rect(self.screen, bg_color, field_rect)
                    pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active else PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    self.property_fields[field_name] = {"rect": field_rect, "type": field_type, "target": "prop"}

                y += 22

            # --- Pivot Mode ---
            y += 5
            separator_text = self.font_small.render("--- Pivot ---", True, (120, 120, 120))
            self.screen.blit(separator_text, (panel_rect.x + 10, y))
            y += 18

            pivot_options = ["top-left", "top-center", "top-right", "center-left", "center", "center-right", "bottom-left", "bottom-center", "bottom-right"]
            current_pivot = self.selected_prop.data.get("pivot", "top-left")

            label_text = self.font_small.render("Pivot:", True, (180, 180, 180))
            self.screen.blit(label_text, (panel_rect.x + 10, y))

            field_x = panel_rect.x + 50
            field_w = panel_rect.width - 60
            field_rect = pygame.Rect(field_x, y, field_w, 18)
            pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
            pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
            val_text = self.font_small.render(current_pivot, True, TEXT_COLOR)
            self.screen.blit(val_text, (field_x + 5, y + 2))

            self.property_fields["__prop_pivot__"] = {"rect": field_rect, "type": "prop_pivot", "options": pivot_options, "expanded": old_expanded.get("__prop_pivot__", False)}

            if self.property_fields["__prop_pivot__"].get("expanded"):
                pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": [(o, o) for o in pivot_options]})
            y += 22

            # --- Collider Settings ---
            y += 5
            separator_text = self.font_small.render("--- Collider ---", True, (120, 120, 120))
            self.screen.blit(separator_text, (panel_rect.x + 10, y))
            y += 18

            # Has Collider checkbox
            has_collider = self.selected_prop.data.get("has_collider", False)
            label_text = self.font_small.render("Has Collider:", True, (180, 180, 180))
            self.screen.blit(label_text, (panel_rect.x + 10, y))

            check_rect = pygame.Rect(panel_rect.x + 100, y + 2, 14, 14)
            pygame.draw.rect(self.screen, (60, 60, 70), check_rect)
            pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, check_rect, 1)
            if has_collider:
                inner = check_rect.inflate(-4, -4)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, inner)
            self.property_fields["__prop_has_collider__"] = {"rect": check_rect, "type": "prop_has_collider"}
            y += 20

            if has_collider:
                # Collider Type dropdown
                collider_type_options = list(self.map_data.schema.get("collider_types", {}).keys())
                current_coll_type = self.selected_prop.data.get("collider_type", "box")

                label_text = self.font_small.render("Type:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 20, y))

                field_x = panel_rect.x + 60
                field_w = panel_rect.width - 70
                field_rect = pygame.Rect(field_x, y, field_w, 18)
                pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
                pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
                val_text = self.font_small.render(current_coll_type, True, TEXT_COLOR)
                self.screen.blit(val_text, (field_x + 5, y + 2))

                self.property_fields["__prop_collider_type__"] = {"rect": field_rect, "type": "prop_collider_type", "options": collider_type_options, "expanded": old_expanded.get("__prop_collider_type__", False)}

                if self.property_fields["__prop_collider_type__"].get("expanded"):
                    pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": [(o, o) for o in collider_type_options]})
                y += 20

                # Collider Size
                coll_w = self.selected_prop.data.get("collider_width", self.selected_prop.src_rect[2] if self.selected_prop.src_rect else 32)
                coll_h = self.selected_prop.data.get("collider_height", self.selected_prop.src_rect[3] if self.selected_prop.src_rect else 32)

                label_text = self.font_small.render("Size:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 20, y))

                # Width field
                field_x = panel_rect.x + 60
                field_w = 50
                field_rect = pygame.Rect(field_x, y, field_w, 18)
                is_active = self.active_input == "__prop_collider_width__"
                bg_color = (70, 70, 90) if is_active else (60, 60, 70)
                pygame.draw.rect(self.screen, bg_color, field_rect)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active else PANEL_BORDER_COLOR, field_rect, 1)
                val_text = self.font_small.render(str(coll_w), True, TEXT_COLOR)
                self.screen.blit(val_text, (field_x + 5, y + 2))
                self.property_fields["__prop_collider_width__"] = {"rect": field_rect, "type": "prop_collider_size", "field": "collider_width"}

                # "x" separator
                x_text = self.font_small.render("x", True, TEXT_COLOR)
                self.screen.blit(x_text, (field_x + field_w + 5, y + 2))

                # Height field
                field_x2 = field_x + field_w + 20
                field_rect2 = pygame.Rect(field_x2, y, field_w, 18)
                is_active2 = self.active_input == "__prop_collider_height__"
                bg_color2 = (70, 70, 90) if is_active2 else (60, 60, 70)
                pygame.draw.rect(self.screen, bg_color2, field_rect2)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active2 else PANEL_BORDER_COLOR, field_rect2, 1)
                val_text2 = self.font_small.render(str(coll_h), True, TEXT_COLOR)
                self.screen.blit(val_text2, (field_x2 + 5, y + 2))
                self.property_fields["__prop_collider_height__"] = {"rect": field_rect2, "type": "prop_collider_size", "field": "collider_height"}
                y += 20

                # Collider Offset
                off_x = self.selected_prop.data.get("collider_offset_x", 0)
                off_y = self.selected_prop.data.get("collider_offset_y", 0)

                label_text = self.font_small.render("Offset:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 20, y))

                # Offset X field
                field_x = panel_rect.x + 60
                field_w = 50
                field_rect = pygame.Rect(field_x, y, field_w, 18)
                is_active = self.active_input == "__prop_collider_offset_x__"
                bg_color = (70, 70, 90) if is_active else (60, 60, 70)
                pygame.draw.rect(self.screen, bg_color, field_rect)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active else PANEL_BORDER_COLOR, field_rect, 1)
                val_text = self.font_small.render(str(off_x), True, TEXT_COLOR)
                self.screen.blit(val_text, (field_x + 5, y + 2))
                self.property_fields["__prop_collider_offset_x__"] = {"rect": field_rect, "type": "prop_collider_offset", "field": "collider_offset_x"}

                # "," separator
                comma_text = self.font_small.render(",", True, TEXT_COLOR)
                self.screen.blit(comma_text, (field_x + field_w + 5, y + 2))

                # Offset Y field
                field_x2 = field_x + field_w + 20
                field_rect2 = pygame.Rect(field_x2, y, field_w, 18)
                is_active2 = self.active_input == "__prop_collider_offset_y__"
                bg_color2 = (70, 70, 90) if is_active2 else (60, 60, 70)
                pygame.draw.rect(self.screen, bg_color2, field_rect2)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active2 else PANEL_BORDER_COLOR, field_rect2, 1)
                val_text2 = self.font_small.render(str(off_y), True, TEXT_COLOR)
                self.screen.blit(val_text2, (field_x2 + 5, y + 2))
                self.property_fields["__prop_collider_offset_y__"] = {"rect": field_rect2, "type": "prop_collider_offset", "field": "collider_offset_y"}
                y += 22

        elif self.selected_collider:
            # Type
            type_text = self.font_small.render(f"Type: {self.selected_collider.type}", True, TEXT_COLOR)
            self.screen.blit(type_text, (panel_rect.x + 10, y))
            y += 20

            # Rect info
            rect_text = self.font_small.render(f"Rect: {self.selected_collider.rect}", True, TEXT_COLOR)
            self.screen.blit(rect_text, (panel_rect.x + 10, y))
            y += 25

            # Fields from schema - editable
            schema_type = self.map_data.schema["collider_types"].get(self.selected_collider.type, {})
            for field in schema_type.get("fields", []):
                field_name = field["name"]
                field_type = field["type"]
                current_value = self.selected_collider.data.get(field_name, field.get("default", ""))

                label_text = self.font_small.render(f"{field_name}:", True, (180, 180, 180))
                self.screen.blit(label_text, (panel_rect.x + 10, y))

                # Draw editable field based on type
                field_x = panel_rect.x + 90
                field_w = panel_rect.width - 100
                field_rect = pygame.Rect(field_x, y, field_w, 18)

                if field_type == "enum":
                    # Dropdown style
                    options = field.get("options", [])
                    pygame.draw.rect(self.screen, (60, 60, 70), field_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    # Store for click handling
                    self.property_fields[field_name] = {"rect": field_rect, "type": "enum", "options": options, "expanded": old_expanded.get(field_name, False)}

                    # Store expanded dropdown for later drawing on top
                    if self.property_fields[field_name].get("expanded"):
                        pending_dropdowns.append({"x": field_x, "y": y, "w": field_w, "options": [(o, o) for o in options]})

                elif field_type == "bool":
                    # Checkbox
                    check_rect = pygame.Rect(field_x, y + 2, 14, 14)
                    pygame.draw.rect(self.screen, (60, 60, 70), check_rect)
                    pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, check_rect, 1)
                    if current_value:
                        inner = check_rect.inflate(-4, -4)
                        pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, inner)
                    self.property_fields[field_name] = {"rect": check_rect, "type": "bool"}

                else:
                    # Text input (string, int, float)
                    is_active = self.active_input == field_name
                    bg_color = (70, 70, 90) if is_active else (60, 60, 70)
                    pygame.draw.rect(self.screen, bg_color, field_rect)
                    pygame.draw.rect(self.screen, HIGHLIGHT_COLOR if is_active else PANEL_BORDER_COLOR, field_rect, 1)
                    val_text = self.font_small.render(str(current_value), True, TEXT_COLOR)
                    self.screen.blit(val_text, (field_x + 5, y + 2))
                    self.property_fields[field_name] = {"rect": field_rect, "type": field_type}

                y += 22

        elif self.selected_tiles:
            tile = self.selected_tiles[0]
            layer_text = self.font_small.render(f"Layer: {tile.layer}", True, TEXT_COLOR)
            self.screen.blit(layer_text, (panel_rect.x + 10, y))
            y += 20

            pos_text = self.font_small.render(f"Pos: {tile.pos}", True, TEXT_COLOR)
            self.screen.blit(pos_text, (panel_rect.x + 10, y))
            y += 20

            src_text = self.font_small.render(f"Src: {tile.src_rect}", True, TEXT_COLOR)
            self.screen.blit(src_text, (panel_rect.x + 10, y))
        else:
            no_sel_text = self.font_small.render("No selection", True, (100, 100, 100))
            self.screen.blit(no_sel_text, (panel_rect.x + 10, y))

        # Draw all expanded dropdowns on top (deferred rendering)
        for dropdown in pending_dropdowns:
            dx, dy, dw = dropdown["x"], dropdown["y"], dropdown["w"]
            for i, (opt_key, opt_display) in enumerate(dropdown["options"]):
                opt_rect = pygame.Rect(dx, dy + (i + 1) * 20, dw, 18)
                pygame.draw.rect(self.screen, (70, 70, 85), opt_rect)
                pygame.draw.rect(self.screen, PANEL_BORDER_COLOR, opt_rect, 1)
                opt_text = self.font_small.render(opt_display, True, TEXT_COLOR)
                self.screen.blit(opt_text, (opt_rect.x + 5, opt_rect.y + 2))

    def _draw_top_bar(self):
        w = self.screen.get_width()
        bar_rect = pygame.Rect(0, 0, w, TOP_BAR_HEIGHT)
        pygame.draw.rect(self.screen, PANEL_COLOR, bar_rect)
        pygame.draw.line(self.screen, PANEL_BORDER_COLOR, (0, TOP_BAR_HEIGHT), (w, TOP_BAR_HEIGHT))

        # Draw tool buttons
        for btn, tool in self.tool_buttons:
            btn.draw(self.screen)

        # Help text
        help_text = self.font_small.render("Ctrl+O/S:Open/Save | T:Tileset | A:Anim | F2:Schema | G:Grid | [/]:Size | N:Snap | H:Hints", True, (150, 150, 150))
        self.screen.blit(help_text, (w - help_text.get_width() - 10, 8))

    def _draw_bottom_bar(self):
        w, h = self.screen.get_size()
        bar_rect = pygame.Rect(0, h - BOTTOM_BAR_HEIGHT, w, BOTTOM_BAR_HEIGHT)
        pygame.draw.rect(self.screen, PANEL_COLOR, bar_rect)
        pygame.draw.line(self.screen, PANEL_BORDER_COLOR, (0, h - BOTTOM_BAR_HEIGHT), (w, h - BOTTOM_BAR_HEIGHT))

        # Status info
        snap_status = "ON" if self.snap_to_grid else "OFF"
        status_parts = [
            f"Tool: {self.current_tool}",
            f"Layer: {self.current_layer}",
            f"Grid: {self.grid_size}px",
            f"Snap: {snap_status}",
            f"Zoom: {int(self.zoom * 100)}%",
            f"Tiles: {len(self.map_data.tiles)}",
            f"Colliders: {len(self.map_data.colliders)}",
            f"Props: {len(self.map_data.props)}",
            f"Objects: {len(self.map_data.objects)}"
        ]
        # Show animation mode indicator
        if self.loaded_animation:
            anim_name = os.path.basename(self.animation_path) if self.animation_path else "Loaded"
            status_parts.insert(1, f"[ANIM: {anim_name}]")
        status_text = " | ".join(status_parts)
        status_surf = self.font_small.render(status_text, True, TEXT_COLOR)
        self.screen.blit(status_surf, (10, h - BOTTOM_BAR_HEIGHT + 5))

    def run(self):
        while self.running:
            self.handle_events()
            self.update()
            self.draw()
            self.clock.tick(60)

        pygame.quit()


def main():
    editor = MapEditor()
    editor.run()


if __name__ == "__main__":
    main()
