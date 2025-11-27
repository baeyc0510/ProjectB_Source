import pygame
import json
import sys
import os
import io
import cv2
import numpy as np

# Fix Unicode output on Windows
if sys.platform == 'win32':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')


def cv2_imread_unicode(filepath, flags=cv2.IMREAD_UNCHANGED):
    """Read image with Unicode path support (Windows)."""
    try:
        # Use numpy to read file bytes, then decode with OpenCV
        with open(filepath, 'rb') as f:
            data = np.frombuffer(f.read(), dtype=np.uint8)
        return cv2.imdecode(data, flags)
    except Exception as e:
        print(f"Error reading image: {e}")
        return None

# --- Tool Config ---
def load_tool_config():
    """Load tool configuration from tool_config.json"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Default project_root to the parent of the Scripts folder
    # This is the ultimate fallback if no config file or invalid path in config.
    default_project_root = os.path.dirname(script_dir) 

    config_path = os.path.join(script_dir, "tool_config.json")
    
    config = {
        "project_root": default_project_root,
        "resources_folder": "Resources",
        "maps_folder": "Maps",
        "animations_folder": "Animations"
    }

    if os.path.exists(config_path):
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                loaded_config = json.load(f)
                config.update(loaded_config) # Update default config with loaded values

            # Normalize path separators
            if "project_root" in config:
                config["project_root"] = config["project_root"].replace("\\", "/")
            
            # Validate project_root and apply fallback if necessary
            configured_project_root = config["project_root"]
            drive, _ = os.path.splitdrive(configured_project_root)
            
            is_drive_missing = drive and not os.path.exists(drive)
            is_path_missing = not os.path.exists(configured_project_root)

            if is_drive_missing or is_path_missing:
                # Only warn if the configured root is different from the default fallback
                if configured_project_root != default_project_root:
                    print(f"Warning: Configured project_root '{configured_project_root}' not found or its drive is missing. Falling back to '{default_project_root}'.")
                config["project_root"] = default_project_root

        except Exception as e:
            print(f"Warning: Failed to load or parse tool_config.json: {e}. Using default configuration.")
    
    return config

def get_relative_path(absolute_path, project_root):
    """Get relative path from project root"""
    abs_path = os.path.abspath(absolute_path).replace("\\", "/")
    proj_root = os.path.abspath(project_root).replace("\\", "/")
    if abs_path.startswith(proj_root):
        rel_path = abs_path[len(proj_root):].lstrip("/")
        return rel_path
    return os.path.basename(absolute_path)

TOOL_CONFIG = load_tool_config()

# --- Configuration ---
BACKGROUND_COLOR = (30, 30, 30)
TEXT_COLOR = (220, 220, 220)
FONT_SIZE = 20

RECT_COLOR = (255, 0, 0)
SAVED_RECT_COLOR = (0, 255, 0)
SELECTED_RECT_COLOR = (0, 150, 255)
HANDLE_COLOR = (255, 255, 0)
NUMBER_COLOR = (255, 255, 0)
RECT_WIDTH = 1
HANDLE_SIZE = 8
FRAME_FONT_SIZE = 16
HELP_TEXT_COLOR = (200, 200, 200)
HELP_BG_COLOR = (60, 60, 60)
PREVIEW_BG_COLOR = (0, 0, 0, 180)
PREVIEW_FPS = 12
SNAP_TOLERANCE = 5
MIN_CONTOUR_AREA = 64

# --- Detail Panel Configuration ---
DETAIL_PANEL_WIDTH = 220
DETAIL_PANEL_COLOR = (45, 45, 45)
DETAIL_LABEL_COLOR = (180, 180, 180)
DETAIL_VALUE_COLOR = (255, 255, 255)
INPUT_BG_COLOR = (60, 60, 60)
INPUT_ACTIVE_COLOR = (80, 80, 120)
INPUT_BORDER_COLOR = (100, 100, 100)
INPUT_ACTIVE_BORDER_COLOR = (100, 150, 255)

# --- Editor Help Text ---
EDITOR_HELP_LINES = [
    "LMB: Drag=New | Click=Select | RMB: Pan | Wheel: Zoom",
    "N: Save new | D: Delete | P: Preview | T: Snap | V: Pivot | G: Show Pivots",
    "A: Auto-Detect | R: Grid Split | F: Split Frame | X: Export | S: Save | O/Q: Open/Quit"
]
PIVOT_CROSS_COLOR = (255, 100, 100)
PIVOT_CROSS_SIZE = 8

# --- Input Field Class ---
class InputField:
    def __init__(self, x, y, width, height, label, font):
        self.rect = pygame.Rect(x, y, width, height)
        self.label = label
        self.font = font
        self.text = ""
        self.active = False
        self.cursor_visible = True
        self.cursor_timer = 0

    def set_value(self, value):
        self.text = str(int(value)) if value is not None else ""

    def get_value(self):
        try:
            return int(self.text) if self.text else 0
        except ValueError:
            return 0

    def handle_event(self, event):
        if event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_BACKSPACE:
                self.text = self.text[:-1]
                return True
            elif event.key in (pygame.K_RETURN, pygame.K_TAB):
                return False  # Let parent handle tab/enter
            elif event.unicode.isdigit() or (event.unicode == '-' and len(self.text) == 0):
                self.text += event.unicode
                return True
        return False

    def update(self, dt):
        self.cursor_timer += dt
        if self.cursor_timer >= 500:
            self.cursor_visible = not self.cursor_visible
            self.cursor_timer = 0

    def draw(self, screen, panel_x):
        actual_rect = pygame.Rect(panel_x + self.rect.x, self.rect.y, self.rect.width, self.rect.height)

        # Draw label
        label_surf = self.font.render(self.label + ":", True, DETAIL_LABEL_COLOR)
        screen.blit(label_surf, (panel_x + 10, self.rect.y + 4))

        # Draw input box
        bg_color = INPUT_ACTIVE_COLOR if self.active else INPUT_BG_COLOR
        border_color = INPUT_ACTIVE_BORDER_COLOR if self.active else INPUT_BORDER_COLOR
        pygame.draw.rect(screen, bg_color, actual_rect)
        pygame.draw.rect(screen, border_color, actual_rect, 2)

        # Draw text
        text_surf = self.font.render(self.text, True, DETAIL_VALUE_COLOR)
        text_x = actual_rect.x + 5
        text_y = actual_rect.y + (actual_rect.height - text_surf.get_height()) // 2
        screen.blit(text_surf, (text_x, text_y))

        # Draw cursor
        if self.active and self.cursor_visible:
            cursor_x = text_x + text_surf.get_width() + 2
            pygame.draw.line(screen, DETAIL_VALUE_COLOR, (cursor_x, text_y), (cursor_x, text_y + text_surf.get_height()), 2)

class StringInputField(InputField):
    def set_value(self, value):
        self.text = str(value) if value is not None else ""

    def get_value(self):
        return self.text

    def handle_event(self, event):
        if event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_BACKSPACE:
                self.text = self.text[:-1]
                return True
            elif event.key in (pygame.K_RETURN, pygame.K_TAB):
                return False  # Let parent handle
            else:
                self.text += event.unicode
                return True
        return False
        
class EventEditor:
    def __init__(self, x, y, font):
        self.x = x
        self.y = y
        self.font = font
        self.event_fields = []
        self.active_field_idx = -1
        self.add_button_rect = None
        self.remove_button_rects = []

    def set_events(self, events):
        self.event_fields = []
        y_offset = 0
        for event_str in events:
            field = StringInputField(self.x, self.y + y_offset, 150, 24, "", self.font)
            field.set_value(event_str)
            self.event_fields.append(field)
            y_offset += 28

    def get_events(self):
        return [field.get_value() for field in self.event_fields]

    def handle_event(self, event, panel_x):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            # Handle add button click
            if self.add_button_rect and self.add_button_rect.collidepoint(event.pos):
                self.event_fields.append(StringInputField(self.x, self.y + len(self.event_fields) * 28, 150, 24, "", self.font))
                return True

            # Handle remove button clicks
            for i, rect in reversed(list(enumerate(self.remove_button_rects))):
                if rect.collidepoint(event.pos):
                    self.event_fields.pop(i)
                    # Deactivate if it was active
                    if self.active_field_idx == i:
                        self.active_field_idx = -1
                    elif self.active_field_idx > i:
                        self.active_field_idx -= 1
                    return True
            
            # Handle activating an input field
            for i, field in enumerate(self.event_fields):
                actual_rect = pygame.Rect(panel_x + field.rect.x, field.rect.y, field.rect.width, field.rect.height)
                if actual_rect.collidepoint(event.pos):
                    self.active_field_idx = i
                    for j, f in enumerate(self.event_fields):
                        f.active = (j == i)
                    return True

        if self.active_field_idx != -1:
            field = self.event_fields[self.active_field_idx]
            if field.handle_event(event):
                return True
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_TAB:
                    field.active = False
                    self.active_field_idx = (self.active_field_idx + 1) % len(self.event_fields)
                    self.event_fields[self.active_field_idx].active = True
                    return True
                if event.key == pygame.K_RETURN:
                    field.active = False
                    self.active_field_idx = -1
                    return True
        return False

    def draw(self, screen, panel_x):
        title_surf = self.font.render("Events:", True, DETAIL_LABEL_COLOR)
        screen.blit(title_surf, (panel_x + 10, self.y - 25))
        
        self.remove_button_rects = []
        y_offset = 0
        for i, field in enumerate(self.event_fields):
            field.rect.y = self.y + y_offset
            field.draw(screen, panel_x)
            
            # Draw remove button
            remove_rect = pygame.Rect(panel_x + field.rect.right + 5, field.rect.y, 20, 24)
            pygame.draw.rect(screen, (180, 50, 50), remove_rect)
            remove_surf = self.font.render("X", True, (255, 255, 255))
            screen.blit(remove_surf, (remove_rect.x + 5, remove_rect.y + 4))
            self.remove_button_rects.append(remove_rect)
            
            y_offset += 28

        # Draw Add Event button
        add_rect = pygame.Rect(panel_x + 10, self.y + y_offset, 100, 24)
        pygame.draw.rect(screen, (50, 180, 50), add_rect)
        add_surf = self.font.render("Add Event", True, (255, 255, 255))
        screen.blit(add_surf, (add_rect.x + 10, add_rect.y + 4))
        self.add_button_rect = add_rect

    def update(self, dt):
        for field in self.event_fields:
            field.update(dt)

# --- Float Input Field Class (for frame_interval) ---
class FloatInputField(InputField):
    def set_value(self, value):
        # Format to a reasonable number of decimal places
        self.text = f"{value:.4f}".rstrip('0').rstrip('.') if value is not None else ""

    def get_value(self):
        try:
            return float(self.text) if self.text else 0.0
        except ValueError:
            return 0.0

    def handle_event(self, event):
        if event.type == pygame.KEYDOWN and self.active:
            if event.key == pygame.K_BACKSPACE:
                self.text = self.text[:-1]
                return True
            elif event.key in (pygame.K_RETURN, pygame.K_TAB):
                return False  # Let parent handle tab/enter
            elif event.unicode.isdigit():
                self.text += event.unicode
                return True
            elif event.unicode == '.' and '.' not in self.text:
                self.text += event.unicode
                return True
        return False

# --- Helper Functions ---
def get_snap_coord(mouse_coord, existing_coords, tolerance):
    for coord in existing_coords:
        if abs(mouse_coord - coord) <= tolerance: return coord
    return mouse_coord


def clamp_rect_to_image(rect, img_width, img_height):
    """Clamp a rect to stay within image boundaries.

    Args:
        rect: pygame.Rect or [x, y, w, h] list
        img_width: Image width
        img_height: Image height

    Returns:
        Clamped [x, y, w, h] list
    """
    if isinstance(rect, pygame.Rect):
        x, y, w, h = rect.x, rect.y, rect.width, rect.height
    else:
        x, y, w, h = rect[0], rect[1], rect[2], rect[3]

    # Clamp position to image bounds
    x = max(0, min(x, img_width - w))
    y = max(0, min(y, img_height - h))

    # Clamp size if it exceeds image bounds
    if x + w > img_width:
        w = img_width - x
    if y + h > img_height:
        h = img_height - y

    # Ensure minimum size
    w = max(1, w)
    h = max(1, h)

    return [int(x), int(y), int(w), int(h)]


def check_rect_overlap(rect1, rect2):
    """Check if two rects overlap.

    Args:
        rect1: [x, y, w, h] list
        rect2: [x, y, w, h] list

    Returns:
        True if rects overlap, False otherwise
    """
    r1 = pygame.Rect(rect1)
    r2 = pygame.Rect(rect2)
    return r1.colliderect(r2)


def get_non_overlapping_position(rect, other_rects, original_rect, img_width, img_height):
    """Find a valid position for rect that doesn't overlap with others.

    Args:
        rect: [x, y, w, h] - the rect being moved/resized
        other_rects: List of other frame rects to check against
        original_rect: [x, y, w, h] - original position to fall back to
        img_width, img_height: Image dimensions

    Returns:
        [x, y, w, h] - valid position (original if overlap detected)
    """
    # First clamp to image bounds
    clamped = clamp_rect_to_image(rect, img_width, img_height)

    # Check for overlaps with other rects
    for other in other_rects:
        if check_rect_overlap(clamped, other):
            # Return original position if overlap detected
            return original_rect

    return clamped


def adjust_rect_to_avoid_overlap(rect, other_rects, img_width, img_height):
    """Adjust a rect to avoid overlapping with other rects.

    Tries to shrink the rect from overlapping sides to fit in available space.

    Args:
        rect: [x, y, w, h] - the rect to adjust
        other_rects: List of [x, y, w, h] rects to avoid
        img_width, img_height: Image dimensions

    Returns:
        Adjusted [x, y, w, h] or None if no valid rect can be created
    """
    if not other_rects:
        return rect

    x, y, w, h = rect
    new_x, new_y, new_w, new_h = x, y, w, h

    # Iterate through overlapping rects and adjust
    for other in other_rects:
        other_rect = pygame.Rect(other)
        current_rect = pygame.Rect(new_x, new_y, new_w, new_h)

        if not current_rect.colliderect(other_rect):
            continue

        # Calculate overlap amounts from each side
        overlap_left = other_rect.right - current_rect.left  # How much to shrink from left
        overlap_right = current_rect.right - other_rect.left  # How much to shrink from right
        overlap_top = other_rect.bottom - current_rect.top  # How much to shrink from top
        overlap_bottom = current_rect.bottom - other_rect.top  # How much to shrink from bottom

        # Find minimum adjustment (smallest change that removes overlap)
        adjustments = []

        # Can we shrink from left? (move left edge right)
        if overlap_left > 0 and overlap_left < new_w:
            adjustments.append(('left', overlap_left))

        # Can we shrink from right? (move right edge left)
        if overlap_right > 0 and overlap_right < new_w:
            adjustments.append(('right', overlap_right))

        # Can we shrink from top? (move top edge down)
        if overlap_top > 0 and overlap_top < new_h:
            adjustments.append(('top', overlap_top))

        # Can we shrink from bottom? (move bottom edge up)
        if overlap_bottom > 0 and overlap_bottom < new_h:
            adjustments.append(('bottom', overlap_bottom))

        if not adjustments:
            # No valid adjustment possible, rect is fully contained or too small
            return None

        # Apply the smallest adjustment
        adjustments.sort(key=lambda a: a[1])
        adj_type, adj_amount = adjustments[0]

        if adj_type == 'left':
            new_x = other_rect.right
            new_w = (x + w) - new_x
        elif adj_type == 'right':
            new_w = other_rect.left - new_x
        elif adj_type == 'top':
            new_y = other_rect.bottom
            new_h = (y + h) - new_y
        elif adj_type == 'bottom':
            new_h = other_rect.top - new_y

    # Validate final rect
    if new_w < 1 or new_h < 1:
        return None

    # Clamp to image bounds
    return clamp_rect_to_image([new_x, new_y, new_w, new_h], img_width, img_height)


def clamp_resize_to_bounds(rect, resize_mode, mx, my, img_width, img_height, other_rects, selected_index):
    """Clamp resize operation to image bounds and prevent overlap.

    Args:
        rect: pygame.Rect being resized
        resize_mode: Which handle is being dragged
        mx, my: Mouse position in image coordinates
        img_width, img_height: Image dimensions
        other_rects: List of all frame rects
        selected_index: Index of selected frame (to exclude from overlap check)

    Returns:
        Modified pygame.Rect
    """
    # Clamp mouse position to image bounds
    mx = max(0, min(mx, img_width))
    my = max(0, min(my, img_height))

    # Store original for overlap fallback
    original = rect.copy()

    # Apply resize based on handle
    if 'right' in resize_mode:
        new_width = mx - rect.x
        rect.width = max(1, min(new_width, img_width - rect.x))
    if 'left' in resize_mode:
        new_x = min(mx, rect.right - 1)
        new_x = max(0, new_x)
        rect.width = rect.right - new_x
        rect.x = new_x
    if 'bottom' in resize_mode:
        new_height = my - rect.y
        rect.height = max(1, min(new_height, img_height - rect.y))
    if 'top' in resize_mode:
        new_y = min(my, rect.bottom - 1)
        new_y = max(0, new_y)
        rect.height = rect.bottom - new_y
        rect.y = new_y

    # Check overlap with other frames
    new_rect = [rect.x, rect.y, rect.width, rect.height]
    for i, other in enumerate(other_rects):
        if i != selected_index and check_rect_overlap(new_rect, other["rect"]):
            return original

    return rect

def get_handles(rect):
    hs = HANDLE_SIZE // 2
    return {'top-left': pygame.Rect(rect.left - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'top-middle': pygame.Rect(rect.centerx - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'top-right': pygame.Rect(rect.right - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'middle-left': pygame.Rect(rect.left - hs, rect.centery - hs, HANDLE_SIZE, HANDLE_SIZE),'middle-right': pygame.Rect(rect.right - hs, rect.centery - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-left': pygame.Rect(rect.left - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-middle': pygame.Rect(rect.centerx - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-right': pygame.Rect(rect.right - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE)}

def get_cursor_for_handle(handle_name):
    if handle_name in ('top-left', 'bottom-right'): return pygame.SYSTEM_CURSOR_SIZENWSE
    if handle_name in ('top-right', 'bottom-left'): return pygame.SYSTEM_CURSOR_SIZENESW
    if 'middle' in handle_name and ('left' in handle_name or 'right' in handle_name): return pygame.SYSTEM_CURSOR_SIZEWE
    if 'middle' in handle_name and ('top' in handle_name or 'bottom' in handle_name): return pygame.SYSTEM_CURSOR_SIZENS
    return pygame.SYSTEM_CURSOR_ARROW

def find_split_points(alpha_region, axis, threshold_ratio=0.05):
    """Find potential split points in a region by analyzing projection.

    CONSERVATIVE: Only splits when there's a very clear gap (near-zero pixels).

    Args:
        alpha_region: 2D numpy array of alpha values
        axis: 0 for horizontal splits (analyze vertical projection), 1 for vertical splits
        threshold_ratio: Ratio of max projection below which is considered a split point

    Returns:
        List of split positions
    """
    # Project alpha values along axis
    projection = np.sum(alpha_region, axis=axis).astype(float)

    if len(projection) == 0:
        return []

    max_val = np.max(projection)
    if max_val == 0:
        return []

    # Very conservative threshold - only split at near-empty rows/columns
    threshold = max_val * threshold_ratio
    split_points = []

    # Find regions below threshold (clear gaps only)
    below_threshold = projection < threshold
    in_gap = False
    gap_start = 0

    for i, is_low in enumerate(below_threshold):
        if is_low and not in_gap:
            in_gap = True
            gap_start = i
        elif not is_low and in_gap:
            in_gap = False
            gap_center = (gap_start + i) // 2
            # Require at least 3 pixel gap for split
            if i - gap_start >= 3:
                split_points.append(gap_center)

    return split_points


def split_connected_sprites_morphological(img_alpha, frame_rect, median_h):
    """Use morphological operations to split connected sprites.

    Args:
        img_alpha: Full image alpha channel
        frame_rect: [x, y, w, h] of the oversized frame
        median_h: Expected single frame height

    Returns:
        List of frame rects for separated sprites
    """
    x, y, w, h = frame_rect
    img_h, img_w = img_alpha.shape

    x = max(0, int(x))
    y = max(0, int(y))
    w = min(int(w), img_w - x)
    h = min(int(h), img_h - y)

    if w <= 0 or h <= 0:
        return [frame_rect]

    # Extract region
    region = img_alpha[y:y+h, x:x+w].copy()

    # Threshold to binary
    _, binary = cv2.threshold(region, 0, 255, cv2.THRESH_BINARY)

    # Apply horizontal erosion to break vertical connections
    # Use a wide, short kernel to break horizontal thin connections between stacked sprites
    kernel_h = np.ones((1, 5), np.uint8)  # Horizontal kernel
    eroded_h = cv2.erode(binary, kernel_h, iterations=2)

    # Apply vertical erosion to break any remaining connections
    kernel_v = np.ones((3, 1), np.uint8)  # Vertical kernel
    eroded = cv2.erode(eroded_h, kernel_v, iterations=1)

    # Find connected components on eroded image
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(eroded, connectivity=8)

    if num_labels <= 2:  # Only background + 1 component (couldn't separate)
        # Try stronger erosion
        kernel_strong = np.ones((5, 3), np.uint8)
        eroded_strong = cv2.erode(binary, kernel_strong, iterations=2)
        num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(eroded_strong, connectivity=8)

    if num_labels <= 2:
        # Still couldn't separate, fall back to median-based split
        return None

    # For each component, find its corresponding full region in the original image
    result_frames = []

    for i in range(1, num_labels):  # Skip background
        comp_x, comp_y, comp_w, comp_h, comp_area = stats[i]

        if comp_area < MIN_CONTOUR_AREA // 4:  # Skip tiny fragments
            continue

        # Create a mask for this component
        comp_mask = (labels == i).astype(np.uint8) * 255

        # Dilate the mask to recover original sprite extent
        kernel_dilate = np.ones((7, 7), np.uint8)
        dilated_mask = cv2.dilate(comp_mask, kernel_dilate, iterations=3)

        # Combine with original to get actual sprite pixels
        sprite_pixels = cv2.bitwise_and(binary, dilated_mask)

        # Find bounds of the recovered sprite
        non_zero = np.where(sprite_pixels > 0)
        if len(non_zero[0]) == 0:
            continue

        min_row = np.min(non_zero[0])
        max_row = np.max(non_zero[0])
        min_col = np.min(non_zero[1])
        max_col = np.max(non_zero[1])

        frame_x = x + min_col
        frame_y = y + min_row
        frame_w = max_col - min_col + 1
        frame_h = max_row - min_row + 1

        # Only add if reasonable size
        if frame_w > 5 and frame_h > 5:
            result_frames.append([int(frame_x), int(frame_y), int(frame_w), int(frame_h)])

    if len(result_frames) > 1:
        return result_frames
    return None


def refine_frame_bounds_strict(img_alpha, frame_rect, padding=1):
    """Refine frame bounds strictly within the given boundaries.

    Only includes content that is actually within the frame_rect boundaries.
    This prevents content from adjacent split regions from being included.

    Args:
        img_alpha: Full image alpha channel
        frame_rect: [x, y, w, h] strict boundary for this frame
        padding: Pixels to add around content (but not exceeding original bounds)

    Returns:
        Refined [x, y, w, h] containing only content within boundaries
    """
    x, y, w, h = frame_rect
    img_h, img_w = img_alpha.shape

    # Clamp to image bounds
    x = max(0, int(x))
    y = max(0, int(y))
    w = min(int(w), img_w - x)
    h = min(int(h), img_h - y)

    if w <= 0 or h <= 0:
        return frame_rect

    # Extract region (strict boundary)
    region = img_alpha[y:y+h, x:x+w]

    # Find non-zero pixels within this strict boundary
    non_zero = np.where(region > 0)

    if len(non_zero[0]) == 0:
        return frame_rect

    # Get bounds of content within this region
    min_row = np.min(non_zero[0])
    max_row = np.max(non_zero[0])
    min_col = np.min(non_zero[1])
    max_col = np.max(non_zero[1])

    # Calculate new bounds with padding, but stay within original frame boundaries
    new_x = x + max(0, min_col - padding)
    new_y = y + max(0, min_row - padding)
    new_right = x + min(w, max_col + 1 + padding)
    new_bottom = y + min(h, max_row + 1 + padding)

    new_w = new_right - new_x
    new_h = new_bottom - new_y

    return [int(new_x), int(new_y), int(new_w), int(new_h)]


def refine_frame_bounds_with_components(img_alpha, frame_rect, padding=1):
    """Refine frame bounds using connected component analysis.

    Finds the main connected component in the region and returns its bounds.

    Args:
        img_alpha: Full image alpha channel
        frame_rect: [x, y, w, h] initial frame rect
        padding: Pixels to add around content

    Returns:
        Refined [x, y, w, h] that contains the main content
    """
    x, y, w, h = frame_rect
    img_h, img_w = img_alpha.shape

    # Clamp to image bounds
    x = max(0, int(x))
    y = max(0, int(y))
    w = min(int(w), img_w - x)
    h = min(int(h), img_h - y)

    if w <= 0 or h <= 0:
        return frame_rect

    # Extract region
    region = img_alpha[y:y+h, x:x+w]

    # Threshold to binary
    _, binary = cv2.threshold(region, 0, 255, cv2.THRESH_BINARY)

    # Find connected components
    num_labels, labels, stats, centroids = cv2.connectedComponentsWithStats(binary, connectivity=8)

    if num_labels <= 1:  # Only background
        return frame_rect

    # Find the component that is most centered vertically in this region
    # (assuming vertical split, the correct frame content should be centered)
    region_center_y = h / 2
    region_center_x = w / 2

    best_component = -1
    best_score = float('inf')

    for i in range(1, num_labels):  # Skip background (0)
        comp_x, comp_y, comp_w, comp_h, comp_area = stats[i]

        if comp_area < MIN_CONTOUR_AREA:
            continue

        # Calculate component center
        comp_center_y = comp_y + comp_h / 2
        comp_center_x = comp_x + comp_w / 2

        # Score based on how centered the component is (prefer centered components)
        # Also prefer larger components
        distance_to_center = abs(comp_center_y - region_center_y) + abs(comp_center_x - region_center_x) * 0.5
        size_bonus = -np.sqrt(comp_area) * 0.1  # Larger is better
        score = distance_to_center + size_bonus

        if score < best_score:
            best_score = score
            best_component = i

    if best_component == -1:
        # No suitable component found, use strict bounds
        return refine_frame_bounds_strict(img_alpha, frame_rect, padding)

    # Get bounds of the best component
    comp_x, comp_y, comp_w, comp_h, _ = stats[best_component]

    # Calculate new bounds with padding, staying within original frame boundaries
    new_x = x + max(0, comp_x - padding)
    new_y = y + max(0, comp_y - padding)
    new_right = x + min(w, comp_x + comp_w + padding)
    new_bottom = y + min(h, comp_y + comp_h + padding)

    new_w = new_right - new_x
    new_h = new_bottom - new_y

    return [int(new_x), int(new_y), int(new_w), int(new_h)]


def refine_frame_bounds(img_alpha, frame_rect, padding=1):
    """Refine frame bounds to tightly fit actual content.

    Args:
        img_alpha: Full image alpha channel
        frame_rect: [x, y, w, h] initial frame rect
        padding: Pixels to add around content

    Returns:
        Refined [x, y, w, h] that contains all non-transparent pixels
    """
    x, y, w, h = frame_rect
    img_h, img_w = img_alpha.shape

    # Clamp to image bounds
    x = max(0, int(x))
    y = max(0, int(y))
    w = min(int(w), img_w - x)
    h = min(int(h), img_h - y)

    if w <= 0 or h <= 0:
        return frame_rect

    # Extract region
    region = img_alpha[y:y+h, x:x+w]

    # Find non-zero pixels
    non_zero = np.where(region > 0)

    if len(non_zero[0]) == 0:
        return frame_rect

    # Get bounds of content
    min_row = np.min(non_zero[0])
    max_row = np.max(non_zero[0])
    min_col = np.min(non_zero[1])
    max_col = np.max(non_zero[1])

    # Calculate new bounds with padding
    new_x = max(0, x + min_col - padding)
    new_y = max(0, y + min_row - padding)
    new_w = min(img_w - new_x, (max_col - min_col + 1) + padding * 2)
    new_h = min(img_h - new_y, (max_row - min_row + 1) + padding * 2)

    return [int(new_x), int(new_y), int(new_w), int(new_h)]


def split_oversized_frame(img_alpha, frame_rect, median_w, median_h, size_threshold=1.8):
    """Split an oversized frame into smaller frames using projection analysis.

    Args:
        img_alpha: Full image alpha channel
        frame_rect: [x, y, w, h] of the frame to potentially split
        median_w, median_h: Median frame dimensions for comparison
        size_threshold: Multiplier above which a frame is considered oversized

    Returns:
        List of frame rects (original if no split needed, or split frames)
    """
    x, y, w, h = frame_rect

    # Check if frame is oversized
    is_wide = w > median_w * size_threshold
    is_tall = h > median_h * size_threshold

    if not is_wide and not is_tall:
        return [frame_rect]

    # Projection-based analysis (more conservative than morphological)
    # Extract the region
    region = img_alpha[y:y+h, x:x+w]

    result_frames = []

    if is_tall and not is_wide:
        # Try horizontal splits (split vertically stacked frames)
        split_points = find_split_points(region, axis=1)  # Sum along columns -> row profile

        if split_points:
            # Add boundaries
            splits = [0] + split_points + [h]
            for i in range(len(splits) - 1):
                new_y = y + splits[i]
                new_h = splits[i + 1] - splits[i]
                if new_h > MIN_CONTOUR_AREA // 8:  # Minimum height check
                    # Use connected component analysis to find the main content
                    refined = refine_frame_bounds_with_components(img_alpha, [x, new_y, w, new_h])
                    result_frames.append(refined)
        else:
            # No good split points, estimate based on median
            num_splits = round(h / median_h)
            if num_splits > 1:
                split_h = h // num_splits
                for i in range(num_splits):
                    refined = refine_frame_bounds_with_components(img_alpha, [x, y + i * split_h, w, split_h])
                    result_frames.append(refined)
            else:
                result_frames.append(frame_rect)

    elif is_wide and not is_tall:
        # Try vertical splits (split horizontally arranged frames)
        split_points = find_split_points(region, axis=0)  # Sum along rows -> column profile

        if split_points:
            splits = [0] + split_points + [w]
            for i in range(len(splits) - 1):
                new_x = x + splits[i]
                new_w = splits[i + 1] - splits[i]
                if new_w > MIN_CONTOUR_AREA // 8:
                    refined = refine_frame_bounds_with_components(img_alpha, [new_x, y, new_w, h])
                    result_frames.append(refined)
        else:
            num_splits = round(w / median_w)
            if num_splits > 1:
                split_w = w // num_splits
                for i in range(num_splits):
                    refined = refine_frame_bounds_with_components(img_alpha, [x + i * split_w, y, split_w, h])
                    result_frames.append(refined)
            else:
                result_frames.append(frame_rect)

    else:
        # Both wide and tall - try both directions
        # First try horizontal splits
        h_splits = find_split_points(region, axis=1)
        v_splits = find_split_points(region, axis=0)

        if h_splits or v_splits:
            # Use the direction with more split points
            if len(h_splits) >= len(v_splits) and h_splits:
                splits = [0] + h_splits + [h]
                for i in range(len(splits) - 1):
                    sub_rect = [x, y + splits[i], w, splits[i + 1] - splits[i]]
                    # Recursively check if sub-frame needs splitting
                    result_frames.extend(split_oversized_frame(img_alpha, sub_rect, median_w, median_h, size_threshold))
            elif v_splits:
                splits = [0] + v_splits + [w]
                for i in range(len(splits) - 1):
                    sub_rect = [x + splits[i], y, splits[i + 1] - splits[i], h]
                    result_frames.extend(split_oversized_frame(img_alpha, sub_rect, median_w, median_h, size_threshold))
            else:
                result_frames.append(frame_rect)
        else:
            result_frames.append(frame_rect)

    return result_frames if result_frames else [frame_rect]


def auto_detect_frames(image_path):
    """Contour-based auto detection with post-correction for oversized frames.

    1st pass: Detect all contours
    2nd pass: Split frames that are significantly larger than median
    """
    try:
        img = cv2_imread_unicode(image_path)
        if img is None or len(img.shape) < 3 or img.shape[2] < 4:
            print("Info: No alpha channel for auto-detection.")
            return []

        img_height, img_width = img.shape[:2]
        alpha_channel = img[:, :, 3]
        _, thresh = cv2.threshold(alpha_channel, 0, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # 1st pass: Detect all frames
        initial_frames = []
        for c in contours:
            if cv2.contourArea(c) > MIN_CONTOUR_AREA:
                x, y, w, h = cv2.boundingRect(c)
                initial_frames.append([x, y, w, h])

        if not initial_frames:
            return []

        # Calculate median size
        widths = [f[2] for f in initial_frames]
        heights = [f[3] for f in initial_frames]
        median_w = np.median(widths)
        median_h = np.median(heights)

        print(f"Frame statistics: median size {int(median_w)}x{int(median_h)}")

        # 2nd pass: Split oversized frames (height > 1.7x median)
        SIZE_THRESHOLD = 1.7
        final_frames = []
        split_count = 0

        for frame_rect in initial_frames:
            x, y, w, h = frame_rect

            # Check if frame is significantly taller than median
            if h > median_h * SIZE_THRESHOLD:
                # Split into equal parts based on median height
                split_result = split_frame_by_median(frame_rect, median_h, img_height)
                if len(split_result) > 1:
                    split_count += len(split_result) - 1
                    final_frames.extend(split_result)
                else:
                    final_frames.append(frame_rect)
            else:
                final_frames.append(frame_rect)

        # Convert to dictionary format
        frames = [{"rect": rect, "events": []} for rect in final_frames]

        # Sort by row then column
        row_height = max(int(median_h * 0.5), 20)
        frames.sort(key=lambda f: (f["rect"][1] // row_height, f["rect"][0]))

        if split_count > 0:
            print(f"Auto-detected {len(frames)} frames ({split_count} from splitting oversized).")
        else:
            print(f"Auto-detected {len(frames)} frames.")

        return frames
    except Exception as e:
        print(f"Error during auto-detection: {e}")
        return []


def get_median_frame_size(frames):
    """Calculate median frame size from existing frames"""
    if not frames:
        return 64, 64
    widths = [f["rect"][2] for f in frames]
    heights = [f["rect"][3] for f in frames]
    return int(np.median(widths)), int(np.median(heights))


def auto_detect_in_region(image_path, region_rect):
    """Auto-detect frames within a specific region of the image.

    Args:
        image_path: Path to the image file
        region_rect: [x, y, w, h] region to detect within

    Returns:
        List of frame dictionaries with 'rect' and 'events' keys
    """
    try:
        img = cv2_imread_unicode(image_path)
        if img is None or len(img.shape) < 3 or img.shape[2] < 4:
            print("Info: No alpha channel for auto-detection.")
            return []

        rx, ry, rw, rh = [int(v) for v in region_rect]
        img_height, img_width = img.shape[:2]

        # Clamp region to image bounds
        rx = max(0, rx)
        ry = max(0, ry)
        rw = min(rw, img_width - rx)
        rh = min(rh, img_height - ry)

        if rw <= 0 or rh <= 0:
            return []

        # Extract region's alpha channel
        alpha_region = img[ry:ry+rh, rx:rx+rw, 3]
        _, thresh = cv2.threshold(alpha_region, 0, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # Create frames (offset by region position)
        frames = []
        for c in contours:
            if cv2.contourArea(c) > MIN_CONTOUR_AREA:
                x, y, w, h = cv2.boundingRect(c)
                # Offset to absolute image coordinates
                frames.append({"rect": [rx + x, ry + y, w, h], "events": []})

        # Sort by row then column
        frames.sort(key=lambda f: (f["rect"][1] // 50, f["rect"][0]))

        print(f"Detected {len(frames)} frames in region.")
        return frames
    except Exception as e:
        print(f"Error during region detection: {e}")
        return []


def split_frame_by_median(frame_rect, median_h, img_height):
    """Split a frame into multiple frames based on median height.

    Args:
        frame_rect: [x, y, w, h] of frame to split
        median_h: Target height for each split frame
        img_height: Total image height for bounds checking

    Returns:
        List of [x, y, w, h] rects
    """
    x, y, w, h = frame_rect

    # Calculate how many frames this should be split into
    num_splits = max(1, round(h / median_h))

    if num_splits <= 1:
        return [frame_rect]

    split_h = h // num_splits
    result = []

    for i in range(num_splits):
        new_y = y + i * split_h
        # Last frame gets remaining height
        new_h = split_h if i < num_splits - 1 else (y + h - new_y)
        result.append([x, new_y, w, new_h])

    return result

def grid_split_frames(image_path, cell_width, cell_height):
    """Split image into grid cells of cell_width x cell_height, skip empty cells"""
    try:
        img = cv2_imread_unicode(image_path)
        if img is None:
            return []

        img_height, img_width = img.shape[:2]
        has_alpha = len(img.shape) > 2 and img.shape[2] >= 4

        frames = []
        cols = img_width // cell_width
        rows = img_height // cell_height

        for r in range(rows):
            for c in range(cols):
                x = c * cell_width
                y = r * cell_height

                # If has alpha, check if cell has content
                if has_alpha:
                    cell = img[y:y+cell_height, x:x+cell_width, 3]
                    if np.sum(cell) < MIN_CONTOUR_AREA * 255:
                        continue  # Skip empty cells
                
                # Append frame in the new dictionary format
                frames.append({"rect": [x, y, cell_width, cell_height], "events": []})

        print(f"Grid split: {len(frames)} frames (cell: {cell_width}x{cell_height})")
        return frames
    except Exception as e:
        print(f"Error during grid split: {e}")
        return []

def slice_export_frames(sprite_sheet, frames, output_dir, base_name="frame"):
    """Export each frame as a separate image file.

    Args:
        sprite_sheet: pygame.Surface of the sprite sheet
        frames: List of frame dictionaries with 'rect' key
        output_dir: Directory to save the exported images
        base_name: Base name for the exported files (default: 'frame')

    Returns:
        Number of successfully exported frames
    """
    if not frames:
        print("No frames to export.")
        return 0

    os.makedirs(output_dir, exist_ok=True)
    exported_count = 0

    for i, frame_data in enumerate(frames):
        frame_rect = frame_data["rect"]
        x, y, w, h = frame_rect

        # Validate frame dimensions
        if w <= 0 or h <= 0:
            print(f"Skipping frame {i}: invalid dimensions ({w}x{h})")
            continue

        try:
            # Extract frame from sprite sheet
            frame_surface = sprite_sheet.subsurface(pygame.Rect(x, y, w, h))

            # Generate output filename
            output_path = os.path.join(output_dir, f"{base_name}_{i:03d}.png")

            # Save the frame
            pygame.image.save(frame_surface, output_path)
            exported_count += 1
        except ValueError as e:
            print(f"Error extracting frame {i}: {e}")
        except Exception as e:
            print(f"Error saving frame {i}: {e}")

    print(f"Exported {exported_count}/{len(frames)} frames to '{output_dir}'")
    return exported_count


def open_folder_dialog(title="Select Output Folder"):
    """Opens a folder selection dialog and returns the selected path."""
    import tkinter as tk
    from tkinter import filedialog

    # Determine default directory from config
    project_root = TOOL_CONFIG.get("project_root", "")
    resources_folder = TOOL_CONFIG.get("resources_folder", "Resources")
    default_dir = os.path.join(project_root, resources_folder)

    if not os.path.exists(default_dir):
        default_dir = os.getcwd()

    root = tk.Tk()
    root.withdraw()

    folder_path = filedialog.askdirectory(
        title=title,
        initialdir=default_dir
    )

    root.destroy()
    return folder_path if folder_path else None


def estimate_cell_size(image_path):
    """Estimate optimal cell size based on content analysis"""
    try:
        img = cv2_imread_unicode(image_path)
        if img is None or len(img.shape) < 3 or img.shape[2] < 4:
            return 64, 64

        alpha = img[:, :, 3]
        img_height, img_width = alpha.shape
        _, thresh = cv2.threshold(alpha, 0, 255, cv2.THRESH_BINARY)

        # Find contours to estimate sprite size
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if not contours:
            return 64, 64

        # Get bounding boxes
        boxes = [cv2.boundingRect(c) for c in contours if cv2.contourArea(c) > MIN_CONTOUR_AREA]
        if not boxes:
            return 64, 64

        # Estimate average sprite size (round to nice numbers)
        avg_width = sum(b[2] for b in boxes) / len(boxes)
        avg_height = sum(b[3] for b in boxes) / len(boxes)

        # Round to nearest power of 2 or multiple of 8
        def round_to_nice(val):
            if val <= 16: return 16
            if val <= 32: return 32
            if val <= 48: return 48
            if val <= 64: return 64
            if val <= 96: return 96
            if val <= 128: return 128
            return int(round(val / 16) * 16)

        return round_to_nice(avg_width), round_to_nice(avg_height)
    except:
        return 64, 64

def open_file_dialog():
    """Opens a file dialog and returns the path of the selected file (PNG or JSON)."""
    import tkinter as tk
    from tkinter import filedialog

    # Determine default directory from config
    project_root = TOOL_CONFIG.get("project_root", "")
    resources_folder = TOOL_CONFIG.get("resources_folder", "Resources")
    default_dir = os.path.join(project_root, resources_folder)

    if not os.path.exists(default_dir):
        default_dir = os.getcwd()

    root = tk.Tk()
    root.withdraw()

    file_path = filedialog.askopenfilename(
        title="Select Sprite Image or Animation JSON",
        initialdir=default_dir,
        filetypes=[
            ("Supported files", "*.png *.jpg *.jpeg *.bmp *.json"),
            ("Image files", "*.png *.jpg *.jpeg *.bmp"),
            ("Animation JSON", "*.json"),
            ("All files", "*.*")
        ]
    )

    root.destroy()
    return file_path if file_path else None


def load_animation_json(json_path):
    """Load animation data from JSON file. Returns (image_path, frames, pivot, frame_interval) or None on error.
    pivot is returned as [x, y] where x and y are floats (0.0=left/top, 0.5=center, 1.0=right/bottom)"""
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        image_rel_path = data.get("image", "")
        loaded_frames = data.get("frames", [])
        # Handle pivot: convert legacy values to [x, y] format
        # Legacy: string "center" or "bottom-center", or single float (Y-axis only)
        # New format: [x, y] where x and y are floats (0.0=left/top, 0.5=center, 1.0=right/bottom)
        pivot_raw = data.get("pivot", [0.5, 1.0])  # Default: center-bottom
        if isinstance(pivot_raw, str):
            # Legacy string conversion
            if pivot_raw == "center":
                pivot = [0.5, 0.5]
            else:  # "bottom-center" or any other legacy value
                pivot = [0.5, 1.0]
        elif isinstance(pivot_raw, list) and len(pivot_raw) == 2:
            # New format: [x, y]
            pivot = [float(pivot_raw[0]), float(pivot_raw[1])]
        else:
            # Legacy single float (Y-axis only, X defaults to center)
            pivot = [0.5, float(pivot_raw)]
        frame_interval = data.get("frame_interval", 1.0 / PREVIEW_FPS) # Default to PREVIEW_FPS if not found

        # --- Data Structure Migration ---
        frames = []
        if loaded_frames:
            # Check the type of the first frame to determine the format
            if isinstance(loaded_frames[0], list):
                # Old format: list of lists -> convert to new format
                for frame_rect in loaded_frames:
                    frames.append({"rect": frame_rect, "events": []})
                print("Info: Migrated old frame format to new format.")
            else:
                # New format: list of dicts, ensure 'events' exists
                for frame_data in loaded_frames:
                    if "events" not in frame_data:
                        frame_data["events"] = []
                    frames.append(frame_data)
        
        if not image_rel_path:
            print(f"Error: No 'image' field in JSON file")
            return None

        # Resolve image path (relative to project root + resources_folder)
        project_root = TOOL_CONFIG.get("project_root", "")
        resources_folder = TOOL_CONFIG.get("resources_folder", "Resources")
        
        # Try path relative to project_root/resources_folder
        image_path = os.path.join(project_root, resources_folder, image_rel_path)

        # If not found, try relative to JSON file location (as a fallback for older saved files or unusual setups)
        if not os.path.exists(image_path):
            json_dir = os.path.dirname(json_path)
            image_path = os.path.join(json_dir, image_rel_path)

        # If still not found, try as absolute path (as a last resort)
        if not os.path.exists(image_path):
            image_path = image_rel_path

        if not os.path.exists(image_path):
            print(f"Error: Image file not found: {image_rel_path}")
            return None

        print(f"Loaded animation: {len(frames)} frames, pivot: {pivot}, interval: {frame_interval:.3f}s")
        return (image_path, frames, pivot, frame_interval)
    except Exception as e:
        print(f"Error loading animation JSON: {e}")
        return None

def run_editor(image_path, loaded_frames=None, loaded_pivot=None, loaded_interval=None):
    """
    Run the animation frame editor.

    Args:
        image_path: Path to the sprite sheet image
        loaded_frames: Optional pre-loaded frames list from JSON
        loaded_pivot: Optional pre-loaded pivot mode from JSON
        loaded_interval: Optional pre-loaded frame interval from JSON
    """
    pygame.init()

    font_help = pygame.font.Font(None, FONT_SIZE)
    font_frame = pygame.font.Font(None, FRAME_FONT_SIZE)
    font_detail = pygame.font.Font(None, 22)
    help_area_height = len(EDITOR_HELP_LINES) * (FONT_SIZE - 5) + 15

    try: temp_image = pygame.image.load(image_path)
    except pygame.error as e: print(f"Error loading image file: {e}"); return

    img_width, img_height = temp_image.get_size()
    # Fixed initial window size, user can resize freely
    screen_width = 1200
    screen_height = 800
    screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
    pygame.display.set_caption(f"Animation Frame Tool - {os.path.basename(image_path)}")
    sprite_sheet = temp_image.convert_alpha()

    # Camera/View settings
    cam_x, cam_y = 0, 0  # Camera offset (panning)
    zoom = 1.0  # Zoom level
    MIN_ZOOM, MAX_ZOOM = 0.1, 5.0
    is_panning = False
    pan_start = None

    # --- Data Migration for loaded_frames ---
    frames = []
    if loaded_frames:
        if isinstance(loaded_frames[0], list): # Old format
            for rect in loaded_frames:
                frames.append({"rect": rect, "events": []})
        else: # New format
            frames = loaded_frames

    # Use pre-loaded data if available, otherwise try to load existing JSON (no auto-detect on load)
    if loaded_frames is not None:
        # loaded_pivot is [x, y] or None
        pivot_x = loaded_pivot[0] if loaded_pivot is not None else 0.5
        pivot_y = loaded_pivot[1] if loaded_pivot is not None else 1.0
        print(f"Using loaded data: {len(frames)} frames, pivot: [{pivot_x}, {pivot_y}]")
    else:
        # Try to load existing JSON data first
        json_path = os.path.splitext(image_path)[0] + ".json"
        frames = []
        pivot_x = 0.5  # Default: center (X-axis ratio: 0.0=left, 0.5=center, 1.0=right)
        pivot_y = 1.0  # Default: bottom (Y-axis ratio: 0.0=top, 0.5=center, 1.0=bottom)
        if os.path.exists(json_path):
            try:
                # Use load_animation_json which handles migration
                _image_path, loaded_data, loaded_pivot, _ = load_animation_json(json_path)
                frames = loaded_data if loaded_data else []
                if loaded_pivot is not None:
                    pivot_x = loaded_pivot[0]
                    pivot_y = loaded_pivot[1]
                print(f"Loaded {len(frames)} frames from '{json_path}'")
            except Exception as e:
                print(f"Error loading JSON: {e}")
                frames = []
        # No auto-detect on file open - user presses 'A' manually

    running = True; snapping_enabled = True; is_previewing = False
    selected_frame_index = -1; show_pivots = True
    current_rect, start_pos, resize_mode, is_moving = None, None, None, False
    clock = pygame.time.Clock()

    # Grid split state
    grid_cell_w, grid_cell_h = estimate_cell_size(image_path)
    grid_mode = False  # Whether grid mode UI is active

    # Save notification
    save_notification = None  # (message, timestamp)
    NOTIFICATION_DURATION = 2000  # ms

    # --- Detail Panel Input Fields ---
    input_x = InputField(70, help_area_height + 80, 80, 24, "X", font_detail)
    input_y = InputField(70, help_area_height + 110, 80, 24, "Y", font_detail)
    input_w = InputField(70, help_area_height + 140, 80, 24, "W", font_detail)
    input_h = InputField(70, help_area_height + 170, 80, 24, "H", font_detail)
    input_fields = [input_x, input_y, input_w, input_h]
    active_input_index = -1  # -1 means no input field is active

    # --- Global Animation Settings ---
    frame_interval = loaded_interval if loaded_interval is not None else 1.0 / PREVIEW_FPS
    input_interval = FloatInputField(110, help_area_height + 50, 80, 24, "Interval (s)", font_detail)
    input_interval.set_value(frame_interval)
    active_interval_input = False

    # --- Pivot Input Fields (for no selection mode) ---
    input_pivot_x = FloatInputField(110, help_area_height + 80, 60, 24, "Pivot X", font_detail)
    input_pivot_y = FloatInputField(110, help_area_height + 110, 60, 24, "Pivot Y", font_detail)
    input_pivot_x.set_value(pivot_x)
    input_pivot_y.set_value(pivot_y)
    active_pivot_x_input = False
    active_pivot_y_input = False

    # --- Grid Mode Input Fields ---
    input_grid_w = InputField(90, help_area_height + 80, 80, 24, "Cell W", font_detail)
    input_grid_h = InputField(90, help_area_height + 110, 80, 24, "Cell H", font_detail)
    grid_input_fields = [input_grid_w, input_grid_h]
    active_grid_input_index = -1

    # --- Event Editor for selected frame ---
    event_editor = EventEditor(10, help_area_height + 220, font_detail)
    event_editor_active = False

    def update_input_fields_from_frame(force_update_events=True):
        """Update input field values from selected frame"""
        if selected_frame_index != -1 and selected_frame_index < len(frames):
            frame = frames[selected_frame_index]
            input_x.set_value(frame["rect"][0])
            input_y.set_value(frame["rect"][1])
            input_w.set_value(frame["rect"][2])
            input_h.set_value(frame["rect"][3])
            # Update event editor with frame's events (skip if event editor is active to preserve focus)
            if force_update_events and not event_editor_active:
                event_editor.set_events(frame.get("events", []))
        else:
            for field in input_fields:
                field.text = ""
            if force_update_events:
                event_editor.set_events([])

    def apply_input_fields_to_frame():
        """Apply input field values to selected frame with boundary and overlap checks"""
        nonlocal save_notification
        if selected_frame_index != -1 and selected_frame_index < len(frames):
            original_rect = frames[selected_frame_index]["rect"].copy()
            w = max(1, input_w.get_value())
            h = max(1, input_h.get_value())
            new_rect = [input_x.get_value(), input_y.get_value(), w, h]

            # Clamp to image bounds
            new_rect = clamp_rect_to_image(new_rect, img_width, img_height)

            # Check for overlaps with other frames
            has_overlap = False
            for i, f in enumerate(frames):
                if i != selected_frame_index and check_rect_overlap(new_rect, f["rect"]):
                    has_overlap = True
                    break

            if has_overlap:
                save_notification = ("Cannot move: overlaps with another frame!", pygame.time.get_ticks())
                # Restore original values in input fields
                input_x.set_value(original_rect[0])
                input_y.set_value(original_rect[1])
                input_w.set_value(original_rect[2])
                input_h.set_value(original_rect[3])
            else:
                frames[selected_frame_index]["rect"] = new_rect
                # Update input fields to show clamped values
                input_x.set_value(new_rect[0])
                input_y.set_value(new_rect[1])
                input_w.set_value(new_rect[2])
                input_h.set_value(new_rect[3])

    def set_active_input(index):
        nonlocal active_input_index
        for i, field in enumerate(input_fields):
            field.active = (i == index)
        active_input_index = index

    def deactivate_all_inputs():
        nonlocal active_input_index, active_grid_input_index, active_interval_input, active_pivot_x_input, active_pivot_y_input, event_editor_active
        for field in input_fields:
            field.active = False
        for field in grid_input_fields:
            field.active = False
        input_interval.active = False
        input_pivot_x.active = False
        input_pivot_y.active = False
        active_input_index = -1
        active_grid_input_index = -1
        active_interval_input = False
        active_pivot_x_input = False
        active_pivot_y_input = False
        # Deactivate event editor fields
        for field in event_editor.event_fields:
            field.active = False
        event_editor.active_field_idx = -1
        event_editor_active = False

    def update_grid_input_fields():
        """Update grid input field values from grid_cell_w/h"""
        input_grid_w.set_value(grid_cell_w)
        input_grid_h.set_value(grid_cell_h)

    def apply_grid_input_fields():
        """Apply grid input field values to grid_cell_w/h"""
        nonlocal grid_cell_w, grid_cell_h
        new_w = input_grid_w.get_value()
        new_h = input_grid_h.get_value()
        if new_w > 0:
            grid_cell_w = min(img_width, max(1, new_w))
        if new_h > 0:
            grid_cell_h = min(img_height, max(1, new_h))

    def set_active_grid_input(index):
        nonlocal active_grid_input_index, active_input_index
        # Deactivate frame inputs
        for field in input_fields:
            field.active = False
        active_input_index = -1
        # Activate grid input
        for i, field in enumerate(grid_input_fields):
            field.active = (i == index)
        active_grid_input_index = index

    while running:
        mouse_pos = pygame.mouse.get_pos()
        # Convert screen coords to image coords (accounting for camera pan and zoom)
        view_area_x = mouse_pos[0]
        view_area_y = mouse_pos[1] - help_area_height
        # Check if mouse is in view area (not in panel or help area)
        in_view_area = view_area_y >= 0 and mouse_pos[0] < screen_width - DETAIL_PANEL_WIDTH
        # Image coordinates (what pixel on the sprite sheet)
        img_x = (view_area_x - cam_x) / zoom
        img_y = (view_area_y - cam_y) / zoom
        translated_mouse_pos = (img_x, img_y)

        if not is_previewing and not start_pos and not is_panning:
            cursor_set = False
            if selected_frame_index != -1 and selected_frame_index < len(frames) and in_view_area:
                selected_rect = pygame.Rect(frames[selected_frame_index]["rect"])
                # Scale handles for zoom
                handles = get_handles(selected_rect)
                for name, h_rect in handles.items():
                    # Check collision in image space
                    if h_rect.collidepoint(translated_mouse_pos):
                        pygame.mouse.set_cursor(get_cursor_for_handle(name)); cursor_set = True; break
            if not cursor_set: pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_ARROW)

        # Update input field cursors
        dt = clock.get_time()
        for field in input_fields:
            field.update(dt)
        for field in grid_input_fields:
            field.update(dt)
        input_interval.update(dt)
        input_pivot_x.update(dt)
        input_pivot_y.update(dt)

        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False
            elif event.type == pygame.VIDEORESIZE:
                screen_width, screen_height = event.w, event.h
                screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
            # Mouse wheel zoom
            elif event.type == pygame.MOUSEWHEEL and in_view_area and not is_previewing:
                old_zoom = zoom
                zoom *= 1.1 if event.y > 0 else 0.9
                zoom = max(MIN_ZOOM, min(MAX_ZOOM, zoom))
                # Zoom towards mouse position
                zoom_factor = zoom / old_zoom
                cam_x = view_area_x - (view_area_x - cam_x) * zoom_factor
                cam_y = view_area_y - (view_area_y - cam_y) * zoom_factor
            # Right-click panning
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 3 and in_view_area:
                is_panning = True
                pan_start = mouse_pos
            elif event.type == pygame.MOUSEBUTTONUP and event.button == 3:
                is_panning = False
                pan_start = None
            elif event.type == pygame.MOUSEMOTION and is_panning:
                if pan_start:
                    dx = mouse_pos[0] - pan_start[0]
                    dy = mouse_pos[1] - pan_start[1]
                    cam_x += dx
                    cam_y += dy
                    pan_start = mouse_pos
            elif event.type == pygame.KEYDOWN:
                # Handle input field events first
                if active_input_index != -1:
                    handled = input_fields[active_input_index].handle_event(event)
                    if handled:
                        continue
                    # Tab to next field
                    if event.key == pygame.K_TAB:
                        apply_input_fields_to_frame()
                        next_index = (active_input_index + 1) % len(input_fields)
                        set_active_input(next_index)
                        continue
                    # Enter to apply and deactivate
                    if event.key == pygame.K_RETURN:
                        apply_input_fields_to_frame()
                        deactivate_all_inputs()
                        continue
                    # Escape to cancel editing
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        update_input_fields_from_frame()
                        continue

                # Handle interval input field events
                if active_interval_input:
                    handled = input_interval.handle_event(event)
                    if handled:
                        frame_interval = input_interval.get_value()
                        continue
                    if event.key in (pygame.K_RETURN, pygame.K_TAB):
                        frame_interval = input_interval.get_value()
                        deactivate_all_inputs()
                        continue
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        input_interval.set_value(frame_interval)
                        continue

                # Handle pivot X input field events
                if active_pivot_x_input:
                    handled = input_pivot_x.handle_event(event)
                    if handled:
                        pivot_x = input_pivot_x.get_value()
                        continue
                    if event.key == pygame.K_TAB:
                        pivot_x = input_pivot_x.get_value()
                        deactivate_all_inputs()
                        active_pivot_y_input = True
                        input_pivot_y.active = True
                        continue
                    if event.key == pygame.K_RETURN:
                        pivot_x = input_pivot_x.get_value()
                        deactivate_all_inputs()
                        continue
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        input_pivot_x.set_value(pivot_x)
                        continue

                # Handle pivot Y input field events
                if active_pivot_y_input:
                    handled = input_pivot_y.handle_event(event)
                    if handled:
                        pivot_y = input_pivot_y.get_value()
                        continue
                    if event.key == pygame.K_TAB:
                        pivot_y = input_pivot_y.get_value()
                        deactivate_all_inputs()
                        active_pivot_x_input = True
                        input_pivot_x.active = True
                        continue
                    if event.key == pygame.K_RETURN:
                        pivot_y = input_pivot_y.get_value()
                        deactivate_all_inputs()
                        continue
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        input_pivot_y.set_value(pivot_y)
                        continue

                # Handle event editor input field events
                if event_editor_active and event_editor.active_field_idx != -1:
                    handled = event_editor.handle_event(event, screen_width - DETAIL_PANEL_WIDTH)
                    if handled:
                        # Save events to frame immediately
                        if selected_frame_index != -1 and selected_frame_index < len(frames):
                            frames[selected_frame_index]["events"] = event_editor.get_events()
                        continue
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        update_input_fields_from_frame()
                        continue

                # Handle grid input field events
                if active_grid_input_index != -1:
                    handled = grid_input_fields[active_grid_input_index].handle_event(event)
                    if handled:
                        apply_grid_input_fields()  # Apply immediately
                        continue
                    # Tab to next grid field
                    if event.key == pygame.K_TAB:
                        apply_grid_input_fields()
                        next_index = (active_grid_input_index + 1) % len(grid_input_fields)
                        set_active_grid_input(next_index)
                        continue
                    # Enter to apply and deactivate
                    if event.key == pygame.K_RETURN:
                        apply_grid_input_fields()
                        deactivate_all_inputs()
                        continue
                    # Escape to cancel editing
                    if event.key == pygame.K_ESCAPE:
                        deactivate_all_inputs()
                        update_grid_input_fields()
                        continue

                if event.key == pygame.K_q: running = False
                elif event.key == pygame.K_ESCAPE:
                    if grid_mode:
                        grid_mode = False
                        save_notification = ("Grid mode cancelled", pygame.time.get_ticks())
                    elif active_input_index == -1:
                        running = False
                elif event.key == pygame.K_a and not is_previewing:
                    if current_rect and current_rect.width > 5 and current_rect.height > 5:
                        # Auto-detect within currently drawn region
                        current_rect.normalize()
                        region_rect = [current_rect.x, current_rect.y, current_rect.width, current_rect.height]
                        detected = auto_detect_in_region(image_path, region_rect)
                        if detected:
                            frames.extend(detected)
                            save_notification = (f"Detected {len(detected)} frames in region", pygame.time.get_ticks())
                        else:
                            save_notification = ("No frames detected in region", pygame.time.get_ticks())
                        current_rect, start_pos = None, None
                    elif selected_frame_index != -1:
                        # Auto-detect within selected frame's region
                        region_rect = frames[selected_frame_index]["rect"]
                        detected = auto_detect_in_region(image_path, region_rect)
                        if detected:
                            # Remove selected frame and insert detected frames
                            frames.pop(selected_frame_index)
                            for i, new_frame in enumerate(detected):
                                frames.insert(selected_frame_index + i, new_frame)
                            save_notification = (f"Detected {len(detected)} frames in region", pygame.time.get_ticks())
                        else:
                            save_notification = ("No frames detected in region", pygame.time.get_ticks())
                        selected_frame_index = -1
                    else:
                        # Auto-detect on entire image
                        frames = auto_detect_frames(image_path)
                        save_notification = (f"Detected {len(frames)} frames", pygame.time.get_ticks())
                    update_input_fields_from_frame()
                elif event.key == pygame.K_v:
                    # Toggle pivot Y between common values: 0.5 (center) and 1.0 (bottom)
                    pivot_y = 0.5 if pivot_y == 1.0 else 1.0
                    input_pivot_y.set_value(pivot_y)
                    print(f"Pivot set to: [{pivot_x}, {pivot_y}]")
                elif event.key == pygame.K_n and not is_previewing:
                    if current_rect and current_rect.width > 0 and current_rect.height > 0:
                        current_rect.normalize()
                        new_rect = [current_rect.x, current_rect.y, current_rect.width, current_rect.height]
                        # Clamp to image bounds
                        new_rect = clamp_rect_to_image(new_rect, img_width, img_height)
                        # Get existing frame rects
                        existing_rects = [f["rect"] for f in frames]
                        # Auto-adjust to avoid overlaps
                        adjusted_rect = adjust_rect_to_avoid_overlap(new_rect, existing_rects, img_width, img_height)
                        if adjusted_rect is None:
                            save_notification = ("Cannot add: no valid space available!", pygame.time.get_ticks())
                        else:
                            frames.append({"rect": adjusted_rect, "events": []})
                            selected_frame_index = len(frames) - 1
                            current_rect, start_pos = None, None
                            update_input_fields_from_frame()
                            # Notify if rect was adjusted
                            if adjusted_rect != new_rect:
                                save_notification = ("Frame adjusted to avoid overlap", pygame.time.get_ticks())
                elif event.key == pygame.K_d and not is_previewing:
                    if selected_frame_index != -1:
                        frames.pop(selected_frame_index)
                        selected_frame_index = -1
                        update_input_fields_from_frame()
                elif event.key == pygame.K_f and not is_previewing:
                    # Manual split: split selected frame by median height
                    if selected_frame_index != -1 and len(frames) > 1:
                        median_w, median_h = get_median_frame_size(frames)
                        frame_rect = frames[selected_frame_index]["rect"]
                        split_result = split_frame_by_median(frame_rect, median_h, img_height)
                        if len(split_result) > 1:
                            # Remove original frame and insert split frames
                            frames.pop(selected_frame_index)
                            for i, new_rect in enumerate(split_result):
                                frames.insert(selected_frame_index + i, {"rect": new_rect, "events": []})
                            save_notification = (f"Split into {len(split_result)} frames", pygame.time.get_ticks())
                            selected_frame_index = -1
                            update_input_fields_from_frame()
                        else:
                            save_notification = ("Frame too small to split", pygame.time.get_ticks())
                    elif selected_frame_index != -1:
                        save_notification = ("Need more frames for median calculation", pygame.time.get_ticks())
                elif event.key == pygame.K_t: snapping_enabled = not snapping_enabled; print(f"Snapping: {'ON' if snapping_enabled else 'OFF'}")
                elif event.key == pygame.K_g: show_pivots = not show_pivots; print(f"Show Pivots: {'ON' if show_pivots else 'OFF'}")
                elif event.key == pygame.K_r and not is_previewing:
                    # Toggle grid mode or apply grid split
                    if grid_mode:
                        # Apply grid split
                        frames = grid_split_frames(image_path, grid_cell_w, grid_cell_h)
                        selected_frame_index = -1
                        update_input_fields_from_frame()
                        grid_mode = False
                        save_notification = (f"Grid {grid_cell_w}x{grid_cell_h}: {len(frames)} frames", pygame.time.get_ticks())
                    else:
                        grid_mode = True
                        update_grid_input_fields()
                        save_notification = ("Grid Mode: Arrows adjust size, R apply, ESC cancel", pygame.time.get_ticks())
                elif grid_mode and event.key == pygame.K_UP:
                    grid_cell_h = max(1, grid_cell_h - 1)
                    update_grid_input_fields()
                elif grid_mode and event.key == pygame.K_DOWN:
                    grid_cell_h = min(img_height, grid_cell_h + 1)
                    update_grid_input_fields()
                elif grid_mode and event.key == pygame.K_LEFT:
                    grid_cell_w = max(1, grid_cell_w - 1)
                    update_grid_input_fields()
                elif grid_mode and event.key == pygame.K_RIGHT:
                    grid_cell_w = min(img_width, grid_cell_w + 1)
                    update_grid_input_fields()
                elif event.key == pygame.K_p and frames: is_previewing = not is_previewing
                elif event.key == pygame.K_x and not is_previewing:
                    # Slice export - export each frame as separate image
                    if frames:
                        output_dir = open_folder_dialog("Select Output Folder for Slice Export")
                        if output_dir:
                            # Use image name as base name for exported files
                            base_name = os.path.splitext(os.path.basename(image_path))[0]
                            exported = slice_export_frames(sprite_sheet, frames, output_dir, base_name)
                            if exported > 0:
                                save_notification = (f"Exported {exported} frames to folder", pygame.time.get_ticks())
                            else:
                                save_notification = ("Export failed!", pygame.time.get_ticks())
                        else:
                            save_notification = ("Export cancelled", pygame.time.get_ticks())
                    else:
                        save_notification = ("No frames to export!", pygame.time.get_ticks())
                # Frame reorder: , key moves frame earlier, . key moves frame later
                elif event.key == pygame.K_COMMA and not is_previewing:
                    if selected_frame_index > 0:
                        frames[selected_frame_index], frames[selected_frame_index - 1] = frames[selected_frame_index - 1], frames[selected_frame_index]
                        selected_frame_index -= 1
                        print(f"Moved frame to index {selected_frame_index}")
                elif event.key == pygame.K_PERIOD and not is_previewing:
                    if selected_frame_index != -1 and selected_frame_index < len(frames) - 1:
                        frames[selected_frame_index], frames[selected_frame_index + 1] = frames[selected_frame_index + 1], frames[selected_frame_index]
                        selected_frame_index += 1
                        print(f"Moved frame to index {selected_frame_index}")
                elif event.key == pygame.K_s:
                    if frames:
                        import tkinter as tk
                        from tkinter import filedialog

                        # Determine default directory from config
                        project_root = TOOL_CONFIG.get("project_root", "")

                        # Validate that the drive from project_root exists
                        if project_root:
                            drive, _ = os.path.splitdrive(project_root)
                            if drive and not os.path.exists(drive):
                                script_dir = os.path.dirname(os.path.abspath(__file__))
                                new_root = os.path.dirname(script_dir)
                                print(f"Warning: Drive '{drive}' from 'project_root' in config not found. Falling back to '{new_root}'.")
                                project_root = new_root

                        resources_folder = TOOL_CONFIG.get("resources_folder", "Resources")
                        animations_folder = TOOL_CONFIG.get("animations_folder", "Animations")
                        default_dir = os.path.join(project_root, resources_folder, animations_folder)

                        # Create directory if it doesn't exist
                        os.makedirs(default_dir, exist_ok=True)

                        # Default filename from image name
                        default_filename = os.path.splitext(os.path.basename(image_path))[0] + ".json"

                        root = tk.Tk()
                        root.withdraw()
                        output_path = filedialog.asksaveasfilename(
                            title="Save Animation File",
                            initialdir=default_dir,
                            initialfile=default_filename,
                            defaultextension=".json",
                            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
                        )
                        root.destroy()

                        if output_path:
                            # Get relative image path from project root
                            image_rel_path = get_relative_path(image_path, project_root)

                            # Ensure the stored path uses .bmp
                            image_rel_path = os.path.splitext(image_rel_path)[0] + ".bmp"

                            resources_folder_name = TOOL_CONFIG.get("resources_folder", "Resources")
                            
                            # Remove the resources folder prefix if present
                            if image_rel_path.startswith(f"{resources_folder_name}/"):
                                image_rel_path = image_rel_path[len(resources_folder_name) + 1:]

                            # Compact JSON: one frame per line
                            with open(output_path, 'w') as f:
                                f.write('{\n')
                                f.write(f'    "image": "{image_rel_path}",\n')
                                f.write(f'    "pivot": [{round(pivot_x, 4)}, {round(pivot_y, 4)}],\n')
                                f.write(f'    "frame_interval": {round(frame_interval, 4)},\n')
                                f.write('    "frames": [\n')
                                for i, frame in enumerate(frames):
                                    comma = "," if i < len(frames) - 1 else ""
                                    f.write(f'        {json.dumps(frame)}{comma}\n')
                                f.write('    ]\n')
                                f.write('}\n')
                            print(f"Saved {len(frames)} frames and pivot [{pivot_x}, {pivot_y}] to '{output_path}'")
                            save_notification = (f"Saved {len(frames)} frames!", pygame.time.get_ticks())
                    else:
                        print("No frames to save.")
                        save_notification = ("No frames to save!", pygame.time.get_ticks())
                elif event.key == pygame.K_o:
                    # Open file browser to select another image
                    return "open_file"

            # Check if clicking on detail panel input fields
            if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                panel_x = screen_width - DETAIL_PANEL_WIDTH
                clicked_on_input = False
                # Check grid input fields first (when in grid mode)
                if mouse_pos[0] >= panel_x and grid_mode:
                    for i, field in enumerate(grid_input_fields):
                        actual_rect = pygame.Rect(panel_x + field.rect.x, field.rect.y, field.rect.width, field.rect.height)
                        if actual_rect.collidepoint(mouse_pos):
                            set_active_grid_input(i)
                            clicked_on_input = True
                            break
                if not clicked_on_input and mouse_pos[0] >= panel_x and selected_frame_index == -1 and not grid_mode:
                    # Check interval input
                    actual_rect = pygame.Rect(panel_x + input_interval.rect.x, input_interval.rect.y, input_interval.rect.width, input_interval.rect.height)
                    if actual_rect.collidepoint(mouse_pos):
                        deactivate_all_inputs()
                        active_interval_input = True
                        input_interval.active = True
                        clicked_on_input = True
                    # Check pivot X input
                    if not clicked_on_input:
                        actual_rect = pygame.Rect(panel_x + input_pivot_x.rect.x, input_pivot_x.rect.y, input_pivot_x.rect.width, input_pivot_x.rect.height)
                        if actual_rect.collidepoint(mouse_pos):
                            deactivate_all_inputs()
                            active_pivot_x_input = True
                            input_pivot_x.active = True
                            clicked_on_input = True
                    # Check pivot Y input
                    if not clicked_on_input:
                        actual_rect = pygame.Rect(panel_x + input_pivot_y.rect.x, input_pivot_y.rect.y, input_pivot_y.rect.width, input_pivot_y.rect.height)
                        if actual_rect.collidepoint(mouse_pos):
                            deactivate_all_inputs()
                            active_pivot_y_input = True
                            input_pivot_y.active = True
                            clicked_on_input = True

                # Check frame input fields (when frame is selected and not in grid mode)
                if not clicked_on_input and mouse_pos[0] >= panel_x and selected_frame_index != -1 and not grid_mode:
                    for i, field in enumerate(input_fields):
                        actual_rect = pygame.Rect(panel_x + field.rect.x, field.rect.y, field.rect.width, field.rect.height)
                        if actual_rect.collidepoint(mouse_pos):
                            set_active_input(i)
                            clicked_on_input = True
                            break
                    # Check event editor clicks (when frame is selected)
                    if not clicked_on_input:
                        if event_editor.handle_event(event, panel_x):
                            clicked_on_input = True
                            event_editor_active = True
                            # Save events to frame when clicking on event editor
                            if selected_frame_index != -1 and selected_frame_index < len(frames):
                                frames[selected_frame_index]["events"] = event_editor.get_events()

                if not clicked_on_input and mouse_pos[0] < panel_x:
                    # Save events before deactivating if a frame is selected
                    if selected_frame_index != -1 and selected_frame_index < len(frames):
                        frames[selected_frame_index]["events"] = event_editor.get_events()
                    deactivate_all_inputs()

            if not is_previewing and translated_mouse_pos[1] >= 0:
                panel_x = screen_width - DETAIL_PANEL_WIDTH
                # Only handle sprite sheet interactions if not clicking on panel
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1 and mouse_pos[0] < panel_x:
                    start_pos = translated_mouse_pos
                    if selected_frame_index != -1 and selected_frame_index < len(frames):
                        handles = get_handles(pygame.Rect(frames[selected_frame_index]["rect"]))
                        for name, h_rect in handles.items():
                            if h_rect.collidepoint(translated_mouse_pos): resize_mode = name; break
                    if not resize_mode:
                        clicked_idx = -1
                        for i in range(len(frames) - 1, -1, -1):
                            if pygame.Rect(frames[i]["rect"]).collidepoint(translated_mouse_pos): clicked_idx = i; break
                        if clicked_idx != -1:
                            # Save events from previous selection before switching
                            if selected_frame_index != -1 and selected_frame_index < len(frames):
                                frames[selected_frame_index]["events"] = event_editor.get_events()
                            selected_frame_index, is_moving = clicked_idx, True
                            update_input_fields_from_frame()
                        else:
                            # Save events from previous selection before deselecting
                            if selected_frame_index != -1 and selected_frame_index < len(frames):
                                frames[selected_frame_index]["events"] = event_editor.get_events()
                            selected_frame_index, current_rect = -1, pygame.Rect(start_pos, (0,0))
                            update_input_fields_from_frame()
                elif event.type == pygame.MOUSEBUTTONUP and event.button == 1:
                    start_pos, resize_mode, is_moving = None, None, False
                    # Update input fields after resize/move
                    if selected_frame_index != -1:
                        update_input_fields_from_frame()
                elif event.type == pygame.MOUSEMOTION:
                    if start_pos and resize_mode and selected_frame_index != -1:
                        rect = pygame.Rect(frames[selected_frame_index]["rect"]); mx, my = translated_mouse_pos
                        # Apply snapping
                        if snapping_enabled:
                            snap_coords_x = [f["rect"][0] for i, f in enumerate(frames) if i != selected_frame_index] + [f["rect"][0] + f["rect"][2] for i, f in enumerate(frames) if i != selected_frame_index]
                            snap_coords_y = [f["rect"][1] for i, f in enumerate(frames) if i != selected_frame_index] + [f["rect"][1] + f["rect"][3] for i, f in enumerate(frames) if i != selected_frame_index]
                            mx = get_snap_coord(mx, snap_coords_x, SNAP_TOLERANCE)
                            my = get_snap_coord(my, snap_coords_y, SNAP_TOLERANCE)
                        # Use clamped resize with boundary and overlap checks
                        rect = clamp_resize_to_bounds(rect, resize_mode, mx, my, img_width, img_height, frames, selected_frame_index)
                        frames[selected_frame_index]["rect"] = [rect.x, rect.y, rect.width, rect.height]
                    elif start_pos and is_moving and selected_frame_index != -1:
                        dx, dy = translated_mouse_pos[0] - start_pos[0], translated_mouse_pos[1] - start_pos[1]

                        rect_data = frames[selected_frame_index]["rect"]
                        original_rect = rect_data.copy()
                        new_x = rect_data[0] + dx
                        new_y = rect_data[1] + dy

                        # Apply snapping
                        if snapping_enabled:
                            snap_coords_x = [f["rect"][0] for i, f in enumerate(frames) if i != selected_frame_index] + [f["rect"][0] + f["rect"][2] for i, f in enumerate(frames) if i != selected_frame_index]
                            snap_coords_y = [f["rect"][1] for i, f in enumerate(frames) if i != selected_frame_index] + [f["rect"][1] + f["rect"][3] for i, f in enumerate(frames) if i != selected_frame_index]
                            snapped_x = get_snap_coord(new_x, snap_coords_x, SNAP_TOLERANCE)
                            snapped_y = get_snap_coord(new_y, snap_coords_y, SNAP_TOLERANCE)
                            if snapped_x != new_x: new_x = snapped_x
                            if snapped_y != new_y: new_y = snapped_y

                        # Create new rect with proposed position
                        new_rect = [new_x, new_y, rect_data[2], rect_data[3]]

                        # Get other rects for overlap check
                        other_rects = [f["rect"] for i, f in enumerate(frames) if i != selected_frame_index]

                        # Apply boundary and overlap checks
                        final_rect = get_non_overlapping_position(new_rect, other_rects, original_rect, img_width, img_height)

                        # Update position
                        frames[selected_frame_index]["rect"] = final_rect
                        start_pos = translated_mouse_pos
                    elif start_pos and current_rect is not None:
                        x1, y1 = start_pos
                        x2, y2 = translated_mouse_pos
                        # Clamp coordinates to image bounds
                        x1 = max(0, min(x1, img_width))
                        y1 = max(0, min(y1, img_height))
                        x2 = max(0, min(x2, img_width))
                        y2 = max(0, min(y2, img_height))
                        current_rect.x = min(x1, x2)
                        current_rect.y = min(y1, y2)
                        current_rect.width = abs(x1 - x2)
                        current_rect.height = abs(y1 - y2)

        screen.fill(BACKGROUND_COLOR)

        # Create a clipping rect for the view area
        view_rect = pygame.Rect(0, help_area_height, screen_width - DETAIL_PANEL_WIDTH, screen_height - help_area_height)
        screen.set_clip(view_rect)

        # Draw scaled sprite sheet with camera offset
        scaled_width = int(img_width * zoom)
        scaled_height = int(img_height * zoom)
        if scaled_width > 0 and scaled_height > 0:
            scaled_sheet = pygame.transform.scale(sprite_sheet, (scaled_width, scaled_height))
            screen.blit(scaled_sheet, (cam_x, help_area_height + cam_y))

        # Draw grid preview if in grid mode
        if grid_mode:
            grid_color = (255, 255, 0)
            cols = img_width // grid_cell_w
            rows = img_height // grid_cell_h
            # Draw vertical lines
            for c in range(cols + 1):
                x = int(c * grid_cell_w * zoom + cam_x)
                pygame.draw.line(screen, grid_color, (x, help_area_height + cam_y), (x, help_area_height + cam_y + scaled_height), 1)
            # Draw horizontal lines
            for r in range(rows + 1):
                y = int(r * grid_cell_h * zoom + cam_y + help_area_height)
                pygame.draw.line(screen, grid_color, (cam_x, y), (cam_x + scaled_width, y), 1)
            # Draw grid size info
            grid_info = f"Cell: {grid_cell_w}x{grid_cell_h} ({cols}x{rows} cells)"
            grid_info_surf = font_help.render(grid_info, True, grid_color)
            screen.blit(grid_info_surf, (10, help_area_height + 10))

        # Convert image coords to screen coords (with zoom and camera)
        def to_screen_coords(p):
            return (int(p[0] * zoom + cam_x), int(p[1] * zoom + cam_y + help_area_height))
        def to_screen_rect_list(r_list):
            return pygame.Rect(to_screen_coords((r_list[0], r_list[1])), (int(r_list[2] * zoom), int(r_list[3] * zoom)))
        def to_screen_rect_obj(r_obj):
            return pygame.Rect(to_screen_coords(r_obj.topleft), (int(r_obj.width * zoom), int(r_obj.height * zoom)))
        
        if is_previewing:
            overlay = pygame.Surface((screen.get_width(), screen.get_height()), pygame.SRCALPHA); overlay.fill(PREVIEW_BG_COLOR); screen.blit(overlay, (0, 0))
            if frames:
                if frame_interval > 0:
                    preview_frame_index = int(pygame.time.get_ticks() / (frame_interval * 1000)) % len(frames)
                else:
                    preview_frame_index = 0

                frame_data = frames[preview_frame_index]
                frame_rect = frame_data["rect"]
                if frame_rect[2] > 0 and frame_rect[3] > 0:
                    try:
                        preview_img = sprite_sheet.subsurface(pygame.Rect(frame_rect))
                        w, h = preview_img.get_size()
                        anchor_pos = (screen.get_width() // 2, screen.get_height() // 2)
                        # pivot_x: 0.0=left, 0.5=center, 1.0=right
                        # pivot_y: 0.0=top, 0.5=center, 1.0=bottom
                        draw_x = anchor_pos[0] - int(w * pivot_x)
                        draw_y = anchor_pos[1] - int(h * pivot_y)
                        screen.blit(preview_img, (draw_x, draw_y))
                    except ValueError: pass
            preview_text_surf = font_help.render(f"PREVIEW MODE (pivot=[{pivot_x:.2f}, {pivot_y:.2f}]) - P to exit", True, HELP_TEXT_COLOR)
            screen.blit(preview_text_surf, (5, screen.get_height() - FONT_SIZE - 5))
        else:
            for i, frame_data in enumerate(frames):
                frame_rect = frame_data["rect"]
                rect_color = SELECTED_RECT_COLOR if i == selected_frame_index else SAVED_RECT_COLOR
                pygame.draw.rect(screen, rect_color, to_screen_rect_list(frame_rect), RECT_WIDTH)
                num_surf = font_frame.render(str(i), True, NUMBER_COLOR); screen.blit(num_surf, (to_screen_rect_list(frame_rect).x + 2, to_screen_rect_list(frame_rect).y + 2))

                # Draw pivot point cross
                if show_pivots:
                    fx, fy, fw, fh = frame_rect
                    # pivot_x: 0.0=left, 0.5=center, 1.0=right
                    # pivot_y: 0.0=top, 0.5=center, 1.0=bottom
                    cross_px = fx + fw * pivot_x
                    cross_py = fy + fh * pivot_y
                    # Convert to screen coords
                    screen_pivot = to_screen_coords((cross_px, cross_py))
                    cross_size = int(PIVOT_CROSS_SIZE * zoom) if zoom > 0.5 else PIVOT_CROSS_SIZE
                    pygame.draw.line(screen, PIVOT_CROSS_COLOR, (screen_pivot[0] - cross_size, screen_pivot[1]), (screen_pivot[0] + cross_size, screen_pivot[1]), 2)
                    pygame.draw.line(screen, PIVOT_CROSS_COLOR, (screen_pivot[0], screen_pivot[1] - cross_size), (screen_pivot[0], screen_pivot[1] + cross_size), 2)

            if current_rect:
                current_rect.normalize(); pygame.draw.rect(screen, RECT_COLOR, to_screen_rect_obj(current_rect), RECT_WIDTH)
            if selected_frame_index != -1 and selected_frame_index < len(frames):
                handles = get_handles(pygame.Rect(frames[selected_frame_index]["rect"]))
                for h_rect in handles.values(): pygame.draw.rect(screen, HANDLE_COLOR, to_screen_rect_obj(h_rect))

        # Reset clipping for UI drawing
        screen.set_clip(None)

        pygame.draw.rect(screen, HELP_BG_COLOR, (0, 0, screen.get_width() - DETAIL_PANEL_WIDTH, help_area_height))
        for i, line in enumerate(EDITOR_HELP_LINES): screen.blit(font_help.render(line, True, HELP_TEXT_COLOR), (5, 5 + i * (FONT_SIZE - 5)))
        status_line_y = 5
        # Show mouse position (image coordinates)
        mouse_img_x, mouse_img_y = translated_mouse_pos
        mouse_text = f"XY: ({int(mouse_img_x)}, {int(mouse_img_y)})"; mouse_surf = font_help.render(mouse_text, True, HELP_TEXT_COLOR)
        screen.blit(mouse_surf, (screen.get_width() - DETAIL_PANEL_WIDTH - mouse_surf.get_width() - 10, status_line_y))
        zoom_text = f"Zoom: {int(zoom * 100)}%"; zoom_surf = font_help.render(zoom_text, True, HELP_TEXT_COLOR)
        screen.blit(zoom_surf, (screen.get_width() - DETAIL_PANEL_WIDTH - mouse_surf.get_width() - zoom_surf.get_width() - 20, status_line_y))
        snap_text = f"Snap: {'ON' if snapping_enabled else 'OFF'}"; snap_surf = font_help.render(snap_text, True, HELP_TEXT_COLOR)
        screen.blit(snap_surf, (screen.get_width() - DETAIL_PANEL_WIDTH - mouse_surf.get_width() - zoom_surf.get_width() - snap_surf.get_width() - 30, status_line_y))
        pivot_text = f"Pivot: [{pivot_x:.2f}, {pivot_y:.2f}]"; pivot_surf = font_help.render(pivot_text, True, HELP_TEXT_COLOR)
        screen.blit(pivot_surf, (screen.get_width() - DETAIL_PANEL_WIDTH - mouse_surf.get_width() - zoom_surf.get_width() - snap_surf.get_width() - pivot_surf.get_width() - 40, status_line_y))

        # --- Draw Detail Panel ---
        panel_x = screen.get_width() - DETAIL_PANEL_WIDTH
        pygame.draw.rect(screen, DETAIL_PANEL_COLOR, (panel_x, 0, DETAIL_PANEL_WIDTH, screen.get_height()))
        pygame.draw.line(screen, INPUT_BORDER_COLOR, (panel_x, 0), (panel_x, screen.get_height()), 2)

        if grid_mode:
            # Grid Mode Panel
            title_surf = font_help.render("Grid Split", True, (255, 255, 0))
            screen.blit(title_surf, (panel_x + 10, 10))

            # Instructions
            hint1 = font_detail.render("Arrows: Adjust size", True, DETAIL_LABEL_COLOR)
            hint2 = font_detail.render("R: Apply | ESC: Cancel", True, DETAIL_LABEL_COLOR)
            screen.blit(hint1, (panel_x + 10, 40))
            screen.blit(hint2, (panel_x + 10, 58))

            # Draw grid input fields
            for field in grid_input_fields:
                field.draw(screen, panel_x)

            # Grid info
            cols = img_width // grid_cell_w if grid_cell_w > 0 else 0
            rows = img_height // grid_cell_h if grid_cell_h > 0 else 0
            grid_info = f"Grid: {cols} x {rows} cells"
            grid_surf = font_detail.render(grid_info, True, DETAIL_LABEL_COLOR)
            screen.blit(grid_surf, (panel_x + 10, help_area_height + 150))

        elif selected_frame_index != -1 and selected_frame_index < len(frames):
            # Panel title
            title_surf = font_help.render("Frame Details", True, DETAIL_VALUE_COLOR)
            screen.blit(title_surf, (panel_x + 10, 10))

            # Frame index display
            frame_info = f"Frame #{selected_frame_index}"
            frame_surf = font_detail.render(frame_info, True, DETAIL_LABEL_COLOR)
            screen.blit(frame_surf, (panel_x + 10, 40))

            # Reorder hint
            reorder_hint = ", / . to reorder"
            hint_surf = font_detail.render(reorder_hint, True, (120, 120, 120))
            screen.blit(hint_surf, (panel_x + 10, 58))

            # Draw input fields
            for field in input_fields:
                field.draw(screen, panel_x)

            # Draw event editor
            event_editor.draw(screen, panel_x)
            event_editor.update(clock.get_time())

            # Frame count info (below event editor)
            event_editor_bottom = event_editor.y + len(event_editor.event_fields) * 28 + 40
            count_text = f"Total: {len(frames)} frames"
            count_surf = font_detail.render(count_text, True, DETAIL_LABEL_COLOR)
            screen.blit(count_surf, (panel_x + 10, event_editor_bottom))
        else:
            # Panel title
            title_surf = font_help.render("Animation Settings", True, DETAIL_VALUE_COLOR)
            screen.blit(title_surf, (panel_x + 10, 10))

            # No selection message
            no_sel_text = "No frame selected"
            no_sel_surf = font_detail.render(no_sel_text, True, (100, 100, 100))
            screen.blit(no_sel_surf, (panel_x + 10, 40))

            if len(frames) > 0:
                count_text = f"Total: {len(frames)} frames"
                count_surf = font_detail.render(count_text, True, DETAIL_LABEL_COLOR)
                screen.blit(count_surf, (panel_x + 10, 70))

            # --- Animation Settings ---
            input_interval.draw(screen, panel_x)
            input_pivot_x.draw(screen, panel_x)
            input_pivot_y.draw(screen, panel_x)

            # Pivot hint
            pivot_hint = font_detail.render("(0=left/top, 0.5=center, 1=right/bottom)", True, (100, 100, 100))
            screen.blit(pivot_hint, (panel_x + 10, help_area_height + 135))

        # --- Draw Save Notification ---
        if save_notification:
            msg, timestamp = save_notification
            elapsed = pygame.time.get_ticks() - timestamp
            if elapsed < NOTIFICATION_DURATION:
                # Fade out effect
                alpha = max(0, 255 - int(255 * elapsed / NOTIFICATION_DURATION))
                notif_surf = font_help.render(msg, True, (100, 255, 100))
                notif_bg = pygame.Surface((notif_surf.get_width() + 20, notif_surf.get_height() + 10), pygame.SRCALPHA)
                notif_bg.fill((0, 0, 0, min(180, alpha)))
                notif_x = (screen.get_width() - DETAIL_PANEL_WIDTH - notif_bg.get_width()) // 2
                notif_y = help_area_height + 20
                screen.blit(notif_bg, (notif_x, notif_y))
                notif_surf.set_alpha(alpha)
                screen.blit(notif_surf, (notif_x + 10, notif_y + 5))
            else:
                save_notification = None

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    print("Tool closed.")
    return None

def process_file_path(file_path):
    """
    Process file path and return (image_path, frames, pivot, frame_interval) tuple.
    For PNG: returns (image_path, None, None, None)
    For JSON: loads animation data and returns (image_path, frames, pivot, frame_interval)
    """
    if not file_path or not os.path.exists(file_path):
        return None, None, None, None

    ext = os.path.splitext(file_path)[1].lower()

    if ext == ".json":
        # Load animation JSON
        result = load_animation_json(file_path)
        if result:
            return result  # (image_path, frames, pivot, frame_interval)
        else:
            print(f"Failed to load animation from: {file_path}")
            return None, None, None, None
    else:
        # Treat as image file
        return file_path, None, None, None


def main():
    """Main entry point. Launches file dialog or editor in a loop."""
    file_path = None
    image_path = None
    loaded_frames = None
    loaded_pivot = None
    loaded_interval = None

    # Initial file selection
    if len(sys.argv) > 1:
        file_path = sys.argv[1]
        if not os.path.exists(file_path):
            print(f"Error: File not found at '{file_path}'")
            return
        image_path, loaded_frames, loaded_pivot, loaded_interval = process_file_path(file_path)
    else:
        file_path = open_file_dialog()
        if file_path:
            image_path, loaded_frames, loaded_pivot, loaded_interval = process_file_path(file_path)

    # Main loop - allows opening multiple files
    while image_path:
        print(f"Starting editor for: {image_path}")
        result = run_editor(image_path, loaded_frames, loaded_pivot, loaded_interval)

        if result == "open_file":
            # Open file dialog to select another file
            file_path = open_file_dialog()
            if file_path:
                image_path, loaded_frames, loaded_pivot, loaded_interval = process_file_path(file_path)
            else:
                image_path = None
        else:
            # Normal exit
            break

    if not image_path and file_path is None:
        print("No file selected. Exiting.")

if __name__ == "__main__":
    main()