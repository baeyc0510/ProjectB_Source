# 애니메이션 동기화와 상태 관리 설계

> **문서 목적**: 애니메이션-이벤트 동기화 및 상태 시스템 설계 과정과 결론 정리
> **최종 수정**: 2024-11

---

## 1. 문제 정의

### 1.1 애니메이션 프레임-이벤트 동기화 문제

액션 게임에서 애니메이션의 특정 프레임에 로직이 실행되어야 함:
- 공격 애니메이션의 특정 프레임에서 히트 판정
- 콤보 윈도우가 열리고 닫히는 타이밍
- 무적 프레임 시작/종료

**문제**: 애니메이션 타이밍과 게임 로직이 분리되어 있어 동기화가 어려움

### 1.2 기존 접근 방식의 한계

**하드코딩 방식:**
```cpp
void Update()
{
    if (currentFrame == 5) OnHitCheck();
    if (currentFrame == 8) OpenComboWindow();
}
```
- 애니메이션 변경 시 코드 수정 필요
- 프레임 수치가 코드에 흩어져 관리 어려움

**타이머 방식:**
```cpp
void Attack()
{
    SetTimer(0.2f, OnHitCheck);
}
```
- 애니메이션 속도 변경 시 타이머도 수정 필요
- 프레임과 정확히 일치하지 않을 수 있음

---

## 2. 해결책: 델리게이트 기반 이벤트 시스템

### 2.1 애니메이션 메타데이터에 이벤트 정의

JSON 애니메이션 파일에 프레임별 이벤트 기록:
```json
{
    "frames": [...],
    "events": [
        { "frame": 5, "name": "HitCheck" },
        { "frame": 8, "name": "ComboWindowOpen" },
        { "frame": 12, "name": "ComboWindowClose" }
    ]
}
```

### 2.2 Animator에서 델리게이트로 이벤트 발생

```cpp
// CAnimator.h
class CAnimator
{
public:
    MulticastDelegate<const wstring&> OnFrameEvent;
};

// CAnimator.cpp (프레임 진행 시)
if (currentFrame has event)
    OnFrameEvent.Broadcast(eventName);
```

### 2.3 Ability에서 이벤트 구독

```cpp
void AttackAbility::Activate()
{
    animator->Play(L"Attack1");

    WaitEvent(EGameEvent::HitCheck, BIND(this, OnHitCheck));
    WaitEvent(EGameEvent::ComboWindowOpen, BIND(this, OpenComboWindow));
}

void AttackAbility::OnHitCheck()
{
    // 히트박스 활성화, 데미지 판정
}
```

### 2.4 이벤트 흐름

```
Animation Frame 5 도달
    ↓
CAnimator::OnFrameEvent.Broadcast("HitCheck")
    ↓
AnimEventHelper::ToGameEvent("HitCheck") → EGameEvent::HitCheck
    ↓
CAbilitySystem::OnEvent.Broadcast(EGameEvent::HitCheck)
    ↓
AttackAbility::OnHitCheck() 실행
```

---

## 3. 상태 관리 설계

### 3.1 초기 FSM 설계의 문제점

기존 `State` 클래스 기반 FSM:
- 단일 상태만 허용 (IDLE → RUN → JUMP)
- 실제 게임에서는 **동시 상태** 필요 (Moving + Attacking, Airborne + Attacking)
- State와 Ability 간 역할 중복

### 3.2 애니메이션 제어권 충돌

`CPlayer::UpdateState()`에서 태그 기반으로 애니메이션 결정:
```cpp
if (stateSystem->HasTag(Tag_Airborne))
    animator->Play(L"Fall");
else if (stateSystem->HasTag(Tag_Moving))
    animator->Play(L"Run");
else
    animator->Play(L"Idle");
```

**문제**: Ability가 자체 애니메이션을 재생해도 `UpdateState()`가 매 프레임 덮어씀

### 3.3 고려한 방안들

**방안 1**: 특정 태그로 스킵
```cpp
if (stateSystem->HasAnyTag(Tag_Attacking | Tag_Sliding | Tag_Hit))
    return;
```
- 단점: Ability 추가 시마다 조건문 수정 필요

**방안 2**: 전용 태그 `Tag_AbilityPlaying` 도입
```cpp
if (stateSystem->HasTag(Tag_AbilityPlaying))
    return;
```
- 장점: Ability가 선택적으로 애니메이션 제어권 획득
- 장점: 새 Ability 추가 시 기존 코드 수정 불필요

---

## 4. 최종 결론

### 4.1 델리게이트 도입으로 애니메이션-이벤트 동기화

- 애니메이션 메타데이터에 이벤트 정의
- `CAnimator::OnFrameEvent` 델리게이트로 프레임 이벤트 발생
- `Ability::WaitEvent()`로 특정 이벤트 구독
- 애니메이션과 로직이 데이터로 분리되어 유지보수 용이

### 4.2 State 클래스 제거, StateTag 비트필드 사용

- FSM의 단일 상태 한계 극복
- 동시 상태 조합 지원 (`Tag_Airborne | Tag_Attacking`)
- Ability가 행동 생명주기 담당

### 4.3 Tag_AbilityPlaying으로 애니메이션 제어권 분리

```cpp
enum StateTag
{
    // 물리 상태
    Tag_Grounded        = 1 << 0,
    Tag_Airborne        = 1 << 1,

    // 동작 상태
    Tag_Attacking       = 1 << 2,
    Tag_Moving          = 1 << 3,
    Tag_Sliding         = 1 << 8,
    Tag_Jumping         = 1 << 10,

    // 효과 상태
    Tag_Invincible      = 1 << 4,
    Tag_CanCombo        = 1 << 6,
    Tag_Hit             = 1 << 7,

    // 제어 상태
    Tag_AbilityPlaying  = 1 << 11,  // Ability가 애니메이션 제어 중
};
```

**기본 애니메이션** (CPlayer): Idle, Run, Fall - `Tag_AbilityPlaying` 없을 때만

**Ability 애니메이션**: Attack, Jump, Slide 등 - `GetTagsToAdd()`에서 `Tag_AbilityPlaying` 반환

### 4.4 이벤트 시스템 독립성 (EGameEvent)

```
Animator ---(string)---> AnimEventHelper ---(EGameEvent)---> AbilitySystem ---(EGameEvent)---> Ability
```

- `AnimEventHelper`: 애니메이션 문자열 → `EGameEvent` 변환
- `AbilitySystem`: 애니메이션 형식 모름, `EGameEvent`만 처리
- 추후 다른 시스템(충돌, 타이머 등)도 `EGameEvent` 활용 가능

---

## 5. 최종 구조

```
CPlayer
├── CStateSystem          // StateTag 비트필드 관리
├── CAbilitySystem        // Ability 생명주기, 이벤트 중계
│   ├── OnEvent           // EGameEvent 브로드캐스트
│   └── abilities[]       // 등록된 Ability들
├── CAnimator             // 애니메이션 재생
│   └── OnFrameEvent      // 프레임 이벤트 (string)
└── AnimEventHelper       // Animator → AbilitySystem 변환/중계
```

---

## 6. 사용 예시

### Attack Ability
```cpp
class AttackAbility : public Ability
{
public:
    StateTag GetTagsToAdd() const override
    {
        return Tag_Attacking | Tag_AbilityPlaying;
    }

    void Activate() override
    {
        Ability::Activate();
        animator->Play(L"Attack1");

        WaitEvent(EGameEvent::HitCheck, BIND(this, OnHitCheck));
        WaitEvent(EGameEvent::ComboWindowOpen, BIND(this, OpenComboWindow));
    }

    void OnHitCheck()
    {
        // 히트박스 활성화, 데미지 판정
    }

    void OpenComboWindow()
    {
        stateSystem->AddTag(Tag_CanCombo);
    }
};
```

### Jump Ability
```cpp
class JumpAbility : public Ability
{
public:
    StateTag GetRequiredTags() const override { return Tag_Grounded; }
    StateTag GetTagsToAdd() const override
    {
        return Tag_Jumping | Tag_Airborne | Tag_AbilityPlaying;
    }
    StateTag GetTagsToRemove() const override { return Tag_Grounded; }

    void Activate() override
    {
        Ability::Activate();
        animator->Play(L"Jump");
        // y속도가 양수가 되면 End() → Tag_AbilityPlaying 제거 → Fall 재생
    }
};
```

---

## 7. 결론

| 문제 | 해결책 |
|------|--------|
| 애니메이션 프레임-이벤트 동기화 | 델리게이트 + 메타데이터 기반 이벤트 시스템 |
| FSM 단일 상태 한계 | StateTag 비트필드로 동시 상태 지원 |
| 애니메이션 제어권 충돌 | Tag_AbilityPlaying으로 명시적 분리 |
| 시스템 간 결합도 | EGameEvent로 느슨한 결합 유지 |

이 설계로 새로운 Ability 추가 시 기존 코드 수정 없이 확장 가능하며, 애니메이션 타이밍과 게임 로직이 데이터로 분리되어 유지보수가 용이함.
