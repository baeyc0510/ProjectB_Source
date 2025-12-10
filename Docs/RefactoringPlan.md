# 리팩터링 계획서

## 개요
Architecture.md에서 도출된 개선 여지들에 대한 구체적인 리팩터링 계획을 기록한다.

---

## 1. 클래스 크기 개선

### 1.1 애니메이션 데이터 외부화 (방안 C)

**상태:** 승인됨

**현재 문제:**
- Player::Init()에 애니메이션 등록 코드가 30라인 이상 하드코딩
- 다른 캐릭터(Enemy, Boss)도 동일한 패턴

**변경 내용:**
```cpp
// 신규 파일: PlayerAnimData.h
struct AnimationEntry
{
    const wchar_t* key;
    const wchar_t* path;
    bool repeat;
};

struct PlayerAnimData
{
    static const vector<AnimationEntry> Animations;
};

// 신규 파일: PlayerAnimData.cpp
const vector<AnimationEntry> PlayerAnimData::Animations = {
    { AnimKey::Idle, TEXT("Animations/Penitent/penitent_idle_anim.json"), true },
    { AnimKey::Run, TEXT("Animations/Penitent/penitent_running_anim.json"), true },
    // ... 나머지 애니메이션
};

// Player::Init() 수정
for (const auto& anim : PlayerAnimData::Animations)
    AddAnimation(anim.key, anim.path, anim.repeat);
```

**영향 범위:**
- 신규: `Game/Data/PlayerAnimData.h`, `Game/Data/PlayerAnimData.cpp`
- 수정: `Player.cpp` (Init 함수 간소화)
- 선택적 확장: `EnemyAnimData`, `BossAnimData` 등

**예상 효과:**
- Player::Init() 약 30라인 감소
- 애니메이션 데이터 중앙 관리
- 향후 외부 파일(JSON) 로딩으로 전환 용이

---

### 1.2 영역 주석 정리 (방안 E)

**상태:** 승인됨

**현재 문제:**
- Player.cpp 600+ 라인에 명확한 영역 구분 없음
- 함수 간 논리적 그룹핑 불명확

**변경 내용:**
```cpp
// Player.cpp 영역 구분 추가

/*~ Initialization ~*/
void Player::Init() { ... }
void Player::InitStartupStats() { ... }

/*~ Lifecycle ~*/
void Player::OnEnable() { ... }
void Player::Update() { ... }
void Player::LateUpdate() { ... }
void Player::Render() { ... }
void Player::OnDisable() { ... }
void Player::Release() { ... }

/*~ Input Processing ~*/
void Player::ProcessActiveInput() { ... }
void Player::HandleCombatInput() { ... }
void Player::HandleActionInput() { ... }

/*~ State Updates ~*/
void Player::UpdatePlayerStates() { ... }
void Player::UpdateMovementState() { ... }
void Player::UpdateLedgeState() { ... }

/*~ Passive Abilities ~*/
void Player::ProcessPassiveAbilities() { ... }

/*~ Animation ~*/
void Player::UpdateAnimation() { ... }
void Player::HandleAnimationEvent() { ... }
void Player::OnFootstep() { ... }

/*~ Collision ~*/
void Player::OnCollisionEnter() { ... }
void Player::OnCollisionStay() { ... }
void Player::OnCollisionExit() { ... }

/*~ Combat (ICombatInterface) ~*/
void Player::OnDamage() { ... }
void Player::ProcessGuardInteraction() { ... }
void Player::ApplyHitReaction() { ... }
void Player::SpawnPlayerDamageVFX() { ... }
void Player::GetKnockbackVelocity() { ... }
void Player::GetPlayerHitVfxKey() { ... }

/*~ State Events ~*/
void Player::OnStateChanged() { ... }
void Player::OnStatChanged() { ... }

/*~ Utilities ~*/
void Player::ShouldIgnorePlatform() { ... }
void Player::CheckVelocitySignChanged() { ... }
```

**영향 범위:**
- 수정: `Player.cpp` (주석 추가만, 로직 변경 없음)
- 선택적 확장: `Character.cpp`, `Enemy.cpp`, `Boss.cpp`

**예상 효과:**
- 코드 탐색 시간 단축
- 유지보수 시 관련 함수 빠르게 파악
- 로직 변경 없이 가독성 개선

---

## 2. AI 시스템 개선

### 2.1 공통 베이스 클래스 추출 (방안 A)

**상태:** 승인됨

**현재 문제:**
- AIController와 BossAIController에 중복 코드 존재
- FindPlayer(), GetDistanceToTarget(), GetDirectionToTarget() 등 동일 구현

**변경 내용:**
```cpp
// 신규 파일: AIControllerBase.h
class AIControllerBase : public Component<GameObject>
{
public:
    // 타겟 관리
    GameObject* GetTarget() const { return target; }
    bool HasTarget() const { return target != nullptr; }

    // 거리/방향 유틸리티
    virtual float GetDistanceToTarget() const;
    float GetDistanceToTargetY() const;
    int GetDirectionToTarget() const;

protected:
    void FindPlayer();

    GameObject* target = nullptr;
    StateSystem* stateSystem = nullptr;
    AbilitySystem* abilitySystem = nullptr;
};

// AIController : AIControllerBase
class AIController : public AIControllerBase
{
    // 순찰, 타겟 감지 로직만 유지
};

// BossAIController : AIControllerBase
class BossAIController : public AIControllerBase
{
    // 공격 선택, 추격 로직만 유지
    // GetDistanceToTarget() 오버라이드 (X축만 사용)
};
```

**영향 범위:**
- 신규: `Game/Component/AIControllerBase.h`, `Game/Component/AIControllerBase.cpp`
- 수정: `AIController.h/cpp`, `BossAIController.h/cpp` (베이스 클래스 변경)

**예상 효과:**
- 중복 코드 약 30라인 제거
- 새 AI 타입 추가 시 베이스 클래스 재사용
- 유지보수 시 공통 로직 한 곳에서 수정

---

### 2.2 AI 유틸리티 함수 분리 (방안 B)

**상태:** 승인됨

**현재 문제:**
- 거리 계산, 방향 계산 등 순수 연산 로직이 클래스에 종속

**변경 내용:**
```cpp
// 신규 파일: AIUtils.h
namespace AIUtils
{
    // Scene에서 Player 찾기
    GameObject* FindPlayer(Scene* scene);

    // 거리 계산
    float GetDistanceX(GameObject* from, GameObject* to);
    float GetDistanceY(GameObject* from, GameObject* to);
    float GetDistance2D(GameObject* from, GameObject* to);

    // 방향 계산
    int GetDirectionX(GameObject* from, GameObject* to);  // -1 또는 1

    // 범위 체크
    bool IsInRange(GameObject* from, GameObject* to, float range);
    bool IsInRangeXY(GameObject* from, GameObject* to, float rangeX, float rangeY);
}

// 사용 예시
float dist = AIUtils::GetDistanceX(owner, target);
int dir = AIUtils::GetDirectionX(owner, target);
```

**영향 범위:**
- 신규: `Game/Util/AIUtils.h`, `Game/Util/AIUtils.cpp`
- 수정: `AIControllerBase.cpp` (유틸리티 함수 호출로 대체)

**예상 효과:**
- 순수 함수로 테스트 용이
- 다른 시스템에서도 재사용 가능 (Ability, Projectile 등)
- AI 로직과 연산 로직 분리

---

## 3. 타입 안전성 개선

### 3.1 이벤트 구조체화 (방안 B)

**상태:** 승인됨

**현재 문제:**
- 이벤트 파라미터가 개별 인자로 전달되어 순서/타입 실수 가능
- 파라미터 추가 시 모든 호출부 수정 필요

**변경 내용:**
```cpp
// 신규 파일: EventData.h
struct FSFXEventData
{
    wstring key;
};

struct FBGMEventData
{
    wstring key;
    float volume = 1.f;
};

struct FVFXEventData
{
    wstring key;
    Vec2 pos;
    int direction;
};

struct FCameraShakeEventData
{
    FShakeParams params;
};

struct FTimeScaleEventData
{
    float scale;
    float duration;
};

// EventBusManager.h 수정
class EventBusManager
{
public:
    MulticastDelegate<GameObject*, const FSFXEventData&> OnPlaySFX;
    MulticastDelegate<GameObject*, const FBGMEventData&> OnPlayBGM;
    MulticastDelegate<GameObject*> OnStopBGM;
    MulticastDelegate<GameObject*, const FVFXEventData&> OnSpawnVFX;
    MulticastDelegate<GameObject*, const FCameraShakeEventData&> OnCameraShake;
    MulticastDelegate<GameObject*, const FTimeScaleEventData&> OnSetTimeScale;
    // ...
};

// Ability.cpp 헬퍼 수정
void Ability::SpawnVFX(const wstring& key, Vec2 pos, int direction)
{
    EVENT->OnSpawnVFX.Broadcast(owner, FVFXEventData{ key, pos, direction });
}
```

**영향 범위:**
- 신규: `Game/Event/EventData.h`
- 수정: `EventBusManager.h` (시그니처 변경)
- 수정: `Ability.cpp` (헬퍼 함수들)
- 수정: 각 Manager의 이벤트 구독 코드

**예상 효과:**
- 파라미터 순서 실수 방지
- 이벤트 데이터 확장 시 구조체만 수정
- IDE 자동완성으로 필드명 확인 가능

---

### 3.2 SafeDelegateHandle (방안 C)

**상태:** 승인됨

**현재 문제:**
- DelegateHandle 수동 관리로 구독 해제 누락 가능
- 객체 소멸 후 콜백 호출 시 dangling pointer

**변경 내용:**
```cpp
// Delegate.h에 추가
template<typename... Args>
class SafeDelegateHandle
{
public:
    SafeDelegateHandle() = default;

    SafeDelegateHandle(MulticastDelegate<Args...>& delegate,
                       std::function<void(Args...)> callback)
        : delegatePtr(&delegate)
    {
        handle = delegate.Add(callback);
    }

    ~SafeDelegateHandle()
    {
        Release();
    }

    // 이동 허용
    SafeDelegateHandle(SafeDelegateHandle&& other) noexcept
        : delegatePtr(other.delegatePtr), handle(other.handle)
    {
        other.delegatePtr = nullptr;
        other.handle = DelegateHandle();
    }

    SafeDelegateHandle& operator=(SafeDelegateHandle&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            delegatePtr = other.delegatePtr;
            handle = other.handle;
            other.delegatePtr = nullptr;
            other.handle = DelegateHandle();
        }
        return *this;
    }

    // 복사 금지
    SafeDelegateHandle(const SafeDelegateHandle&) = delete;
    SafeDelegateHandle& operator=(const SafeDelegateHandle&) = delete;

    // 수동 해제
    void Release()
    {
        if (delegatePtr && handle.IsValid())
        {
            delegatePtr->Remove(handle);
            delegatePtr = nullptr;
        }
    }

    bool IsValid() const { return handle.IsValid(); }

private:
    MulticastDelegate<Args...>* delegatePtr = nullptr;
    DelegateHandle handle;
};

// 사용 예시
class AIController
{
private:
    SafeDelegateHandle<EStateTag, EStateTag> stateChangedHandle;

    void ComponentInit()
    {
        stateChangedHandle = SafeDelegateHandle(
            stateSystem->OnStateChanged,
            [this](EStateTag old, EStateTag newTags) {
                OnStateChanged(old, newTags);
            }
        );
    }
    // 소멸 시 자동으로 구독 해제
};
```

**영향 범위:**
- 수정: `Engine/Util/Delegate.h` (SafeDelegateHandle 추가)
- 수정: 이벤트 구독하는 컴포넌트들 (점진적 마이그레이션)
  - `Character.cpp`, `AIController.cpp`, `BossAIController.cpp` 등

**예상 효과:**
- 구독 해제 누락으로 인한 크래시 방지
- RAII로 메모리 안전성 확보
- 기존 DelegateHandle과 공존 가능 (점진적 전환)

**마이그레이션 우선순위:**
1. 컴포넌트 클래스 (수명이 복잡한 객체)
2. Ability 클래스 (이미 ClearEventHandles로 관리 중이므로 후순위)

---

## 4. 테스트 인프라

**상태:** 스킵

---

## 5. 최적화

### 5.1 타겟 감지 주기 조절 (방안 B)

**상태:** 승인됨

**현재 문제:**
- 모든 AI가 매 프레임 타겟 감지 및 거리 계산 수행
- 다수의 적이 있을 때 불필요한 연산 누적

**변경 내용:**
```cpp
// AIController.h - FAIConfig에 추가
struct FAIConfig
{
    // 기존 설정...
    float detectionInterval = 0.1f;  // 감지 주기 (초)
};

// AIController.h - 멤버 변수 추가
float detectionTimer = 0.f;

// AIController.cpp
void AIController::ComponentUpdate()
{
    if (stateSystem->HasAnyTag(TAG_AI_BLOCKED))
        return;

    // 감지는 주기적으로만
    detectionTimer += DT;
    if (detectionTimer >= config.detectionInterval)
    {
        detectionTimer = 0.f;
        UpdateTargetDetection();
    }

    // 순찰/추격은 매 프레임 (이동 필요)
    UpdatePatrol();
}

// BossAIController도 동일하게 적용
```

**영향 범위:**
- 수정: `AIController.h` (FAIConfig에 detectionInterval 추가, 멤버 변수)
- 수정: `AIController.cpp` (ComponentUpdate 수정)
- 수정: `BossAIController.h/cpp` (동일 패턴 적용)

**예상 효과:**
- 감지 연산 10배 감소 (60fps 기준, 매 프레임 → 0.1초마다)
- 최대 0.1초 반응 지연 (게임플레이에 거의 영향 없음)

---

### 5.2 조건부 AI 업데이트 (방안 E)

**상태:** 승인됨

**현재 문제:**
- 화면 밖 AI도 동일하게 업데이트 수행
- 플레이어와 먼 거리의 AI도 불필요하게 연산

**변경 내용:**
```cpp
// AIController.h - 상수 및 함수 추가
private:
    static constexpr float MAX_UPDATE_DISTANCE = 500.f;  // 업데이트 최대 거리
    static constexpr float SCREEN_MARGIN = 50.f;         // 화면 밖 마진

    bool ShouldUpdate() const;
    bool IsOnScreen() const;

// AIController.cpp
bool AIController::ShouldUpdate() const
{
    // 타겟이 없으면 항상 업데이트 (탐색 필요)
    if (!target)
        return true;

    // 화면 내 또는 가까운 거리면 업데이트
    return IsOnScreen() || GetDistanceToTarget() <= MAX_UPDATE_DISTANCE;
}

bool AIController::IsOnScreen() const
{
    Vec2 screenPos = CAMERA->WorldToScreen(owner->GetPos());
    Vec2 screenSize = CAMERA->GetScreenSize();

    return screenPos.x >= -SCREEN_MARGIN
        && screenPos.x <= screenSize.x + SCREEN_MARGIN
        && screenPos.y >= -SCREEN_MARGIN
        && screenPos.y <= screenSize.y + SCREEN_MARGIN;
}

void AIController::ComponentUpdate()
{
    // 업데이트 조건 체크
    if (!ShouldUpdate())
        return;

    if (stateSystem->HasAnyTag(TAG_AI_BLOCKED))
        return;

    // 기존 로직...
}
```

**영향 범위:**
- 수정: `AIController.h` (상수, 헬퍼 함수 추가)
- 수정: `AIController.cpp` (ShouldUpdate, IsOnScreen, ComponentUpdate)
- 선택적: `BossAIController` (보스는 항상 업데이트할 수도 있음)

**예상 효과:**
- 화면 밖 AI 연산 완전 제거
- 대규모 맵에서 성능 향상
- 플레이어 근처 AI만 활성 동작

**주의사항:**
- 보스는 화면 밖에서도 업데이트 필요할 수 있음 (별도 처리)
- 화면 복귀 시 자연스러운 행동 전환 필요

---

## 변경 이력

| 날짜 | 항목 | 상태 |
|------|------|------|
| 2025-12-10 | 1.1 애니메이션 데이터 외부화 | 승인 |
| 2025-12-10 | 1.2 영역 주석 정리 | 승인 |
| 2025-12-10 | 2.1 AI 베이스 클래스 추출 | 승인 |
| 2025-12-10 | 2.2 AI 유틸리티 함수 분리 | 승인 |
| 2025-12-10 | 3.1 이벤트 구조체화 | 승인 |
| 2025-12-10 | 3.2 SafeDelegateHandle | 승인 |
| 2025-12-10 | 5.1 타겟 감지 주기 조절 | 승인 |
| 2025-12-10 | 5.2 조건부 AI 업데이트 | 승인 |
