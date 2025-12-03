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
class SpawnPoint:
    spawn_id: int
    x: float
    y: float

@dataclass
class ObjectPoint:
    name: str
    x: float
    y: float

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

# 기본 콜라이더 태그 (자주 사용되는 것들)
DEFAULT_COLLIDER_TAGS = ['Solid', 'OneWay', 'Ladder', 'WallClimb', 'Ledge', 'Damage', 'Platform']

# 태그별 기본 색상 (없으면 회색 사용)
TAG_COLORS = {
    'Solid': (255, 80, 80),
    'OneWay': (80, 255, 80),
    'Ladder': (80, 80, 255),
    'WallClimb': (255, 160, 80),
    'Ledge': (80, 255, 255),
    'Damage': (255, 255, 80),
    'Platform': (180, 80, 255),
}

def get_tag_color(tags: List[str]) -> tuple:
    """태그 리스트에서 첫 번째 매칭되는 색상 반환"""
    for tag in tags:
        if tag in TAG_COLORS:
            return TAG_COLORS[tag]
    return (150, 150, 150)  # 기본 회색

@dataclass
class BoxCollider:
    x: float
    y: float
    w: float
    h: float
    tags: List[str] = field(default_factory=lambda: ['Solid'])

@dataclass
class SlopeCollider:
    x1: float  # 시작점
    y1: float
    x2: float  # 끝점
    y2: float
    tags: List[str] = field(default_factory=lambda: ['Solid'])

@dataclass
class MapData:
    layers: List[Layer] = field(default_factory=list)
    spawn_points: List[SpawnPoint] = field(default_factory=lambda: [SpawnPoint(0, 100, 500)])
    bounds: List[float] = field(default_factory=lambda: [0, 0, 2000, 1080])
    checkpoints: List[Checkpoint] = field(default_factory=list)
    transitions: List[Transition] = field(default_factory=list)
    object_points: List[ObjectPoint] = field(default_factory=list)
    box_colliders: List[BoxCollider] = field(default_factory=list)
    slope_colliders: List[SlopeCollider] = field(default_factory=list)
    map_directory: str = ""

    def get_next_spawn_id(self) -> int:
        """사용 가능한 다음 spawnId 반환 (중복 방지)"""
        if not self.spawn_points:
            return 0
        used_ids = {sp.spawn_id for sp in self.spawn_points}
        next_id = 0
        while next_id in used_ids:
            next_id += 1
        return next_id

    def is_spawn_id_unique(self, spawn_id: int, exclude_spawn: Optional[SpawnPoint] = None) -> bool:
        """spawnId가 고유한지 확인"""
        for sp in self.spawn_points:
            if sp is exclude_spawn:
                continue
            if sp.spawn_id == spawn_id:
                return False
        return True

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
        self.placing_box = None  # 박스 콜라이더 배치 시작점
        self.placing_slope = None  # 슬로프 콜라이더 시작점

        # 콜라이더 태그 설정
        self.current_collider_tags = ['Solid']  # 현재 선택된 태그
        self.show_colliders = True  # 콜라이더 표시 여부

        # 선택된 오브젝트 추적
        self.selected_object = None  # (type, object) 튜플 - type: 'spawn', 'checkpoint', 'transition', 'box_collider', 'slope_collider'
        self.dragging_object = False
        self.drag_offset = (0, 0)

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
        x = CANVAS_WIDTH - 480
        self.btn_toggle_meta = Button(x, y, 90, btn_h, "Meta: ON", self._toggle_meta)
        self.toolbar_buttons.append(self.btn_toggle_meta)
        x += 95
        self.btn_toggle_grid = Button(x, y, 75, btn_h, "Grid: ON", self._toggle_grid)
        self.toolbar_buttons.append(self.btn_toggle_grid)
        x += 80
        self.btn_toggle_colliders = Button(x, y, 80, btn_h, "Col: ON", self._toggle_colliders)
        self.toolbar_buttons.append(self.btn_toggle_colliders)
        x += 85
        self.btn_toggle_cam_bound = Button(x, y, 100, btn_h, "Bound: OFF", self._toggle_cam_bound)
        self.toolbar_buttons.append(self.btn_toggle_cam_bound)

        # 사이드바 - 툴 버튼
        self.sidebar_buttons = []
        x = CANVAS_WIDTH + 15
        y = 20
        w = SIDEBAR_WIDTH - 30

        tools = [
            ('select', 'Select'), ('spawn', 'Spawn'),
            ('object', 'Object'), ('checkpoint', 'Checkpoint'),
            ('transition', 'Transition'), ('bounds', 'Bounds'),
            ('box_collider', 'Box Col'), ('slope_collider', 'Slope Col')
        ]
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
        self.selected_object = None  # 레이어 선택 시 오브젝트 선택 해제
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
        # 명시적으로 새 리스트들로 초기화
        self.map_data.layers = []
        self.map_data.spawn_points = [SpawnPoint(0, 100, 500)]
        self.map_data.checkpoints = []
        self.map_data.transitions = []
        self.map_data.object_points = []
        self.map_data.box_colliders = []
        self.map_data.slope_colliders = []
        self.map_data.bounds = [0, 0, 2000, 1080]
        self.map_data.map_directory = ""

        self.selected_layer_index = -1
        self.selected_object = None
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
            self.map_data.spawn_points = []  # 초기화
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

            # 스폰 포인트 로드 (새 형식 또는 기존 형식 지원)
            if 'spawnPoints' in data:
                # 새 형식: 여러 개의 스폰 포인트
                for sp_data in data['spawnPoints']:
                    sp = SpawnPoint(sp_data['spawnId'], sp_data['pos'][0], sp_data['pos'][1])
                    self.map_data.spawn_points.append(sp)
            elif 'playerSpawn' in data:
                # 기존 형식: 단일 스폰 포인트 -> spawnId 0으로 변환
                pos = data['playerSpawn']
                self.map_data.spawn_points.append(SpawnPoint(0, pos[0], pos[1]))
            else:
                # 기본값
                self.map_data.spawn_points.append(SpawnPoint(0, 100, 500))

            self.map_data.bounds = data.get('bounds', [0, 0, 2000, 1080])

            for cp_data in data.get('checkpoints', []):
                cp = Checkpoint(cp_data['id'], cp_data['pos'][0], cp_data['pos'][1])
                self.map_data.checkpoints.append(cp)

            for obj_data in data.get('objects', []):
                op = ObjectPoint(obj_data['name'], obj_data['pos'][0], obj_data['pos'][1])
                self.map_data.object_points.append(op)

            for trans_data in data.get('transitions', []):
                trans = Transition(
                    trans_data['rect'][0], trans_data['rect'][1],
                    trans_data['rect'][2], trans_data['rect'][3],
                    trans_data['target'], trans_data.get('spawnId', 0)
                )
                self.map_data.transitions.append(trans)

            # 박스 콜라이더 로드
            for box_data in data.get('boxColliders', []):
                rect = box_data['rect']
                tags = box_data.get('tags', ['Solid'])
                box = BoxCollider(rect[0], rect[1], rect[2], rect[3], tags)
                self.map_data.box_colliders.append(box)

            # 슬로프 콜라이더 로드
            for slope_data in data.get('slopeColliders', []):
                points = slope_data['points']
                tags = slope_data.get('tags', ['Solid'])
                slope = SlopeCollider(points[0], points[1], points[2], points[3], tags)
                self.map_data.slope_colliders.append(slope)

            self.selected_layer_index = 0 if self.map_data.layers else -1
            self.selected_object = None
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
            'spawnPoints': [],
            'bounds': self.map_data.bounds,
            'checkpoints': [],
            'objects': [],
            'transitions': [],
            'boxColliders': [],
            'slopeColliders': []
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

        for sp in sorted(self.map_data.spawn_points, key=lambda s: s.spawn_id):
            data['spawnPoints'].append({
                'spawnId': sp.spawn_id,
                'pos': [sp.x, sp.y]
            })

        for cp in sorted(self.map_data.checkpoints, key=lambda c: c.id):
            data['checkpoints'].append({
                'id': cp.id,
                'pos': [cp.x, cp.y]
            })

        for op in sorted(self.map_data.object_points, key=lambda o: o.name):
            data['objects'].append({
                'name': op.name,
                'pos': [op.x, op.y]
            })

        for trans in self.map_data.transitions:
            data['transitions'].append({
                'rect': [trans.x, trans.y, trans.w, trans.h],
                'target': trans.target,
                'spawnId': trans.spawn_id
            })

        for box in self.map_data.box_colliders:
            data['boxColliders'].append({
                'rect': [box.x, box.y, box.w, box.h],
                'tags': box.tags
            })

        for slope in self.map_data.slope_colliders:
            data['slopeColliders'].append({
                'points': [slope.x1, slope.y1, slope.x2, slope.y2],
                'tags': slope.tags
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

    def _toggle_colliders(self):
        self.show_colliders = not self.show_colliders
        self.btn_toggle_colliders.text = f"Col: {'ON' if self.show_colliders else 'OFF'}"

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
        self.placing_box = None
        self.placing_slope = None
        # 툴 변경 시 오브젝트 선택 해제
        if tool != 'select':
            self.selected_object = None

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

    def _find_object_at(self, world_x, world_y):
        """주어진 월드 좌표에서 오브젝트 찾기 (type, object) 반환"""
        hit_radius = 20 / self.zoom

        # 스폰 포인트 체크
        for sp in self.map_data.spawn_points:
            if abs(sp.x - world_x) < hit_radius and abs(sp.y - world_y) < hit_radius:
                return ('spawn', sp)

        # 오브젝트 포인트 체크
        for op in self.map_data.object_points:
            if abs(op.x - world_x) < hit_radius and abs(op.y - world_y) < hit_radius:
                return ('object', op)

        # 체크포인트 체크
        for cp in self.map_data.checkpoints:
            if abs(cp.x - world_x) < hit_radius and abs(cp.y - world_y) < hit_radius:
                return ('checkpoint', cp)

        # 트랜지션 체크
        for trans in self.map_data.transitions:
            if trans.x <= world_x <= trans.x + trans.w and trans.y <= world_y <= trans.y + trans.h:
                return ('transition', trans)

        # 박스 콜라이더 체크
        for box in self.map_data.box_colliders:
            if box.x <= world_x <= box.x + box.w and box.y <= world_y <= box.y + box.h:
                return ('box_collider', box)

        # 슬로프 콜라이더 체크 (선분 근처인지)
        for slope in self.map_data.slope_colliders:
            # 선분과의 거리 계산
            dist = self._point_to_line_distance(world_x, world_y, slope.x1, slope.y1, slope.x2, slope.y2)
            if dist < hit_radius:
                return ('slope_collider', slope)

        return None

    def _point_to_line_distance(self, px, py, x1, y1, x2, y2):
        """점에서 선분까지의 거리 계산"""
        dx = x2 - x1
        dy = y2 - y1
        if dx == 0 and dy == 0:
            return ((px - x1)**2 + (py - y1)**2)**0.5

        t = max(0, min(1, ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy)))
        proj_x = x1 + t * dx
        proj_y = y1 + t * dy
        return ((px - proj_x)**2 + (py - proj_y)**2)**0.5

    def _handle_canvas_click(self, pos, button):
        world_x, world_y = self._screen_to_world(pos[0], pos[1])

        if button == 1:  # 좌클릭
            if self.current_tool == 'select':
                # 오브젝트 선택
                found = self._find_object_at(world_x, world_y)
                if found:
                    self.selected_object = found
                    self.selected_layer_index = -1  # 레이어 선택 해제
                else:
                    self.selected_object = None

            elif self.current_tool == 'spawn':
                # 새 스폰 포인트 추가
                new_id = self.map_data.get_next_spawn_id()
                new_spawn = SpawnPoint(new_id, world_x, world_y)
                self.map_data.spawn_points.append(new_spawn)
                self.selected_object = ('spawn', new_spawn)
                self.selected_layer_index = -1

            elif self.current_tool == 'object':
                # 새 오브젝트 포인트 추가
                name = simpledialog.askstring("Object", "Object name:")
                if name:
                    new_obj = ObjectPoint(name, world_x, world_y)
                    self.map_data.object_points.append(new_obj)
                    self.selected_object = ('object', new_obj)
                    self.selected_layer_index = -1

            elif self.current_tool == 'checkpoint':
                new_id = len(self.map_data.checkpoints) + 1
                new_cp = Checkpoint(new_id, world_x, world_y)
                self.map_data.checkpoints.append(new_cp)
                self.selected_object = ('checkpoint', new_cp)
                self.selected_layer_index = -1

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
                        new_trans = Transition(x, y, w, h, target, spawn_id)
                        self.map_data.transitions.append(new_trans)
                        self.selected_object = ('transition', new_trans)
                        self.selected_layer_index = -1
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

            elif self.current_tool == 'box_collider':
                if self.placing_box is None:
                    self.placing_box = (world_x, world_y)
                else:
                    start_x, start_y = self.placing_box
                    w = abs(world_x - start_x)
                    h = abs(world_y - start_y)
                    x = min(start_x, world_x)
                    y = min(start_y, world_y)
                    if w > 5 and h > 5:  # 최소 크기 체크
                        new_box = BoxCollider(x, y, w, h, list(self.current_collider_tags))
                        self.map_data.box_colliders.append(new_box)
                        self.selected_object = ('box_collider', new_box)
                        self.selected_layer_index = -1
                    self.placing_box = None

            elif self.current_tool == 'slope_collider':
                if self.placing_slope is None:
                    self.placing_slope = (world_x, world_y)
                else:
                    start_x, start_y = self.placing_slope
                    # 거리 체크
                    dist = ((world_x - start_x)**2 + (world_y - start_y)**2)**0.5
                    if dist > 10:  # 최소 길이 체크
                        new_slope = SlopeCollider(start_x, start_y, world_x, world_y, list(self.current_collider_tags))
                        self.map_data.slope_colliders.append(new_slope)
                        self.selected_object = ('slope_collider', new_slope)
                        self.selected_layer_index = -1
                    self.placing_slope = None

        elif button == 3:  # 우클릭 - 빈 곳 클릭시 선택 해제
            if self.current_tool == 'select':
                self.selected_object = None

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
                    # 드래그 시작 (select 모드에서 선택된 오브젝트가 있을 때)
                    if event.button == 1 and self.current_tool == 'select' and self.selected_object:
                        world_x, world_y = self._screen_to_world(event.pos[0], event.pos[1])
                        obj_type, obj = self.selected_object
                        # 선택된 오브젝트 위에서 클릭했는지 확인
                        hit = self._find_object_at(world_x, world_y)
                        if hit and hit[1] is obj:
                            self.dragging_object = True
                            if obj_type in ('spawn', 'object', 'checkpoint'):
                                self.drag_offset = (world_x - obj.x, world_y - obj.y)
                            elif obj_type == 'transition':
                                self.drag_offset = (world_x - obj.x, world_y - obj.y)

            # 마우스 버튼 업 - 드래그 종료
            if event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1:
                    self.dragging_object = False

            # 마우스 이동 - 드래그 중 오브젝트 이동
            if event.type == pygame.MOUSEMOTION:
                if self.dragging_object and self.selected_object and canvas_rect.collidepoint(event.pos):
                    world_x, world_y = self._screen_to_world(event.pos[0], event.pos[1])
                    obj_type, obj = self.selected_object
                    if obj_type in ('spawn', 'object', 'checkpoint'):
                        obj.x = world_x - self.drag_offset[0]
                        obj.y = world_y - self.drag_offset[1]
                    elif obj_type == 'transition':
                        obj.x = world_x - self.drag_offset[0]
                        obj.y = world_y - self.drag_offset[1]

            # 키보드
            if event.type == pygame.KEYDOWN:
                if pygame.K_1 <= event.key <= pygame.K_9:
                    idx = event.key - pygame.K_1
                    if idx < len(self.map_data.layers):
                        self._select_layer(idx)

                if event.key == pygame.K_ESCAPE:
                    self.placing_transition = None
                    self.placing_bounds = None
                    self.placing_box = None
                    self.placing_slope = None
                    self.current_tool = 'select'

                if event.key == pygame.K_DELETE:
                    # 오브젝트가 선택되어 있으면 오브젝트 삭제, 아니면 레이어 삭제
                    if self.selected_object:
                        self._delete_selected_object()
                    else:
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
        # 스폰 포인트들
        for sp in self.map_data.spawn_points:
            spawn_x, spawn_y = self._world_to_screen(sp.x, sp.y)
            is_selected = self.selected_object and self.selected_object[0] == 'spawn' and self.selected_object[1] is sp

            # 선택 표시 (외곽 글로우)
            if is_selected:
                pygame.draw.circle(self.screen, (255, 255, 100), (int(spawn_x), int(spawn_y)), int(16 * self.zoom), 3)

            pygame.draw.circle(self.screen, (0, 220, 0), (int(spawn_x), int(spawn_y)), int(10 * self.zoom))
            border_color = (255, 255, 100) if is_selected else (255, 255, 255)
            pygame.draw.circle(self.screen, border_color, (int(spawn_x), int(spawn_y)), int(10 * self.zoom), 2)
            spawn_text = self.font_small.render(f"SP:{sp.spawn_id}", True, (255, 255, 255))
            self.screen.blit(spawn_text, (spawn_x - 15, spawn_y - 22))

        # 오브젝트 포인트들
        for op in self.map_data.object_points:
            obj_x, obj_y = self._world_to_screen(op.x, op.y)
            is_selected = self.selected_object and self.selected_object[0] == 'object' and self.selected_object[1] is op

            # 선택 표시
            if is_selected:
                pygame.draw.circle(self.screen, (255, 255, 100), (int(obj_x), int(obj_y)), int(16 * self.zoom), 3)

            # 오렌지색 다이아몬드 모양
            size = int(8 * self.zoom)
            points = [(obj_x, obj_y - size), (obj_x + size, obj_y), (obj_x, obj_y + size), (obj_x - size, obj_y)]
            pygame.draw.polygon(self.screen, (255, 140, 0), points)
            border_color = (255, 255, 100) if is_selected else (255, 255, 255)
            pygame.draw.polygon(self.screen, border_color, points, 2)
            obj_text = self.font_small.render(op.name, True, (255, 200, 100))
            self.screen.blit(obj_text, (obj_x - len(op.name) * 3, obj_y - 22))

        # 체크포인트
        for cp in self.map_data.checkpoints:
            cp_x, cp_y = self._world_to_screen(cp.x, cp.y)
            size = int(12 * self.zoom)
            is_selected = self.selected_object and self.selected_object[0] == 'checkpoint' and self.selected_object[1] is cp

            # 선택 표시
            if is_selected:
                pygame.draw.rect(self.screen, (255, 255, 100), (cp_x - size//2 - 4, cp_y - size - 4, size + 8, size * 2 + 8), 3)

            border_color = (255, 255, 100) if is_selected else (255, 200, 0)
            pygame.draw.rect(self.screen, border_color, (cp_x - size//2, cp_y - size, size, size * 2), 2)
            cp_text = self.font_small.render(f"CP{cp.id}", True, (255, 200, 0))
            self.screen.blit(cp_text, (cp_x - 12, cp_y - size - 15))

        # 트랜지션
        for trans in self.map_data.transitions:
            tx, ty = self._world_to_screen(trans.x, trans.y)
            tw, th = trans.w * self.zoom, trans.h * self.zoom
            trans_rect = pygame.Rect(tx, ty, tw, th)
            is_selected = self.selected_object and self.selected_object[0] == 'transition' and self.selected_object[1] is trans

            # 선택 표시
            if is_selected:
                select_rect = pygame.Rect(tx - 3, ty - 3, tw + 6, th + 6)
                pygame.draw.rect(self.screen, (255, 255, 100), select_rect, 3)

            border_color = (255, 255, 100) if is_selected else (255, 0, 255)
            pygame.draw.rect(self.screen, border_color, trans_rect, 2)
            trans_text = self.font_small.render(f"→{trans.target}[{trans.spawn_id}]", True, (255, 0, 255))
            self.screen.blit(trans_text, (tx + 3, ty + 3))

        # 트랜지션 배치 중 미리보기
        if self.placing_transition:
            mouse_pos = pygame.mouse.get_pos()
            if canvas_rect.collidepoint(mouse_pos):
                start_x, start_y = self._world_to_screen(self.placing_transition[0], self.placing_transition[1])
                preview_rect = pygame.Rect(start_x, start_y, mouse_pos[0] - start_x, mouse_pos[1] - start_y)
                preview_rect.normalize()
                pygame.draw.rect(self.screen, (255, 0, 255), preview_rect, 1)

        # 콜라이더 렌더링
        if self.show_colliders:
            # 박스 콜라이더
            for box in self.map_data.box_colliders:
                bx, by = self._world_to_screen(box.x, box.y)
                bw, bh = box.w * self.zoom, box.h * self.zoom
                box_rect = pygame.Rect(bx, by, bw, bh)
                is_selected = self.selected_object and self.selected_object[0] == 'box_collider' and self.selected_object[1] is box

                # 반투명 채우기
                color = get_tag_color(box.tags)
                fill_surface = pygame.Surface((int(bw), int(bh)), pygame.SRCALPHA)
                fill_surface.fill((*color, 80))
                self.screen.blit(fill_surface, (bx, by))

                # 테두리
                border_color = (255, 255, 100) if is_selected else color
                border_width = 3 if is_selected else 2
                pygame.draw.rect(self.screen, border_color, box_rect, border_width)

                # 태그 표시
                tag_text = ','.join(box.tags[:2])  # 처음 2개 태그만 표시
                if len(box.tags) > 2:
                    tag_text += '...'
                text_surface = self.font_small.render(tag_text, True, color)
                self.screen.blit(text_surface, (bx + 3, by + 3))

            # 슬로프 콜라이더
            for slope in self.map_data.slope_colliders:
                sx1, sy1 = self._world_to_screen(slope.x1, slope.y1)
                sx2, sy2 = self._world_to_screen(slope.x2, slope.y2)
                is_selected = self.selected_object and self.selected_object[0] == 'slope_collider' and self.selected_object[1] is slope

                color = get_tag_color(slope.tags)
                line_color = (255, 255, 100) if is_selected else color
                line_width = 4 if is_selected else 3
                pygame.draw.line(self.screen, line_color, (sx1, sy1), (sx2, sy2), line_width)

                # 끝점 표시
                pygame.draw.circle(self.screen, color, (int(sx1), int(sy1)), 5)
                pygame.draw.circle(self.screen, color, (int(sx2), int(sy2)), 5)

                # 태그 표시
                mid_x, mid_y = (sx1 + sx2) / 2, (sy1 + sy2) / 2
                tag_text = ','.join(slope.tags[:2])
                text_surface = self.font_small.render(tag_text, True, color)
                self.screen.blit(text_surface, (mid_x - 20, mid_y - 15))

        # 박스 콜라이더 배치 중 미리보기
        if self.placing_box:
            mouse_pos = pygame.mouse.get_pos()
            if canvas_rect.collidepoint(mouse_pos):
                start_x, start_y = self._world_to_screen(self.placing_box[0], self.placing_box[1])
                preview_rect = pygame.Rect(start_x, start_y, mouse_pos[0] - start_x, mouse_pos[1] - start_y)
                preview_rect.normalize()
                color = get_tag_color(self.current_collider_tags)
                pygame.draw.rect(self.screen, color, preview_rect, 2)

        # 슬로프 콜라이더 배치 중 미리보기
        if self.placing_slope:
            mouse_pos = pygame.mouse.get_pos()
            if canvas_rect.collidepoint(mouse_pos):
                start_x, start_y = self._world_to_screen(self.placing_slope[0], self.placing_slope[1])
                color = get_tag_color(self.current_collider_tags)
                pygame.draw.line(self.screen, color, (start_x, start_y), mouse_pos, 2)
                pygame.draw.circle(self.screen, color, (int(start_x), int(start_y)), 5)

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

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 오브젝트가 선택되어 있으면 오브젝트 프로퍼티 표시
        if self.selected_object:
            y = self._render_object_properties(x, y)
        # 레이어가 선택되어 있으면 레이어 프로퍼티 표시
        elif 0 <= self.selected_layer_index < len(self.map_data.layers):
            y = self._render_layer_properties(x, y)
        else:
            self.sidebar_btn_rects = {}
            no_sel_text = self.font.render("No selection", True, COLOR_TEXT_DIM)
            self.screen.blit(no_sel_text, (x, y))

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

    def _render_object_properties(self, x, y):
        """선택된 오브젝트의 프로퍼티 렌더링"""
        self.sidebar_btn_rects = {}
        obj_type, obj = self.selected_object

        if obj_type == 'spawn':
            return self._render_spawn_properties(x, y, obj)
        elif obj_type == 'object':
            return self._render_objectpoint_properties(x, y, obj)
        elif obj_type == 'checkpoint':
            return self._render_checkpoint_properties(x, y, obj)
        elif obj_type == 'transition':
            return self._render_transition_properties(x, y, obj)
        elif obj_type == 'box_collider':
            return self._render_box_collider_properties(x, y, obj)
        elif obj_type == 'slope_collider':
            return self._render_slope_collider_properties(x, y, obj)
        return y

    def _render_spawn_properties(self, x, y, spawn: SpawnPoint):
        """스폰 포인트 프로퍼티 렌더링"""
        title = self.font_large.render("Spawn Point", True, (0, 220, 0))
        self.screen.blit(title, (x, y))
        y += 30

        # Spawn ID
        id_text = self.font.render(f"Spawn ID: {spawn.spawn_id}", True, COLOR_TEXT)
        self.screen.blit(id_text, (x, y))
        y += 25

        # ID 변경 버튼
        change_id_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['change_spawn_id'] = change_id_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, change_id_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Change ID", True, COLOR_TEXT), (x + 28, y + 4))
        y += 35

        # 위치
        pos_text = self.font.render(f"Position:", True, COLOR_TEXT)
        self.screen.blit(pos_text, (x, y))
        y += 22

        x_text = self.font_small.render(f"X: {int(spawn.x)}", True, COLOR_TEXT_DIM)
        y_text = self.font_small.render(f"Y: {int(spawn.y)}", True, COLOR_TEXT_DIM)
        self.screen.blit(x_text, (x, y))
        self.screen.blit(y_text, (x + 80, y))
        y += 22

        # 좌표 수정 버튼
        edit_pos_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_pos'] = edit_pos_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_pos_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Position", True, COLOR_TEXT), (x + 22, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Spawn Point", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_objectpoint_properties(self, x, y, obj: ObjectPoint):
        """오브젝트 포인트 프로퍼티 렌더링"""
        title = self.font_large.render("Object Point", True, (255, 140, 0))
        self.screen.blit(title, (x, y))
        y += 30

        # Object Name
        name_text = self.font.render(f"Name: {obj.name}", True, COLOR_TEXT)
        self.screen.blit(name_text, (x, y))
        y += 25

        # Name 변경 버튼
        change_name_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['change_obj_name'] = change_name_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, change_name_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Change Name", True, COLOR_TEXT), (x + 18, y + 4))
        y += 35

        # 위치
        pos_text = self.font.render(f"Position:", True, COLOR_TEXT)
        self.screen.blit(pos_text, (x, y))
        y += 22

        x_text = self.font_small.render(f"X: {int(obj.x)}", True, COLOR_TEXT_DIM)
        y_text = self.font_small.render(f"Y: {int(obj.y)}", True, COLOR_TEXT_DIM)
        self.screen.blit(x_text, (x, y))
        self.screen.blit(y_text, (x + 80, y))
        y += 22

        # 좌표 수정 버튼
        edit_pos_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_pos'] = edit_pos_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_pos_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Position", True, COLOR_TEXT), (x + 22, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Object", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_checkpoint_properties(self, x, y, checkpoint: Checkpoint):
        """체크포인트 프로퍼티 렌더링"""
        title = self.font_large.render("Checkpoint", True, (255, 200, 0))
        self.screen.blit(title, (x, y))
        y += 30

        # Checkpoint ID
        id_text = self.font.render(f"Checkpoint ID: {checkpoint.id}", True, COLOR_TEXT)
        self.screen.blit(id_text, (x, y))
        y += 25

        # ID 변경 버튼
        change_id_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['change_cp_id'] = change_id_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, change_id_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Change ID", True, COLOR_TEXT), (x + 28, y + 4))
        y += 35

        # 위치
        pos_text = self.font.render(f"Position:", True, COLOR_TEXT)
        self.screen.blit(pos_text, (x, y))
        y += 22

        x_text = self.font_small.render(f"X: {int(checkpoint.x)}", True, COLOR_TEXT_DIM)
        y_text = self.font_small.render(f"Y: {int(checkpoint.y)}", True, COLOR_TEXT_DIM)
        self.screen.blit(x_text, (x, y))
        self.screen.blit(y_text, (x + 80, y))
        y += 22

        # 좌표 수정 버튼
        edit_pos_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_pos'] = edit_pos_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_pos_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Position", True, COLOR_TEXT), (x + 22, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Checkpoint", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_transition_properties(self, x, y, trans: Transition):
        """트랜지션 프로퍼티 렌더링"""
        title = self.font_large.render("Transition", True, (255, 0, 255))
        self.screen.blit(title, (x, y))
        y += 30

        # Target Scene
        target_label = self.font.render("Target Scene:", True, COLOR_TEXT)
        self.screen.blit(target_label, (x, y))
        y += 22

        target_text = self.font_small.render(trans.target, True, COLOR_ACCENT)
        self.screen.blit(target_text, (x, y))
        y += 25

        # Target 변경 버튼
        change_target_rect = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['change_target'] = change_target_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, change_target_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Change Target", True, COLOR_TEXT), (x + 18, y + 4))
        y += 35

        # Spawn ID
        spawn_label = self.font.render(f"Spawn ID: {trans.spawn_id}", True, COLOR_TEXT)
        self.screen.blit(spawn_label, (x, y))
        y += 25

        # Spawn ID 변경 버튼
        change_spawn_rect = pygame.Rect(x, y, 140, 24)
        self.sidebar_btn_rects['change_trans_spawn_id'] = change_spawn_rect
        pygame.draw.rect(self.screen, COLOR_BUTTON, change_spawn_rect, border_radius=3)
        self.screen.blit(self.font_small.render("Change Spawn ID", True, COLOR_TEXT), (x + 18, y + 4))
        y += 35

        # Rect 정보
        rect_label = self.font.render("Rect:", True, COLOR_TEXT)
        self.screen.blit(rect_label, (x, y))
        y += 22

        rect_info = self.font_small.render(f"X:{int(trans.x)} Y:{int(trans.y)}", True, COLOR_TEXT_DIM)
        self.screen.blit(rect_info, (x, y))
        y += 18
        size_info = self.font_small.render(f"W:{int(trans.w)} H:{int(trans.h)}", True, COLOR_TEXT_DIM)
        self.screen.blit(size_info, (x, y))
        y += 22

        # Rect 수정 버튼
        edit_rect_btn = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_rect'] = edit_rect_btn
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_rect_btn, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Rect", True, COLOR_TEXT), (x + 32, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Transition", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_box_collider_properties(self, x, y, box: BoxCollider):
        """박스 콜라이더 프로퍼티 렌더링"""
        color = get_tag_color(box.tags)
        title = self.font_large.render("Box Collider", True, color)
        self.screen.blit(title, (x, y))
        y += 30

        # Rect 정보
        rect_label = self.font.render("Rect:", True, COLOR_TEXT)
        self.screen.blit(rect_label, (x, y))
        y += 22

        rect_info = self.font_small.render(f"X:{int(box.x)} Y:{int(box.y)}", True, COLOR_TEXT_DIM)
        self.screen.blit(rect_info, (x, y))
        y += 18
        size_info = self.font_small.render(f"W:{int(box.w)} H:{int(box.h)}", True, COLOR_TEXT_DIM)
        self.screen.blit(size_info, (x, y))
        y += 25

        # Rect 수정 버튼
        edit_rect_btn = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_box_rect'] = edit_rect_btn
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_rect_btn, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Rect", True, COLOR_TEXT), (x + 32, y + 4))
        y += 35

        # 태그 섹션
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        tags_label = self.font.render("Tags:", True, COLOR_TEXT)
        self.screen.blit(tags_label, (x, y))
        y += 25

        # 현재 태그 표시
        for i, tag in enumerate(box.tags):
            tag_color = TAG_COLORS.get(tag, (150, 150, 150))
            tag_rect = pygame.Rect(x + (i % 3) * 85, y + (i // 3) * 25, 80, 22)
            pygame.draw.rect(self.screen, tag_color, tag_rect, border_radius=3)
            tag_text = self.font_small.render(tag[:8], True, (255, 255, 255))
            text_rect = tag_text.get_rect(center=tag_rect.center)
            self.screen.blit(tag_text, text_rect)
        y += ((len(box.tags) - 1) // 3 + 1) * 25 + 10

        # 태그 편집 버튼
        edit_tags_btn = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_tags'] = edit_tags_btn
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_tags_btn, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Tags", True, COLOR_TEXT), (x + 30, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Collider", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_slope_collider_properties(self, x, y, slope: SlopeCollider):
        """슬로프 콜라이더 프로퍼티 렌더링"""
        color = get_tag_color(slope.tags)
        title = self.font_large.render("Slope Collider", True, color)
        self.screen.blit(title, (x, y))
        y += 30

        # 좌표 정보
        pos_label = self.font.render("Start Point:", True, COLOR_TEXT)
        self.screen.blit(pos_label, (x, y))
        y += 22

        start_info = self.font_small.render(f"X:{int(slope.x1)} Y:{int(slope.y1)}", True, COLOR_TEXT_DIM)
        self.screen.blit(start_info, (x, y))
        y += 22

        pos_label2 = self.font.render("End Point:", True, COLOR_TEXT)
        self.screen.blit(pos_label2, (x, y))
        y += 22

        end_info = self.font_small.render(f"X:{int(slope.x2)} Y:{int(slope.y2)}", True, COLOR_TEXT_DIM)
        self.screen.blit(end_info, (x, y))
        y += 25

        # 좌표 수정 버튼
        edit_pos_btn = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_slope_pos'] = edit_pos_btn
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_pos_btn, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Points", True, COLOR_TEXT), (x + 28, y + 4))
        y += 35

        # 태그 섹션
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        tags_label = self.font.render("Tags:", True, COLOR_TEXT)
        self.screen.blit(tags_label, (x, y))
        y += 25

        # 현재 태그 표시
        for i, tag in enumerate(slope.tags):
            tag_color = TAG_COLORS.get(tag, (150, 150, 150))
            tag_rect = pygame.Rect(x + (i % 3) * 85, y + (i // 3) * 25, 80, 22)
            pygame.draw.rect(self.screen, tag_color, tag_rect, border_radius=3)
            tag_text = self.font_small.render(tag[:8], True, (255, 255, 255))
            text_rect = tag_text.get_rect(center=tag_rect.center)
            self.screen.blit(tag_text, text_rect)
        y += ((len(slope.tags) - 1) // 3 + 1) * 25 + 10

        # 태그 편집 버튼
        edit_tags_btn = pygame.Rect(x, y, 120, 24)
        self.sidebar_btn_rects['edit_tags'] = edit_tags_btn
        pygame.draw.rect(self.screen, COLOR_BUTTON, edit_tags_btn, border_radius=3)
        self.screen.blit(self.font_small.render("Edit Tags", True, COLOR_TEXT), (x + 30, y + 4))
        y += 35

        # 구분선
        pygame.draw.line(self.screen, COLOR_TEXT_DIM, (x, y), (x + SIDEBAR_WIDTH - 30, y))
        y += 15

        # 삭제 버튼
        del_rect = pygame.Rect(x, y, SIDEBAR_WIDTH - 30, 28)
        self.sidebar_btn_rects['delete_object'] = del_rect
        pygame.draw.rect(self.screen, (100, 60, 60), del_rect, border_radius=3)
        del_text = self.font.render("Delete Collider", True, COLOR_TEXT)
        text_rect = del_text.get_rect(center=del_rect.center)
        self.screen.blit(del_text, text_rect)
        y += 40

        return y

    def _render_layer_properties(self, x, y):
        """레이어 프로퍼티 렌더링"""
        title = self.font_large.render("Layer Properties", True, COLOR_TEXT)
        self.screen.blit(title, (x, y))
        y += 30

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

        return y + 30

    def _handle_sidebar_clicks(self, event):
        """사이드바 내 추가 클릭 처리 - 저장된 rect 사용"""
        if event.type != pygame.MOUSEBUTTONDOWN or event.button != 1:
            return

        if not self.sidebar_btn_rects:
            return

        # 오브젝트 삭제 버튼
        if 'delete_object' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['delete_object'].collidepoint(event.pos):
                self._delete_selected_object()
                return

        # 좌표 수정 버튼
        if 'edit_pos' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['edit_pos'].collidepoint(event.pos):
                self._edit_object_position()
                return

        # Rect 수정 버튼 (Transition용)
        if 'edit_rect' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['edit_rect'].collidepoint(event.pos):
                self._edit_transition_rect()
                return

        # 스폰 포인트 ID 변경 버튼
        if 'change_spawn_id' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['change_spawn_id'].collidepoint(event.pos):
                self._change_spawn_id()
                return

        # 오브젝트 이름 변경 버튼
        if 'change_obj_name' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['change_obj_name'].collidepoint(event.pos):
                self._change_object_name()
                return

        # 체크포인트 ID 변경 버튼
        if 'change_cp_id' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['change_cp_id'].collidepoint(event.pos):
                self._change_checkpoint_id()
                return

        # 트랜지션 타겟 변경 버튼
        if 'change_target' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['change_target'].collidepoint(event.pos):
                self._change_transition_target()
                return

        # 트랜지션 스폰 ID 변경 버튼
        if 'change_trans_spawn_id' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['change_trans_spawn_id'].collidepoint(event.pos):
                self._change_transition_spawn_id()
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

        # 박스 콜라이더 Rect 수정 버튼
        if 'edit_box_rect' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['edit_box_rect'].collidepoint(event.pos):
                self._edit_box_collider_rect()
                return

        # 슬로프 콜라이더 좌표 수정 버튼
        if 'edit_slope_pos' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['edit_slope_pos'].collidepoint(event.pos):
                self._edit_slope_collider_points()
                return

        # 태그 편집 버튼
        if 'edit_tags' in self.sidebar_btn_rects:
            if self.sidebar_btn_rects['edit_tags'].collidepoint(event.pos):
                self._edit_collider_tags()
                return

    def _edit_object_position(self):
        """선택된 오브젝트의 좌표 수정"""
        if not self.selected_object:
            return

        obj_type, obj = self.selected_object

        if obj_type in ('spawn', 'object', 'checkpoint'):
            new_x = simpledialog.askinteger("Position", "Enter X coordinate:",
                                            initialvalue=int(obj.x))
            if new_x is not None:
                new_y = simpledialog.askinteger("Position", "Enter Y coordinate:",
                                                initialvalue=int(obj.y))
                if new_y is not None:
                    obj.x = float(new_x)
                    obj.y = float(new_y)
        elif obj_type == 'transition':
            new_x = simpledialog.askinteger("Position", "Enter X coordinate:",
                                            initialvalue=int(obj.x))
            if new_x is not None:
                new_y = simpledialog.askinteger("Position", "Enter Y coordinate:",
                                                initialvalue=int(obj.y))
                if new_y is not None:
                    obj.x = float(new_x)
                    obj.y = float(new_y)

    def _edit_transition_rect(self):
        """트랜지션 Rect (위치+크기) 수정"""
        if not self.selected_object or self.selected_object[0] != 'transition':
            return

        trans = self.selected_object[1]

        new_x = simpledialog.askinteger("Rect", "Enter X:", initialvalue=int(trans.x))
        if new_x is None:
            return
        new_y = simpledialog.askinteger("Rect", "Enter Y:", initialvalue=int(trans.y))
        if new_y is None:
            return
        new_w = simpledialog.askinteger("Rect", "Enter Width:", initialvalue=int(trans.w), minvalue=1)
        if new_w is None:
            return
        new_h = simpledialog.askinteger("Rect", "Enter Height:", initialvalue=int(trans.h), minvalue=1)
        if new_h is None:
            return

        trans.x = float(new_x)
        trans.y = float(new_y)
        trans.w = float(new_w)
        trans.h = float(new_h)

    def _delete_selected_object(self):
        """선택된 오브젝트 삭제"""
        if not self.selected_object:
            return

        obj_type, obj = self.selected_object

        if obj_type == 'spawn':
            if obj in self.map_data.spawn_points:
                # 최소 하나의 스폰 포인트는 유지
                if len(self.map_data.spawn_points) > 1:
                    self.map_data.spawn_points.remove(obj)
                    self.selected_object = None
        elif obj_type == 'object':
            if obj in self.map_data.object_points:
                self.map_data.object_points.remove(obj)
                self.selected_object = None
        elif obj_type == 'checkpoint':
            if obj in self.map_data.checkpoints:
                self.map_data.checkpoints.remove(obj)
                self.selected_object = None
        elif obj_type == 'transition':
            if obj in self.map_data.transitions:
                self.map_data.transitions.remove(obj)
                self.selected_object = None
        elif obj_type == 'box_collider':
            if obj in self.map_data.box_colliders:
                self.map_data.box_colliders.remove(obj)
                self.selected_object = None
        elif obj_type == 'slope_collider':
            if obj in self.map_data.slope_colliders:
                self.map_data.slope_colliders.remove(obj)
                self.selected_object = None

    def _change_spawn_id(self):
        """스폰 포인트 ID 변경 (중복 방지)"""
        if not self.selected_object or self.selected_object[0] != 'spawn':
            return

        spawn = self.selected_object[1]
        new_id = simpledialog.askinteger("Spawn ID", "Enter new Spawn ID:",
                                         initialvalue=spawn.spawn_id, minvalue=0)
        if new_id is not None:
            # 중복 체크
            if self.map_data.is_spawn_id_unique(new_id, spawn):
                spawn.spawn_id = new_id
            else:
                # 중복된 ID가 있으면 자동으로 사용 가능한 ID 제안
                available_id = self.map_data.get_next_spawn_id()
                use_available = simpledialog.askstring(
                    "ID Conflict",
                    f"ID {new_id} already exists. Use {available_id} instead? (yes/no)",
                    initialvalue="yes"
                )
                if use_available and use_available.lower() in ('yes', 'y'):
                    spawn.spawn_id = available_id

    def _change_object_name(self):
        """오브젝트 이름 변경"""
        if not self.selected_object or self.selected_object[0] != 'object':
            return

        obj = self.selected_object[1]
        new_name = simpledialog.askstring("Object Name", "Enter new object name:",
                                          initialvalue=obj.name)
        if new_name:
            obj.name = new_name

    def _change_checkpoint_id(self):
        """체크포인트 ID 변경"""
        if not self.selected_object or self.selected_object[0] != 'checkpoint':
            return

        checkpoint = self.selected_object[1]
        new_id = simpledialog.askinteger("Checkpoint ID", "Enter new Checkpoint ID:",
                                         initialvalue=checkpoint.id, minvalue=1)
        if new_id is not None:
            checkpoint.id = new_id

    def _change_transition_target(self):
        """트랜지션 타겟 씬 변경"""
        if not self.selected_object or self.selected_object[0] != 'transition':
            return

        trans = self.selected_object[1]
        new_target = simpledialog.askstring("Target Scene", "Enter target scene name:",
                                            initialvalue=trans.target)
        if new_target:
            trans.target = new_target

    def _change_transition_spawn_id(self):
        """트랜지션의 스폰 ID 변경"""
        if not self.selected_object or self.selected_object[0] != 'transition':
            return

        trans = self.selected_object[1]
        new_id = simpledialog.askinteger("Spawn ID", "Enter target spawn ID:",
                                         initialvalue=trans.spawn_id, minvalue=0)
        if new_id is not None:
            trans.spawn_id = new_id

    def _edit_box_collider_rect(self):
        """박스 콜라이더 Rect 수정"""
        if not self.selected_object or self.selected_object[0] != 'box_collider':
            return

        box = self.selected_object[1]

        new_x = simpledialog.askinteger("Rect", "Enter X:", initialvalue=int(box.x))
        if new_x is None:
            return
        new_y = simpledialog.askinteger("Rect", "Enter Y:", initialvalue=int(box.y))
        if new_y is None:
            return
        new_w = simpledialog.askinteger("Rect", "Enter Width:", initialvalue=int(box.w), minvalue=1)
        if new_w is None:
            return
        new_h = simpledialog.askinteger("Rect", "Enter Height:", initialvalue=int(box.h), minvalue=1)
        if new_h is None:
            return

        box.x = float(new_x)
        box.y = float(new_y)
        box.w = float(new_w)
        box.h = float(new_h)

    def _edit_slope_collider_points(self):
        """슬로프 콜라이더 좌표 수정"""
        if not self.selected_object or self.selected_object[0] != 'slope_collider':
            return

        slope = self.selected_object[1]

        new_x1 = simpledialog.askinteger("Start Point", "Enter X1:", initialvalue=int(slope.x1))
        if new_x1 is None:
            return
        new_y1 = simpledialog.askinteger("Start Point", "Enter Y1:", initialvalue=int(slope.y1))
        if new_y1 is None:
            return
        new_x2 = simpledialog.askinteger("End Point", "Enter X2:", initialvalue=int(slope.x2))
        if new_x2 is None:
            return
        new_y2 = simpledialog.askinteger("End Point", "Enter Y2:", initialvalue=int(slope.y2))
        if new_y2 is None:
            return

        slope.x1 = float(new_x1)
        slope.y1 = float(new_y1)
        slope.x2 = float(new_x2)
        slope.y2 = float(new_y2)

    def _edit_collider_tags(self):
        """콜라이더 태그 편집"""
        if not self.selected_object:
            return

        obj_type, obj = self.selected_object
        if obj_type not in ('box_collider', 'slope_collider'):
            return

        # 현재 태그를 쉼표로 구분된 문자열로 변환
        current_tags = ','.join(obj.tags)

        # 태그 입력 다이얼로그
        new_tags_str = simpledialog.askstring(
            "Edit Tags",
            f"Enter tags (comma-separated):\nAvailable: {', '.join(DEFAULT_COLLIDER_TAGS)}",
            initialvalue=current_tags
        )

        if new_tags_str is not None:
            # 쉼표로 분리하고 공백 제거
            new_tags = [tag.strip() for tag in new_tags_str.split(',') if tag.strip()]
            if new_tags:
                obj.tags = new_tags
                # 현재 선택된 태그도 업데이트
                self.current_collider_tags = list(new_tags)

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
