# 기술적 도전 과제 및 해결 방안 보고서

> **프로젝트**: 블래스퍼머스 WinAPI 모작
> **범위**: 2주 MVP 구현
> **최종 수정**: 2025-11-28

---

## 목차

1. [개요](#1-개요)
2. [도전 과제 1: 상태 머신 기반 캐릭터 제어](#2-도전-과제-1-상태-머신-기반-캐릭터-제어)
3. [도전 과제 2: 콤보 시스템 및 입력 버퍼링](#3-도전-과제-2-콤보-시스템-및-입력-버퍼링)
4. [도전 과제 3: 충돌 시스템](#4-도전-과제-3-충돌-시스템)
5. [도전 과제 4: 패리 타이밍 판정](#5-도전-과제-4-패리-타이밍-판정)
6. [도전 과제 5: 적 AI 행동 패턴](#6-도전-과제-5-적-ai-행동-패턴)
7. [도전 과제 6: 프레임 독립적 게임 루프](#7-도전-과제-6-프레임-독립적-게임-루프)
8. [도전 과제 7: 더블 버퍼링 렌더링](#8-도전-과제-7-더블-버퍼링-렌더링)
9. [도전 과제 8: 애니메이션-로직 동기화](#9-도전-과제-8-애니메이션-로직-동기화)
10. [권장 접근 방식 요약](#10-권장-접근-방식-요약)

---

## 1. 개요

### 1.1 보고서 목적

2주 내 구현 예정인 블래스퍼머스 모작 프로젝트에서 기술적으로 도전적인 요소들을 식별하고, 각 도전에 대한 다양한 해결 방안을 제시하여 개발 리스크를 최소화한다.

### 1.2 도전 과제 요약

| 순위 | 도전 과제 | 난이도 | 영향도 | 상태 |
|------|-----------|--------|--------|------|
| 1 | 상태 머신 기반 캐릭터 제어 | ★★★★☆ | 높음 | ✅ 완료 (State/Tag 시스템) |
| 2 | 콤보 시스템 및 입력 버퍼링 | ★★★★☆ | 높음 | ✅ 완료 (ComboWindow 이벤트) |
| 3 | 충돌 시스템 | ★★★★★ | 높음 | ✅ 완료 (BoxTrace + Layer) |
| 4 | 패리 타이밍 판정 | ★★★☆☆ | 중간 | ✅ 완료 (ParryWindow 이벤트) |
| 5 | 적 AI 행동 패턴 | ★★★☆☆ | 중간 | 🔄 진행 중 (기본 공격만) |
| 6 | 프레임 독립적 게임 루프 | ★★★★☆ | 높음 | ✅ 완료 |
| 7 | 더블 버퍼링 렌더링 | ★★★☆☆ | 높음 | ✅ 완료 |
| 8 | 애니메이션-로직 동기화 | ★★★★☆ | 높음 | ✅ 완료 (이벤트 시스템) |

---

## 2. 도전 과제 1: 상태 머신 기반 캐릭터 제어

### 2.1 문제 정의

플레이어 캐릭터는 17개 이상의 상태를 가지며, 각 상태 간 전이 규칙이 복잡하다.

**주요 어려움:**
- 상태 전이 조건의 복잡성 (입력, 물리, 애니메이션 조건 혼재)
- 상태별 행동 코드 분리
- 새로운 상태 추가 시 기존 코드 영향 최소화
- 상태 전이 중 버그 추적의 어려움

### 2.2 해결 방안

#### 방안 A: 열거형 + Switch문 방식

**개요:** 가장 단순한 구현 방식

**구조:**
```
enum PlayerState { IDLE, RUN, JUMP, ATTACK_1, ... }

Update() {
    switch(currentState) {
        case IDLE: UpdateIdle(); break;
        case RUN: UpdateRun(); break;
        ...
    }
}
```

**장점:**
- 구현이 직관적이고 빠름
- 디버깅 용이
- 작은 규모에서 효율적

**단점:**
- 상태가 많아지면 switch문 비대화
- 상태 전이 로직이 분산됨
- 확장성 낮음

**적합성:** ★★★☆☆ (프로토타입용)

---

#### 방안 B: 상태 패턴 (State Pattern)

**개요:** 각 상태를 별도 클래스로 분리

**구조:**
```
class IPlayerState {
    virtual void Enter() = 0;
    virtual void Update() = 0;
    virtual void Exit() = 0;
    virtual IPlayerState* CheckTransition() = 0;
}

class IdleState : public IPlayerState { ... }
class RunState : public IPlayerState { ... }
```

**장점:**
- 상태별 코드 완전 분리
- 새 상태 추가가 기존 코드에 영향 없음
- 상태 전이 로직이 명확함
- 테스트 용이

**단점:**
- 클래스 수 증가 (17개 이상)
- 상태 간 데이터 공유 방법 필요
- 초기 설계 시간 필요

**적합성:** ★★★★★ (권장)

---

#### 방안 C: 계층적 상태 머신 (HFSM)

**개요:** 상태를 계층 구조로 그룹화

**구조:**
```
GroundedState (부모)
  ├── IdleState
  ├── RunState
  └── AttackState
      ├── Attack1State
      ├── Attack2State
      └── Attack3State

AirborneState (부모)
  ├── JumpState
  ├── FallState
  └── AirAttackState
```

**장점:**
- 공통 로직을 부모 상태에서 처리
- 상태 그룹 단위 전이 가능
- 복잡한 상태 관계 관리 용이

**단점:**
- 설계 복잡도 증가
- 계층 구조 설계 실수 시 수정 어려움
- 2주 내 구현에는 과도할 수 있음

**적합성:** ★★★☆☆ (시간 여유 시)

---

#### 방안 D: 데이터 기반 상태 머신

**개요:** 상태 전이를 데이터(테이블)로 정의

**구조:**
```
StateTransition transitions[] = {
    { IDLE, INPUT_MOVE, RUN },
    { IDLE, INPUT_JUMP, JUMP },
    { RUN, INPUT_NONE, IDLE },
    ...
}
```

**장점:**
- 전이 규칙 수정이 데이터 변경만으로 가능
- 디버깅 시 전이 테이블 확인 용이
- 에디터 툴 연동 가능

**단점:**
- 복잡한 조건(AND, OR)표현 어려움
- 초기 테이블 설계 필요
- 조건 함수 매핑 필요

**적합성:** ★★★☆☆ (확장성 중시 시)

---

### 2.3 권장 해결 방안

**1순위: 방안 B (상태 패턴)**
- 2주 내 구현 가능
- 확장성과 유지보수성 균형

**빠른 대안: 방안 A + 상태별 함수 분리**
- 시간이 촉박할 경우
- 나중에 방안 B로 리팩토링

---

## 3. 도전 과제 2: 콤보 시스템 및 입력 버퍼링

### 3.1 문제 정의

3타 콤보 시스템에서 "캔슬 윈도우" 내 입력만 다음 공격으로 연결해야 한다.

**주요 어려움:**
- 정확한 타이밍 윈도우 판정
- 입력 버퍼링 (미리 누른 입력 처리)
- 콤보 리셋 조건
- 애니메이션과의 동기화

### 3.2 해결 방안

#### 방안 A: 프레임 카운터 방식

**개요:** 공격 시작 후 프레임을 카운트하여 윈도우 판정

**구조:**
```
Attack1 시작 → frameCounter = 0
매 프레임: frameCounter++
if (frameCounter >= 9 && frameCounter <= 21) {
    // 캔슬 윈도우 (프레임 9~21)
    if (attackInput) → Attack2로 전이
}
```

**장점:**
- 구현 단순
- 프레임 단위 정밀 제어
- 디버깅 용이

**단점:**
- 프레임 레이트 의존적
- 애니메이션 변경 시 수치 재조정 필요

**적합성:** ★★★★☆

---

#### 방안 B: 시간 기반 방식

**개요:** 경과 시간으로 윈도우 판정

**구조:**
```
Attack1 시작 → elapsedTime = 0
매 프레임: elapsedTime += deltaTime
if (elapsedTime >= 0.15f && elapsedTime <= 0.35f) {
    // 캔슬 윈도우 (0.15초~0.35초)
    if (attackInput) → Attack2로 전이
}
```

**장점:**
- 프레임 레이트 독립적
- 직관적인 시간 단위

**단점:**
- 부동소수점 오차 가능성
- 애니메이션 프레임과 불일치 가능

**적합성:** ★★★★★ (권장)

---

#### 방안 C: 입력 버퍼 큐 방식

**개요:** 입력을 버퍼에 저장하고 적절한 시점에 소비

**구조:**
```
InputBuffer {
    queue<InputEvent> buffer;
    float bufferTime = 0.2f;  // 입력 유지 시간

    void AddInput(InputEvent e) {
        e.timestamp = currentTime;
        buffer.push(e);
    }

    bool ConsumeAttack() {
        // 유효 시간 내 공격 입력 확인 및 소비
    }
}
```

**장점:**
- 선입력 허용으로 조작감 향상
- 입력 누락 방지
- 격투 게임 스타일 구현 가능

**단점:**
- 버퍼 관리 복잡
- 의도치 않은 입력 처리 가능성

**적합성:** ★★★★☆

---

#### 방안 D: 애니메이션 이벤트 방식

**개요:** 애니메이션 특정 프레임에 이벤트 발생

**구조:**
```
애니메이션 데이터:
Attack1: {
    frame 5: "CANCEL_WINDOW_START"
    frame 12: "CANCEL_WINDOW_END"
    frame 15: "ANIMATION_END"
}

OnAnimationEvent("CANCEL_WINDOW_START") {
    canCancel = true;
}
```

**장점:**
- 애니메이션과 완벽 동기화
- 데이터 드리븐 방식
- 아티스트 친화적

**단점:**
- 애니메이션 시스템 구현 필요
- 이벤트 파싱 시스템 필요

**적합성:** ★★★☆☆ (시스템 완성도 높을 때)

---

### 3.3 권장 해결 방안

**1순위: 방안 B + 방안 C 조합**
- 시간 기반 윈도우 판정
- 간단한 입력 버퍼 (0.1초)
- 입력 응답성과 정확성 모두 확보

---

## 4. 도전 과제 3: 충돌 시스템

### 4.1 문제 정의

게임에는 여러 종류의 충돌이 존재하며, 각각 다른 처리가 필요하다.

**충돌 유형:**
1. 캐릭터 ↔ 지형 (이동 제한)
2. 캐릭터 ↔ 플랫폼 (단방향)
3. 공격 히트박스 ↔ 피격 허트박스
4. 캐릭터 ↔ 트리거 (이벤트 발생)

**주요 어려움:**
- 다양한 충돌 유형 처리
- 타일맵 기반 지형 충돌
- 슬로프 처리
- 충돌 순서 및 우선순위

### 4.2 해결 방안

#### 방안 A: 단순 AABB 충돌

**개요:** 축 정렬 경계 상자(Axis-Aligned Bounding Box) 사용

**구조:**
```
struct AABB {
    float x, y, width, height;
}

bool Intersect(AABB a, AABB b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}
```

**장점:**
- 구현 매우 단순
- 연산 비용 낮음
- 대부분의 경우 충분

**단점:**
- 회전 불가
- 복잡한 형태 표현 어려움
- 슬로프 처리 별도 필요

**적합성:** ★★★★★ (필수 기본)

---

#### 방안 B: 타일 기반 충돌

**개요:** 타일맵 좌표로 충돌 검사

**구조:**
```
// 플레이어 위치 → 타일 좌표 변환
int tileX = (int)(player.x / TILE_SIZE);
int tileY = (int)(player.y / TILE_SIZE);

// 주변 타일 검사
for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
        if (IsCollidable(tileX + dx, tileY + dy)) {
            // 충돌 처리
        }
    }
}
```

**장점:**
- 넓은 맵에서 효율적
- 타일 속성으로 다양한 충돌 처리
- 레벨 디자인과 직결

**단점:**
- 타일 크기에 의존
- 타일 경계에서 오차 가능
- 곡선/슬로프 표현 제한

**적합성:** ★★★★★ (지형 충돌 필수)

---

#### 방안 C: 분리축 정리 (SAT)

**개요:** 볼록 다각형 간 충돌 검사

**장점:**
- 회전된 도형 지원
- 정확한 충돌 판정
- 충돌 깊이/방향 계산 가능

**단점:**
- 구현 복잡
- 연산 비용 높음
- 2주 내 구현에 과도

**적합성:** ★★☆☆☆ (불필요)

---

#### 방안 D: 레이캐스트 방식

**개요:** 이동 방향으로 광선을 쏘아 충돌 검사

**구조:**
```
// 수평 이동 전 검사
RaycastHit hit = Raycast(player.pos, moveDirection, moveDistance);
if (hit.collided) {
    player.x = hit.point.x - player.width/2;
}
```

**장점:**
- 고속 이동 시 터널링 방지
- 슬로프 처리에 유용
- 정밀한 충돌 지점 계산

**단점:**
- 구현 복잡도 중간
- 여러 광선 필요 (비용 증가)

**적합성:** ★★★★☆ (슬로프/고속이동 시)

---

#### 방안 E: 충돌 레이어 시스템

**개요:** 충돌 대상을 레이어로 분류

**구조:**
```
enum Layer {
    GROUND = 1 << 0,
    PLATFORM = 1 << 1,
    PLAYER = 1 << 2,
    ENEMY = 1 << 3,
    PLAYER_ATTACK = 1 << 4,
    ENEMY_ATTACK = 1 << 5,
}

// 충돌 매트릭스
CollisionMatrix[PLAYER_ATTACK] = ENEMY;  // 플레이어 공격은 적만
CollisionMatrix[PLAYER] = GROUND | PLATFORM;  // 플레이어는 지형만
```

**장점:**
- 불필요한 충돌 검사 제거
- 명확한 충돌 규칙 정의
- 확장 용이

**단점:**
- 초기 설계 필요
- 레이어 관리 복잡해질 수 있음

**적합성:** ★★★★★ (권장)

---

### 4.3 권장 해결 방안

**조합 접근:**
1. **기본:** 방안 A (AABB) - 모든 충돌의 기본
2. **지형:** 방안 B (타일 기반) - 맵 충돌
3. **관리:** 방안 E (레이어) - 충돌 대상 필터링
4. **선택:** 방안 D (레이캐스트) - 슬로프 필요 시

---

## 5. 도전 과제 4: 패리 타이밍 판정

### 5.1 문제 정의

패리는 0.15초라는 매우 짧은 윈도우 내에서 적 공격과 동기화되어야 한다.

**주요 어려움:**
- 정밀한 타이밍 판정
- 적 공격과의 동기화
- 네트워크 지연 대응 (향후)
- 플레이어 체감 공정성

### 5.2 해결 방안

#### 방안 A: 단순 시간 윈도우

**개요:** 패리 입력 후 일정 시간 내 피격 시 성공

**구조:**
```
OnParryInput() {
    parryWindowStart = currentTime;
    isParrying = true;
}

OnEnemyAttackHit() {
    if (isParrying &&
        currentTime - parryWindowStart < 0.15f) {
        ParrySuccess();
    } else {
        TakeDamage();
    }
}
```

**장점:**
- 구현 단순
- 이해하기 쉬움

**단점:**
- 네트워크 지연에 취약
- 공격 타이밍과 무관

**적합성:** ★★★★☆

---

#### 방안 B: 공격 히트박스 기반

**개요:** 적 공격 히트박스 활성 시점에 패리 판정

**구조:**
```
// 적 공격 히트박스 활성화 시
OnAttackHitboxActive(Enemy attacker) {
    if (player.isParrying &&
        Distance(player, attacker) < PARRY_RANGE) {
        ParrySuccess(attacker);
        attacker.InterruptAttack();
    }
}
```

**장점:**
- 공격과 정확히 동기화
- 직관적인 판정

**단점:**
- 적 공격 타이밍 정보 필요
- 구현 복잡도 증가

**적합성:** ★★★★★ (권장)

---

#### 방안 C: 히트스톱 활용

**개요:** 패리 성공 시 히트스톱으로 판정 보정

**구조:**
```
// 패리 성공 직전 프레임에도 판정 허용
OnEnemyAttackHit() {
    // 패리 종료 직후에도 짧은 유예 (Coyote Time 개념)
    float grace = 0.05f;
    if (isParrying ||
        currentTime - parryWindowEnd < grace) {
        ParrySuccess();
        ApplyHitStop(0.1f);  // 연출로 체감 향상
    }
}
```

**장점:**
- 플레이어 친화적
- 조작감 향상
- 연출 효과

**단점:**
- 게임이 쉬워질 수 있음
- 밸런스 조정 필요

**적합성:** ★★★★☆

---

#### 방안 D: 입력 선행 허용

**개요:** 피격 직전 입력도 패리로 인정

**구조:**
```
// 입력 버퍼에 패리가 있으면
if (inputBuffer.HasParryWithin(0.1f)) {
    // 피격 시점에 패리 발동
    ParrySuccess();
}
```

**장점:**
- 반응 시간 보정
- 초보자 친화적

**단점:**
- 패리가 너무 쉬워질 수 있음
- 숙련자에게 덜 보람

**적합성:** ★★★☆☆

---

### 5.3 권장 해결 방안

**1순위: 방안 B + 방안 C 조합**
- 공격 히트박스 기반 판정
- 약간의 유예 시간 (0.03초)
- 히트스톱으로 성공 연출

---

## 6. 도전 과제 5: 적 AI 행동 패턴

### 6.1 문제 정의

3종의 적이 각각 다른 행동 패턴을 가지며, 플레이어에 반응해야 한다.

**주요 어려움:**
- 상태 기반 AI 설계
- 플레이어 감지 로직
- 자연스러운 행동 전이
- 적 유형별 코드 재사용

### 6.2 해결 방안

#### 방안 A: 단순 상태 머신

**개요:** 플레이어와 동일한 상태 머신 적용

**구조:**
```
enum EnemyState { IDLE, PATROL, CHASE, ATTACK, HIT, DEATH }

Update() {
    switch(state) {
        case PATROL:
            MovePatrol();
            if (DetectPlayer()) state = CHASE;
            break;
        case CHASE:
            MoveTowardPlayer();
            if (InAttackRange()) state = ATTACK;
            break;
        ...
    }
}
```

**장점:**
- 구현 단순
- 예측 가능한 행동
- 디버깅 용이

**단점:**
- 복잡한 행동 표현 한계
- 적 유형마다 중복 코드

**적합성:** ★★★★★ (MVP에 적합)

---

#### 방안 B: 행동 트리 (Behavior Tree)

**개요:** 트리 구조로 행동 우선순위 정의

**구조:**
```
Root (Selector)
├── Sequence: Combat
│   ├── Condition: PlayerInRange
│   └── Action: Attack
├── Sequence: Chase
│   ├── Condition: PlayerDetected
│   └── Action: MoveToPlayer
└── Action: Patrol
```

**장점:**
- 복잡한 행동 표현 가능
- 재사용 가능한 노드
- 시각적 디버깅 가능

**단점:**
- 구현 시간 소요
- 학습 곡선
- 간단한 AI에는 과도

**적합성:** ★★★☆☆ (향후 확장용)

---

#### 방안 C: 컴포넌트 기반 AI

**개요:** AI 기능을 컴포넌트로 분리

**구조:**
```
Enemy {
    PatrolComponent patrol;
    DetectionComponent detection;
    AttackComponent attack;
}

// 각 컴포넌트 조합으로 다양한 적 생성
Type_A: patrol + melee_attack
Type_B: patrol + ranged_attack
Type_C: charge_patrol + shield_attack
```

**장점:**
- 높은 재사용성
- 새 적 유형 빠른 생성
- 관심사 분리

**단점:**
- 컴포넌트 간 통신 필요
- 초기 설계 비용

**적합성:** ★★★★☆

---

#### 방안 D: 데이터 기반 AI

**개요:** AI 파라미터를 데이터로 분리

**구조:**
```
// enemy_data.json
{
    "type_a": {
        "patrol_range": 200,
        "detect_range": 150,
        "attack_range": 50,
        "attack_delay": 0.3,
        "attack_damage": 15
    }
}

// 코드는 동일, 데이터만 다름
class Enemy {
    void LoadData(string type);
}
```

**장점:**
- 밸런스 조정 용이
- 비프로그래머 수정 가능
- 적 추가가 데이터 추가만으로 가능

**단점:**
- 파싱 시스템 필요
- 완전히 다른 행동은 코드 필요

**적합성:** ★★★★☆

---

### 6.3 권장 해결 방안

**1순위: 방안 A + 방안 D 조합**
- 단순 상태 머신으로 로직 구현
- 수치는 데이터 파일로 분리
- 2주 내 3종 적 충분히 구현 가능

---

## 7. 도전 과제 6: 프레임 독립적 게임 루프

### 7.1 문제 정의

다양한 하드웨어에서 일관된 게임 속도를 유지해야 한다.

**주요 어려움:**
- 프레임 레이트 변동 대응
- 물리 시뮬레이션 안정성
- 입력 응답성 유지
- 애니메이션 속도 일관성

### 7.2 해결 방안

#### 방안 A: 가변 시간 스텝 (Variable Timestep)

**개요:** 매 프레임 경과 시간으로 업데이트

**구조:**
```
while (running) {
    float deltaTime = CalculateDeltaTime();
    Update(deltaTime);
    Render();
}

void Update(float dt) {
    player.x += velocity.x * dt;
    player.y += velocity.y * dt;
}
```

**장점:**
- 구현 단순
- 부드러운 움직임

**단점:**
- 물리 불안정 (높은 dt에서)
- 충돌 터널링 가능
- 재현 불가능한 버그

**적합성:** ★★★☆☆

---

#### 방안 B: 고정 시간 스텝 (Fixed Timestep)

**개요:** 일정한 시간 간격으로 로직 업데이트

**구조:**
```
const float FIXED_DT = 1.0f / 60.0f;  // 60 FPS
float accumulator = 0;

while (running) {
    float frameTime = CalculateDeltaTime();
    accumulator += frameTime;

    while (accumulator >= FIXED_DT) {
        Update(FIXED_DT);
        accumulator -= FIXED_DT;
    }

    Render();
}
```

**장점:**
- 물리 안정성 보장
- 결정적 시뮬레이션
- 디버깅/재현 용이

**단점:**
- 느린 PC에서 "나선의 죽음" 가능
- 렌더링과 로직 불일치

**적합성:** ★★★★★ (권장)

---

#### 방안 C: 고정 + 보간

**개요:** 고정 스텝 + 렌더링 보간

**구조:**
```
while (running) {
    float frameTime = CalculateDeltaTime();
    accumulator += frameTime;

    while (accumulator >= FIXED_DT) {
        previousState = currentState;
        Update(FIXED_DT);
        accumulator -= FIXED_DT;
    }

    float alpha = accumulator / FIXED_DT;
    RenderState interpolated = Lerp(previousState, currentState, alpha);
    Render(interpolated);
}
```

**장점:**
- 부드러운 렌더링
- 물리 안정성 유지
- 최고의 품질

**단점:**
- 구현 복잡
- 이전 상태 저장 필요
- 메모리 사용 증가

**적합성:** ★★★★☆ (시간 여유 시)

---

### 7.3 권장 해결 방안

**1순위: 방안 B (고정 시간 스텝)**
- 60 FPS 고정
- 안정적이고 예측 가능
- 2주 내 충분히 구현 가능

**향후: 방안 C로 업그레이드**

---

## 8. 도전 과제 7: 더블 버퍼링 렌더링

### 8.1 문제 정의

WinAPI GDI로 렌더링 시 화면 깜빡임(flickering)이 발생한다.

**주요 어려움:**
- GDI 직접 렌더링의 깜빡임
- 다수 스프라이트 렌더링 성능
- 투명 처리 (알파 블렌딩)
- Z-order 정렬

### 8.2 해결 방안

#### 방안 A: 메모리 DC 더블 버퍼링

**개요:** 메모리에 그린 후 화면에 복사

**구조:**
```
// 초기화
HDC memDC = CreateCompatibleDC(hdc);
HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
SelectObject(memDC, memBitmap);

// 렌더링
FillRect(memDC, &rect, bgBrush);  // 배경
DrawSprites(memDC);               // 스프라이트
BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);  // 화면 복사
```

**장점:**
- WinAPI 표준 방식
- 구현 단순
- 추가 라이브러리 불필요

**단점:**
- 알파 블렌딩 제한
- 성능 한계

**적합성:** ★★★★★ (기본 필수)

---

#### 방안 B: GDI+ 사용

**개요:** GDI+로 고급 렌더링 기능 활용

**구조:**
```
#include <gdiplus.h>

Graphics graphics(memDC);
graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);

Image sprite(L"sprite.png");
graphics.DrawImage(&sprite, x, y, w, h);
```

**장점:**
- PNG 알파 채널 지원
- 회전, 스케일링 지원
- 안티앨리어싱 옵션

**단점:**
- GDI보다 느림
- 초기화 코드 필요

**적합성:** ★★★★★ (권장)

---

#### 방안 C: DIB Section 직접 조작

**개요:** 비트맵 메모리 직접 접근

**구조:**
```
BITMAPINFO bmi = {...};
void* pixels;
HBITMAP dib = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);

// 픽셀 직접 조작
DWORD* p = (DWORD*)pixels;
p[y * width + x] = 0xFFRRGGBB;
```

**장점:**
- 최고 성능
- 완전한 제어
- 커스텀 블렌딩 가능

**단점:**
- 구현 복잡
- 모든 렌더링 직접 구현 필요

**적합성:** ★★★☆☆ (성능 최적화 필요 시)

---

#### 방안 D: 레이어 기반 렌더링

**개요:** 레이어별로 버퍼 관리

**구조:**
```
layers[] = {
    { name: "background", zOrder: 0 },
    { name: "entities", zOrder: 1 },
    { name: "foreground", zOrder: 2 },
    { name: "ui", zOrder: 3 }
}

// 각 레이어에 렌더링 후 합성
for (layer in layers) {
    DrawLayer(layer);
    AlphaBlend(memDC, layer.buffer);
}
```

**장점:**
- 명확한 렌더링 순서
- 레이어별 효과 적용 가능
- 병렬화 가능

**단점:**
- 메모리 사용 증가
- 합성 비용

**적합성:** ★★★★☆

---

### 8.3 권장 해결 방안

**1순위: 방안 A + 방안 B 조합**
- 메모리 DC 더블 버퍼링 기본
- GDI+로 스프라이트 렌더링
- 알파 블렌딩 지원

---

## 9. 도전 과제 8: 애니메이션-로직 동기화

### 9.1 문제 정의

애니메이션의 특정 프레임에서 게임 로직이 실행되어야 한다.

**예시:**
- 공격 애니메이션 3번째 프레임에서 히트박스 활성화
- 슬라이드 2~5번째 프레임에서 무적
- 점프 애니메이션 끝에서 Fall 전환

**주요 어려움:**
- 프레임 정확도
- 애니메이션 속도 변경 대응
- 이벤트 누락 방지
- 데이터 관리

### 9.2 해결 방안

#### 방안 A: 프레임 인덱스 직접 체크

**개요:** 현재 프레임 번호로 조건 체크

**구조:**
```
class Animation {
    int currentFrame;
    float frameTimer;
    float frameTime = 0.1f;

    void Update(float dt) {
        frameTimer += dt;
        if (frameTimer >= frameTime) {
            currentFrame++;
            frameTimer = 0;
        }
    }
}

// 사용
if (attackAnim.currentFrame >= 3 && attackAnim.currentFrame <= 5) {
    hitbox.active = true;
}
```

**장점:**
- 구현 단순
- 직관적

**단점:**
- 조건문 분산
- 하드코딩
- 프레임 변경 시 수정 많음

**적합성:** ★★★☆☆

---

#### 방안 B: 애니메이션 이벤트 시스템

**개요:** 애니메이션에 이벤트 데이터 포함

**구조:**
```
struct AnimationEvent {
    int frame;
    string eventName;
}

struct AnimationData {
    vector<AnimationEvent> events;
    // Attack1: frame 3 "HITBOX_ON", frame 6 "HITBOX_OFF"
}

void OnAnimationEvent(string eventName) {
    if (eventName == "HITBOX_ON") hitbox.active = true;
    if (eventName == "HITBOX_OFF") hitbox.active = false;
}
```

**장점:**
- 데이터 드리븐
- 코드와 데이터 분리
- 확장 용이

**단점:**
- 이벤트 시스템 구현 필요
- 데이터 파일 관리

**적합성:** ★★★★★ (권장)

---

#### 방안 C: 콜백 함수 방식

**개요:** 특정 프레임에 콜백 등록

**구조:**
```
animation.OnFrame(3, []() {
    hitbox.active = true;
});

animation.OnFrame(6, []() {
    hitbox.active = false;
});

animation.OnComplete([]() {
    ChangeState(IDLE);
});
```

**장점:**
- 유연한 로직 연결
- 코드 응집도 높음

**단점:**
- 람다 캡처 관리
- 메모리 관리 주의

**적합성:** ★★★★☆

---

#### 방안 D: 애니메이션 상태 분리

**개요:** 애니메이션을 서브 상태로 분리

**구조:**
```
Attack1Animation:
    - WindUp (frame 0-2): 준비, 히트박스 OFF
    - Active (frame 3-5): 활성, 히트박스 ON
    - Recovery (frame 6-8): 회복, 히트박스 OFF

// 서브 상태 전이로 로직 처리
OnEnterSubState(Active) {
    hitbox.active = true;
}
```

**장점:**
- 명확한 상태 분리
- 로직 캡슐화

**단점:**
- 상태 수 증가
- 단순 애니메이션에는 과도

**적합성:** ★★★☆☆

---

### 9.3 권장 해결 방안

**1순위: 방안 B (애니메이션 이벤트)**
- 간단한 이벤트 시스템 구현
- JSON/텍스트 파일로 이벤트 데이터 관리
- 확장성과 유지보수성 확보

---

## 10. 권장 접근 방식 요약

| 도전 과제 | 권장 방안 | 대안 |
|-----------|-----------|------|
| 상태 머신 | 상태 패턴 (클래스 분리) | Switch문 + 함수 분리 |
| 콤보 시스템 | 시간 기반 + 입력 버퍼 | 프레임 카운터 |
| 충돌 시스템 | AABB + 타일 기반 + 레이어 | 단순 AABB만 |
| 패리 판정 | 히트박스 기반 + 유예 시간 | 시간 윈도우만 |
| 적 AI | 상태 머신 + 데이터 분리 | 단순 상태 머신 |
| 게임 루프 | 고정 시간 스텝 (60 FPS) | 가변 + dt 제한 |
| 렌더링 | 더블 버퍼링 + GDI+ | 더블 버퍼링만 |
| 애니메이션 동기화 | 이벤트 시스템 | 프레임 직접 체크 |

---

## 부록: 구현 현황

### 완료된 단계 (Day 1-4)

| 단계 | 항목 | 상태 |
|------|------|------|
| **기반** | 고정 시간 스텝 게임 루프 | ✅ |
| | 더블 버퍼링 렌더링 | ✅ |
| | 기본 입력 시스템 | ✅ |
| **플레이어** | State/Tag 시스템 (Ability 기반) | ✅ |
| | 기본 이동/점프/슬라이드 | ✅ |
| | 애니메이션 이벤트 시스템 | ✅ |
| | 3타 콤보 공격 | ✅ |
| | 공중 공격 | ✅ |
| | 패리/반격 시스템 | ✅ |
| **전투** | BoxTrace 충돌 | ✅ |
| | Layer 기반 충돌 필터링 | ✅ |
| | 피격 반응 (넉백) | ✅ |
| | 기본 적 1종 | ✅ |

### 남은 10일 계획 (Plan B)

| 일차 | 작업 | 관련 도전 과제 |
|------|------|---------------|
| Day 1 | HP 시스템 + 사망 | - |
| Day 2 | UI/HUD | - |
| Day 3 | 적 AI 개선 | #5 적 AI 행동 패턴 |
| Day 4 | 추가 적 타입 | #5 적 AI 행동 패턴 |
| Day 5 | Bile Flask 회복 | - |
| Day 6 | 체크포인트 + 부활 | - |
| Day 7 | 레벨 디자인 | - |
| Day 8 | 카메라 + 화면 효과 | - |
| Day 9-10 | 폴리싱 + 테스트 | - |

---

*보고서 끝*
