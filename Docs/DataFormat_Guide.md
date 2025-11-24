# Animation & Map Data Format Guide

WinAPI 기반 게임 프로젝트에서 애니메이션 및 맵 데이터를 활용하는 방법에 대한 가이드입니다.

---

## 1. 좌표계

### 화면 좌표계
- **원점**: 왼쪽 상단 (0, 0)
- **X축**: 오른쪽으로 증가
- **Y축**: 아래쪽으로 증가

```
(0,0) ────────────→ +X
  │
  │
  │
  ↓
 +Y
```

### 주요 개념
- 모든 위치 데이터는 픽셀 단위
- WinAPI `RECT`, GDI 함수들과 동일한 좌표계 사용

---

## 2. Animation Data Format

### 파일 구조
```json
{
    "image": "Resources/Sprites/player_idle.png",
    "pivot": "bottom-center",
    "frames": [
        [0, 0, 64, 64],
        [64, 0, 64, 64],
        [128, 0, 64, 64]
    ]
}
```

### 필드 설명

| 필드 | 타입 | 설명 |
|------|------|------|
| `image` | string | 스프라이트시트 이미지 경로 (프로젝트 루트 기준 상대경로) |
| `pivot` | string | 피벗 포인트 위치 |
| `frames` | array | 프레임 정보 배열 `[x, y, width, height]` |

### Pivot 종류

| Pivot | 설명 | 용도 |
|-------|------|------|
| `top-left` | 왼쪽 상단 | 기본값, UI 요소 |
| `top-center` | 상단 중앙 | 떨어지는 오브젝트 |
| `center` | 중앙 | 회전하는 오브젝트, 이펙트 |
| `bottom-center` | 하단 중앙 | **캐릭터, NPC (권장)** |
| `bottom-left` | 왼쪽 하단 | 특수한 경우 |

### 활용 방법

#### 1) 데이터 로드
- JSON 파싱 라이브러리 사용 (nlohmann/json, rapidjson 등)
- 또는 직접 파싱

#### 2) 이미지 로드
- `image` 경로에서 스프라이트시트 로드
- WinAPI: `LoadImage()`, GDI+: `Bitmap::FromFile()`

#### 3) 프레임 재생
- 프레임 인덱스를 시간에 따라 순환
- 각 프레임의 `[x, y, w, h]`로 스프라이트시트에서 영역 추출

#### 4) 렌더링 위치 계산
```
실제 렌더링 위치 = 오브젝트 위치 - 피벗 오프셋

pivot이 "bottom-center"이고, 프레임 크기가 64x64일 때:
- pivot_offset_x = 64 / 2 = 32
- pivot_offset_y = 64

render_x = object.x - 32
render_y = object.y - 64
```

---

## 3. Map Data Format

### 파일 구조
```json
{
    "schema_version": "1.0",
    "map": {
        "name": "stage_01",
        "size": [1920, 1080],
        "tilesets": [
            "Resources/Tilesets/ground.png",
            "Resources/Tilesets/props.png"
        ],
        "layers": [...],
        "colliders": [...],
        "props": [...],
        "objects": [...]
    }
}
```

### 3.1 Layers (타일 레이어)

```json
"layers": [
    {
        "name": "bg_far",
        "depth": -20,
        "parallax": 0.3,
        "tiles": [
            {"tileset": 0, "src": [0, 0, 32, 32], "pos": [100, 200]}
        ]
    },
    {
        "name": "main",
        "depth": 0,
        "parallax": 1.0,
        "tiles": [...]
    }
]
```

| 필드 | 설명 |
|------|------|
| `name` | 레이어 이름 |
| `depth` | 렌더링 순서 (낮을수록 뒤에) |
| `parallax` | 패럴렉스 스크롤 비율 (1.0 = 카메라와 동일) |
| `tiles[].tileset` | 타일셋 인덱스 |
| `tiles[].src` | 타일셋 내 영역 `[x, y, w, h]` |
| `tiles[].pos` | 맵 내 위치 `[x, y]` (타일 왼쪽 상단 기준) |

#### 활용: 렌더링 순서
```
1. depth 기준 오름차순 정렬
2. depth가 낮은 레이어부터 렌더링 (뒤 → 앞)
```

#### 활용: 패럴렉스 스크롤
```
render_x = tile.pos.x - (camera.x * layer.parallax)
render_y = tile.pos.y - (camera.y * layer.parallax)
```

### 3.2 Colliders

```json
"colliders": [
    {
        "type": "box",
        "rect": [100, 500, 200, 32],
        "data": {"layer": "ground", "one_way": false}
    },
    {
        "type": "slope_left",
        "rect": [300, 468, 64, 64],
        "data": {"angle": 45}
    }
]
```

| 타입 | 설명 | 충돌 처리 |
|------|------|----------|
| `box` | 사각형 충돌체 | AABB 충돌 |
| `slope_left` | 왼쪽 경사면 (╱) | 경사면 충돌 |
| `slope_right` | 오른쪽 경사면 (╲) | 경사면 충돌 |
| `trigger` | 트리거 존 | 충돌 감지만 (이벤트 발생) |
| `damage` | 데미지 존 | 충돌 시 데미지 |

#### 활용: 충돌 시스템 구축
1. 모든 collider를 리스트로 관리
2. 플레이어/적 이동 시 collider와 충돌 검사
3. `type`에 따라 다른 반응 처리
4. `data` 필드의 추가 정보 활용

### 3.3 Props

```json
"props": [
    {
        "type": "destructible",
        "tileset": 1,
        "src": [0, 0, 32, 48],
        "pos": [500, 300],
        "data": {
            "pivot": "bottom-center",
            "has_collider": true,
            "collider_type": "box",
            "collider_width": 24,
            "collider_height": 32,
            "collider_offset_x": -12,
            "collider_offset_y": -32
        }
    }
]
```

#### Prop 위치 계산

**pos는 pivot 위치**입니다. 렌더링 시:

```
// 1. 스프라이트 렌더링 위치
render_x = prop.pos.x - get_pivot_offset_x(pivot, src.width)
render_y = prop.pos.y - get_pivot_offset_y(pivot, src.height)

// 2. 콜라이더 위치 (pivot 기준)
collider.left = prop.pos.x + collider_offset_x
collider.top = prop.pos.y + collider_offset_y
collider.right = collider.left + collider_width
collider.bottom = collider.top + collider_height
```

#### Animated Prop

타일셋 대신 애니메이션 사용:
```json
{
    "type": "decoration",
    "tileset": -1,
    "animation": "Resources/Animations/torch.json",
    "pos": [600, 400],
    "data": {"pivot": "bottom-center"}
}
```

### 3.4 Objects

```json
"objects": [
    {
        "type": "spawn_point",
        "pos": [100, 500],
        "data": {"player_id": 1}
    },
    {
        "type": "enemy_spawn",
        "pos": [800, 500],
        "data": {"enemy_type": "skeleton", "patrol_range": 200}
    }
]
```

오브젝트는 시각적 요소 없이 게임 로직에만 사용:
- 스폰 포인트
- 이벤트 트리거 위치
- AI 웨이포인트
- 아이템 생성 위치

---

## 4. 권장 로드 순서

### 게임 시작 시
1. `tilesets` 배열의 모든 이미지 로드
2. `layers`를 depth 기준으로 정렬
3. `colliders`를 공간 분할 구조에 등록 (쿼드트리 등)
4. `props` 순회하며 오브젝트 생성
5. `objects` 순회하며 게임 엔티티 생성

### 렌더링 루프
```
1. 배경 레이어 (depth < 0) 렌더링
2. 게임 오브젝트 렌더링 (플레이어, 적, prop 등)
3. 전경 레이어 (depth > 0) 렌더링
4. UI 렌더링
```

---

## 5. Pivot Offset 계산 함수

```
get_pivot_offset(pivot, width, height):
    switch(pivot):
        "top-left":      return (0, 0)
        "top-center":    return (width/2, 0)
        "top-right":     return (width, 0)
        "center-left":   return (0, height/2)
        "center":        return (width/2, height/2)
        "center-right":  return (width, height/2)
        "bottom-left":   return (0, height)
        "bottom-center": return (width/2, height)
        "bottom-right":  return (width, height)
```

---

## 6. 주의사항

### 경로 처리
- 모든 경로는 프로젝트 루트 기준 상대경로
- 슬래시(`/`) 사용 권장 (Windows에서도 동작)
- 절대경로로 변환 시 프로젝트 루트 경로와 결합

### 좌표계 일관성
- 에디터와 게임 엔진 모두 Y-down 좌표계 사용
- 물리 엔진에서 Y-up이 필요하면 로드 시 변환

### 콜라이더 offset
- offset은 pivot 위치 기준
- offset (0, 0) = 콜라이더 왼쪽 상단이 pivot에 위치
- 음수 offset = 왼쪽/위쪽으로 이동

---

## 7. 디버그 렌더링 권장

개발 중에는 다음을 시각화하면 도움됨:
- 콜라이더 영역 (타입별 색상 구분)
- Prop의 pivot 위치 (작은 십자 표시)
- 오브젝트 위치 및 영역
- 레이어 depth 정보
