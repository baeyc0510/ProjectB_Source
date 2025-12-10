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
- 상수를 클래스 내 정적 멤버로 정의하여 매직 넘버 제거
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

**평가**:
- (+) 명확한 계층 분리로 책임이 잘 구분됨
- (+) Character 기반 클래스가 공통 기능을 캡슐화
- (-) Character가 너무 많은 컴포넌트를 직접 보유 (7개)
- (-) Player 클래스가 비대해지는 경향 (130+ 라인 헤더)

---

## 3. 핵심 시스템

### 3.1 StateSystem (상태 관리)

#### 개념

비트필드 기반 태그 시스템으로 캐릭터의 현재 상태를 관리한다.
여러 상태가 동시에 활성화될 수 있으며, 참조 카운팅으로 중첩 상태를 지원한다.

#### 구조

```
EStateTag (비트필드 enum)
    |
    +-- Tag_Grounded     (1 << 0)   지면에 있음
    +-- Tag_Airborne     (1 << 1)   공중에 있음
    +-- Tag_Attacking    (1 << 2)   공격 중
    +-- Tag_Invincible   (1 << 4)   무적
    +-- Tag_Stunned      (1 << 5)   스턴
    +-- ...

StateSystem
    +-- currentTags: EStateTag        현재 활성 태그들
    +-- bitCountMap: map<tag, count>  태그별 참조 카운트
    +-- OnStateChanged: Delegate      상태 변경 이벤트
```

#### 동작 흐름

```
AddTag(Tag_Attacking)
    |
    v
bitCountMap[Tag_Attacking]++ == 1?  --> 최초 추가
    |                                      |
    v                                      v
currentTags |= Tag_Attacking          OnStateChanged 브로드캐스트
```

#### 사용 예시

```
// Ability가 발동 조건 확인
EStateTag required = Tag_Grounded;
EStateTag blocked = Tag_Attacking | Tag_Hit;

if (stateSystem->HasAllTags(required) &&
    !stateSystem->HasAnyTag(blocked))
{
    // Ability 발동 가능
}
```

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
    |
    +-- abilities: map<EAbility, unique_ptr<Ability>>
    +-- activeAbilities: vector<Ability*>
    +-- stateSystem: StateSystem*
    +-- OnEvent: MulticastDelegate

Ability (추상 클래스)
    |
    +-- owner: GameObject*
    +-- abilitySystem: AbilitySystem*
    +-- isActive: bool
    +-- cooldownRemaining: float
    +-- eventHandles: vector<DelegateHandle>
    |
    +-- GetRequiredTags()   발동에 필요한 태그
    +-- GetBlockedTags()    발동을 막는 태그
    +-- GetTagsToAdd()      발동 시 추가할 태그
    +-- GetTagsToRemove()   발동 시 제거할 태그
    +-- GetCancelTags()     취소할 다른 Ability의 태그
```

#### Ability 생명주기

```
TryActivateAbility(EAbility::Attack)
    |
    v
CanActivateAbility() 체크
    - HasAllTags(RequiredTags)?
    - !HasAnyTag(BlockedTags)?
    - !IsOnCooldown()?
    |
    v (통과)
ActivateAbility()
    - CancelAbilitiesWithTag(CancelTags)
    - AddTag(TagsToAdd)
    - RemoveTag(TagsToRemove)
    - ability->Activate()
    |
    v
OnActivate() (파생 클래스 구현)
    - 애니메이션 재생
    - 이벤트 대기 등록
    |
    v
이벤트 발생 (HitCheck, ComboWindow 등)
    |
    v
EndAbility()
    - RemoveTag(TagsToAdd)
    - ClearEventHandles()
    - OnEnded 브로드캐스트
```

#### 이벤트 대기 메커니즘

```
// Ability 내부에서 애니메이션 이벤트 대기
void Ability_ComboAttack::OnActivate()
{
    WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
    WaitEvent(EGameEvent::ComboWindowOpen, BIND_EVENT(this, OnComboWindowOpen));
}

// 애니메이션에서 이벤트 트리거
Animator -> TriggerEvent(HitCheck) -> AbilitySystem -> 등록된 콜백 호출
```

**평가**:
- (+) 선언적 발동 조건으로 로직이 명확함
- (+) 이벤트 기반으로 애니메이션과 게임 로직이 잘 분리됨
- (+) unique_ptr로 Ability 수명 관리 자동화
- (-) Ability 파생 클래스 수가 많아 파일이 분산됨 (15+개)
- (-) WaitEvent/EndWaitEvent 수동 관리 필요
- (-) 복잡한 Ability는 상태 머신이 암묵적으로 콜백에 분산됨

---

### 3.3 CharacterMovement (이동 및 충돌 처리)

#### 개념

캐릭터의 물리적 이동과 지면/벽/경사면 충돌을 처리하는 전용 컴포넌트이다.
Character는 이 컴포넌트의 상태를 폴링하여 StateSystem을 갱신한다.

#### 구조

```
CharacterMovement
    |
    +-- config: FMovementConfig
    |       +-- maxSlopeAngle      오를 수 있는 최대 경사각
    |       +-- bCanDropThrough    플랫폼 드롭다운 가능 여부
    |       +-- bBlockAtEdges      가장자리에서 정지 (AI용)
    |
    +-- groundState: FGroundState
    |       +-- bIsGrounded        지면 착지 여부
    |       +-- bIsOnSteepSlope    가파른 경사면 여부
    |       +-- activeGroundID     현재 밟고 있는 Ground ID
    |       +-- groundMinX/MaxX    Ground의 X 범위
    |
    +-- frameFlags: FFrameFlags
    |       +-- bHitWall           벽 충돌 여부
    |       +-- bReachedEdge       가장자리 도달 여부
    |       +-- bBeingSquashed     끼임 상태 여부
    |
    +-- rigidbody, collider        캐시된 컴포넌트
```

#### 충돌 처리 흐름

```
OnCollisionStay(other)
    |
    v
레이어 확인 (Ground / Platform)
    |
    +-- LineCollider? --> HandleLineGround (경사면)
    |                         |
    |                         +-- 가파른 경사 --> 미끄러짐
    |                         +-- 완만한 경사 --> 속도 보정 + 스냅
    |
    +-- BoxCollider? --> HandleBoxGround (평면)
                             |
                             +-- 바닥 충돌 --> SetGrounded(true) + 스냅
                             +-- 천장 충돌 --> 속도 0 + 밀어냄
                             +-- 벽 충돌 --> 속도 0 + 밀어냄 + 플래그 설정
                             +-- 끼임 상태 --> 밀려남 처리
```

#### 경사면 처리 로직

```
경사각 측정
    |
    +-- > maxSlopeAngle (50도) --> HandleSteepSlope
    |                                 - bIsOnSteepSlope = true
    |                                 - 경사면 방향으로 미끄러짐 속도 적용
    |                                 - Y위치를 경사면에 스냅
    |
    +-- <= maxSlopeAngle ---------> HandleGentleSlope
                                      - 오르막 시 속도 감속 보정
                                      - Y위치를 경사면에 스냅
                                      - 낙하 속도 0
```

#### AI용 엣지 감지

```
HandleCollisionExit
    |
    v
config.bBlockAtEdges == true?
    |
    v
CheckGroundAhead(direction)
    - 이동 방향 앞에 BoxTrace
    - Ground 또는 Platform 감지
    |
    +-- 없음 --> frameFlags.bReachedEdge = true
                  속도 0, 안전 위치로 보정
```

**평가**:
- (+) 충돌 로직이 한 컴포넌트에 집중되어 관리 용이
- (+) Config로 캐릭터별 다른 이동 특성 적용 가능
- (+) frameFlags로 충돌 정보를 캡슐화하여 폴링 가능
- (-) HandleBoxGround가 100+라인으로 복잡함
- (-) Ground/Platform 외 충돌 타입 확장이 어려움
- (-) 경사면 스냅 로직이 프레임 의존적 (DT 사용)

---

### 3.4 AI 시스템

#### 3.4.1 AIController (일반 적 AI)

```
AIController
    |
    +-- config: FAIConfig
    |       +-- detectRange       감지 범위
    |       +-- attackRange       공격 범위
    |       +-- patrolRange       순찰 범위
    |       +-- chaseSpeed        추격 속도
    |
    +-- target: GameObject*       현재 타겟 (Player)
    +-- patrolOrigin: Vec2        순찰 시작점
    +-- patrolDirection: int      순찰 방향 (-1/+1)

동작 흐름:
ComponentUpdate()
    |
    +-- UpdateTargetDetection()
    |       - 감지 범위 내 플레이어 탐색
    |       - 발견 시 Tag_HasTarget 추가, AI_TargetDetected 이벤트
    |       - 놓침 시 Tag_HasTarget 제거, AI_TargetLost 이벤트
    |
    +-- UpdatePatrol()
            - Tag_AIPatrol 활성화 시 순찰 진행
            - 경계 도달 시 방향 전환 또는 정지
```

#### 3.4.2 BossAIController (보스 AI)

```
BossAIController
    |
    +-- attacks: vector<FBossAttackData>
    |       +-- ability           공격 종류
    |       +-- minRange/maxRange 유효 거리
    |       +-- weight            선택 가중치
    |
    +-- chaseConfig: FBossChaseConfig
    |       +-- chaseSpeed        추격 속도
    |       +-- chaseRange        추격 시작 거리
    |       +-- stopRange         정지 거리
    |
    +-- decisionTimer             공격 결정 타이머

공격 선택 로직:
SelectNextAttack()
    |
    v
유효 공격 필터링
    - 현재 거리가 minRange ~ maxRange 내
    - 쿨다운 아님
    |
    v
가중치 기반 랜덤 선택
    - totalWeight 계산
    - 랜덤값으로 공격 선택
```

**평가**:
- (+) 설정값 분리로 다양한 AI 패턴 구현 가능
- (+) 이벤트 기반으로 Ability와 자연스럽게 연동
- (+) 보스 공격 선택이 데이터 주도적
- (-) AIController와 BossAIController 간 코드 중복 존재
- (-) 상태 기반 AI가 단순한 편 (Behavior Tree 미사용)
- (-) 타겟 감지가 매 프레임 실행 (최적화 여지)

---

### 3.5 전투 시스템

#### 구조

```
ICombatInterface
    |
    +-- OnDamage(source, context)   데미지 처리 인터페이스

CombatContext
    +-- damageType: EDamageType     데미지 타입 (Slash, Heavy, Parry)
    +-- value: float                데미지 양
    +-- vfxKey: wstring             이펙트 키
    +-- hitResult: HitResult        충돌 정보

AttackData
    +-- traceOffset, traceSize      히트박스 정보
    +-- damage: float               기본 데미지
    +-- damageType: EDamageType     데미지 타입
    +-- vfxKey: const wchar_t*      이펙트 키

CombatHelper (정적 유틸리티)
    +-- ApplyDamageInBox()          BoxTrace + 데미지 적용
    +-- ApplyDamageWithAttackData() AttackData 기반 공격
```

#### 데미지 흐름

```
Ability_ComboAttack::OnHitCheck()
    |
    v
CombatHelper::ApplyDamageWithAttackData(source, attackData, layers)
    |
    v
COLLISION->BoxTrace(center, size, layer)
    |
    v
for each hitResult:
    ICombatInterface* combat = dynamic_cast<>(hit.gameObject)
    combat->OnDamage(source, context)
        |
        v
    Target::OnDamage()
        - StatComponent->TakeDamage()
        - 넉백 적용
        - VFX 생성
        - HitReaction Ability 발동
```

**평가**:
- (+) 인터페이스로 데미지 수신자 추상화
- (+) CombatHelper가 공통 로직 캡슐화
- (+) AttackData로 공격 정보 구조화
- (-) 데미지 타입별 처리가 OnDamage 내부에 하드코딩
- (-) 방어력, 크리티컬 등 확장 시스템 부재

---

### 3.6 EventBusManager (전역 이벤트)

#### 구조

```
EventBusManager (Singleton)
    |
    +-- OnPlaySFX: MulticastDelegate         사운드 재생
    +-- OnPlayBGM: MulticastDelegate         배경음 재생
    +-- OnStopBGM: MulticastDelegate         배경음 정지
    +-- OnSpawnVFX: MulticastDelegate        이펙트 생성
    +-- OnCameraShake: MulticastDelegate     카메라 흔들림
    +-- OnCameraFadeIn/Out: MulticastDelegate 페이드 효과
    +-- OnSetTimeScale: MulticastDelegate    시간 조작
```

#### 사용 패턴

```
// 구독 측 (Manager)
EventBusManager::GetInstance()->OnSpawnVFX.AddLambda(
    [](GameObject*, const wstring& key, Vec2 pos, int dir) {
        VFXManager::GetInstance()->Spawn(key, pos, dir);
    }
);

// 발행 측 (Ability)
void Ability::SpawnVFX(const wstring& key, Vec2 pos, int direction)
{
    EVENT->OnSpawnVFX.Broadcast(owner, key, pos, direction);
}
```

**평가**:
- (+) 시스템 간 결합도 감소
- (+) Ability가 Manager에 직접 의존하지 않음
- (-) 이벤트 타입이 추가될 때마다 EventBusManager 수정 필요
- (-) 타입 안전성이 부족 (런타임 에러 가능)
- (-) 구독 해제 관리가 수동적

---

## 4. 컴포넌트 초기화 및 수명

### 초기화 순서

```
Character::Init()
    |
    +-- AddChild(new Rigidbody())        물리
    +-- AddChild(new BoxCollider())      충돌
    +-- AddChild(new Animator())         애니메이션
    +-- AddChild(new StateSystem())      상태
    +-- AddChild(new StatComponent())    스탯
    +-- AddChild(new AbilitySystem())    능력
    +-- AddChild(new CharacterMovement()) 이동
    |
    v
각 컴포넌트 ComponentInit() 호출
    - 필수 의존성 assert
    - 캐시 초기화
    |
    v
파생 클래스 Init()
    - Ability 등록
    - Config 설정
    - 스탯 초기화
```

### 소유권 모델

```
GameObject (엔진 레이어)
    |
    +-- children: vector<unique_ptr<GameObject>>
            |
            +-- 모든 자식(컴포넌트 포함) 소유권 관리
            +-- 소멸 시 자동 해제

컴포넌트 간 참조: raw pointer (소유권 없음)
    - CharacterMovement -> Rigidbody* (캐시)
    - AIController -> StateSystem*, AbilitySystem* (캐시)
    - Ability -> owner*, abilitySystem* (lazy 캐싱)
```

**평가**:
- (+) 소유권이 명확하여 메모리 누수 방지
- (+) 컴포넌트 간 참조가 단순
- (-) 초기화 순서 의존성이 암묵적
- (-) 컴포넌트 수가 많아 Init() 함수가 길어짐

---

## 5. 종합 평가

### 강점

| 영역 | 내용 |
|------|------|
| 아키텍처 | 컴포넌트 기반 설계로 재사용성 높음 |
| 상태 관리 | 비트필드 + 참조 카운팅으로 효율적 |
| Ability | 선언적 조건 정의로 확장 용이 |
| 이벤트 | 느슨한 결합으로 유지보수성 향상 |
| 충돌 처리 | 경사면, 플랫폼 등 복잡한 케이스 커버 |

### 개선 여지

| 영역 | 내용 |
|------|------|
| 클래스 크기 | Player, Character 등 비대한 클래스 분리 필요 |
| AI 시스템 | Behavior Tree 등 고급 패턴 도입 검토 |
| 타입 안전성 | 이벤트 시스템의 컴파일 타임 검증 부족 |
| 테스트 | 단위 테스트 인프라 부재 |
| 최적화 | 매 프레임 타겟 감지 등 불필요한 연산 존재 |

### 확장 시 고려사항

1. **새 캐릭터 추가**: Character 상속 + 컴포넌트 조합
2. **새 Ability 추가**: Ability 상속 + 태그 조건 정의
3. **새 상태 추가**: EStateTag에 비트 추가 (28개 제한 주의)
4. **새 AI 패턴**: AIController 파생 또는 Config 확장
5. **새 충돌 타입**: ELayer 추가 + CharacterMovement 핸들러 추가

---

*문서 작성일: 2025-12-10*
