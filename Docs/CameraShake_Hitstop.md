# 카메라 쉐이크와 히트스톱 이론

이 문서는 게임 피드백 시스템의 핵심 요소인 카메라 쉐이크와 히트스톱의 구현 이론을 정리합니다.

---

## 1. 카메라 쉐이크 (Camera Shake)

### 1.1 개념

카메라 쉐이크는 화면을 흔들어 **충격감과 타격감**을 전달하는 시각적 피드백 기법입니다.

```
    정상 상태:                  쉐이크 적용:
    ┌─────────────┐            ┌─────────────┐
    │             │            │  ↖ ↗ ↙ ↘   │
    │   게임화면   │     →     │   게임화면   │
    │             │            │  흔들림     │
    └─────────────┘            └─────────────┘
```

### 1.2 오프셋 기반 접근

카메라 위치를 직접 변경하지 않고, **쉐이크 오프셋**을 별도로 계산하여 합산합니다.

```cpp
// 실제 카메라 위치 = 기본 위치 + 쉐이크 오프셋
Vec2 GetLookAt() const { return lookAt + shakeOffset; }
```

**장점:**
- 카메라 추적 로직과 쉐이크 로직이 분리됨
- 쉐이크 종료 시 원래 위치로 자연스럽게 복귀
- 여러 쉐이크가 동시에 발생해도 관리가 용이

### 1.3 진동 생성 방식

#### 단순 사인파 (규칙적)

```cpp
shakeOffset.x = sin(time * frequency);
shakeOffset.y = cos(time * frequency);
```

```
    시간 →
    ↑
    │   ╱╲   ╱╲   ╱╲
  0 │──╱──╲─╱──╲─╱──╲──
    │ ╱    ╲    ╲
    ↓╱      ╲    ╲

    예측 가능한 규칙적 진동
```

#### Perlin-like 노이즈 (불규칙적)

여러 주파수의 사인파를 조합하여 자연스러운 랜덤 진동을 생성합니다.

```cpp
// 세 개의 사인파 조합 (가중치: 0.5, 0.3, 0.2)
shakeOffset.x = sin(time * 1.0 + seed) * 0.5f
             + sin(time * 2.3 + seed) * 0.3f
             + sin(time * 4.1 + seed) * 0.2f;
```

```
    저주파 (50%):    ╱‾‾‾╲___╱‾‾‾╲
    중주파 (30%):    ╱╲_╱╲___╱╲_╱╲
    고주파 (20%):    ∿∿∿∿∿∿∿∿∿∿∿∿
    ─────────────────────────────
    합성 결과:       자연스러운 불규칙 진동
```

**원리:**
- 저주파: 큰 움직임의 기본 형태
- 중주파: 중간 크기의 변화 추가
- 고주파: 미세한 떨림 추가
- 랜덤 시드: 매번 다른 패턴 생성

### 1.4 감쇠 (Decay)

시간이 지남에 따라 쉐이크 강도가 줄어듭니다.

```cpp
float progress = elapsed / duration;           // 0 → 1
float decayMultiplier = 1.0f - (progress * decay);  // 1 → 0
float currentIntensity = intensity * decayMultiplier;
```

```
    강도
    ↑
  1 │●
    │ ╲
    │  ╲
    │   ╲  decay = 1.0 (선형 감쇠)
    │    ╲
  0 │─────●──────→ 시간
        duration
```

### 1.5 픽셀 스냅

2D 픽셀 게임에서는 소수점 좌표가 블러링을 유발합니다.

```cpp
// 정수 픽셀로 반올림
shakeOffset.x = floor(shakeOffset.x + 0.5f);
shakeOffset.y = floor(shakeOffset.y + 0.5f);
```

```
    소수점 좌표:              픽셀 스냅:
    ┌───┬───┬───┐            ┌───┬───┬───┐
    │   │▒▒▒│   │            │   │███│   │
    │   │▒▒▒│   │    →       │   │███│   │
    │   │   │   │            │   │   │   │
    └───┴───┴───┘            └───┴───┴───┘
    흐릿한 중간값              선명한 픽셀
```

### 1.6 TimeScale 독립성

쉐이크는 게임 시간이 아닌 **실제 시간**을 기준으로 동작해야 합니다.

```cpp
// 잘못된 방식: 히트스톱 중 쉐이크도 멈춤
shakeRemaining -= DT;  // DT = dt * timeScale

// 올바른 방식: 히트스톱과 무관하게 쉐이크 진행
shakeRemaining -= GetUnscaledDT();  // 실제 경과 시간
```

```
    TimeScale = 0 (히트스톱):

    잘못된 구현:              올바른 구현:
    게임: ▓▓▓▓▓▓ (정지)       게임: ▓▓▓▓▓▓ (정지)
    쉐이크: ▓▓▓▓ (정지)       쉐이크: ↔↔↔↔ (계속)

    → 어색한 연출             → 자연스러운 피드백
```

---

## 2. 히트스톱 (Hitstop / Freeze Frame)

### 2.1 개념

타격 순간 게임 시간을 **일시 정지 또는 감속**하여 충격감을 강조하는 기법입니다.

```
    일반 공격:
    ══════════════════════════════════→ 시간

    히트스톱 적용:
    ════════════▓▓▓▓════════════════→ 시간
               ↑    ↑
               정지  재개
               (0.04초)
```

### 2.2 TimeScale 기반 구현

게임 전체의 시간 흐름을 조절하는 TimeScale을 활용합니다.

```cpp
// DT(Delta Time) 계산
float GetDT() { return dt * timeScale; }

// 히트스톱: 0.04초간 시간 정지
SetTimeScale(0.0f, 0.04f);
```

**TimeScale 값에 따른 효과:**
| TimeScale | 효과 |
|-----------|------|
| 0.0 | 완전 정지 (Freeze) |
| 0.1~0.5 | 슬로우 모션 |
| 1.0 | 정상 속도 |
| 2.0+ | 가속 |

### 2.3 Duration 기반 자동 복귀

히트스톱은 일정 시간 후 자동으로 정상 속도로 돌아와야 합니다.

```cpp
void SetTimeScale(float scale, float duration)
{
    if (duration <= 0)
    {
        // 즉시 적용 (영구)
        timeScale = scale;
    }
    else
    {
        // 임시 적용 후 복귀
        timeScale = scale;
        targetTimeScale = 1.0f;    // 복귀 목표
        timeScaleRemaining = duration;
    }
}
```

```
    TimeScale
    ↑
  1 │────●           ●────────
    │    │           │
    │    │           │
  0 │    ●───────────●
    └────┼───────────┼────────→ 시간
       적용        duration 후
                   자동 복귀
```

### 2.4 UnscaledDT의 필요성

TimeScale이 0일 때도 **duration 카운트다운**은 진행되어야 합니다.

```cpp
void Update()
{
    // unscaled dt로 duration 체크
    if (timeScaleRemaining > 0)
    {
        timeScaleRemaining -= dt;  // 원본 dt 사용 (scaled 아님)
        if (timeScaleRemaining <= 0)
        {
            timeScale = targetTimeScale;
        }
    }
}
```

---

## 3. 두 효과의 조합

### 3.1 피격 피드백 패턴

```cpp
void OnDamage(...)
{
    // 1. 히트스톱 (0.04초간 정지)
    TIMER->SetTimeScale(0.0f, 0.04f);

    // 2. 카메라 쉐이크 (히트스톱과 동시에 진행)
    CAMERA->Shake(ShakePreset::Light);
}
```

### 3.2 타임라인

```
    시간 →    0ms    40ms        150ms
              │      │           │
    TimeScale │▓▓▓▓▓▓│───────────│
              │ 0.0  │   1.0     │
              │      │           │
    Shake     │↔↔↔↔↔↔│↔↔↔↔↔↔↔↔↔↔↔│
              │흔들림│ 감쇠      │ 종료
              │      │           │
    게임      │ 정지 │ 정상 진행 │
```

### 3.3 프리셋 활용

상황별로 다른 강도의 피드백을 적용합니다.

```cpp
namespace ShakePreset
{
    // 약한 피격 (일반 몬스터 공격)
    constexpr FShakeParams Light  = { 0.15f, 2.f,  30.f, 1.f, true };

    // 중간 피격 (플레이어 피격)
    constexpr FShakeParams Medium = { 0.25f, 5.f,  25.f, 1.f, true };

    // 강한 피격 (크리티컬, 처형)
    constexpr FShakeParams Heavy  = { 0.4f,  10.f, 20.f, 0.8f, true };

    // 보스 공격
    constexpr FShakeParams Boss   = { 0.6f,  15.f, 15.f, 0.6f, true };
}
```

---

## 4. 파라미터 가이드

### 4.1 카메라 쉐이크

| 파라미터 | 설명 | 권장 범위 |
|----------|------|-----------|
| duration | 지속 시간 (초) | 0.1 ~ 0.6 |
| intensity | 최대 흔들림 (픽셀) | 2 ~ 15 |
| frequency | 초당 진동 횟수 | 15 ~ 30 |
| decay | 감쇠율 (0=없음, 1=선형) | 0.6 ~ 1.0 |
| bRandomOffset | 불규칙 진동 여부 | true 권장 |

### 4.2 히트스톱

| 상황 | TimeScale | Duration |
|------|-----------|----------|
| 일반 타격 | 0.0 | 0.03 ~ 0.05초 |
| 강공격 | 0.0 | 0.05 ~ 0.08초 |
| 처형기 | 0.0 | 0.1 ~ 0.15초 |
| 슬로우 모션 | 0.3 | 0.5 ~ 1.0초 |

---

## 5. 관련 코드 참조

- `CCameraManager::Shake()` - 쉐이크 시작
- `CCameraManager::UpdateShake()` - 쉐이크 오프셋 계산
- `CCameraManager::GetLookAt()` - lookAt + shakeOffset 반환
- `CTimeManager::SetTimeScale()` - 타임스케일 설정
- `CTimeManager::GetDT()` - scaled delta time
- `CTimeManager::GetUnscaledDT()` - 원본 delta time
- `CEnemy::OnDamage()` - 히트스톱 + 쉐이크 적용 예시
