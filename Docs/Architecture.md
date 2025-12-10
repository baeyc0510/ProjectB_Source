# Blasphemous WinAPI Clone - 시스템 아키텍처 문서

## 1. 설계 철학

### 1.1 핵심 원칙

**Composition over Inheritance (상속보다 조합)**
- 행동과 기능을 컴포넌트로 분리하여 재사용성 극대화
- Character 클래스는 공통 컴포넌트(StateSystem, AbilitySystem 등)를 조합하여 기능 구현
- 새로운 캐릭터 타입 추가 시 기존 컴포넌트 재사용 가능

**Event-Driven Communication (이벤트 기반 통신)**
- 시스템 간 직접 참조 대신 이벤트를 통한 느슨한 결합
- EventBusManager를 통한 전역 이벤트 브로드캐스트
- Delegate/MulticastDelegate 패턴으로 옵저버 구현

**State-Based Logic (상태 기반 로직)**
- boolean 플래그 대신 비트필드 태그 시스템 사용
- 상태 조합과 전이를 명시적으로 관리
- Ability 발동 조건을 선언적으로 정의 (RequiredTags, BlockedTags)

**Data-Driven Design (데이터 주도 설계)**
- Config 구조체로 행동 파라미터 분리 (FAIConfig, FMovementConfig)
- 리소스 데이터를 별도 파일로 외부화 (AnimData, SFXData, VFXData)
- 애니메이션 이벤트가 게임 로직 트리거

---

## 2. 아키텍처 개요

### 2.1 계층 구조

```
+------------------------------------------------------------------+
|                        Game Layer                                 |
|  +------------+  +------------+  +------------+  +------------+   |
|  |  Scenes    |  |  Objects   |  |  Managers  |  |    UI      |   |
|  +------------+  +------------+  +------------+  +------------+   |
+------------------------------------------------------------------+
                              |
+------------------------------------------------------------------+
|                     Component Layer                               |
|  +---------------+  +---------------+  +---------------+          |
|  | AbilitySystem |  |  StateSystem  |  |CharacterMove |          |
|  +---------------+  +---------------+  +---------------+          |
|  +---------------+  +---------------+  +---------------+          |
|  | AIController  |  | StatComponent |  |   Rigidbody  |          |
|  +---------------+  +---------------+  +---------------+          |
+------------------------------------------------------------------+
                              |
+------------------------------------------------------------------+
|                      Engine Layer                                 |
|  +------------+  +------------+  +------------+  +------------+   |
|  | GameObject |  | Component  |  | Collider   |  |  Animator  |   |
|  +------------+  +------------+  +------------+  +------------+   |
+------------------------------------------------------------------+
```

### 2.2 핵심 클래스 관계

```
GameObject
    |
    +-- Character (공통 컴포넌트 보유)
    |       |
    |       +-- Player (ICombatInterface)
    |       |
    |       +-- Enemy (ICombatInterface, AIController)
    |       |       |
    |       |       +-- Enemy_Acolyte
    |       |
    |       +-- Boss (ICombatInterface, BossAIController)
    |               |
    |               +-- Boss_TenPiedad
    |
    +-- VFX, Projectile, Hazard, WorldCollider, ...
```

### 2.3 AI 컨트롤러 계층 (리팩터링 후)

```
AIControllerBase (공통 기반)
    |
    +-- target, stateSystem, abilitySystem 캐싱
    +-- FindPlayer(), GetDistanceToTarget(), GetDirectionToTarget()
    |
    +-- AIController (일반 적 AI)
    |       +-- 순찰, 타겟 감지
    |       +-- 감지 주기 조절 (detectionInterval)
    |       +-- 화면 밖 업데이트 스킵 (IsOnScreen)
    |
    +-- BossAIController (보스 AI)
            +-- 공격 선택, 추격
            +-- X축 거리만 사용 (GetDistanceToTarget 오버라이드)
```

**평가**:
- (+) 명확한 계층 분리로 책임이 잘 구분됨
- (+) Character 기반 클래스가 공통 기능을 캡슐화
- (+) AI 컨트롤러 베이스 클래스로 중복 제거
- (-) Character가 많은 컴포넌트를 직접 보유 (7개)

---

## 3. 핵심 시스템

### 3.1 StateSystem (상태 관리)

#### 개념

비트필드 기반 태그 시스템으로 캐릭터의 현재 상태를 관리한다.
여러 상태가 동시에 활성화될 수 있으며, 참조 카운팅으로 중첩 상태를 지원한다.

#### 구조

```
EStateTag (비트필드 enum)
    +-- Tag_Grounded, Tag_Airborne, Tag_Attacking, Tag_Invincible, ...

StateSystem
    +-- currentTags: EStateTag        현재 활성 태그들
    +-- bitCountMap: map<tag, count>  태그별 참조 카운트
    +-- OnStateChanged: Delegate      상태 변경 이벤트
```

#### 동작 원리

- `AddTag`: 참조 카운트 증가, 최초 추가 시 비트 설정 + 이벤트 발생
- `RemoveTag`: 참조 카운트 감소, 0이 되면 비트 해제 + 이벤트 발생
- `HasTag/HasAllTags/HasAnyTag`: 비트 연산으로 O(1) 체크

**평가**:
- (+) 비트 연산으로 빠른 상태 체크 가능
- (+) 참조 카운팅으로 중첩 상태 안전하게 관리
- (+) 상태 변경 이벤트로 반응형 로직 구현 용이
- (-) 태그가 28개 이상으로 증가하면 int 범위 초과
- (-) 태그 간 상호 배타 관계가 코드에 암묵적으로만 존재

---

### 3.2 AbilitySystem (능력 시스템)

#### 개념

캐릭터의 모든 액션(공격, 점프, 슬라이드 등)을 Ability로 추상화하여 관리한다.
각 Ability는 발동 조건, 상태 태그 변화, 이벤트 응답을 선언적으로 정의한다.

#### 구조

```
AbilitySystem
    +-- abilities: map<EAbility, unique_ptr<Ability>>
    +-- activeAbilities: vector<Ability*>
    +-- OnEvent: MulticastDelegate

Ability (추상 클래스)
    +-- GetRequiredTags()   발동에 필요한 태그
    +-- GetBlockedTags()    발동을 막는 태그
    +-- GetTagsToAdd()      발동 시 추가할 태그
    +-- GetCancelTags()     취소할 다른 Ability의 태그
```

#### Ability 생명주기

```
TryActivateAbility -> CanActivateAbility 체크 -> ActivateAbility
    -> CancelAbilitiesWithTag -> AddTag -> ability->Activate()
    -> WaitEvent로 애니메이션 이벤트 대기
    -> EndAbility -> RemoveTag -> ClearEventHandles
```

**평가**:
- (+) 선언적 발동 조건으로 로직이 명확함
- (+) 이벤트 기반으로 애니메이션과 게임 로직이 잘 분리됨
- (+) unique_ptr로 Ability 수명 관리 자동화
- (-) Ability 파생 클래스 수가 많아 파일이 분산됨 (15+개)
- (-) WaitEvent/EndWaitEvent 수동 관리 필요

---

### 3.3 CharacterMovement (이동 및 충돌 처리)

#### 개념

캐릭터의 물리적 이동과 지면/벽/경사면 충돌을 처리하는 전용 컴포넌트이다.

#### 구조

```
CharacterMovement
    +-- config: FMovementConfig
    |       +-- maxSlopeAngle      오를 수 있는 최대 경사각
    |       +-- bCanDropThrough    플랫폼 드롭다운 가능 여부
    |       +-- bBlockAtEdges      가장자리에서 정지 (AI용)
    |
    +-- groundState: FGroundState
    |       +-- bIsGrounded        지면 착지 여부
    |       +-- bIsOnSteepSlope    가파른 경사면 여부
    |
    +-- frameFlags: FFrameFlags
            +-- bHitWall           벽 충돌 여부
            +-- bReachedEdge       가장자리 도달 여부
```

#### 충돌 처리 흐름

```
OnCollisionStay
    +-- LineCollider -> HandleLineGround (경사면)
    |       +-- 가파른 경사 -> 미끄러짐
    |       +-- 완만한 경사 -> 속도 보정 + 스냅
    |
    +-- BoxCollider -> HandleBoxGround (평면)
            +-- 바닥/천장/벽 충돌 처리
            +-- 끼임 상태 처리
```

**평가**:
- (+) 충돌 로직이 한 컴포넌트에 집중되어 관리 용이
- (+) Config로 캐릭터별 다른 이동 특성 적용 가능
- (+) frameFlags로 충돌 정보를 캡슐화하여 폴링 가능
- (-) HandleBoxGround가 100+라인으로 복잡함
- (-) Ground/Platform 외 충돌 타입 확장이 어려움

---

### 3.4 AI 시스템

#### 리팩터링 이전 구조

```
AIController (독립 클래스)
    +-- target, stateSystem, abilitySystem 멤버
    +-- FindPlayer(), GetDistanceToTarget(), GetDirectionToTarget()

BossAIController (독립 클래스)
    +-- target, stateSystem, abilitySystem 멤버 (중복)
    +-- FindPlayer(), GetDistanceToTarget(), GetDirectionToTarget() (중복)
```

**문제점**:
- 두 클래스 간 30라인 이상 중복 코드
- 동일한 타겟 관리/거리 계산 로직이 분산

#### 리팩터링 이후 구조

```
AIControllerBase (공통 베이스)
    +-- target, stateSystem, abilitySystem 캐싱
    +-- FindPlayer() -> AIUtils::FindPlayer() 호출
    +-- GetDistanceToTarget() -> AIUtils::GetDistance2D() 호출
    +-- GetDistanceToTargetY() -> AIUtils::GetDistanceY() 호출
    +-- GetDirectionToTarget() -> AIUtils::GetDirectionX() 호출

AIController : AIControllerBase
    +-- 순찰 로직 (UpdatePatrol)
    +-- 타겟 감지 (UpdateTargetDetection)
    +-- 감지 주기 조절 (detectionInterval = 0.1초)
    +-- 조건부 업데이트 (ShouldUpdate, IsOnScreen)

BossAIController : AIControllerBase
    +-- 공격 선택 (SelectNextAttack)
    +-- 추격 로직 (ShouldChase)
    +-- GetDistanceToTarget() 오버라이드 (X축만 사용)

AIUtils (순수 유틸리티 네임스페이스)
    +-- FindPlayer(Scene*)
    +-- GetDistanceX/Y/2D(from, to)
    +-- GetDirectionX(from, to)
    +-- IsInRange/IsInRangeXY(from, to, range)
```

#### 성능 최적화

**감지 주기 조절**
- detectionInterval(0.1초)마다만 UpdateTargetDetection 실행
- 매 프레임 감지 -> 10프레임당 1회 감지로 연산량 감소

**조건부 AI 업데이트**
- IsOnScreen(): 카메라 중심에서 화면 절반 + 마진(50px) 범위 체크
- ShouldUpdate(): 타겟 없으면 항상 업데이트, 있으면 화면 내/500거리 내만 업데이트
- 화면 밖 + 원거리 AI는 업데이트 스킵

**평가 (리팩터링 후)**:
- (+) 공통 로직이 베이스 클래스로 통합되어 유지보수 용이
- (+) AIUtils로 순수 함수 분리되어 재사용성/테스트 용이성 향상
- (+) 감지 주기 조절로 연산량 10배 감소
- (+) 화면 밖 AI 스킵으로 대규모 맵 성능 향상
- (-) 0.1초 감지 지연이 있으나 체감 어려움
- (-) Behavior Tree 등 고급 패턴은 미도입

---

### 3.5 전투 시스템

#### 구조

```
ICombatInterface
    +-- OnDamage(source, context)   데미지 처리 인터페이스

CombatContext
    +-- damageType, value, vfxKey, hitResult

CombatHelper (정적 유틸리티)
    +-- ApplyDamageInBox()          BoxTrace + 데미지 적용
    +-- ApplyDamageWithAttackData() AttackData 기반 공격
```

#### 데미지 흐름

```
Ability::OnHitCheck
    -> CombatHelper::ApplyDamageWithAttackData
    -> COLLISION->BoxTrace
    -> ICombatInterface::OnDamage
    -> StatComponent->TakeDamage + 넉백 + VFX
```

**평가**:
- (+) 인터페이스로 데미지 수신자 추상화
- (+) CombatHelper가 공통 로직 캡슐화
- (+) AttackData로 공격 정보 구조화
- (-) 데미지 타입별 처리가 OnDamage 내부에 하드코딩
- (-) 방어력, 크리티컬 등 확장 시스템 부재

---

### 3.6 이벤트 시스템 (EventBusManager)

#### 리팩터링 이전 구조

```
EventBusManager
    +-- OnPlaySFX: MulticastDelegate<GameObject*, const wstring&>
    +-- OnSpawnVFX: MulticastDelegate<GameObject*, const wstring&, Vec2, int>
    +-- OnCameraShake: MulticastDelegate<GameObject*, const FShakeParams&>
    ...
```

**문제점**:
- 개별 파라미터로 전달되어 순서/타입 실수 가능
- 파라미터 추가 시 모든 호출부 수정 필요
- 구독 해제 수동 관리로 누락 가능

#### 리팩터링 이후 구조

```
EventData.h (이벤트 구조체 정의)
    +-- SFXEventData { key }
    +-- BGMEventData { key, volume }
    +-- VFXEventData { key, pos, direction }
    +-- CameraShakeEventData { params }
    +-- CameraFadeEventData { duration }
    +-- TimeScaleEventData { scale, duration }

EventBusManager
    +-- OnPlaySFX: MulticastDelegate<GameObject*, const SFXEventData&>
    +-- OnPlayBGM: MulticastDelegate<GameObject*, const BGMEventData&>
    +-- OnSpawnVFX: MulticastDelegate<GameObject*, const VFXEventData&>
    +-- OnCameraShake: MulticastDelegate<GameObject*, const CameraShakeEventData&>
    +-- OnCameraFadeIn/Out: MulticastDelegate<GameObject*, const CameraFadeEventData&>
    +-- OnSetTimeScale: MulticastDelegate<GameObject*, const TimeScaleEventData&>

SafeDelegateHandle<Args...> (RAII 구독 관리)
    +-- 생성자에서 자동 구독
    +-- 소멸자에서 자동 해제
    +-- 이동만 허용, 복사 금지
    +-- Release()로 수동 해제 가능
```

**평가 (리팩터링 후)**:
- (+) 구조체로 파라미터 묶어 순서 실수 방지
- (+) 이벤트 데이터 확장 시 구조체만 수정
- (+) SafeDelegateHandle로 구독 해제 누락 방지
- (+) RAII 패턴으로 메모리 안전성 확보
- (-) 이벤트 타입 추가 시 EventBusManager 수정 필요 (구조적 한계)

---

## 4. 리소스 데이터 외부화

### 리팩터링 이전 구조

각 캐릭터의 Init() 함수에 애니메이션 등록 코드가 하드코딩되어 있었다.

**문제점**:
- Player::Init()에 30라인 이상의 애니메이션 등록 코드
- 리소스 경로 변경 시 코드 수정 필요
- 동일한 패턴이 Enemy, Boss에도 반복

### 리팩터링 이후 구조

```
Game/Data/
    +-- ResourceTypes.h      공통 구조체 정의
    |       +-- AnimationEntry { key, path, repeat }
    |       +-- SoundEntry { key, path }
    |       +-- VFXEntry { key, path }
    |
    +-- AnimKey.h            애니메이션 키 상수
    +-- SFXKeys.h            사운드 키 상수
    +-- VFXKeys.h            VFX 키 상수
    |
    +-- PlayerAnimData.h     플레이어 애니메이션 목록 (28개)
    +-- EnemyAnimData.h      적 애니메이션 목록
    +-- BossAnimData.h       보스 애니메이션 목록
    +-- SFXData.h            전체 사운드 목록 (45개)
    +-- VFXData.h            전체 VFX 목록 (10개)
```

#### 데이터 구조 예시

```
PlayerAnimData::GetAnimations()
    -> Locomotion: Idle, Run, Jump, Fall, Landed
    -> Combat: Combo1-3, AirCombo1-2, CrouchAttack, Parry
    -> Actions: Slide, Crouch, UseFlask, Climbing, LedgeClimb
    -> Hit Reactions: Pushback, Rising, Dead

SFXData::GetAllSounds()
    -> Player: Attack, Hit, Damage, Death, Movement, Parry, Misc
    -> Acolyte: PrepareAttack, ReleaseAttack, Death, Footsteps
    -> Boss: Slash, Smash, Spit, Stomp, Death
    -> BGM & System
```

**평가**:
- (+) 리소스 데이터가 코드에서 분리되어 관리 용이
- (+) 키 상수로 오타 방지 및 IDE 자동완성 지원
- (+) Init() 함수가 간소화되어 가독성 향상
- (+) 향후 외부 파일(JSON) 로딩으로 전환 용이
- (-) static 데이터 초기화 순서 의존성 있음

---

## 5. 코드 조직화

### Player.cpp 영역 구분

리팩터링 후 8개 영역으로 구분하여 탐색 용이성 향상:

```
/*~ Initialization ~*/     Init, InitStartupStats
/*~ Lifecycle ~*/          OnEnable, Update, LateUpdate, Render, OnDisable
/*~ Input Processing ~*/   ProcessActiveInput, HandleCombatInput, HandleActionInput
/*~ State Updates ~*/      UpdatePlayerStates, UpdateMovementState, UpdateLedgeState
/*~ Animation ~*/          UpdateAnimation, HandleAnimationEvent, OnFootstep
/*~ Collision ~*/          OnCollisionEnter, OnCollisionStay, OnCollisionExit
/*~ Combat ~*/             OnDamage, ProcessGuardInteraction, ApplyHitReaction
/*~ State Events ~*/       OnStateChanged, OnStatChanged
```

**평가**:
- (+) 관련 함수가 그룹화되어 유지보수 시 빠른 탐색 가능
- (+) 로직 변경 없이 가독성만 개선
- (-) 주석 기반이므로 IDE 지원 제한적

---

## 6. 컴포넌트 초기화 및 수명

### 초기화 순서

```
Character::Init()
    +-- AddChild(Rigidbody, BoxCollider, Animator)
    +-- AddChild(StateSystem, StatComponent, AbilitySystem, CharacterMovement)
    -> 각 컴포넌트 ComponentInit() 호출
    -> 파생 클래스 Init() (Ability 등록, Config 설정)
```

### 소유권 모델

```
GameObject
    +-- children: vector<unique_ptr<GameObject>>  소유권 관리

컴포넌트 간 참조: raw pointer (소유권 없음)
    - CharacterMovement -> Rigidbody* (캐시)
    - AIController -> StateSystem*, AbilitySystem* (캐시)
```

**평가**:
- (+) 소유권이 명확하여 메모리 누수 방지
- (+) 컴포넌트 간 참조가 단순
- (-) 초기화 순서 의존성이 암묵적

---

## 7. 종합 평가

### 리팩터링 전후 비교

| 영역 | 이전 | 이후 | 개선 효과 |
|------|------|------|----------|
| AI 코드 중복 | 30+ 라인 중복 | 베이스 클래스로 통합 | 유지보수성 향상 |
| AI 연산량 | 매 프레임 감지 | 0.1초 주기 + 화면 체크 | 연산량 90%+ 감소 |
| 이벤트 타입 안전성 | 개별 파라미터 | 구조체 기반 | 컴파일 타임 검증 |
| 구독 관리 | 수동 해제 | SafeDelegateHandle | 메모리 안전성 확보 |
| 리소스 관리 | Init() 하드코딩 | Data 클래스 분리 | 데이터-코드 분리 |
| 코드 탐색 | 구분 없음 | 영역 주석 | 가독성 향상 |

### 현재 강점

| 영역 | 내용 |
|------|------|
| 아키텍처 | 컴포넌트 기반 설계로 재사용성 높음 |
| 상태 관리 | 비트필드 + 참조 카운팅으로 효율적 |
| Ability | 선언적 조건 정의로 확장 용이 |
| 이벤트 | 구조체 + RAII로 안전한 통신 |
| AI | 베이스 클래스 + 유틸리티로 중복 제거, 최적화 적용 |
| 리소스 | 데이터 외부화로 관리 용이 |

### 남은 개선 여지

| 영역 | 내용 |
|------|------|
| 충돌 처리 | HandleBoxGround 분리 검토 |
| AI 고급 패턴 | Behavior Tree 도입 검토 (복잡한 AI 필요 시) |
| 테스트 | 단위 테스트 인프라 구축 |
| 타입 이벤트 | TypedEventBus로 완전한 타입 안전성 (대규모 리팩터링 시) |

### 확장 시 고려사항

1. **새 캐릭터 추가**: Character 상속 + AnimData 클래스 추가
2. **새 Ability 추가**: Ability 상속 + 태그 조건 정의
3. **새 AI 패턴**: AIControllerBase 상속 또는 Config 확장
4. **새 이벤트**: EventData.h에 구조체 추가 + EventBusManager에 Delegate 추가
5. **새 리소스**: ResourceTypes.h 구조체 사용 + 해당 Data 클래스에 등록

---

*문서 작성일: 2025-12-10*
*최종 수정: 리팩터링 완료 후 구조 반영*
