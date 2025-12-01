#include "pch.h"
#include "CCharacter.h"

#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Util/AnimEventHelper.h"
#include "Game/Manager/CMapManager.h"

// 충돌 처리에 필요한 데이터
struct CCharacter::CollisionContext
{
    CMetaMap* metaMap;
    Vec2 pos;               // 캐릭터 위치 (수정 가능)
    Vec2 pixelCenter;       // 콜라이더 중심 (픽셀 좌표)
    Vec2 colliderOffset;
    float halfWidth;
    float halfHeight;
    bool wasGrounded;
};

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
    UpdateMetaCollision();
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
    if (other->GetLayer() == Layer::Ground)
    {
        HandleGroundCollision(other);
    }
}

void CCharacter::OnCollisionStay(CCollider* other)
{
    if (other->GetLayer() == Layer::Ground)
    {
        bIsGrounded = true;
        HandleGroundCollision(other);

        // 아래로 떨어지는 속도 제거
        if (rigidbody)
        {
            Vec2 velocity = rigidbody->GetVelocity();
            if (velocity.y > 0)
            {
                velocity.y = 0.f;
                rigidbody->SetVelocity(velocity);
            }
        }
    }
}

void CCharacter::OnCollisionExit(CCollider* other)
{
    if (other->GetLayer() == Layer::Ground)
    {
        bIsGrounded = false;
    }
}

void CCharacter::UpdateGroundState()
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

void CCharacter::HandleGroundCollision(CCollider* ground)
{
    if (!collider || !rigidbody)
        return;

    // 땅을 뚫고 내려가는 것을 방지하기 위해 위치 보정
    float characterBottom = collider->GetPos().y + collider->GetScale().y / 2.f;
    float groundTop = ground->GetPos().y - ground->GetScale().y / 2.f;

    float overlap = characterBottom - groundTop;
    if (overlap > 0)
    {
        Vec2 characterPos = GetPos();
        characterPos.y -= overlap;
        SetPos(characterPos);
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
    constexpr int SLOPE_CHECK_DEPTH = 16;

    int feetY = (int)(ctx.pixelCenter.y + ctx.halfHeight + 1);
    int leftX = (int)(ctx.pixelCenter.x - ctx.halfWidth * 0.5f);
    int centerX = (int)ctx.pixelCenter.x;
    int rightX = (int)(ctx.pixelCenter.x + ctx.halfWidth * 0.5f);

    // 1. 기본 바닥 체크
    bIsGrounded = ctx.metaMap->IsGroundAt(leftX, feetY) ||
                  ctx.metaMap->IsGroundAt(centerX, feetY) ||
                  ctx.metaMap->IsGroundAt(rightX, feetY);

    // 2. 내리막길 스냅 (이전에 grounded였고 지금 아닐 때, 상승 중이 아닐 때)
    int slopeGroundY = -1;
    Vec2 velocity = rigidbody->GetVelocity();

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
    // - 수평 이동 중 오르막 파묻힘 (velocity.x != 0 && isInsideGround)
    bool isInsideGround = ctx.metaMap->IsGroundAt(centerX, feetY - 1);
    bool isMovingOnSlope = !IsNearlyZero(velocity.x) && isInsideGround;
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
    int bodyTopY = (int)(ctx.pixelCenter.y - ctx.halfHeight * 0.5f);
    int bodyCenterY = (int)ctx.pixelCenter.y;
    int bodyBottomY = (int)(ctx.pixelCenter.y + ctx.halfHeight * 0.5f);

    int wallX = (int)(ctx.pixelCenter.x + (ctx.halfWidth + 1) * direction);

    // grounded 상태에서는 하단 포인트 제외 (경사면을 벽으로 인식하지 않도록)
    bool hitWall = ctx.metaMap->IsSolid(wallX, bodyTopY) ||
                   ctx.metaMap->IsSolid(wallX, bodyCenterY) ||
                   (!bIsGrounded && ctx.metaMap->IsSolid(wallX, bodyBottomY));

    if (!hitWall)
        return;

    Vec2 velocity = rigidbody->GetVelocity();
    bool movingIntoWall = (direction < 0) ? velocity.x < 0 : velocity.x > 0;

    if (!movingIntoWall)
        return;

    velocity.x = 0.f;
    rigidbody->SetVelocity(velocity);

    // 위치 보정
    int safeX = wallX - 2 * direction;  // 벽에서 2픽셀 떨어진 위치
    ctx.pos.x = MAP->PixelToWorld((float)safeX, 0).x - ctx.halfWidth * direction - ctx.colliderOffset.x;
    SetPos(ctx.pos);
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
}
