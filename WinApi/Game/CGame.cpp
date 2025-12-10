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
	SINGLE(CEngine)->Init(hInst, hWnd, WINSIZE, VIRTUALSIZE);
	SINGLE(CTimeManager)->Init();
	SINGLE(CRenderManager)->Init();
	SINGLE(CInputManager)->Init();
	SINGLE(CSceneManager)->Init();
	SINGLE(CWorldManager)->Init();
	SINGLE(CCollisionManager)->Init();
	SINGLE(CPathManager)->Init();
	SINGLE(CResourceManager)->Init();
	// 리소스 경로 설정
	// x64\Debug 또는 x64\Release에서 실행되므로 두 단계 위로 올라가야 함
	SINGLE(CResourceManager)->SetResourceFolder(PATH + TEXT("\\..\\Resources\\"));

	SINGLE(CCameraManager)->Init();
	SINGLE(CSoundManager)->Init();
	SINGLE(CUIManager)->Init();
	SINGLE(MapManager)->Init();
	SINGLE(GameUIManager)->Init();

	// Event Bus 리스너 등록
	EVENT->OnPlaySFX.Add([](CGameObject* source, const wstring& key) {
		SFX->PlayOnce(key);
	});
	EVENT->OnPlayBGM.Add([](CGameObject* source, const wstring& key, float volume) {
		SFX->PlayBGM(key, volume);
	});
	EVENT->OnStopBGM.Add([](CGameObject* source) {
		SFX->StopBGM();
	});
	EVENT->OnSpawnVFX.Add([](CGameObject* source, const wstring& key, Vec2 pos, int dir) {
		if (VFXObject* vfx = VFX->CreateVFX(key, pos, dir))
		{
			vfx->PlayVFX();
		}
	});
	EVENT->OnCameraShake.Add([](CGameObject* source, const FShakeParams& params) {
		CAMERA->Shake(params);
	});
	EVENT->OnCameraFadeIn.Add([](CGameObject* source, float duration) {
		CAMERA->FadeIn(duration);
	});
	EVENT->OnCameraFadeOut.Add([](CGameObject* source, float duration) {
		CAMERA->FadeOut(duration);
	});
	EVENT->OnSetTimeScale.Add([](CGameObject* source, float scale, float duration) {
		TIMER->SetTimeScale(scale, duration);
	});

	// TODO : 씬 추가
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Title,	new SceneTitle());
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage01,	new Stage_Beginning());
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage02,	new SimpleStage(TEXT("Maps/stage02.json")));
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage03,	new SimpleStage(TEXT("Maps/stage03.json")));
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage04,	new SimpleStage(TEXT("Maps/stage04.json")));
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage05,	new SimpleStage(TEXT("Maps/stage05.json")));
	SINGLE(CSceneManager)->AddScene((int)ESceneType::Stage_Boss01,	new Stage_Boss01());

	// 충돌 레이어 설정
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Monster);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ground);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Transition);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ladder);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Platform);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Player, (UINT)ELayer::Ledge);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Monster, (UINT)ELayer::Ground);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Monster, (UINT)ELayer::Platform);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Projectile, (UINT)ELayer::Ground);
	SINGLE(CCollisionManager)->CheckLayer((UINT)ELayer::Projectile, (UINT)ELayer::Player);

	// 리소스 프리로드
	SINGLE(VFXManager)->PreLoad();
	SINGLE(SFXManager)->PreLoad();
	
	// 씬 시작
	SINGLE(CSceneManager)->SetStartScene((int)ESceneType::Title);
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
	SINGLE(CEngine)->Release();
	SINGLE(CTimeManager)->Release();
	SINGLE(CRenderManager)->Release();
	SINGLE(CInputManager)->Release();
	SINGLE(CSceneManager)->Release();
	SINGLE(CWorldManager)->Release();
	SINGLE(CCollisionManager)->Release();
	SINGLE(CPathManager)->Release();
	SINGLE(CResourceManager)->Release();
	SINGLE(CCameraManager)->Release();
	SINGLE(CSoundManager)->Release();
	SINGLE(CUIManager)->Release();
	SINGLE(MapManager)->Release();
	SINGLE(GameUIManager)->Release();
}

void CGame::Input()
{
	// 게임의 입력 진행
	SINGLE(CInputManager)->Update();
}

void CGame::Update()
{
	// 게임의 처리 진행
	// 순서 주의! : 월드 매니저는 업데이트 가장 초기에 진행
	// 같은 프레임내에 모든 게임 오브젝트가 동일한 상황을 기준으로 처리하기 위해
	SINGLE(CWorldManager)->Update();
	SINGLE(CTimeManager)->Update();
	SINGLE(CUIManager)->Update();
	SINGLE(GameUIManager)->Update();
	SINGLE(CSceneManager)->Update();
	SINGLE(CCameraManager)->Update();
	SINGLE(CSoundManager)->Update();
	SINGLE(CCollisionManager)->Update();
}

void CGame::Render()
{
	SINGLE(CRenderManager)->BeginDraw();

	// 게임의 표현 진행
	SINGLE(CSceneManager)->Render();
	SINGLE(CCameraManager)->Render();

	// 디버그 드로우
	SINGLE(CCollisionManager)->RenderDebug();

	// Game UI
	SINGLE(GameUIManager)->Render();

	// 게임의 우상단에 게임 FPS 출력 (60프레임 이상을 목표로 최적화 해야함)
	wstring frame = to_wstring(FPS);
	RENDER->SetText(10, RGB(0, 255, 0), TextAlign::Right);
	RENDER->Text(VIRTUALSIZE.x - 15, 5, frame);
	RENDER->SetText();

	SINGLE(CRenderManager)->EndDraw();
}
