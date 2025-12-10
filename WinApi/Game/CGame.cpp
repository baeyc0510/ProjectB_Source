#include "pch.h"
#include "CGame.h"

#include "Enum.h"
#include "Resource.h"
#include "Manager/VFXManager.h"
#include "Manager/SFXManager.h"
#include "Manager/MapManager.h"
#include "Manager/GameUIManager.h"
#include "Manager/EventBusManager.h"
#include "Object/VFXObject.h"
#include "Scene/Stage_Beginning.h"
#include "Scene/SceneTitle.h"
#include "Scene/SimpleStage.h"
#include "Scene/Stage_Boss01.h"

const Vec2 CGame::WINSTART		= Vec2(100, 100);
const Vec2 CGame::WINSIZE		= Vec2(1280, 720);	// 실제 윈도우 크기
const Vec2 CGame::VIRTUALSIZE	= Vec2(640, 360);	// 가상 해상도

const Vec2 CGame::DEFAULT_CAMERA_OFFSET = Vec2(0.f,-100.f); 
const Vec2 CGame::DEFAULT_CAMERA_DEADZONE = Vec2(100.f,100.f);
const float CGame::DEFAULT_CAMERA_SMOOTH = 5.f;

CGame::CGame()
{
	hInst	= 0;
	hWnd	= 0;
}

CGame::~CGame()
{
}

void CGame::Init(HINSTANCE hInstance)
{
	// 게임의 초기화 진행
	const UINT MAX_LOADSTRING = 100;
	WCHAR szTitle[MAX_LOADSTRING];
	WCHAR szWindowClass[MAX_LOADSTRING];
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINAPI, szWindowClass, MAX_LOADSTRING);

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	hWnd = CreateWindow(	// 윈도우 생성 및 핸들값 반환
		szWindowClass,		// 윈도우 클래스 이름
		szTitle,			// 윈도우 타이틀 이름
		WINSTYLE,			// 윈도우 스타일
		(int)WINSTART.x,	// 윈도우 화면 X
		(int)WINSTART.y,	// 윈도우 화면 Y
		(int)WINSIZE.x,		// 윈도우 가로 크기
		(int)WINSIZE.y,		// 윈도우 세로 크기
		nullptr,			// 부모 윈도우
		nullptr,			// 메뉴 핸들
		hInstance,			// 인스턴스 지정
		nullptr				// 추가 매개변수
	);

	assert(hWnd != nullptr && "Create window failed");

	// 게임 윈도우 크기(테두리, 메뉴바의 크기를 제외한 크기)를 구하기 위해 AdjustWindowRect 사용
	RECT rc = { 0, 0, (LONG)WINSIZE.x, (LONG)WINSIZE.y };

	// 윈도우 스타일에 따라 화면 컨텐츠의 크기에 맞게 나온다.
	AdjustWindowRect(&rc, WINSTYLE, FALSE);
	// 윈도우 사이즈를 설정
	SetWindowPos(hWnd, HWND_TOPMOST, (int)WINSTART.x, (int)WINSTART.y, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// 게임엔진 초기화 (가상 해상도 적용)
	SINGLE(EngineInstance)->Init(hInst, hWnd, WINSIZE, VIRTUALSIZE);
	SINGLE(TimeManager)->Init();
	SINGLE(RenderManager)->Init();
	SINGLE(InputManager)->Init();
	SINGLE(SceneManager)->Init();
	SINGLE(WorldManager)->Init();
	SINGLE(CollisionManager)->Init();
	SINGLE(PathManager)->Init();
	SINGLE(ResourceManager)->Init();
	// 리소스 경로 설정
	// x64\Debug 또는 x64\Release에서 실행되므로 두 단계 위로 올라가야 함
	SINGLE(ResourceManager)->SetResourceFolder(PATH + TEXT("\\..\\Resources\\"));

	SINGLE(CameraManager)->Init();
	SINGLE(SoundManager)->Init();
	SINGLE(UIManager)->Init();
	SINGLE(MapManager)->Init();
	SINGLE(GameUIManager)->Init();

	// Event Bus 리스너 등록
	EVENT->OnPlaySFX.Add([](GameObject* source, const FSFXEventData& data) {
		SFX->PlayOnce(data.key);
	});
	EVENT->OnPlayBGM.Add([](GameObject* source, const FBGMEventData& data) {
		SFX->PlayBGM(data.key, data.volume);
	});
	EVENT->OnStopBGM.Add([](GameObject* source) {
		SFX->StopBGM();
	});
	EVENT->OnSpawnVFX.Add([](GameObject* source, const FVFXEventData& data) {
		if (VFXObject* vfx = VFX->CreateVFX(data.key, data.pos, data.direction))
		{
			vfx->PlayVFX();
		}
	});
	EVENT->OnCameraShake.Add([](GameObject* source, const FCameraShakeEventData& data) {
		CAMERA->Shake(data.params);
	});
	EVENT->OnCameraFadeIn.Add([](GameObject* source, const FCameraFadeEventData& data) {
		CAMERA->FadeIn(data.duration);
	});
	EVENT->OnCameraFadeOut.Add([](GameObject* source, const FCameraFadeEventData& data) {
		CAMERA->FadeOut(data.duration);
	});
	EVENT->OnSetTimeScale.Add([](GameObject* source, const FTimeScaleEventData& data) {
		TIMER->SetTimeScale(data.scale, data.duration);
	});

	SINGLE(SceneManager)->AddScene((int)ESceneType::Title,	new SceneTitle());
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage01,	new Stage_Beginning());
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage02,	new SimpleStage(TEXT("Maps/stage02.json")));
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage03,	new SimpleStage(TEXT("Maps/stage03.json")));
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage04,	new SimpleStage(TEXT("Maps/stage04.json")));
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage05,	new SimpleStage(TEXT("Maps/stage05.json")));
	SINGLE(SceneManager)->AddScene((int)ESceneType::Stage_Boss01,	new Stage_Boss01());

	// 충돌 레이어 설정
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Monster);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ground);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Transition);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ladder);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Platform);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ledge);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Monster, (UINT)ELayer::Ground);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Monster, (UINT)ELayer::Platform);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Projectile, (UINT)ELayer::Ground);
	SINGLE(CollisionManager)->CheckLayer((UINT)ELayer::Projectile, (UINT)ELayer::Player);

	// 리소스 프리로드
	SINGLE(VFXManager)->PreLoad();
	SINGLE(SFXManager)->PreLoad();
	
	// 씬 시작
	SINGLE(SceneManager)->SetStartScene((int)ESceneType::Title);
}

void CGame::Run()
{
	// 게임의 동작 진행

	Input();
	Update();
	Render();
}

void CGame::Release()
{
	// 게임의 마무리 진행

	// 게임엔진 마무리
	SINGLE(EngineInstance)->Release();
	SINGLE(TimeManager)->Release();
	SINGLE(RenderManager)->Release();
	SINGLE(InputManager)->Release();
	SINGLE(SceneManager)->Release();
	SINGLE(WorldManager)->Release();
	SINGLE(CollisionManager)->Release();
	SINGLE(PathManager)->Release();
	SINGLE(ResourceManager)->Release();
	SINGLE(CameraManager)->Release();
	SINGLE(SoundManager)->Release();
	SINGLE(UIManager)->Release();
	SINGLE(MapManager)->Release();
	SINGLE(GameUIManager)->Release();
}

void CGame::Input()
{
	// 게임의 입력 진행
	SINGLE(InputManager)->Update();
}

void CGame::Update()
{
	// 게임의 처리 진행
	// 순서 주의! : 월드 매니저는 업데이트 가장 초기에 진행
	// 같은 프레임내에 모든 게임 오브젝트가 동일한 상황을 기준으로 처리하기 위해
	SINGLE(WorldManager)->Update();
	SINGLE(TimeManager)->Update();
	SINGLE(UIManager)->Update();
	SINGLE(GameUIManager)->Update();
	SINGLE(SceneManager)->Update();
	SINGLE(CameraManager)->Update();
	SINGLE(SoundManager)->Update();
	SINGLE(CollisionManager)->Update();
}

void CGame::Render()
{
	SINGLE(RenderManager)->BeginDraw();

	// 게임의 표현 진행
	SINGLE(SceneManager)->Render();
	SINGLE(CameraManager)->Render();

	// 디버그 드로우
	SINGLE(CollisionManager)->RenderDebug();

	// Game UI
	SINGLE(GameUIManager)->Render();

	// 게임의 우상단에 게임 FPS 출력 (60프레임 이상을 목표로 최적화 해야함)
	wstring frame = to_wstring(FPS);
	RENDER->SetText(10, RGB(0, 255, 0), TextAlign::Right);
	RENDER->Text(VIRTUALSIZE.x - 15, 5, frame);
	RENDER->SetText();

	SINGLE(RenderManager)->EndDraw();
}
