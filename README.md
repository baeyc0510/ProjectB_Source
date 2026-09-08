# Blasphemous Clone

<p align="center">
  <strong>Win32 API 기반 2D 액션 플랫폼 게임 · Blasphemous 모작</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Win32_API-0078D4?style=flat-square&logo=windows&logoColor=white" alt="Win32 API">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++17">
  <img src="https://img.shields.io/badge/Audio-FMOD-000000?style=flat-square" alt="FMOD">
  <img src="https://img.shields.io/badge/Platform-Windows-0078D4?style=flat-square&logo=windows&logoColor=white" alt="Platform">
</p>

> [!IMPORTANT]
> 저작권 보호를 위해 원작의 **이미지·사운드 리소스, 시연 자료 및 실행 파일**은 저장소에서 제거했습니다.
> 따라서 이 저장소만으로는 게임을 실행할 수 없으며, 소스 코드와 시스템 설계 확인을 목적으로 공개합니다.

---

## 시연 영상

[![Blasphemous Clone 시연 영상](https://img.youtube.com/vi/KzNYZCQiYio/maxresdefault.jpg)](https://youtu.be/KzNYZCQiYio)

---

## 문서 자료

[📄 문서 자료 바로가기 / 다운로드](<Docs/[배유찬]블래스퍼머스1 모작.pdf>)

---

## 게임 소개

`Blasphemous Clone`은 Win32 API와 C++로 구현한 2D 액션 플랫폼 게임입니다.
원작의 이동과 전투 감각, 스테이지 탐색, 일반 몬스터 전투 및 **Ten Piedad** 보스전을 재현했습니다.

---

## 개발 정보

| 항목 | 내용 |
|:---|:---|
| 장르 | 2D 액션 플랫폼 / 메트로배니아 |
| 원작 | Blasphemous |
| 기술 | Win32 API / C++17 / GDI |
| 사운드 | FMOD |
| 플랫폼 | Windows |
| 개발 인원 | 프로그래머 1인 |
| 개발 기간 | 2025.11.24 ~ 2025.12.14 |

## 👨‍💻 개발자

<table>
  <tr>
    <td align="center">
      <strong>배유찬</strong><br><br>
      <a href="https://github.com/baeyc0510">
        <img src="https://img.shields.io/badge/GitHub-baeyc0510-181717?style=flat-square&logo=github&logoColor=white" alt="baeyc0510 GitHub">
      </a>
    </td>
  </tr>
</table>

---

## 핵심 구현

### 조작법

| 키 | 동작 |
|:---:|:---|
| `A` / `D` | 좌우 이동 |
| `W` | 사다리 오르기 / 난간 오르기 |
| `S` | 앉기 / 사다리 내려가기 |
| `Space` | 점프 |
| `Shift` | 슬라이드 |
| `K` | 공격 / 연속 입력 시 콤보 공격 |
| `J` | 패링 |
| `E` | 상호작용 |
| `F` | 회복 플라스크 사용 |
| `I` | 인벤토리 열기 / 닫기 |
| `Esc` | 열린 UI 닫기 |
| `F2` | 충돌체 디버그 표시 전환 |

타이틀 화면에서는 `↑` / `↓`로 메뉴를 선택하고 `Enter`로 결정합니다.
사망 화면에서는 `Space`로 마지막 체크포인트에서 재시작하거나 `Esc`로 타이틀 화면에 돌아갈 수 있습니다.

### 캐릭터 액션 및 전투

- 달리기, 점프, 슬라이드, 앉기, 사다리 이동, 난간 매달리기와 오르기
- 지상·공중·앉기 공격 및 3단 콤보 공격
- 패링, 반격, 피격, 넉백, 사망, 회복 플라스크 처리
- 요구·차단·취소 태그와 쿨다운을 지원하는 어빌리티 시스템
- Rigidbody와 박스·라인 콜라이더 기반 이동 및 충돌 처리

### 적 및 보스 AI

- 감지 범위, 공격 범위, 순찰 범위에 따른 일반 몬스터 행동
- 플랫폼 경계를 고려한 안전한 순찰과 추격
- 화면 밖 AI 업데이트를 제한하는 거리 기반 최적화
- 거리, 쿨다운, 가중치를 조합한 Ten Piedad 보스 패턴 선택
- 베기, 밟기, 가시 투사체, 지면 강타 등 보스 어빌리티 구현

### 데이터 기반 스테이지

- JSON으로 배경·메인·전경 레이어와 패럴랙스 정보 관리
- 스폰 지점, 체크포인트, 씬 전환 영역, 충돌체, 월드 오브젝트 배치
- 사다리, 원웨이 플랫폼, 난간, 위험 지형 등 태그 기반 환경 구성
- 체크포인트 활성화와 사망 후 리스폰 흐름 구현

### 제작 도구

- Pygame 기반 맵 에디터로 레이어, 충돌체, 스폰, 체크포인트, 전환 영역 편집
- 스프라이트 시트에서 애니메이션 프레임·피벗·이벤트를 생성하는 애니메이션 도구
- 편집 결과를 JSON으로 저장해 런타임에서 바로 로드하는 리소스 파이프라인

---

## 프로젝트 구조

```text
ProjectB_Source/
├─ Engine/                # 엔진 모듈
│  ├─ Asset/              # 이미지·애니메이션·사운드 리소스 로더
│  ├─ Components/         # 애니메이터 및 충돌 컴포넌트
│  ├─ Core/               # GameObject·Component·Scene
│  ├─ Managers/           # 렌더링·입력·충돌·카메라 등
│  └─ UI/                 # 공통 UI 요소
├─ Blas/                  # 게임 로직 모듈
│  └─ Game/
│     ├─ Ability/         # 플레이어·몬스터·보스 행동
│     ├─ Component/       # 상태·능력·이동·AI 시스템
│     ├─ Map/             # JSON 맵 로더
│     ├─ Object/          # 캐릭터·월드·투사체·위험 요소
│     ├─ Scene/           # 타이틀·스테이지·보스전·사망 씬
│     └─ UI/              # HUD·인벤토리·메뉴
├─ Resources/             # 애니메이션·맵 JSON 데이터
├─ Scripts/               # 맵·애니메이션 제작 도구
├─ External/              # FMOD 및 JSON 라이브러리
└─ Docs/                  # 설계 다이어그램
```

---

## 개발 범위

본 프로젝트는 학습 목적으로 제작한 비상업적 팬 프로젝트입니다.
원작 전체가 아닌 일부 스테이지와 Ten Piedad 보스전을 중심으로 핵심 이동·전투·연출을 구현했습니다.
