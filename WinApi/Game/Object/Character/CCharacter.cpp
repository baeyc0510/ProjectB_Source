#include "pch.h"
#include "CCharacter.h"

#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Util/AnimEventHelper.h"
#include "Game/Manager/CMapManager.h"


CCharacter::CCharacter()
    : animator(nullptr)
    , stateSystem(nullptr)
    , abilitySystem(nullptr)
    , rigidbody(nullptr)
    , collider(nullptr)
    , bIsGrounded(false)
{
    scale = Vec2(100, 100);
}

CCharacter::~CCharacter()
{
}

wstring CCharacter::GetRandomBloodVfxKey() const
{
    return VFXKey::Blood1;
}

void CCharacter::Init()
{
    // StateSystem
    stateSystem = new CStateSystem();
    AddChild(stateSystem);

    // AbilitySystem
    abilitySystem = new CAbilitySystem();
    AddChild(abilitySystem);

    // Animator
    animator = new CAnimator();
    AddChild(animator);

    // Animator -> AbilitySystem 이벤트 연결
    AnimEventHelper::ConnectAbilitySystem(animator, abilitySystem);
}

void CCharacter::OnEnable()
{
}

void CCharacter::Update()
{
}

void CCharacter::Render()
{
}

void CCharacter::OnDisable()
{
}

void CCharacter::Release()
{
}

void CCharacter::OnCollisionEnter(CCollider* other)
{
}

void CCharacter::OnCollisionStay(CCollider* other)
{
}

void CCharacter::OnCollisionExit(CCollider* other)
{
}

void CCharacter::UpdateStates()
{
    // 착지 체크
    if (bIsGrounded && stateSystem->HasTag(Tag_Airborne))
    {
        stateSystem->RemoveTag(Tag_Airborne);
        stateSystem->AddTagUnique(Tag_Grounded);
    }
    if (!bIsGrounded)
    {
        stateSystem->RemoveTag(Tag_Grounded);
        stateSystem->AddTagUnique(Tag_Airborne);
    }
}


void CCharacter::AddAnimation(const wstring& aniName, const wstring& path, bool bShouldRepeat)
{
    assert(animator);
    CAnimation* animation = LOADANIMATION(name + L"_" + aniName, path);
    assert(animation);
    animation->SetRepeat(bShouldRepeat);
    animator->AddAnimation(aniName, animation);
}

// 바닥 충돌 처리
void CCharacter::ProcessGroundCollision(CollisionContext& ctx)
{
    constexpr int SLOPE_CHECK_DEPTH = 4;

    int feetY = (int)(ctx.pixelCenter.y + ctx.halfHeight + 1);
    int leftX = (int)(ctx.pixelCenter.x - ctx.halfWidth * 0.5f);
    int centerX = (int)ctx.pixelCenter.x;
    int rightX = (int)(ctx.pixelCenter.x + ctx.halfWidth * 0.5f);

    // 1. 기본 바닥 체크
    bIsGrounded = ctx.metaMap->IsGroundAt(leftX, feetY) ||
                  ctx.metaMap->IsGroundAt(centerX, feetY) ||
                  ctx.metaMap->IsGroundAt(rightX, feetY);

    // 2. 내리막길 스냅
    int slopeGroundY = -1;
    Vec2 velocity = rigidbody->GetVelocity();

    //이전에 grounded였고 지금 아닐 때, 상승 중이 아닐 때
    if (ctx.wasGrounded && !bIsGrounded && velocity.y >= 0)
    {
        for (int checkY = feetY; checkY < feetY + SLOPE_CHECK_DEPTH; ++checkY)
        {
            if (ctx.metaMap->IsGroundAt(leftX, checkY) ||
                ctx.metaMap->IsGroundAt(centerX, checkY) ||
                ctx.metaMap->IsGroundAt(rightX, checkY))
            {
                bIsGrounded = true;
                slopeGroundY = checkY;
                break;
            }
        }
    }

    // 3. 착지 처리
    if (!bIsGrounded)
    {
        rigidbody->SetGrounded(false);
        return;
    }

    rigidbody->SetGrounded(true);

    // 4. 위치 보정 필요 여부 판단
    // - 하강 중 (velocity.y > 0)
    // - 내리막 스냅 발생 (slopeGroundY > 0)
    // - 수평 이동 중 오르막 파묻힘 (velocity.x != 0 && isInsideGround && 전방에 벽 없음)
    bool isInsideGround = ctx.metaMap->IsGroundAt(centerX, feetY - 1);

    // 전방에 수직 벽이 있는지 체크 (경사면과 구분)
    bool hasWallAhead = false;
    if (!IsNearlyZero(velocity.x) && isInsideGround)
    {
        int moveDir = (velocity.x > 0) ? 1 : -1;
        int wallCheckX = (int)(ctx.pixelCenter.x + (ctx.halfWidth + 1) * moveDir);
        int bodyMidY = (int)ctx.pixelCenter.y;

        // 몸통 중앙 높이에 벽이 있으면 수직 벽
        hasWallAhead = ctx.metaMap->IsSolid(wallCheckX, bodyMidY);
    }

    bool isMovingOnSlope = !IsNearlyZero(velocity.x) && isInsideGround && !hasWallAhead;
    bool needsCorrection = velocity.y > 0 || slopeGroundY > 0 || isMovingOnSlope;

    if (needsCorrection)
    {
        int searchStartY = (slopeGroundY > 0) ? slopeGroundY : feetY;
        int groundY = searchStartY - 1;

        while (ctx.metaMap->IsGroundAt(centerX, groundY) && groundY > searchStartY - 20)
            groundY--;

        ctx.pos.y = MAP->PixelToWorld(0, (float)groundY).y - ctx.colliderOffset.y - ctx.halfHeight;
        SetPos(ctx.pos);
    }

    // 5. 하강 속도 제거
    if (velocity.y > 0)
    {
        velocity.y = 0.f;
        rigidbody->SetVelocity(velocity);
    }
}

// 벽 충돌 처리 (direction: -1 = 왼쪽, 1 = 오른쪽)
void CCharacter::ProcessWallCollision(CollisionContext& ctx, int direction)
{
    int bodyCenterY = (int)ctx.pixelCenter.y;

    // 1. 벽 바로 앞에 있는지 체크 (1픽셀 앞)
    int aheadX = (int)(ctx.pixelCenter.x + (ctx.halfWidth + 1) * direction);
    bool wallAhead = ctx.metaMap->IsSolid(aheadX, bodyCenterY);

    // 2. 벽 안에 있는지 체크 (콜라이더 가장자리)
    int edgeX = (int)(ctx.pixelCenter.x + ctx.halfWidth * direction);
    bool insideWall = ctx.metaMap->IsSolid(edgeX, bodyCenterY);

    if (!wallAhead && !insideWall)
        return;

    // 벽 방향으로 이동 중이면 속도 정지
    Vec2 velocity = rigidbody->GetVelocity();
    bool movingIntoWall = (direction < 0) ? velocity.x < 0 : velocity.x > 0;
    if (movingIntoWall)
    {
        velocity.x = 0.f;
        rigidbody->SetVelocity(velocity);
    }

    // 벽 안에 있을 때만 밖으로 밀어냄
    if (insideWall)
    {
        int safeX = edgeX;
        constexpr int MAX_SEARCH = 20;
        for (int i = 0; i < MAX_SEARCH; ++i)
        {
            if (!ctx.metaMap->IsSolid(safeX, bodyCenterY))
                break;
            safeX -= direction;
        }

        ctx.pos.x = MAP->PixelToWorld((float)safeX, 0).x - ctx.halfWidth * direction - ctx.colliderOffset.x;
        ctx.pixelCenter.x = MAP->WorldToPixel(ctx.pos + ctx.colliderOffset).x;
        SetPos(ctx.pos);
    }
}

// 천장 충돌 처리
void CCharacter::ProcessCeilingCollision(CollisionContext& ctx)
{
    int headY = (int)(ctx.pixelCenter.y - ctx.halfHeight - 1);
    int leftX = (int)(ctx.pixelCenter.x - ctx.halfWidth * 0.5f);
    int centerX = (int)ctx.pixelCenter.x;
    int rightX = (int)(ctx.pixelCenter.x + ctx.halfWidth * 0.5f);

    bool hitCeiling = ctx.metaMap->IsSolid(leftX, headY) ||
                      ctx.metaMap->IsSolid(centerX, headY) ||
                      ctx.metaMap->IsSolid(rightX, headY);

    if (!hitCeiling)
        return;

    Vec2 velocity = rigidbody->GetVelocity();
    if (velocity.y >= 0)
        return;

    velocity.y = 0.f;
    rigidbody->SetVelocity(velocity);

    // 위치 보정: 천장 아래로 밀어냄
    int safeY = headY + 1;
    for (int i = 0; i < 10 && ctx.metaMap->IsSolid(centerX, safeY); ++i)
        safeY++;

    ctx.pos.y = MAP->PixelToWorld(0, (float)safeY).y + ctx.halfHeight - ctx.colliderOffset.y;
    SetPos(ctx.pos);
}

// 메타맵 충돌 처리
void CCharacter::UpdateMetaCollision()
{
    if (!collider || !rigidbody)
        return;

    CMetaMap* metaMap = MAP->GetMetaMap();
    if (!metaMap || !metaMap->IsLoaded())
        return;

    // 컨텍스트 초기화
    Vec2 pos = GetPos();
    Vec2 colliderOffset = collider->GetOffset();
    Vec2 colliderScale = collider->GetScale();
    Vec2 colliderCenter = pos + colliderOffset;

    CollisionContext ctx;
    ctx.metaMap = metaMap;
    ctx.pos = pos;
    ctx.pixelCenter = MAP->WorldToPixel(colliderCenter);
    ctx.colliderOffset = colliderOffset;
    ctx.halfWidth = colliderScale.x / 2.f;
    ctx.halfHeight = colliderScale.y / 2.f;
    ctx.wasGrounded = bIsGrounded;

    // 충돌 처리
    ProcessGroundCollision(ctx);    // 바닥
    ProcessWallCollision(ctx, -1);  // 왼쪽
    ProcessWallCollision(ctx, 1);   // 오른쪽
    ProcessCeilingCollision(ctx);   // 천장
    ProcessMetaCollision(ctx);
}
