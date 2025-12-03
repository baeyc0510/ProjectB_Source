# 충돌 처리: 슬로프와 MTD 이론

이 문서는 `CCharacter.cpp`에 구현된 지형 충돌 처리 시스템의 핵심 이론을 정리합니다.

## 1. MTD (Minimum Translation Distance)

### 1.1 개념

**MTD(최소 이동 거리)**는 두 물체가 겹쳐 있을 때, 겹침을 해소하기 위해 필요한 **가장 짧은 이동 벡터**를 의미합니다.

```
    ┌─────────┐
    │    A    │
    │   ┌─────┼───┐
    │   │█████│   │   ← 겹침 영역(█)
    └───┼─────┘   │
        │    B    │
        └─────────┘
```

### 1.2 AABB 충돌에서의 MTD 계산

AABB(Axis-Aligned Bounding Box) 충돌에서는 각 축의 겹침을 독립적으로 계산합니다.

```cpp
// 겹침 계산
float overlapX = (myHalf.x + otherHalf.x) - abs(myPos.x - otherPos.x);
float overlapY = (myHalf.y + otherHalf.y) - abs(myPos.y - otherPos.y);
```

**수식 설명:**
- `myHalf.x + otherHalf.x`: 두 콜라이더 너비의 합의 절반 (두 중심이 겹치지 않는 최소 거리)
- `abs(myPos.x - otherPos.x)`: 두 중심 사이의 실제 거리
- 차이가 양수면 겹침이 발생한 것

```
    ← halfA →← halfB →
    ┌───────┬───────┐
    │   A   │   B   │  ← 겹치지 않음 (거리 = halfA + halfB)
    └───────┴───────┘

    ← halfA →
        ← halfB →
    ┌───────┬───┐
    │   A ██│ B │      ← 겹침 = (halfA + halfB) - 거리
    └───────┴───┘
```

### 1.3 분리 축 선택

**더 작은 겹침 방향으로 밀어내는 것**이 MTD의 핵심입니다.

```cpp
if (overlapY <= overlapX)
{
    // 수직 방향으로 밀어냄 (바닥/천장)
}
else
{
    // 수평 방향으로 밀어냄 (벽)
}
```

**이유**: 작은 쪽으로 밀어야 물체가 최소한으로 이동합니다.

```
    예시: overlapX=5, overlapY=2

    ┌─────────┐
    │    A    │
    │   ┌─────┼───┐     Y방향으로 2만큼 밀면 분리됨
    └───┼─────┘   │     X방향으로 밀면 5가 필요
        │    B    │
        └─────────┘

    결과: overlapY(2)가 더 작으므로 Y방향으로 분리
```

### 1.4 방향 결정

겹침을 해소할 방향은 두 물체의 상대 위치로 결정합니다.

```cpp
// 수직 충돌
if (myPos.y < otherPos.y)
{
    // 내가 위에 있음 → 위로 밀어냄 (바닥 충돌)
    newPos.y -= overlapY;
}
else
{
    // 내가 아래에 있음 → 아래로 밀어냄 (천장 충돌)
    newPos.y += overlapY;
}

// 수평 충돌
float pushDir = (myPos.x < otherPos.x) ? -1.f : 1.f;
newPos.x += pushDir * overlapX;
```

---

## 2. 슬로프(경사면) 처리

### 2.1 Line Collider 기반 슬로프

프로젝트에서는 **Line Collider**를 사용하여 경사면을 표현합니다.

```
    ────────────────
                   ╲
                    ╲  ← Line Collider (시작점 → 끝점)
                     ╲
    ──────────────────╲────
```

### 2.2 슬로프 위의 Y 좌표 계산

Line Collider는 주어진 X 좌표에서의 Y 값을 선형 보간으로 계산합니다.

```cpp
float slopeY = lineCollider->GetYAt(myPos.x);
```

**선형 보간 공식:**
```
                (endY - startY)
slopeY = startY + ───────────────── × (x - startX)
                (endX - startX)
```

```
    start(x1, y1)
         ●
          ╲
           ╲  ← 캐릭터 X 위치에서의 Y 계산
            ●───── slopeY
             ╲
              ╲
               ● end(x2, y2)
```

### 2.3 침투 깊이(Penetration) 계산

캐릭터가 슬로프에 얼마나 파고들었는지 계산합니다.

```cpp
float footY = myPos.y + myHalf.y;     // 캐릭터 발 위치
float penetration = footY - slopeY;    // 침투 깊이
```

```
    penetration < 0: 공중에 있음
         ┌───┐
         │   │
         └───┘
           │
           ▼  (음수 = 슬로프 위)
    ═══════════════

    penetration > 0: 파고들어 있음
    ═══════════════
           ▲  (양수 = 슬로프 아래)
           │
         ┌─┼─┐
         │ │ │
         └───┘
```

### 2.4 슬로프 충돌 조건

```cpp
if (penetration > -5.f && penetration < 50.f)
{
    // 슬로프 위에 있거나 약간 파고들었을 때
    if (velocity.y >= 0)  // 상승 중이 아닐 때만
    {
        // 착지 처리
    }
}
```

**조건 설명:**
- `penetration > -5.f`: 슬로프 바로 위 또는 아래에 있음 (여유 마진 5픽셀)
- `penetration < 50.f`: 너무 깊이 파고들지 않음 (비정상 상태 방지)
- `velocity.y >= 0`: 점프 상승 중에는 처리하지 않음

### 2.5 위치 보정

슬로프에 착지하면 캐릭터를 슬로프 표면에 스냅합니다.

```cpp
Vec2 newPos = GetPos();
newPos.y = slopeY - myHalf.y - collider->GetOffset().y;
SetPos(newPos);
```

```
    보정 전:              보정 후:
         ┌───┐                 ┌───┐
         │   │                 │   │
    ═════╪═══╪═══         ═════└───┘═══
         └───┘ (파묻힘)         (표면에 정렬)
```

### 2.6 최대 등반 각도 제한

캐릭터가 오를 수 있는 경사면의 최대 각도를 제한합니다.

```cpp
// CCharacter.h
static constexpr float MAX_SLOPE_ANGLE = 50.0f;  // 도 단위
static constexpr float MAX_SLOPE_ANGLE_RAD = MAX_SLOPE_ANGLE * PI / 180.0f;
```

#### 슬로프 각도 계산

```cpp
// CLineCollider::GetSlopeAngle()
float dx = end.x - start.x;
float dy = end.y - start.y;
return atan2f(-dy, abs(dx));  // Y축 반전 보정
```

```
    각도 계산 원리:
                       ● start
                      ╱│
                     ╱ │ dy (Y가 아래로 증가하므로 -dy 사용)
                    ╱  │
                   ╱θ  │
              end ●────┘
                    dx

    θ = atan2(-dy, |dx|)
```

#### 오르막/내리막 판별

```cpp
// 슬로프 방향: 왼→오 올라가면 true
bool slopeGoesUpRight = (end.y < start.y);
if (end.x < start.x) slopeGoesUpRight = !slopeGoesUpRight;

// 오르막 판별: 이동 방향과 슬로프 상승 방향이 같으면 오르막
bool isClimbing = (velocity.x > 0 && slopeGoesUpRight) ||
                  (velocity.x < 0 && !slopeGoesUpRight);
```

#### 각도 초과 시 처리

```cpp
if (isClimbing && slopeAngle > MAX_SLOPE_ANGLE_RAD)
{
    // 이동 방향 반대로 밀어냄
    float pushDir = (velocity.x > 0) ? -1.f : 1.f;
    newPos.x += pushDir * (abs(penetration) + 1.f);

    // X 속도 제거 (벽처럼 처리)
    velocity.x = 0.f;
}
```

```
    50° 이하: 오를 수 있음          50° 초과: 벽처럼 처리

        ┌───┐                           ┌───┐
        │ → │                           │ ✗ │←── 밀려남
        └───┘                           └───┘
           ╲  45°                          │  60°
            ╲                              │
             ╲                             │
```

### 2.7 경사면 속도 보정

경사면을 오를 때 X축 이동량을 보정하여 실제 이동 속도를 일정하게 유지합니다.

#### 문제점

평지에서 X로 1만큼 이동하면, 경사면에서는 같은 X 이동에 더 긴 거리를 이동합니다.

```
    평지:                    경사면 (45°):

    ●───────●               ●
    ← 1.0 →                  ╲
                              ╲  실제 이동: √2 ≈ 1.414
                               ╲
                                ●
                     ← 1.0 →
```

**피타고라스 정리:**
```
실제 이동 거리 = √(dx² + dy²) = dx / cos(θ)
```

#### 해결책: X 이동량 보정

```cpp
if (isClimbing && abs(velocity.x) > 0.1f)
{
    float cosAngle = cosf(slopeAngle);
    float velocityAdjust = velocity.x * (1.f - cosAngle) * DT;
    newPos.x -= velocityAdjust;
}

// 보정된 X 위치에서 슬로프 Y 계산 후 위로 올리기
float adjustedSlopeY = lineCollider->GetYAt(newPos.x + offset.x);
newPos.y = adjustedSlopeY - halfHeight - offset.y;
```

```
    보정 원리:

    원래 X 이동량: vx × dt
    보정 계수: cos(θ)
    보정 후 X 이동량: vx × cos(θ) × dt

    예시 (45° 경사):
    - cos(45°) ≈ 0.707
    - 원래 100픽셀 이동 → 보정 후 70.7픽셀 이동
    - 실제 경로 길이: 70.7 / cos(45°) = 100픽셀 (일정!)
```

```
    보정 전 (동일 시간):        보정 후 (동일 시간):

        ●                           ●
         ╲                           ╲
          ╲                           ●  ← 더 짧은 X 이동
           ●  ← 너무 멀리 이동

    평지보다 빠름                평지와 같은 속도
```

---

## 3. 충돌 처리 우선순위

`OnCollisionStay`에서의 처리 순서:

1. **Line Collider 체크** (경사면)
   - 동적으로 Y 좌표 계산
   - 정밀한 경사면 이동 지원

2. **Box Collider MTD** (평면)
   - 단순하고 빠른 충돌 해결
   - 바닥, 벽, 천장 처리

---

## 4. 속도 처리

### 4.1 바닥 충돌 시
```cpp
if (velocity.y > 0)  // 하강 중일 때만
{
    velocity.y = 0.f;
    rigidbody->SetVelocity(velocity);
}
```

### 4.2 천장 충돌 시
```cpp
if (velocity.y < 0)  // 상승 중일 때만
{
    velocity.y = 0.f;
    rigidbody->SetVelocity(velocity);
}
```

### 4.3 벽 충돌 시
```cpp
bool movingIntoWall = (pushDir < 0 && velocity.x > 0) ||
                      (pushDir > 0 && velocity.x < 0);
if (movingIntoWall)
{
    velocity.x = 0.f;
    rigidbody->SetVelocity(velocity);
}
```

**중요**: 벽에서 멀어지는 방향으로 이동 중이면 속도를 유지합니다.

---

## 5. 구현 핵심 포인트

| 요소 | 설명 |
|------|------|
| MTD 분리축 | 겹침이 적은 축을 선택하여 최소 이동 |
| 방향 결정 | 상대 위치에 따라 밀어내는 방향 결정 |
| 슬로프 Y 계산 | 선형 보간으로 정확한 표면 위치 계산 |
| 침투 마진 | 약간의 허용 범위로 떨림 방지 |
| 속도 조건 | 이동 방향에 따라 선택적 속도 제거 |

---

## 6. 관련 코드 참조

- `CCharacter::OnCollisionStay()` - MTD 및 슬로프 처리 메인 로직
- `CCharacter::MAX_SLOPE_ANGLE` - 최대 등반 각도 상수 (50°)
- `CLineCollider::GetYAt()` - 슬로프 Y 좌표 계산
- `CLineCollider::GetSlopeAngle()` - 슬로프 각도 계산 (라디안)
- `CLineCollider::GetSlopeAngleDegrees()` - 슬로프 각도 계산 (도)
- `CLineCollider::IsInXRange()` - X 범위 유효성 체크
