
## 발표 스크립트

### 1. 게임소개 (1분)

> 안녕하세요. Blasphemous 1 모작 프로젝트를 발표하겠습니다.
>
> Blasphemous는 2019년 The Game Kitchen에서 개발한 2D 메트로배니아 액션 게임입니다.
>
> **원작의 주요 특징 3가지**:
> 1. **하드코어 액션**: 패리, 회피, 콤보 공격 등 타이밍 기반 전투
> 2. **메트로배니아 탐험**: 상호 연결된 맵, 다양한 지형 인터랙션 (사다리, 벽타기, 레지 클라임)
> 3. **어두운 세계관**: 종교적 모티브의 고딕 픽셀아트
>
> 이 프로젝트에서는 원작의 핵심 게임플레이인 전투와 탐험 시스템을 재현하는 것을 목표로 했습니다.

---

### 2. 기술스택 (1분)

> 기술스택입니다.
>
> - **C++ / WinAPI / GDI+**: 그래픽스 API 없이 순수 Windows API로 렌더링
> - **FMOD**: 전문 게임 오디오 미들웨어로 BGM, 효과음, 앰비언트 사운드 관리
> - **Python 맵 에디터**: 레이어 기반 맵 편집, 메타 이미지로 충돌 영역 정의, 체크포인트/씬 전환 트리거 배치
>
> 엔진과 게임 로직을 분리하여 Engine 라이브러리와 Game 프로젝트로 구성했습니다.

---

### 3. 핵심 구현 기능 (1p, 2분)

> **3-1. 전투 시스템 (콤보 + 패리)**
> - **공통**: 애니메이션 이벤트로 타이밍 윈도우 제어, 상태 태그로 발동 조건 관리
> - **콤보**: 윈도우 내 입력 → 선입력 저장 → 애니메이션 종료 시 다음 콤보 연결
> - **패리**: 윈도우 내 피격 → 패리 성공 → 데미지 타입에 따라 카운터 연계
>
> **3-2. AI 시스템 (일반 적 + 보스)**
> - **핵심**: 플레이어와의 거리 측정 → 행동 결정 (추격/공격/대기)
> - **상태 전환**: 감지 범위 진입 시 순찰→추격, 공격 범위 진입 시 공격 실행
> - **보스 추가**: 가중치 기반 공격 패턴 선택, 방향 전환(턴어라운드) 처리
>
> **3-3. 지형 시스템 (충돌 처리)**
> - **지면 착지**: 충돌 시 겹친 크기 계산 → 수직/수평 중 작은 쪽으로 밀어냄 → 지면 상단에 스냅
> - **원웨이 플랫폼**: 상승 중이면 통과, 하강 중일 때만 착지 판정
> - **낙하 터널링 보정**: 이전~현재 프레임 발 위치 사이를 BoxTrace → 통과한 지면에 스냅

---

### 4-1. 상세설계: 컴포넌트 구성 (1분)

> 상세설계 첫 번째, 컴포넌트 구성입니다.
>
> **설계 철학: 상속보다 조합**
> - `Character` 클래스는 직접 기능을 구현하지 않고, 컴포넌트를 조합
> - `StateSystem`, `AbilitySystem`, `Rigidbody`, `CharacterMovement`, `StatComponent` 등
>
> **계층 구조**:
> ```
> GameObject
>   └── Character (공통 컴포넌트 보유)
>         ├── Player
>         ├── Enemy (+ AIController)
>         │     └── Enemy_Acolyte, Enemy_Stoner
>         └── Boss (+ BossAIController)
>               └── Boss_TenPiedad
> ```
>
> 새로운 캐릭터 추가 시 기존 컴포넌트 재사용, Ability만 새로 정의하면 됩니다.
>
> **장점**: 기능 재사용성 높음, 새 캐릭터 추가 시 조합만으로 구현 가능
> **단점**: 컴포넌트 간 의존성 관리 필요, 초기 설계 비용 높음

---

### 4-2. 상세설계: StateSystem (1분)

> StateSystem입니다.
>
> **비트필드 태그로 상태 관리**:
> - `EStateTag` 열거형: `Tag_Grounded`, `Tag_Airborne`, `Tag_Attacking`, `Tag_Hit` 등
> - 비트 OR로 복합 상태 표현: `Tag_Grounded | Tag_Attacking`
>
> **참조 카운팅**:
> - 같은 태그를 여러 곳에서 추가할 수 있음 (예: 여러 Ability가 `Tag_BlockMovement` 추가)
> - `bitCountMap`으로 카운트 관리, 0이 되어야 실제 태그 제거
>
> **상태 변경 이벤트**:
> - `OnStateChanged` 델리게이트로 상태 변경 시 구독자에게 알림
> - AI가 이 이벤트를 구독하여 순찰↔추격 전환
>
> **장점**: O(1) 상태 검사, 복합 상태 표현 간결, bool 플래그 난립 방지
> **단점**: 태그 개수 제한 (32개), 디버깅 시 비트 해석 필요

---

### 4-3. 상세설계: AbilitySystem (1분)

> AbilitySystem입니다.
>
> **태그 기반 발동 조건** (슬라이드에 코드 첨부):
> ```cpp
> EStateTag required = ability->GetRequiredTags();  // 필수 태그
> EStateTag blocked = ability->GetBlockedTags();    // 차단 태그
> if (!stateSystem->HasAllTags(required)) return false;
> if (stateSystem->HasAnyTag(blocked)) return false;
> ```
>
> 예시 - 콤보 공격:
> - `RequiredTags = Tag_Grounded` → 지상에서만 발동
> - `BlockedTags = Tag_BlockAbility | Tag_Hit` → 피격 중이면 발동 불가
>
> **Ability 생명주기**:
> 1. `TryActivateAbility()` → 조건 검사
> 2. `ActivateAbility()` → `TagsToRemove` 제거, `TagsToAdd` 추가, `CancelTags`에 해당하는 Ability 취소
> 3. `EndAbility()` → `TagsToAdd` 자동 제거, `activeAbilities`에서 제거
>
> **장점**: 발동 조건을 선언적으로 정의, 새 Ability 추가 시 if문 없이 태그만 설정
> **단점**: 복잡한 조건은 태그 조합만으로 표현 어려움, 런타임 조건 검사 필요 시 별도 로직

---

### 4-4. 상세설계: 이벤트 기반 설계 (1분)

> 이벤트 기반 설계입니다.
>
> **WaitEvent 패턴** (슬라이드에 코드 첨부):
> ```cpp
> WaitEvent(EGameEvent::HitCheck, BIND_EVENT(this, OnHitCheck));
> WaitEvent(EGameEvent::ComboWindowOpen, BIND_EVENT(this, OnComboWindowOpen));
> WaitEvent(EGameEvent::Input_Attack_Pressed, BIND_EVENT(this, OnInputAttack));
> ```
>
> - Ability가 특정 이벤트를 구독하고, 이벤트 발생 시 콜백 실행
> - 애니메이션에서 `TriggerEvent(EGameEvent::HitCheck)` 호출 → 구독 중인 Ability의 `OnHitCheck()` 실행
>
> **전역 이벤트 버스 (EventBusManager)**:
> - `OnPlaySFX`, `OnSpawnVFX`, `OnCameraShake` 등 전역 이벤트
> - Ability가 직접 Manager를 참조하지 않고 이벤트만 발행 → 느슨한 결합
>
> **Delegate 패턴**:
> - `Delegate<>`: 단일 구독자, `MulticastDelegate<>`: 복수 구독자
> - `SafeDelegateHandle`: RAII 방식 자동 해제로 메모리 누수 방지
>
> **장점**: 시스템 간 느슨한 결합, 발행자는 구독자를 몰라도 됨
> **단점**: 실행 흐름 추적 어려움, 이벤트 순서 보장 필요 시 추가 관리

---

### 5. 데모 플레이 (2분)

> 이제 데모 플레이를 보여드리겠습니다.
>
> **시연 순서**:
> 1. 기본 이동, 점프, 슬라이드
> 2. 콤보 공격 (3단 콤보, 선입력 시연)
> 3. 패리 → 카운터 공격
> 4. 지형 인터랙션: 사다리, 레지 클라임, 원웨이 플랫폼
> 5. 일반 적과 전투 (AI 순찰/추격 확인)
> 6. 체크포인트 활성화
> 7. 보스전 (Ten Piedad) - 다양한 공격 패턴, 턴어라운드
>
> 감사합니다. 질문 받겠습니다.
