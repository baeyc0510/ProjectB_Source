"""
Map Editor for Blasphemous Clone
- 레이어 추가/관리 + Parallax 설정
- 실시간 미리보기 (Parallax 적용)
- 카메라 이동 (화살표 키) + 줌 (휠 스크롤)
- 오브젝트 배치 (체크포인트, 스폰, 씬 전환)
- 메타 이미지 오버레이 (최상단)
- JSON 저장/로드
"""

import pygame
import numpy as np
import json
import os
import sys
from dataclasses import dataclass, field
from typing import List, Optional, Dict, Any
from tkinter import Tk, filedialog
import tkinter.simpledialog as simpledialog

# 초기화
pygame.init()
Tk().withdraw()

# 프로젝트 경로 설정
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)
RESOURCES_PATH = os.path.join(PROJECT_ROOT, "Resources")

def get_relative_path(filepath):
    """Resources 폴더 기준 상대 경로 반환"""
    try:
        return os.path.relpath(filepath, RESOURCES_PATH).replace("\\", "/")
    except ValueError:
        return os.path.basename(filepath)

def get_absolute_path(relative_path):
    """Resources 폴더 기준 상대 경로를 절대 경로로 변환"""
    return os.path.join(RESOURCES_PATH, relative_path.replace("/", os.sep))

def load_image_with_transparency(filepath):
    """이미지 로드 후 마젠타(255,0,255)를 투명 처리 - 게임 엔진과 동일"""
    img = pygame.image.load(filepath).convert_alpha()
    # 픽셀 배열로 변환하여 마젠타 투명 처리
    pixels = pygame.surfarray.pixels3d(img)
    alpha = pygame.surfarray.pixels_alpha(img)

    # 마젠타 (255,0,255) 투명 처리
    magenta_mask = (pixels[:,:,0] == 255) & (pixels[:,:,1] == 0) & (pixels[:,:,2] == 255)
    alpha[magenta_mask] = 0

    del pixels
    del alpha
    return img

# 상수
WINDOW_WIDTH = 1400
WINDOW_HEIGHT = 800
SIDEBAR_WIDTH = 300
TOOLBAR_HEIGHT = 40
LAYER_TAB_HEIGHT = 35
CANVAS_WIDTH = WINDOW_WIDTH - SIDEBAR_WIDTH
CANVAS_HEIGHT = WINDOW_HEIGHT - TOOLBAR_HEIGHT - LAYER_TAB_HEIGHT

# 색상
COLOR_BG = (25, 25, 30)
COLOR_SIDEBAR = (40, 40, 48)
COLOR_TOOLBAR = (35, 35, 42)
COLOR_BUTTON = (55, 55, 65)
COLOR_BUTTON_HOVER = (70, 70, 82)
COLOR_BUTTON_ACTIVE = (90, 120, 180)
COLOR_TEXT = (220, 220, 225)
COLOR_TEXT_DIM = (140, 140, 150)
COLOR_ACCENT = (100, 150, 255)
COLOR_GRID = (45, 45, 52)
COLOR_LAYER_TAB = (50, 50, 58)
COLOR_LAYER_TAB_SELECTED = (70, 100, 160)

# 메타 색상 정의
META_COLORS = {
    'Empty': (0, 0, 0),
    'Solid': (255, 0, 0),
    'OneWay': (0, 255, 0),
    'Ladder': (0, 0, 255),
    'WallClimb': (255, 128, 0),
    'Ledge': (0, 255, 255),
    'Swamp': (128, 0, 128),
    'Damage': (255, 255, 0),
}

@dataclass
class Layer:
    name: str = "Layer"
    image_path: str = ""
    parallax: float = 1.0
    scale: float = 1.0  # 렌더링 스케일
    offset_x: float = 0.0  # X 오프셋
    offset_y: float = 0.0  # Y 오프셋
    image: Optional[pygame.Surface] = None
    meta_path: str = ""
    meta_image: Optional[pygame.Surface] = None
    visible: bool = True
    is_main: bool = False  # 메인 레이어 여부 (메타데이터 포함)

@dataclass
class Checkpoint:
    id: int
    x: float
    y: float

@dataclass
class Transition:
    x: float
    y: float
    w: float
    h: float
    target: str
    spawn_id: int

@dataclass
class MapData:
    layers: List[Layer] = field(default_factory=list)
    player_spawn: List[float] = field(default_factory=lambda: [100, 500])
    bounds: List[float] = field(default_factory=lambda: [0, 0, 2000, 1080])
    checkpoints: List[Checkpoint] = field(default_factory=list)
    transitions: List[Transition] = field(default_factory=list)
    map_directory: str = ""

class Button:
    def __init__(self, x, y, w, h, text, callback=None, font_size=14):
        self.rect = pygame.Rect(x, y, w, h)
        self.text = text
        self.callback = callback
        self.hovered = False
        self.active = False
        self.font_size = font_size
        self.enabled = True

    def draw(self, screen, font):
        if not self.enabled:
            color = (40, 40, 45)
        elif self.active:
            color = COLOR_BUTTON_ACTIVE
        elif self.hovered:
            color = COLOR_BUTTON_HOVER
        else:
            color = COLOR_BUTTON

        pygame.draw.rect(screen, color, self.rect, border_radius=4)
        border_color = COLOR_ACCENT if self.active else (COLOR_TEXT_DIM if self.enabled else (60, 60, 65))
        pygame.draw.rect(screen, border_color, self.rect, 1, border_radius=4)

        text_color = COLOR_TEXT if self.enabled else COLOR_TEXT_DIM
        text_surface = font.render(self.text, True, text_color)
        text_rect = text_surface.get_rect(center=self.rect.center)
        screen.blit(text_surface, text_rect)

    def handle_event(self, event):
        if not self.enabled:
            return False
        if event.type == pygame.MOUSEMOTION:
            self.hovered = self.rect.collidepoint(event.pos)
        elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.rect.collidepoint(event.pos) and self.callback:
                self.callback()
                return True
        return False

class Slider:
    def __init__(self, x, y, w, h, min_val, max_val, value, label, on_change=None):
        self.rect = pygame.Rect(x, y, w, h)
        self.min_val = min_val
        self.max_val = max_val
        self.value = value
        self.label = label
        self.dragging = False
        self.on_change = on_change

    def draw(self, screen, font):
        label_surface = font.render(f"{self.label}: {self.value:.2f}", True, COLOR_TEXT)
        screen.blit(label_surface, (self.rect.x, self.rect.y - 18))

        pygame.draw.rect(screen, COLOR_BUTTON, self.rect, border_radius=4)

        ratio = (self.value - self.min_val) / (self.max_val - self.min_val) if self.max_val != self.min_val else 0
        fill_rect = pygame.Rect(self.rect.x, self.rect.y, int(self.rect.w * ratio), self.rect.h)
        pygame.draw.rect(screen, COLOR_ACCENT, fill_rect, border_radius=4)

        handle_x = self.rect.x + int(self.rect.w * ratio)
        pygame.draw.circle(screen, COLOR_TEXT, (handle_x, self.rect.centery), 7)

    def handle_event(self, event):
        if event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
            if self.rect.collidepoint(event.pos):
                self.dragging = True
                self._update_value(event.pos[0])
                return True
        elif event.type == pygame.MOUSEBUTTONUP and event.button == 1:
            self.dragging = False
        elif event.type == pygame.MOUSEMOTION and self.dragging:
            self._update_value(event.pos[0])
            return True
        return False

    def _update_value(self, mouse_x):
        ratio = (mouse_x - self.rect.x) / self.rect.w
        ratio = max(0, min(1, ratio))
        self.value = self.min_val + ratio * (self.max_val - self.min_val)
        if self.on_change:
            self.on_change(self.value)

class MapEditor:
    def __init__(self):
        self.screen = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
        pygame.display.set_caption("Map Editor - Blasphemous Clone")

        self.font = pygame.font.SysFont("Segoe UI", 14)
        self.font_small = pygame.font.SysFont("Segoe UI", 12)
        self.font_large = pygame.font.SysFont("Segoe UI", 16, bold=True)

        self.map_data = MapData()
        self.camera_x = 0.0
        self.camera_y = 0.0
        self.camera_speed = 8
        self.zoom = 1.0
        self.min_zoom = 0.25
        self.max_zoom = 4.0

        self.selected_layer_index = -1
        self.show_meta_overlay = True
        self.meta_overlay_alpha = 160
        self.show_grid = True
        self.grid_size = 32
        self.camera_bound_enabled = False  # 카메라 바운딩 토글

        self.current_tool = 'select'
        self.placing_transition = None
        self.placing_bounds = None

        self.running = True
        self.clock = pygame.time.Clock()

        self.layer_tabs = []
        self._setup_ui()

    def _setup_ui(self):
        # 툴바 버튼
        self.toolbar_buttons = []
        x = 10
        y = 8
        btn_w = 70
        btn_h = 26
        gap = 5

        self.toolbar_buttons.append(Button(x, y, btn_w, btn_h, "New", self._new_map))
        x += btn_w + gap
        self.toolbar_buttons.append(Button(x, y, btn_w, btn_h, "Load", self._load_map))
        x += btn_w + gap
        self.toolbar_buttons.append(Button(x, y, btn_w, btn_h, "Save", self._save_map))
        x += btn_w + gap + 20

        self.toolbar_buttons.append(Button(x, y, btn_w + 10, btn_h, "+ Layer", self._add_layer))
        x += btn_w + 10 + gap

        # 뷰 토글
        x = CANVAS_WIDTH - 380
        self.btn_toggle_meta = Button(x, y, 90, btn_h, "Meta: ON", self._toggle_meta)
        self.toolbar_buttons.append(self.btn_toggle_meta)
        x += 95
        self.btn_toggle_grid = Button(x, y, 75, btn_h, "Grid: ON", self._toggle_grid)
        self.toolbar_buttons.append(self.btn_toggle_grid)
        x += 80
        self.btn_toggle_cam_bound = Button(x, y, 100, btn_h, "Bound: OFF", self._toggle_cam_bound)
        self.toolbar_buttons.append(self.btn_toggle_cam_bound)

        # 사이드바 - 툴 버튼
        self.sidebar_buttons = []
        x = CANVAS_WIDTH + 15
        y = 20
        w = SIDEBAR_WIDTH - 30

        tools = [('select', 'Select'), ('spawn', 'Set Spawn'), ('checkpoint', 'Checkpoint'), ('transition', 'Transition'), ('bounds', 'Set Bounds')]
        for i, (tool_id, tool_name) in enumerate(tools):
            btn = Button(x + (i % 2) * (w//2 + 5), y + (i // 2) * 32, w//2 - 5, 28, tool_name,
                        lambda t=tool_id: self._set_tool(t))
            self.sidebar_buttons.append((tool_id, btn))

        # 레이어 속성 슬라이더
        self.parallax_slider = Slider(
            CANVAS_WIDTH + 15, 150, SIDEBAR_WIDTH - 30, 8,
            0.0, 2.0, 1.0, "Parallax",
            on_change=self._on_parallax_change
        )
        self.scale_slider = Slider(
            CANVAS_WIDTH + 15, 200, SIDEBAR_WIDTH - 30, 8,
            0.1, 4.0, 1.0, "Scale",
            on_change=self._on_scale_change
        )
        self.offset_x_slider = Slider(
            CANVAS_WIDTH + 15, 250, SIDEBAR_WIDTH - 30, 8,
            -2000, 2000, 0, "Offset X",
            on_change=self._on_offset_x_change
        )
        self.offset_y_slider = Slider(
            CANVAS_WIDTH + 15, 300, SIDEBAR_WIDTH - 30, 8,
            -2000, 2000, 0, "Offset Y",
            on_change=self._on_offset_y_change
        )

        # 사이드바 버튼 rect 저장용 (클릭 핸들링에 사용)
        self.sidebar_btn_rects = {}

    def _update_layer_tabs(self):
        """레이어 탭 버튼 업데이트"""
        self.layer_tabs = []
        if not self.map_data.layers:
            return

        tab_width = min(120, (CANVAS_WIDTH - 20) // max(1, len(self.map_data.layers)))
        x = 10
        y = TOOLBAR_HEIGHT + 5

        for i, layer in enumerate(self.map_data.layers):
            name = layer.name[:12] + "..." if len(layer.name) > 12 else layer.name
            btn = Button(x, y, tab_width - 5, LAYER_TAB_HEIGHT - 10, name,
                        lambda idx=i: self._select_layer(idx))
            self.layer_tabs.append(btn)
            x += tab_width

    def _select_layer(self, index):
        self.selected_layer_index = index
        if 0 <= index < len(self.map_data.layers):
            layer = self.map_data.layers[index]
            self.parallax_slider.value = layer.parallax
            self.scale_slider.value = layer.scale
            self.offset_x_slider.value = layer.offset_x
            self.offset_y_slider.value = layer.offset_y

    def _on_parallax_change(self, value):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            self.map_data.layers[self.selected_layer_index].parallax = value

    def _on_scale_change(self, value):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            self.map_data.layers[self.selected_layer_index].scale = value

    def _on_offset_x_change(self, value):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            self.map_data.layers[self.selected_layer_index].offset_x = value

    def _on_offset_y_change(self, value):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            self.map_data.layers[self.selected_layer_index].offset_y = value

    def _new_map(self):
        self.map_data = MapData()
        self.selected_layer_index = -1
        self.camera_x = 0
        self.camera_y = 0
        self.zoom = 1.0
        self._update_layer_tabs()

    def _load_map(self):
        filepath = filedialog.askopenfilename(
            title="Load Map JSON",
            initialdir=RESOURCES_PATH,
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        if filepath:
            self._load_map_from_file(filepath)

    def _load_map_from_file(self, filepath):
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)

            self.map_data = MapData()
            self.map_data.map_directory = os.path.dirname(filepath)

            for i, layer_data in enumerate(data.get('layers', [])):
                layer = Layer()
                layer.name = f"Layer {i+1}"
                layer.image_path = layer_data.get('image', '')
                layer.parallax = layer_data.get('parallax', 1.0)
                layer.scale = layer_data.get('scale', 1.0)
                layer.offset_x = layer_data.get('offsetX', 0.0)
                layer.offset_y = layer_data.get('offsetY', 0.0)
                layer.meta_path = layer_data.get('meta', '')
                layer.is_main = layer_data.get('isMain', False)

                if layer.image_path:
                    img_path = get_absolute_path(layer.image_path)
                    if os.path.exists(img_path):
                        layer.image = load_image_with_transparency(img_path)
                        # 이름을 파일명에서 추출
                        layer.name = os.path.splitext(os.path.basename(layer.image_path))[0]

                if layer.meta_path:
                    meta_path = get_absolute_path(layer.meta_path)
                    if os.path.exists(meta_path):
                        layer.meta_image = load_image_with_transparency(meta_path)

                self.map_data.layers.append(layer)

            self.map_data.player_spawn = data.get('playerSpawn', [100, 500])
            self.map_data.bounds = data.get('bounds', [0, 0, 2000, 1080])

            for cp_data in data.get('checkpoints', []):
                cp = Checkpoint(cp_data['id'], cp_data['pos'][0], cp_data['pos'][1])
                self.map_data.checkpoints.append(cp)

            for trans_data in data.get('transitions', []):
                trans = Transition(
                    trans_data['rect'][0], trans_data['rect'][1],
                    trans_data['rect'][2], trans_data['rect'][3],
                    trans_data['target'], trans_data.get('spawnId', 0)
                )
                self.map_data.transitions.append(trans)

            self.selected_layer_index = 0 if self.map_data.layers else -1
            self._update_layer_tabs()
            if self.selected_layer_index >= 0:
                layer = self.map_data.layers[0]
                self.parallax_slider.value = layer.parallax
                self.scale_slider.value = layer.scale
                self.offset_x_slider.value = layer.offset_x
                self.offset_y_slider.value = layer.offset_y

        except Exception as e:
            print(f"Error loading map: {e}")

    def _save_map(self):
        initial_dir = self.map_data.map_directory if self.map_data.map_directory else RESOURCES_PATH
        filepath = filedialog.asksaveasfilename(
            title="Save Map JSON",
            initialdir=initial_dir,
            defaultextension=".json",
            filetypes=[("JSON files", "*.json")]
        )
        if not filepath:
            return
        self.map_data.map_directory = os.path.dirname(filepath)

        data = {
            'layers': [],
            'playerSpawn': self.map_data.player_spawn,
            'bounds': self.map_data.bounds,
            'checkpoints': [],
            'transitions': []
        }

        for layer in self.map_data.layers:
            layer_data = {
                'image': layer.image_path,
                'parallax': round(layer.parallax, 2),
                'scale': round(layer.scale, 2),
                'offsetX': round(layer.offset_x),
                'offsetY': round(layer.offset_y),
                'isMain': layer.is_main
            }
            if layer.meta_path:
                layer_data['meta'] = layer.meta_path
            data['layers'].append(layer_data)

        for cp in self.map_data.checkpoints:
            data['checkpoints'].append({
                'id': cp.id,
                'pos': [cp.x, cp.y]
            })

        for trans in self.map_data.transitions:
            data['transitions'].append({
                'rect': [trans.x, trans.y, trans.w, trans.h],
                'target': trans.target,
                'spawnId': trans.spawn_id
            })

        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2)

    def _add_layer(self):
        filepath = filedialog.askopenfilename(
            title="Select Layer Image",
            initialdir=RESOURCES_PATH,
            filetypes=[("Image files", "*.png;*.jpg;*.bmp"), ("All files", "*.*")]
        )
        if filepath:
            layer = Layer()
            layer.image_path = get_relative_path(filepath)
            layer.name = os.path.splitext(os.path.basename(filepath))[0]
            layer.image = load_image_with_transparency(filepath)

            self.map_data.layers.append(layer)
            self.selected_layer_index = len(self.map_data.layers) - 1
            self._update_layer_tabs()

    def _remove_selected_layer(self):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            del self.map_data.layers[self.selected_layer_index]
            self.selected_layer_index = min(self.selected_layer_index, len(self.map_data.layers) - 1)
            self._update_layer_tabs()

    def _move_layer_up(self):
        if self.selected_layer_index > 0:
            layers = self.map_data.layers
            layers[self.selected_layer_index], layers[self.selected_layer_index - 1] = \
                layers[self.selected_layer_index - 1], layers[self.selected_layer_index]
            self.selected_layer_index -= 1
            self._update_layer_tabs()

    def _move_layer_down(self):
        if self.selected_layer_index < len(self.map_data.layers) - 1:
            layers = self.map_data.layers
            layers[self.selected_layer_index], layers[self.selected_layer_index + 1] = \
                layers[self.selected_layer_index + 1], layers[self.selected_layer_index]
            self.selected_layer_index += 1
            self._update_layer_tabs()

    def _set_meta_image(self):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            filepath = filedialog.askopenfilename(
                title="Select Meta Image",
                initialdir=RESOURCES_PATH,
                filetypes=[("Image files", "*.png;*.bmp"), ("All files", "*.*")]
            )
            if filepath:
                layer = self.map_data.layers[self.selected_layer_index]
                layer.meta_path = get_relative_path(filepath)
                layer.meta_image = load_image_with_transparency(filepath)
                layer.is_main = True

    def _clear_meta_image(self):
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            layer = self.map_data.layers[self.selected_layer_index]
            layer.meta_path = ""
            layer.meta_image = None
            layer.is_main = False

    def _toggle_main_layer(self):
        """현재 레이어를 메인으로 설정/해제 (메인은 하나만 가능)"""
        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            layer = self.map_data.layers[self.selected_layer_index]
            if layer.is_main:
                # 이미 메인이면 해제
                layer.is_main = False
            else:
                # 다른 레이어의 메인 해제하고 현재 레이어를 메인으로
                for l in self.map_data.layers:
                    l.is_main = False
                layer.is_main = True

    def _toggle_meta(self):
        self.show_meta_overlay = not self.show_meta_overlay
        self.btn_toggle_meta.text = f"Meta: {'ON' if self.show_meta_overlay else 'OFF'}"

    def _toggle_grid(self):
        self.show_grid = not self.show_grid
        self.btn_toggle_grid.text = f"Grid: {'ON' if self.show_grid else 'OFF'}"

    def _toggle_cam_bound(self):
        self.camera_bound_enabled = not self.camera_bound_enabled
        self.btn_toggle_cam_bound.text = f"Bound: {'ON' if self.camera_bound_enabled else 'OFF'}"
        if self.camera_bound_enabled:
            self._clamp_camera_to_bounds()

    def _clamp_camera_to_bounds(self):
        """카메라를 바운딩 영역 내로 제한"""
        bounds = self.map_data.bounds
        if bounds[2] <= 0 or bounds[3] <= 0:
            return  # 바운드가 설정되지 않음

        # 뷰포트 크기 (월드 좌표 기준)
        viewport_w = CANVAS_WIDTH / self.zoom
        viewport_h = CANVAS_HEIGHT / self.zoom

        # 카메라 위치 클램핑
        min_x = bounds[0]
        min_y = bounds[1]
        max_x = bounds[0] + bounds[2] - viewport_w
        max_y = bounds[1] + bounds[3] - viewport_h

        # 바운드가 뷰포트보다 작으면 중앙 정렬
        if max_x < min_x:
            self.camera_x = bounds[0] + bounds[2] / 2 - viewport_w / 2
        else:
            self.camera_x = max(min_x, min(max_x, self.camera_x))

        if max_y < min_y:
            self.camera_y = bounds[1] + bounds[3] / 2 - viewport_h / 2
        else:
            self.camera_y = max(min_y, min(max_y, self.camera_y))

    def _set_tool(self, tool):
        self.current_tool = tool
        self.placing_transition = None
        self.placing_bounds = None

    def _world_to_screen(self, world_x, world_y):
        """월드 좌표를 스크린 좌표로 변환"""
        screen_x = (world_x - self.camera_x) * self.zoom
        screen_y = (world_y - self.camera_y) * self.zoom + TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT
        return screen_x, screen_y

    def _screen_to_world(self, screen_x, screen_y):
        """스크린 좌표를 월드 좌표로 변환"""
        world_x = screen_x / self.zoom + self.camera_x
        world_y = (screen_y - TOOLBAR_HEIGHT - LAYER_TAB_HEIGHT) / self.zoom + self.camera_y
        return world_x, world_y

    def _handle_canvas_click(self, pos, button):
        world_x, world_y = self._screen_to_world(pos[0], pos[1])

        if button == 1:  # 좌클릭
            if self.current_tool == 'spawn':
                self.map_data.player_spawn = [world_x, world_y]

            elif self.current_tool == 'checkpoint':
                new_id = len(self.map_data.checkpoints) + 1
                self.map_data.checkpoints.append(Checkpoint(new_id, world_x, world_y))

            elif self.current_tool == 'transition':
                if self.placing_transition is None:
                    self.placing_transition = (world_x, world_y)
                else:
                    start_x, start_y = self.placing_transition
                    w = abs(world_x - start_x)
                    h = abs(world_y - start_y)
                    x = min(start_x, world_x)
                    y = min(start_y, world_y)

                    target = simpledialog.askstring("Transition", "Target scene name:")
                    if target:
                        spawn_id = simpledialog.askinteger("Transition", "Spawn ID:", initialvalue=0) or 0
                        self.map_data.transitions.append(Transition(x, y, w, h, target, spawn_id))
                    self.placing_transition = None

            elif self.current_tool == 'bounds':
                if self.placing_bounds is None:
                    self.placing_bounds = (world_x, world_y)
                else:
                    start_x, start_y = self.placing_bounds
                    w = abs(world_x - start_x)
                    h = abs(world_y - start_y)
                    x = min(start_x, world_x)
                    y = min(start_y, world_y)
                    self.map_data.bounds = [x, y, w, h]
                    self.placing_bounds = None

        elif button == 3:  # 우클릭 - 삭제
            for i, cp in enumerate(self.map_data.checkpoints):
                if abs(cp.x - world_x) < 20/self.zoom and abs(cp.y - world_y) < 20/self.zoom:
                    del self.map_data.checkpoints[i]
                    return
            for i, trans in enumerate(self.map_data.transitions):
                if trans.x <= world_x <= trans.x + trans.w and trans.y <= world_y <= trans.y + trans.h:
                    del self.map_data.transitions[i]
                    return

    def _handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
                return

            # 툴바 버튼
            for btn in self.toolbar_buttons:
                btn.handle_event(event)

            # 레이어 탭
            for i, btn in enumerate(self.layer_tabs):
                btn.active = (i == self.selected_layer_index)
                btn.handle_event(event)

            # 사이드바 툴 버튼
            for tool_id, btn in self.sidebar_buttons:
                btn.handle_event(event)
                btn.active = (self.current_tool == tool_id)

            # 슬라이더
            self.parallax_slider.handle_event(event)
            self.scale_slider.handle_event(event)
            self.offset_x_slider.handle_event(event)
            self.offset_y_slider.handle_event(event)

            # 사이드바 버튼 클릭 처리
            self._handle_sidebar_clicks(event)

            # 캔버스 영역 체크
            canvas_rect = pygame.Rect(0, TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT, CANVAS_WIDTH, CANVAS_HEIGHT)

            # 마우스 휠 - 줌
            if event.type == pygame.MOUSEWHEEL:
                if canvas_rect.collidepoint(pygame.mouse.get_pos()):
                    zoom_factor = 1.1 if event.y > 0 else 0.9
                    new_zoom = self.zoom * zoom_factor
                    new_zoom = max(self.min_zoom, min(self.max_zoom, new_zoom))

                    # 마우스 위치 기준 줌
                    mouse_x, mouse_y = pygame.mouse.get_pos()
                    world_x, world_y = self._screen_to_world(mouse_x, mouse_y)

                    self.zoom = new_zoom

                    # 줌 후 마우스 위치가 같은 월드 좌표를 가리키도록 카메라 조정
                    new_world_x, new_world_y = self._screen_to_world(mouse_x, mouse_y)
                    self.camera_x += world_x - new_world_x
                    self.camera_y += world_y - new_world_y

                    # 줌 후 카메라 바운딩 적용
                    if self.camera_bound_enabled:
                        self._clamp_camera_to_bounds()

            # 캔버스 클릭
            if event.type == pygame.MOUSEBUTTONDOWN:
                if canvas_rect.collidepoint(event.pos):
                    self._handle_canvas_click(event.pos, event.button)

            # 키보드
            if event.type == pygame.KEYDOWN:
                if pygame.K_1 <= event.key <= pygame.K_9:
                    idx = event.key - pygame.K_1
                    if idx < len(self.map_data.layers):
                        self._select_layer(idx)

                if event.key == pygame.K_ESCAPE:
                    self.placing_transition = None
                    self.placing_bounds = None
                    self.current_tool = 'select'

                if event.key == pygame.K_DELETE:
                    self._remove_selected_layer()

        # 카메라 이동 (화살표 키)
        keys = pygame.key.get_pressed()
        move_speed = self.camera_speed / self.zoom
        if keys[pygame.K_LSHIFT] or keys[pygame.K_RSHIFT]:
            move_speed *= 3

        if keys[pygame.K_LEFT]:
            self.camera_x -= move_speed
        if keys[pygame.K_RIGHT]:
            self.camera_x += move_speed
        if keys[pygame.K_UP]:
            self.camera_y -= move_speed
        if keys[pygame.K_DOWN]:
            self.camera_y += move_speed

        # 카메라 바운딩 적용
        if self.camera_bound_enabled:
            self._clamp_camera_to_bounds()

    def _render_canvas(self):
        # 캔버스 배경
        canvas_rect = pygame.Rect(0, TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT, CANVAS_WIDTH, CANVAS_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_BG, canvas_rect)

        # 클리핑 영역 설정
        self.screen.set_clip(canvas_rect)

        # 그리드
        if self.show_grid:
            grid_size_scaled = self.grid_size * self.zoom
            if grid_size_scaled > 8:  # 너무 작으면 그리지 않음
                start_x = -((self.camera_x * self.zoom) % grid_size_scaled)
                start_y = TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT - ((self.camera_y * self.zoom) % grid_size_scaled)

                for x in range(int(start_x), CANVAS_WIDTH, max(1, int(grid_size_scaled))):
                    pygame.draw.line(self.screen, COLOR_GRID, (x, canvas_rect.top), (x, canvas_rect.bottom))
                for y in range(int(start_y), canvas_rect.bottom, max(1, int(grid_size_scaled))):
                    pygame.draw.line(self.screen, COLOR_GRID, (0, y), (CANVAS_WIDTH, y))

        # 레이어 렌더링 (Parallax + Scale + Offset 적용)
        for i, layer in enumerate(self.map_data.layers):
            if not layer.visible or not layer.image:
                continue

            # Parallax 적용된 오프셋 계산
            parallax_camera_x = self.camera_x * layer.parallax
            parallax_camera_y = self.camera_y * layer.parallax

            # 줌 + 레이어 스케일 적용
            total_scale = self.zoom * layer.scale
            scaled_image = pygame.transform.scale(
                layer.image,
                (int(layer.image.get_width() * total_scale),
                 int(layer.image.get_height() * total_scale))
            )

            # 오프셋 적용
            draw_x = (-parallax_camera_x + layer.offset_x) * self.zoom
            draw_y = TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT + (-parallax_camera_y + layer.offset_y) * self.zoom

            self.screen.blit(scaled_image, (draw_x, draw_y))

        # 메타 오버레이 (모든 레이어 위에, 선택된 레이어의 메타만)
        if self.show_meta_overlay:
            for layer in self.map_data.layers:
                if layer.meta_image:
                    parallax_camera_x = self.camera_x * layer.parallax
                    parallax_camera_y = self.camera_y * layer.parallax

                    # 메타 이미지도 레이어 스케일 적용
                    total_scale = self.zoom * layer.scale
                    scaled_meta = pygame.transform.scale(
                        layer.meta_image,
                        (int(layer.meta_image.get_width() * total_scale),
                         int(layer.meta_image.get_height() * total_scale))
                    )
                    scaled_meta.set_alpha(self.meta_overlay_alpha)

                    # 오프셋 적용
                    draw_x = (-parallax_camera_x + layer.offset_x) * self.zoom
                    draw_y = TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT + (-parallax_camera_y + layer.offset_y) * self.zoom

                    self.screen.blit(scaled_meta, (draw_x, draw_y))

        # 오브젝트들 렌더링 (월드 좌표 기준, parallax 1.0)
        # 플레이어 스폰
        spawn_x, spawn_y = self._world_to_screen(self.map_data.player_spawn[0], self.map_data.player_spawn[1])
        pygame.draw.circle(self.screen, (0, 220, 0), (int(spawn_x), int(spawn_y)), int(10 * self.zoom))
        pygame.draw.circle(self.screen, (255, 255, 255), (int(spawn_x), int(spawn_y)), int(10 * self.zoom), 2)
        spawn_text = self.font_small.render("SPAWN", True, (255, 255, 255))
        self.screen.blit(spawn_text, (spawn_x - 18, spawn_y - 22))

        # 체크포인트
        for cp in self.map_data.checkpoints:
            cp_x, cp_y = self._world_to_screen(cp.x, cp.y)
            size = int(12 * self.zoom)
            pygame.draw.rect(self.screen, (255, 200, 0), (cp_x - size//2, cp_y - size, size, size * 2), 2)
            cp_text = self.font_small.render(f"CP{cp.id}", True, (255, 200, 0))
            self.screen.blit(cp_text, (cp_x - 12, cp_y - size - 15))

        # 트랜지션
        for trans in self.map_data.transitions:
            tx, ty = self._world_to_screen(trans.x, trans.y)
            tw, th = trans.w * self.zoom, trans.h * self.zoom
            trans_rect = pygame.Rect(tx, ty, tw, th)
            pygame.draw.rect(self.screen, (255, 0, 255), trans_rect, 2)
            trans_text = self.font_small.render(f"→{trans.target}", True, (255, 0, 255))
            self.screen.blit(trans_text, (tx + 3, ty + 3))

        # 트랜지션 배치 중 미리보기
        if self.placing_transition:
            mouse_pos = pygame.mouse.get_pos()
            if canvas_rect.collidepoint(mouse_pos):
                start_x, start_y = self._world_to_screen(self.placing_transition[0], self.placing_transition[1])
                preview_rect = pygame.Rect(start_x, start_y, mouse_pos[0] - start_x, mouse_pos[1] - start_y)
                preview_rect.normalize()
                pygame.draw.rect(self.screen, (255, 0, 255), preview_rect, 1)

        # 카메라 바운드 렌더링
        bounds = self.map_data.bounds
        if bounds[2] > 0 and bounds[3] > 0:  # 너비/높이가 있으면
            bx, by = self._world_to_screen(bounds[0], bounds[1])
            bw, bh = bounds[2] * self.zoom, bounds[3] * self.zoom
            bounds_rect = pygame.Rect(bx, by, bw, bh)
            pygame.draw.rect(self.screen, (0, 200, 255), bounds_rect, 3)
            bounds_text = self.font_small.render("BOUNDS", True, (0, 200, 255))
            self.screen.blit(bounds_text, (bx + 5, by + 5))

        # 바운드 배치 중 미리보기
        if self.placing_bounds:
            mouse_pos = pygame.mouse.get_pos()
            if canvas_rect.collidepoint(mouse_pos):
                start_x, start_y = self._world_to_screen(self.placing_bounds[0], self.placing_bounds[1])
                preview_rect = pygame.Rect(start_x, start_y, mouse_pos[0] - start_x, mouse_pos[1] - start_y)
                preview_rect.normalize()
                pygame.draw.rect(self.screen, (0, 200, 255), preview_rect, 1)

        # 클리핑 해제
        self.screen.set_clip(None)

        # 캔버스 테두리
        pygame.draw.rect(self.screen, COLOR_ACCENT, canvas_rect, 1)

        # 마우스 좌표 표시 (뷰포트 우측 상단)
        mouse_pos = pygame.mouse.get_pos()
        if canvas_rect.collidepoint(mouse_pos):
            world_x, world_y = self._screen_to_world(mouse_pos[0], mouse_pos[1])
            coord_text = self.font_small.render(f"X: {int(world_x)}  Y: {int(world_y)}", True, COLOR_TEXT)
            coord_bg = pygame.Rect(CANVAS_WIDTH - 130, TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT + 5, 125, 20)
            pygame.draw.rect(self.screen, (30, 30, 35, 200), coord_bg, border_radius=3)
            self.screen.blit(coord_text, (CANVAS_WIDTH - 125, TOOLBAR_HEIGHT + LAYER_TAB_HEIGHT + 8))

    def _render_toolbar(self):
        toolbar_rect = pygame.Rect(0, 0, WINDOW_WIDTH, TOOLBAR_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_TOOLBAR, toolbar_rect)

        for btn in self.toolbar_buttons:
            btn.draw(self.screen, self.font)

    def _render_layer_tabs(self):
        tab_area = pygame.Rect(0, TOOLBAR_HEIGHT, CANVAS_WIDTH, LAYER_TAB_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_LAYER_TAB, tab_area)

        for i, btn in enumerate(self.layer_tabs):
            btn.active = (i == self.selected_layer_index)
            btn.draw(self.screen, self.font_small)

        # + 버튼 (끝에)
        if self.layer_tabs:
            last_btn = self.layer_tabs[-1]
            plus_x = last_btn.rect.right + 10
        else:
            plus_x = 10
        plus_text = self.font.render("+ Add Layer", True, COLOR_TEXT_DIM)
        self.screen.blit(plus_text, (plus_x, TOOLBAR_HEIGHT + 10))

    def _render_sidebar(self):
        sidebar_rect = pygame.Rect(CANVAS_WIDTH, 0, SIDEBAR_WIDTH, WINDOW_HEIGHT)
        pygame.draw.rect(self.screen, COLOR_SIDEBAR, sidebar_rect)

        x = CANVAS_WIDTH + 15
        y = 15

        # 툴 섹션
        title = self.font_large.render("Tools", True, COLOR_TEXT)
        self.screen.blit(title, (x, y))
        y += 25

        for tool_id, btn in self.sidebar_buttons:
            btn.draw(self.screen, self.font)
        y += 80

        # 선택된 레이어 정보
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        title = self.font_large.render("Layer Properties", True, COLOR_TEXT)
        self.screen.blit(title, (x, y))
        y += 30

        if 0 <= self.selected_layer_index < len(self.map_data.layers):
            layer = self.map_data.layers[self.selected_layer_index]

            # 레이어 이름
            name_text = self.font.render(f"Name: {layer.name}", True, COLOR_TEXT)
            self.screen.blit(name_text, (x, y))
            y += 22

            # 이미지 경로
            path_text = self.font_small.render(f"Image: {layer.image_path[:25]}...", True, COLOR_TEXT_DIM)
            self.screen.blit(path_text, (x, y))
            y += 20

            # Parallax 슬라이더
            self.parallax_slider.rect.y = y + 18
            self.parallax_slider.draw(self.screen, self.font)
            y += 50

            # Scale 슬라이더
            self.scale_slider.rect.y = y + 18
            self.scale_slider.draw(self.screen, self.font)
            y += 50

            # Offset X 슬라이더
            self.offset_x_slider.rect.y = y + 18
            self.offset_x_slider.draw(self.screen, self.font)
            y += 50

            # Offset Y 슬라이더
            self.offset_y_slider.rect.y = y + 18
            self.offset_y_slider.draw(self.screen, self.font)
            y += 50

            # 메타 이미지
            meta_label = self.font.render("Meta Image:", True, COLOR_TEXT)
            self.screen.blit(meta_label, (x, y))
            y += 22

            if layer.meta_path:
                meta_text = self.font_small.render(layer.meta_path[:30], True, COLOR_ACCENT)
                self.screen.blit(meta_text, (x, y))
                y += 20
                # Clear 버튼
                clear_btn_rect = pygame.Rect(x, y, 80, 24)
                self.sidebar_btn_rects['clear_meta'] = clear_btn_rect
                pygame.draw.rect(self.screen, COLOR_BUTTON, clear_btn_rect, border_radius=3)
                clear_text = self.font_small.render("Clear", True, COLOR_TEXT)
                self.screen.blit(clear_text, (x + 25, y + 4))

            # Set Meta 버튼 (Clear 옆 또는 None 옆)
            set_btn_rect = pygame.Rect(x + 90, y, 100, 24)
            self.sidebar_btn_rects['set_meta'] = set_btn_rect
            pygame.draw.rect(self.screen, COLOR_BUTTON, set_btn_rect, border_radius=3)
            set_text = self.font_small.render("Set Meta", True, COLOR_TEXT)
            self.screen.blit(set_text, (x + 108, y + 4))

            if not layer.meta_path:
                meta_text = self.font_small.render("None", True, COLOR_TEXT_DIM)
                self.screen.blit(meta_text, (x, y + 4))

            y += 35

            # Main Layer 설정
            main_label = self.font.render("Main Layer:", True, COLOR_TEXT)
            self.screen.blit(main_label, (x, y))
            y += 22

            # Set Main 버튼 (토글)
            main_btn_color = COLOR_BUTTON_ACTIVE if layer.is_main else COLOR_BUTTON
            main_btn_rect = pygame.Rect(x, y, 120, 24)
            self.sidebar_btn_rects['set_main'] = main_btn_rect
            pygame.draw.rect(self.screen, main_btn_color, main_btn_rect, border_radius=3)
            main_text = "★ MAIN" if layer.is_main else "Set as Main"
            main_text_surface = self.font_small.render(main_text, True, COLOR_TEXT)
            self.screen.blit(main_text_surface, (x + 25, y + 4))

            y += 35

            # 레이어 순서 버튼
            pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
            y += 15

            order_label = self.font.render("Layer Order:", True, COLOR_TEXT)
            self.screen.blit(order_label, (x, y))
            y += 25

            # Up/Down/Delete 버튼
            btn_w = 60
            up_rect = pygame.Rect(x, y, btn_w, 24)
            down_rect = pygame.Rect(x + btn_w + 5, y, btn_w, 24)
            del_rect = pygame.Rect(x + (btn_w + 5) * 2, y, btn_w + 20, 24)

            # rect 저장
            self.sidebar_btn_rects['up'] = up_rect
            self.sidebar_btn_rects['down'] = down_rect
            self.sidebar_btn_rects['delete'] = del_rect

            pygame.draw.rect(self.screen, COLOR_BUTTON, up_rect, border_radius=3)
            pygame.draw.rect(self.screen, COLOR_BUTTON, down_rect, border_radius=3)
            pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)

            self.screen.blit(self.font_small.render("Up", True, COLOR_TEXT), (x + 20, y + 4))
            self.screen.blit(self.font_small.render("Down", True, COLOR_TEXT), (x + btn_w + 15, y + 4))
            self.screen.blit(self.font_small.render("Delete", True, COLOR_TEXT), (x + (btn_w + 5) * 2 + 15, y + 4))

        else:
            self.sidebar_btn_rects = {}  # 레이어 미선택 시 버튼 rect 클리어
            no_layer_text = self.font.render("No layer selected", True, COLOR_TEXT_DIM)
            self.screen.blit(no_layer_text, (x, y))

        # 하단 정보
        y = WINDOW_HEIGHT - 80
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 10

        tool_text = self.font.render(f"Tool: {self.current_tool.upper()}", True, COLOR_ACCENT)
        self.screen.blit(tool_text, (x, y))
        y += 20

        cam_text = self.font_small.render(f"Camera: ({int(self.camera_x)}, {int(self.camera_y)})", True, COLOR_TEXT_DIM)
        self.screen.blit(cam_text, (x, y))
        y += 18

        zoom_text = self.font_small.render(f"Zoom: {self.zoom:.1f}x", True, COLOR_TEXT_DIM)
        self.screen.blit(zoom_text, (x + 130, y - 18))

        help_text = self.font_small.render("Arrows: Move | Wheel: Zoom | Del: Remove", True, COLOR_TEXT_DIM)
        self.screen.blit(help_text, (x, y))

    def _handle_sidebar_clicks(self, event):
        """사이드바 내 추가 클릭 처리 - 저장된 rect 사용"""
        if event.type != pygame.MOUSEBUTTONDOWN or event.button != 1:
            return

        if not self.sidebar_btn_rects:
            return

        # Set Meta 버튼
        if 'set_meta' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['set_meta'].collidepoint(event.pos):
                self._set_meta_image()
                return

        # Clear 버튼
        if 'clear_meta' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['clear_meta'].collidepoint(event.pos):
                self._clear_meta_image()
                return

        # Set Main 버튼
        if 'set_main' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['set_main'].collidepoint(event.pos):
                self._toggle_main_layer()
                return

        # Up 버튼
        if 'up' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['up'].collidepoint(event.pos):
                self._move_layer_up()
                return

        # Down 버튼
        if 'down' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['down'].collidepoint(event.pos):
                self._move_layer_down()
                return

        # Delete 버튼
        if 'delete' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['delete'].collidepoint(event.pos):
                self._remove_selected_layer()
                return

    def run(self):
        while self.running:
            self._handle_events()

            self.screen.fill(COLOR_BG)
            self._render_toolbar()
            self._render_layer_tabs()
            self._render_canvas()
            self._render_sidebar()

            pygame.display.flip()
            self.clock.tick(60)

        pygame.quit()

if __name__ == "__main__":
    editor = MapEditor()
    editor.run()
