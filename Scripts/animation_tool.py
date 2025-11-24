import pygame
import json
import sys
import os
import cv2
import numpy as np

# --- Configuration ---
BACKGROUND_COLOR = (30, 30, 30)
TEXT_COLOR = (220, 220, 220)
SELECTED_TEXT_COLOR = (255, 255, 0)
DIR_COLOR = (150, 180, 255)
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

# --- Editor Help Text ---
EDITOR_HELP_LINES = [
    "LMB: Drag=New | Click=Select | Drag Selected=Move | Drag Handle=Resize",
    "N: Save new | D: Delete selected | P: Preview | T: Snap | V: Pivot | A: Auto-Detect",
    "S: Save all frames to JSON & quit | Q/ESC: Quit without saving"
]
# --- Browser Help Text ---
BROWSER_HELP_LINES = ["Up/Down: Navigate | Enter: Select | Q/ESC: Quit"]

# --- Helper Functions ---
def get_snap_coord(mouse_coord, existing_coords, tolerance):
    for coord in existing_coords:
        if abs(mouse_coord - coord) <= tolerance: return coord
    return mouse_coord

def get_handles(rect):
    hs = HANDLE_SIZE // 2
    return {'top-left': pygame.Rect(rect.left - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'top-middle': pygame.Rect(rect.centerx - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'top-right': pygame.Rect(rect.right - hs, rect.top - hs, HANDLE_SIZE, HANDLE_SIZE),'middle-left': pygame.Rect(rect.left - hs, rect.centery - hs, HANDLE_SIZE, HANDLE_SIZE),'middle-right': pygame.Rect(rect.right - hs, rect.centery - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-left': pygame.Rect(rect.left - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-middle': pygame.Rect(rect.centerx - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE),'bottom-right': pygame.Rect(rect.right - hs, rect.bottom - hs, HANDLE_SIZE, HANDLE_SIZE)}

def get_cursor_for_handle(handle_name):
    if handle_name in ('top-left', 'bottom-right'): return pygame.SYSTEM_CURSOR_SIZENWSE
    if handle_name in ('top-right', 'bottom-left'): return pygame.SYSTEM_CURSOR_SIZENESW
    if 'middle' in handle_name and ('left' in handle_name or 'right' in handle_name): return pygame.SYSTEM_CURSOR_SIZEWE
    if 'middle' in handle_name and ('top' in handle_name or 'bottom' in handle_name): return pygame.SYSTEM_CURSOR_SIZENS
    return pygame.SYSTEM_CURSOR_ARROW

def auto_detect_frames(image_path):
    try:
        img = cv2.imread(image_path, cv2.IMREAD_UNCHANGED)
        if img is None or img.shape[2] < 4:
            print("Info: No alpha channel for auto-detection.")
            return []
        alpha_channel = img[:, :, 3]
        _, thresh = cv2.threshold(alpha_channel, 0, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        frames = [[x, y, w, h] for c in contours if cv2.contourArea(c) > MIN_CONTOUR_AREA for x, y, w, h in [cv2.boundingRect(c)]]
        frames.sort(key=lambda r: (r[1], r[0]))
        print(f"Auto-detected {len(frames)} frames.")
        return frames
    except Exception as e:
        print(f"Error during auto-detection: {e}")
        return []

def run_file_browser(screen, font):
    """Displays a file browser and returns the path of the selected image."""
    current_path = os.getcwd()
    selected_index = 0
    scroll_offset = 0
    running = True
    clock = pygame.time.Clock()

    while running:
        try:
            items = os.listdir(current_path)
            dirs = sorted([d for d in items if os.path.isdir(os.path.join(current_path, d))])
            files = sorted([f for f in items if os.path.isfile(os.path.join(current_path, f)) and f.lower().endswith(('.png', '.jpg', '.jpeg', '.bmp'))])
            
            display_items = ["[..]"] + dirs + files
        except OSError as e:
            print(f"Error reading path: {e}")
            current_path = os.path.dirname(current_path)
            continue

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return None
            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE):
                    return None
                elif event.key == pygame.K_UP:
                    if len(display_items) > 0: selected_index = (selected_index - 1) % len(display_items)
                elif event.key == pygame.K_DOWN:
                    if len(display_items) > 0: selected_index = (selected_index + 1) % len(display_items)
                elif event.key == pygame.K_RETURN:
                    if not display_items: continue
                    selected_item = display_items[selected_index]
                    new_path = os.path.normpath(os.path.join(current_path, selected_item))
                    if selected_item == "[..]":
                        current_path = os.path.dirname(current_path)
                        selected_index = 0
                    elif os.path.isdir(new_path):
                        current_path = new_path
                        selected_index = 0
                    else:
                        return new_path
            elif event.type == pygame.VIDEORESIZE:
                screen = pygame.display.set_mode((event.w, event.h), pygame.RESIZABLE)

        screen.fill(BACKGROUND_COLOR)
        
        # --- Drawing ---
        max_visible_items = screen.get_height() // FONT_SIZE - 4
        if selected_index >= scroll_offset + max_visible_items:
            scroll_offset = selected_index - max_visible_items + 1
        if selected_index < scroll_offset:
            scroll_offset = selected_index

        path_surf = font.render(f"Current Path: {current_path}", True, TEXT_COLOR)
        screen.blit(path_surf, (5, 5))

        for i, item in enumerate(display_items[scroll_offset:scroll_offset + max_visible_items]):
            actual_index = i + scroll_offset
            color = TEXT_COLOR
            if actual_index == selected_index:
                color = SELECTED_TEXT_COLOR
            
            is_dir = os.path.isdir(os.path.join(current_path, item)) or item == "[..]"
            display_text = f"> {item}" if is_dir else f"  {item}"
            if is_dir: color = DIR_COLOR if actual_index != selected_index else SELECTED_TEXT_COLOR
            
            item_surf = font.render(display_text, True, color)
            screen.blit(item_surf, (10, 35 + i * FONT_SIZE))
        
        for i, line in enumerate(BROWSER_HELP_LINES):
             screen.blit(font.render(line, True, HELP_TEXT_COLOR), (5, screen.get_height() - (len(BROWSER_HELP_LINES) - i) * (FONT_SIZE)))

        pygame.display.flip()
        clock.tick(30)
    return None

def run_editor(image_path):
    pygame.init()
    
    font_help = pygame.font.Font(None, FONT_SIZE)
    font_frame = pygame.font.Font(None, FRAME_FONT_SIZE)
    help_area_height = len(EDITOR_HELP_LINES) * (FONT_SIZE - 5) + 15

    try: temp_image = pygame.image.load(image_path)
    except pygame.error as e: print(f"Error loading image file: {e}"); return
    
    img_width, img_height = temp_image.get_size()
    screen_width = max(img_width, 900)
    screen_height = img_height + help_area_height
    screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
    pygame.display.set_caption(f"Animation Frame Tool - {os.path.basename(image_path)}")
    sprite_sheet = temp_image.convert_alpha()
    sprite_sheet_offset = (0, help_area_height)

    frames = auto_detect_frames(image_path)
    
    running = True; snapping_enabled = True; is_previewing = False
    pivot_mode = "bottom-center"; selected_frame_index = -1
    current_rect, start_pos, resize_mode, is_moving = None, None, None, False
    clock = pygame.time.Clock()

    while running:
        mouse_pos = pygame.mouse.get_pos()
        translated_mouse_pos = (mouse_pos[0] - sprite_sheet_offset[0], mouse_pos[1] - sprite_sheet_offset[1])
        
        if not is_previewing and not start_pos:
            cursor_set = False
            if selected_frame_index != -1 and selected_frame_index < len(frames):
                selected_rect = pygame.Rect(frames[selected_frame_index])
                handles = get_handles(selected_rect)
                for name, h_rect in handles.items():
                    if h_rect.collidepoint(translated_mouse_pos):
                        pygame.mouse.set_cursor(get_cursor_for_handle(name)); cursor_set = True; break
            if not cursor_set: pygame.mouse.set_cursor(pygame.SYSTEM_CURSOR_ARROW)

        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False
            elif event.type == pygame.VIDEORESIZE:
                screen_width, screen_height = event.w, event.h
                screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
            elif event.type == pygame.KEYDOWN:
                if event.key in (pygame.K_q, pygame.K_ESCAPE): running = False
                elif event.key == pygame.K_a and not is_previewing:
                    frames = auto_detect_frames(image_path)
                    selected_frame_index = -1
                elif event.key == pygame.K_v: pivot_mode = "center" if pivot_mode == "bottom-center" else "bottom-center"; print(f"Pivot mode set to: {pivot_mode}")
                elif event.key == pygame.K_n and not is_previewing:
                    if current_rect and current_rect.width > 0 and current_rect.height > 0:
                        frames.append(list(current_rect.normalize())); selected_frame_index = len(frames) - 1; current_rect, start_pos = None, None
                elif event.key == pygame.K_d and not is_previewing:
                    if selected_frame_index != -1: frames.pop(selected_frame_index); selected_frame_index = -1
                elif event.key == pygame.K_t: snapping_enabled = not snapping_enabled
                elif event.key == pygame.K_p and frames: is_previewing = not is_previewing
                elif event.key == pygame.K_s:
                    if frames:
                        output_data = {"pivot": pivot_mode, "frames": frames}
                        output_filename = os.path.splitext(os.path.basename(image_path))[0] + ".json"
                        output_path = os.path.join(os.path.dirname(image_path), output_filename)
                        with open(output_path, 'w') as f: json.dump(output_data, f, indent=4)
                        print(f"Saved {len(frames)} frames and pivot '{pivot_mode}' to '{output_path}'")
                    running = False

            if not is_previewing and translated_mouse_pos[1] >= 0:
                if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                    start_pos = translated_mouse_pos
                    if selected_frame_index != -1 and selected_frame_index < len(frames):
                        handles = get_handles(pygame.Rect(frames[selected_frame_index]))
                        for name, h_rect in handles.items():
                            if h_rect.collidepoint(translated_mouse_pos): resize_mode = name; break
                    if not resize_mode:
                        clicked_idx = -1
                        for i in range(len(frames) - 1, -1, -1):
                            if pygame.Rect(frames[i]).collidepoint(translated_mouse_pos): clicked_idx = i; break
                        if clicked_idx != -1: selected_frame_index, is_moving = clicked_idx, True
                        else: selected_frame_index, current_rect = -1, pygame.Rect(start_pos, (0,0))
                elif event.type == pygame.MOUSEBUTTONUP and event.button == 1: start_pos, resize_mode, is_moving = None, None, False
                elif event.type == pygame.MOUSEMOTION:
                    if start_pos and resize_mode and selected_frame_index != -1:
                        rect = pygame.Rect(frames[selected_frame_index]); mx, my = translated_mouse_pos
                        if 'right' in resize_mode: rect.width = mx - rect.x
                        if 'left' in resize_mode: rect.width += rect.x - mx; rect.x = mx
                        if 'bottom' in resize_mode: rect.height = my - rect.y
                        if 'top' in resize_mode: rect.height += rect.y - my; rect.y = my
                        if rect.width < 1: rect.width = 1
                        if rect.height < 1: rect.height = 1
                        frames[selected_frame_index] = [rect.x, rect.y, rect.width, rect.height]
                    elif start_pos and is_moving and selected_frame_index != -1:
                        dx, dy = translated_mouse_pos[0] - start_pos[0], translated_mouse_pos[1] - start_pos[1]
                        frames[selected_frame_index][0] += dx; frames[selected_frame_index][1] += dy
                        start_pos = translated_mouse_pos
                    elif start_pos and current_rect is not None:
                        x1, y1 = start_pos
                        x2, y2 = translated_mouse_pos
                        current_rect.x = min(x1, x2)
                        current_rect.y = min(y1, y2)
                        current_rect.width = abs(x1 - x2)
                        current_rect.height = abs(y1 - y2)

        screen.fill(BACKGROUND_COLOR); screen.blit(sprite_sheet, sprite_sheet_offset)

        def to_screen_coords(p): return (p[0] + sprite_sheet_offset[0], p[1] + sprite_sheet_offset[1])
        def to_screen_rect_list(r_list): return pygame.Rect(to_screen_coords((r_list[0], r_list[1])), (r_list[2], r_list[3]))
        def to_screen_rect_obj(r_obj): return pygame.Rect(to_screen_coords(r_obj.topleft), r_obj.size)
        
        if is_previewing:
            overlay = pygame.Surface((screen.get_width(), screen.get_height()), pygame.SRCALPHA); overlay.fill(PREVIEW_BG_COLOR); screen.blit(overlay, (0, 0))
            if frames:
                preview_frame_index = (pygame.time.get_ticks() // (1000 // PREVIEW_FPS)) % len(frames)
                frame_rect_data = frames[preview_frame_index]
                if frame_rect_data[2] > 0 and frame_rect_data[3] > 0:
                    try:
                        preview_img = sprite_sheet.subsurface(pygame.Rect(frame_rect_data))
                        anchor_pos = (screen.get_width() // 2, screen.get_height() // 2)
                        if pivot_mode == 'bottom-center': w, h = preview_img.get_size(); draw_pos = (anchor_pos[0] - w // 2, anchor_pos[1] - h); screen.blit(preview_img, draw_pos)
                        else: screen.blit(preview_img, preview_img.get_rect(center=anchor_pos))
                    except ValueError: pass
            preview_text_surf = font_help.render(f"PREVIEW MODE ({pivot_mode}) - P to exit", True, HELP_TEXT_COLOR)
            screen.blit(preview_text_surf, (5, screen.get_height() - FONT_SIZE - 5))
        else:
            for i, frame_coords in enumerate(frames):
                rect_color = SELECTED_RECT_COLOR if i == selected_frame_index else SAVED_RECT_COLOR
                pygame.draw.rect(screen, rect_color, to_screen_rect_list(frame_coords), RECT_WIDTH)
                num_surf = font_frame.render(str(i), True, NUMBER_COLOR); screen.blit(num_surf, (to_screen_rect_list(frame_coords).x + 2, to_screen_rect_list(frame_coords).y + 2))
            if current_rect:
                current_rect.normalize(); pygame.draw.rect(screen, RECT_COLOR, to_screen_rect_obj(current_rect), RECT_WIDTH)
            if selected_frame_index != -1 and selected_frame_index < len(frames):
                handles = get_handles(pygame.Rect(frames[selected_frame_index]))
                for h_rect in handles.values(): pygame.draw.rect(screen, HANDLE_COLOR, to_screen_rect_obj(h_rect))

        pygame.draw.rect(screen, HELP_BG_COLOR, (0, 0, screen.get_width(), help_area_height))
        for i, line in enumerate(EDITOR_HELP_LINES): screen.blit(font_help.render(line, True, HELP_TEXT_COLOR), (5, 5 + i * (FONT_SIZE - 5)))
        status_line_y = 5
        snap_text = f"Snapping: {'ON' if snapping_enabled else 'OFF'}"; snap_surf = font_help.render(snap_text, True, HELP_TEXT_COLOR)
        screen.blit(snap_surf, (screen.get_width() - snap_surf.get_width() - 5, status_line_y))
        pivot_text = f"Pivot: {pivot_mode}"; pivot_surf = font_help.render(pivot_text, True, HELP_TEXT_COLOR)
        screen.blit(pivot_surf, (screen.get_width() - pivot_surf.get_width() - snap_surf.get_width() - 15, status_line_y))
        display_rect_data = None
        if current_rect: display_rect_data = current_rect.normalize()
        elif selected_frame_index != -1 and selected_frame_index < len(frames): display_rect_data = pygame.Rect(frames[selected_frame_index])
        if display_rect_data:
            size_text = f"Size: {display_rect_data.width}x{display_rect_data.height}"
            screen.blit(font_help.render(size_text, True, HELP_TEXT_COLOR), (screen.get_width() - font_help.size(size_text)[0] - 5, 25))

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    print("Tool closed.")

def main():
    """Main entry point. Launches browser or editor."""
    image_path = None
    if len(sys.argv) > 1:
        image_path = sys.argv[1]
        if not os.path.exists(image_path):
            print(f"Error: Image file not found at '{image_path}'")
            return
    else:
        pygame.init()
        screen = pygame.display.set_mode((800, 600), pygame.RESIZABLE)
        font = pygame.font.Font(None, FONT_SIZE)
        pygame.display.set_caption("File Browser")
        image_path = run_file_browser(screen, font)
        if image_path: pygame.quit() # Quit browser's pygame instance before starting editor's

    if image_path:
        print(f"Starting editor for: {image_path}")
        run_editor(image_path)
    else:
        print("No file selected. Exiting.")

if __name__ == "__main__":
    main()