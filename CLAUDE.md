# Project B - Blasphemous 1 모작 프로젝트

## 프로젝트 개요
- **장르**: 2D 메트로배니아 액션 게임
- **원작**: Blasphemous 1 (The Game Kitchen, 2019)
- **개발 환경**: C++ / WinAPI / GDI+
- **사운드**: FMOD
- **도구**: Python 기반 맵 에디터

## 프로젝트 구조
```
Project_B/
├── Engine/          # Winapi game engine library
│   ├── Core/        # GameObject, Scene
│   ├── Components/  # Collider, Animator
│   ├── Managers/    # Render, Input, Collision, Camera, Sound
│   └── UI/          # UIBase, Panel, Button
├── WinApi/Game/     # 게임 로직
│   ├── Object/      # Player, Enemy, Boss, Projectile, Hazard
│   ├── Component/   # AbilitySystem, StateSystem, AIController
│   ├── Ability/     # Player/Monster/Boss 어빌리티
│   ├── Scene/       # 스테이지, 타이틀, 사망씬
│   ├── Map/         # 맵 레이어, 패럴랙스
│   ├── Manager/     # VFX, SFX, Save, GameUI
│   └── UI/          # HUD, Inventory, Overlay
├── Scripts/         # Python 맵 에디터
├── Resources/       # 이미지, 사운드, 데이터
└── Docs/            # 설계 문서
```

## 핵심 기술 특징
1. **컴포넌트 기반 아키텍처**: 상속보다 조합 우선
2. **Ability System**: 태그 기반 발동 조건 (RequiredTags, BlockedTags)
3. **State System**: 비트필드 태그로 상태 관리
4. **이벤트 기반 통신**: Delegate/MulticastDelegate 패턴
5. **데이터 주도 설계**: 애니메이션 이벤트가 게임 로직 트리거

## 구현된 주요 기능
### 플레이어
- 기본 이동, 점프, 슬라이드
- 콤보 공격, 공중 공격, 웅크려 공격
- 사다리 타기, 벽 매달리기, 레지 클라이밍
- 패리, 회복 플라스크
- 체크포인트/리스폰 시스템

### 적/보스
- Enemy_Acolyte, Enemy_Stoner
- Boss: Ten Piedad (다양한 공격 패턴)
- AI Controller 기반 행동 제어

### 시스템
- 충돌 (박스/라인 콜라이더, 원웨이 플랫폼)
- 카메라 (팔로우, 쉐이크, 히트스톱)
- 세이브/로드
- 씬 전환 (TransitionArea)

## 발표자료 구성 (10분)

### 1. 게임소개 (1p) - 1분
- 원작 Blasphemous 1 주요 특징 3가지

### 2. 기술스택 (1p) - 1분
- C++ / WinAPI / GDI+
- FMOD 사운드
- Python 맵 에디터 (가볍게 언급)

### 3. 핵심 구현 기능 (3p) - 2분

#### 3-1. 전투 시스템: 콤보 공격 + 패리
**공통**: 애니메이션 이벤트 기반 타이밍 윈도우, 히트 판정 프레임 동기화, 입력버퍼, 상태 태그로 발동 조건 제어

#### 3-2. AI 시스템: 일반 적 + 보스
**공통**: 태그 기반 상태 차단, AbilitySystem 연동, 거리 기반 행동 결정
| 일반 적 AI | 보스 AI |
|-----------|---------|
| ""| 가중치 기반 공격 선택 |
| 감지 범위/주기 최적화 | " |
| 화면 밖 업데이트 스킵 | ""|

#### 3-3. 지형 시스템: 충돌 + 클라이밍
**공통**: 콜라이더 기반 감지, 위치 스냅/보정, 상태 태그로 이동 제어
| 원웨이 플랫폼 | 레지 클라임 |
|--------------|------------|
| 방향별 충돌 판정 | 절벽 방향 체크 |
| 슬로프 각도별 처리 (미끄러짐/등반) | "" |
| 낙하 터널링 보정 (프레임간 스윕) | "" |

### 4. 상세설계 (4p) - 4분
- 컴포넌트 구성
- StateSystem
- AbilitySystem
- 이벤트 기반 설계

### 5. 데모 플레이 - 2분

## 참고 자료
- `Docs/` 설계 문서
- `Docs/DesignPPT/` 발표 자료
- `Docs/Diagrams/` 다이어그램

---

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

### 3-1. 전투 시스템: 콤보 공격 + 패리 (40초)

> 전투 시스템입니다. 콤보 공격과 패리 모두 **애니메이션 이벤트 기반 타이밍 윈도우**로 동작합니다.
>
> **콤보 공격**:
> - 애니메이션에서 `ComboWindowOpen/Close` 이벤트가 발생하면 다음 입력을 받을 수 있는 윈도우가 열립니다
> - 윈도우가 열린 상태에서 공격 입력이 들어오면 `bSavedCombo = true`로 **선입력 저장**
> - 현재 애니메이션이 끝나면 저장된 입력이 있으면 다음 콤보로 연결, 없으면 종료
> - `HitCheck` 이벤트에서 실제 데미지 판정 수행 → **프레임 동기화**
>
> **패리**:
> - 마찬가지로 `ParryWindowOpen/Close` 이벤트로 패리 판정 윈도우 제어
> - 윈도우 내 피격 시 패리 성공 → 상대방 스턴 + 카운터 공격 연계
> - 데미지 타입이 Heavy면 카운터 불가, Normal이면 카운터 가능으로 분기

---

### 3-2. AI 시스템: 일반 적 + 보스 (40초)

> AI 시스템입니다. 일반 적과 보스 모두 **태그 기반 상태 차단**과 **AbilitySystem 연동**이 공통입니다.
>
> **일반 적 AI (AIController)**:
> - 순찰 ↔ 추격 상태 전환: `Tag_HasTarget` 추가 시 추격, 제거 시 순찰로 복귀
> - 감지 주기 최적화: `detectionInterval`마다 타겟 탐색 (매 프레임 X)
> - 화면 밖 업데이트 스킵: `IsOnScreen()` 체크로 불필요한 연산 방지
>
> **보스 AI (BossAIController)**:
> - **가중치 기반 공격 선택**: 각 공격에 `minRange, maxRange, weight` 설정
> - 거리 조건 만족하는 공격들 중 가중치 확률로 랜덤 선택
> - 턴어라운드 시스템: 플레이어가 뒤에 있으면 먼저 방향 전환 후 공격
> - 아레나 경계 제한: `arenaMinX/MaxX`를 벗어나지 않도록 이동 제한

---

### 3-3. 지형 시스템: 충돌 + 클라이밍 (40초)

> 지형 시스템입니다. **콜라이더 기반 감지**와 **위치 스냅/보정**이 공통입니다.
>
> **원웨이 플랫폼 (CharacterMovement)**:
> - 방향별 충돌 판정: 상승 중(`velocity.y < 0`)이면 플랫폼 통과
> - 슬로프 각도별 처리: `maxSlopeAngle` 초과 시 미끄러짐, 이하면 등반
> - **낙하 터널링 보정**: 프레임 드랍으로 플랫폼을 통과했을 때 복구
>   - 이전 프레임 발 위치 ~ 현재 발 위치 사이를 BoxTrace로 스윕
>   - 통과한 지면 발견 시 해당 지면에 스냅
>
> **레지 클라임 (Ability_LedgeClimb)**:
> - `LedgeHelper`가 절벽 방향과 높이 계산, 복수 레지 중 최상위 선택
> - 매달림 시 위치 스냅: 콜라이더 크기/오프셋 고려하여 손 위치 맞춤
> - 올라가기 완료 시 발이 플랫폼 위에 오도록 재스냅

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

---

### 4-2. 상세설계: StateSystem (1분)

> StateSystem입니다.
>
> **비트필드 태그로 상태 관리**:
> - `EStateTag` 열거형: `Tag_Grounded`, `Tag_Airborne`, `Tag_Attacking`, `Tag_Hit` 등 30개
> - 비트 OR로 복합 상태 표현: `Tag_Grounded | Tag_Attacking`
>
> **참조 카운팅**:
> - 같은 태그를 여러 곳에서 추가할 수 있음 (예: 여러 Ability가 `Tag_BlockMovement` 추가)
> - `bitCountMap`으로 카운트 관리, 0이 되어야 실제 태그 제거
>
> **상호 배타적 태그**:
> - `Tag_Grounded` 추가 시 `Tag_Airborne` 자동 제거, 반대도 동일
>
> **상태 변경 이벤트**:
> - `OnStateChanged` 델리게이트로 상태 변경 시 구독자에게 알림
> - AI가 이 이벤트를 구독하여 순찰↔추격 전환

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
