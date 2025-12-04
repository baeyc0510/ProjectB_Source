#include "pch.h"
#include "CCharacter.h"

#include "Game/VFXKeys.h"
#include "Game/Component/CAbilitySystem.h"
#include "Game/Component/CRigidbody.h"
#include "Game/Component/CStateSystem.h"
#include "Game/Util/AnimEventHelper.h"
#include "Game/Manager/CMapManager.h"
#include "Components/CLineCollider.h"


CCharacter::CCharacter()
    : animator(nullptr)
    , stateSystem(nullptr)
    , abilitySystem(nullptr)
    , rigidbody(nullptr)
    , collider(nullptr)
    , bIsGrounded(false)
    , bIsOnSteepSlope(false)
    , bWasOnSteepSlope(false)
    , ignoredPlatformID(0)
    , activeGroundID(0)
    , activeGroundTop(-FLT_MAX)
    , platformMinX(-FLT_MAX)
    , platformMaxX(FLT_MAX)
{
    scale = Vec2(100, 100);
}

UINT CCharacter::GetCurrentGroundID() const
{
    return activeGroundID;
}

CCharacter::~CCharacter()
{
}

wstring CCharacter::GetRandomBloodVfxKey() const
{
    return VFXKey::Blood1;
}

void CCharacter::SetIsGrounded(bool inIsGrounded)
{
    bIsGrounded = inIsGrounded;
    if (rigidbody)
    {
        rigidbody->SetGrounded(bIsGrounded);
    }
}

void CCharacter::Init()
{
    // Rigidbody
    rigidbody = new CRigidbody();
    AddChild(rigidbody);
    
    // Collider
    collider = new CBoxCollider();
    AddChild(collider);
    
    // StateSystem
    stateSystem = new CStateSystem();
    stateSystem->OnStateChanged.Add([this](EStateTag oldTags, EStateTag newTags)
    {
        OnStateChanged(oldTags, newTags);
    });
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
    // 상태 초기화 (씬 전환 시 persistent 오브젝트 상태 리셋)
    bIsGrounded = false;
    bIsOnSteepSlope = false;
    bWasOnSteepSlope = false;
    ignoredPlatformID = 0;
    activeGroundID = 0;
    activeGroundTop = -FLT_MAX;
    platformMinX = -FLT_MAX;
    platformMaxX = FLT_MAX;
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
    OnCollisionStay(other);
}

void CCharacter::OnCollisionStay(CCollider* other)
{
    ELayer layer = static_cast<ELayer>(other->GetLayer());
    bool isGround = (layer == ELayer::Ground);
    bool isPlatform = (layer == ELayer::Platform);

    // Ground 또는 Platform 레이어와의 충돌만 처리
    if (!isGround && !isPlatform)
        return;

    // 특정 플랫폼 통과 중이면 해당 플랫폼만 무시
    if (isPlatform && other->GetID() == ignoredPlatformID)
        return;

    if (!collider || !rigidbody)
        return;

    Vec2 characterColPos = collider->GetPos();
    Vec2 characterColHalf = collider->GetScale() * 0.5f;
    Vec2 velocity = rigidbody->GetVelocity();

    // 라인 콜라이더 처리
    CLineCollider* lineCollider = dynamic_cast<CLineCollider*>(other);
    if (lineCollider)
    {
        // 슬로프 충돌 처리
        float footY = characterColPos.y + characterColHalf.y;

        if (lineCollider->IsInXRange(characterColPos.x))
        {
            float slopeY = lineCollider->GetYAt(characterColPos.x);
            float penetration = footY - slopeY;

            // 원웨이 플랫폼: 상승 중이면 통과
            if (isPlatform && velocity.y < 0)
                return;

            // 슬로프 위에 있거나 약간 파고들었을 때
            if (penetration > -5.f && penetration < 50.f)
            {
                // 더 아래(Y가 큰) 슬로프를 activeGround로 선택
                if (activeGroundID == 0 || slopeY > activeGroundTop)
                {
                    activeGroundID = other->GetID();
                    activeGroundTop = slopeY;
                    // 슬로프 경계 저장
                    Vec2 start = lineCollider->GetWorldStart();
                    Vec2 end = lineCollider->GetWorldEnd();
                    platformMinX = min(start.x, end.x);
                    platformMaxX = max(start.x, end.x);
                }

                // activeGround가 아니면 스냅하지 않음
                if (other->GetID() != activeGroundID)
                    return;

                // 경사면 위에 있으므로 착지 상태
                SetIsGrounded_Internal(true);

                // 슬로프 방향 판별
                Vec2 slopeStart = lineCollider->GetWorldStart();
                Vec2 slopeEnd = lineCollider->GetWorldEnd();
                bool bIsUpRight = slopeEnd.y < slopeStart.y;  // Y가 감소하면 올라감

                float slopeAngle = lineCollider->GetSlopeAngle();
                bool bIsGoingUp = (velocity.x > 0 && bIsUpRight) || (velocity.x < 0 && !bIsUpRight);

                // 가파른 경사면
                if (slopeAngle > MAX_SLOPE_ANGLE_RAD)
                {
                    bIsOnSteepSlope = true;

                    Vec2 newPos = GetPos();
                    float snapY = lineCollider->GetYAt(newPos.x + collider->GetOffset().x);
                    newPos.y = snapY - characterColHalf.y - collider->GetOffset().y;
                    SetPos(newPos);

                    constexpr float SLIDE_SPEED = 300.f;
                    float slideDir = bIsUpRight ? -1.f : 1.f;
                    velocity.x = slideDir * SLIDE_SPEED * cosf(slopeAngle);
                    velocity.y = SLIDE_SPEED * sinf(slopeAngle);
                    rigidbody->SetVelocity(velocity);
                    return;
                }

                // 완만한 경사면
                bIsOnSteepSlope = false;

                // 상승 중이면 착지 처리 안함
                if (velocity.y < 0)
                {
                    return;
                }

                // 경사면 속도 보정: 오르막일 때 X 이동량을 cos(angle)로 보정
                Vec2 newCharacterPos = GetPos();
                if (bIsGoingUp && abs(velocity.x) > 0.1f)
                {
                    // 경사면에서는 실제 이동 거리가 더 길어지므로 X를 줄여서 보정
                    float cosAngle = cosf(slopeAngle);
                    // 줄여야 되는 x 성분 벡터
                    float velocityAdjust = velocity.x * (1.f - cosAngle) * DT;
                    newCharacterPos.x -= velocityAdjust;
                }

                // 보정된 x 위치에서의 슬로프 Y 계산 후 위로 올리기
                float adjustedSlopeY = lineCollider->GetYAt(newCharacterPos.x + collider->GetOffset().x);
                newCharacterPos.y = adjustedSlopeY - characterColHalf.y - collider->GetOffset().y;
                SetPos(newCharacterPos);

                // 하강 속도 제거
                if (velocity.y > 0)
                {
                    velocity.y = 0.f;
                    rigidbody->SetVelocity(velocity);
                }
            }
        }
        return;
    }

    // 박스 콜라이더 처리
    Vec2 otherPos = other->GetPos();
    Vec2 otherHalf = other->GetScale() * 0.5f;
    float otherTop = otherPos.y - otherHalf.y;
    float otherBottom = otherPos.y + otherHalf.y;
    
    Vec2 myPos = collider->GetPos();
    Vec2 myHalf = collider->GetScale() * 0.5f;
    float myHead = myPos.y - myHalf.y;
    float myFoot = myPos.y + myHalf.y;
    
    // 더 아래(Y가 큰) Ground를 activeGround로 선택
    if (activeGroundID == 0 || otherTop > activeGroundTop)
    {
        activeGroundID = other->GetID();
        activeGroundTop = otherTop;
        // 플랫폼 경계 저장
        platformMinX = otherPos.x - otherHalf.x;
        platformMaxX = otherPos.x + otherHalf.x;
    }
    
    // 원웨이 플랫폼: 위에서 착지할 때만 충돌
    if (isPlatform)
    {
        // 상승 중이면 통과
        if (velocity.y < 0)
            return;
        
        // 플랫폼 아래 있으면 통과
        if (myFoot > otherBottom)
            return;
    }
    
    // AABB 충돌에서 겹친 크기
    float overlapX = (characterColHalf.x + otherHalf.x) - abs(characterColPos.x - otherPos.x);
    float overlapY = (characterColHalf.y + otherHalf.y) - abs(characterColPos.y - otherPos.y);

    if (overlapX <= 0 || overlapY <= 0)
        return;

    // x축과 y축 중 겹친 크기가 더 작은 쪽으로 밀어냄
    // 1. y축 겹칩이 더 작은 경우 -> 수직 충돌 (바닥 또는 천장)
    if (overlapY <= overlapX)
    {
        // 바닥 충돌
        if (myFoot >= otherTop)
        {
            // 하강 중이거나 정지 시에만 처리
            if (velocity.y >= 0)
            {
                // 더 아래(Y가 큰) 지면을 activeGround로 선택
                if (activeGroundID == 0 || otherTop > activeGroundTop)
                {
                    activeGroundID = other->GetID();
                    activeGroundTop = otherTop;
                    // 플랫폼 경계 저장
                    platformMinX = otherPos.x - otherHalf.x;
                    platformMaxX = otherPos.x + otherHalf.x;
                }

                // activeGround가 아니면 위치 보정하지 않음
                if (other->GetID() != activeGroundID)
                    return;

                SetIsGrounded_Internal(true);

                // groundTop 기준으로 스냅 (1픽셀 침투 유지)
                Vec2 newPos = GetPos();
                newPos.y = otherTop + 1.f;
                SetPos(newPos);

                // 하강 속도 제거
                if (velocity.y > 0)
                {
                    velocity.y = 0.f;
                    rigidbody->SetVelocity(velocity);
                }
            }
        }
        // 천장 충돌
        else if (myHead <= otherBottom)
        {
            if (velocity.y < 0)
            {
                // 상승 속도 제거
                velocity.y = 0.f;
                rigidbody->SetVelocity(velocity);

                // 겹침만큼 아래로 밀어냄
                Vec2 newPos = GetPos();
                newPos.y += overlapY;
                SetPos(newPos);
            }
        }
    }
    // 2. x축 겹침이 더 작은 경우 -> 수평 충돌 (벽)
    else
    {
        // 단차가 아주 작은 경우는 그냥 넘어감.
        if (abs(otherTop - myFoot) < 2.5f)
        {
            return;
        }
        
        float pushDir = (characterColPos.x < otherPos.x) ? -1.f : 1.f;

        // 벽 방향으로 이동 중일 때만 속도 정지
        bool movingIntoWall = (pushDir < 0 && velocity.x > 0) || (pushDir > 0 && velocity.x < 0);
        if (movingIntoWall)
        {
            velocity.x = 0.f;
            rigidbody->SetVelocity(velocity);
        }

        // 겹침만큼 옆으로 밀어냄
        Vec2 newPos = GetPos();
        newPos.x += pushDir * overlapX;
        SetPos(newPos);
    }
}

void CCharacter::OnCollisionExit(CCollider* other)
{
    ELayer layer = static_cast<ELayer>(other->GetLayer());

    // Ground 또는 Platform 레이어와의 충돌 해제
    if (layer == ELayer::Ground || layer == ELayer::Platform)
    {
        // activeGround가 exit되면 리셋 (다음 프레임 Stay에서 새로 설정됨)
        if (other->GetID() == activeGroundID)
        {
            SetIsGrounded_Internal(false);
        }

        // 라인 콜라이더에서 벗어나면 미끄러짐 상태 해제
        if (layer == ELayer::Ground && dynamic_cast<CLineCollider*>(other))
        {
            bIsOnSteepSlope = false;
        }
    }
}

void CCharacter::UpdateStates()
{
    // 착지 체크
    if (bIsGrounded)
    {
        stateSystem->RemoveTag(Tag_Airborne);
        stateSystem->AddTagUnique(Tag_Grounded);
    }
    else
    {
        stateSystem->RemoveTag(Tag_Grounded);
        stateSystem->AddTagUnique(Tag_Airborne);
    }

    // 가파른 경사면 미끄러짐 상태 (변화 시에만 태그 조작)
    if (bIsOnSteepSlope && !bWasOnSteepSlope)
    {
        stateSystem->AddTag(Tag_BlockMovement);
    }
    else if (!bIsOnSteepSlope && bWasOnSteepSlope)
    {
        stateSystem->RemoveTag(Tag_BlockMovement);
    }
    bWasOnSteepSlope = bIsOnSteepSlope;
}

void CCharacter::OnStateChanged(EStateTag oldTags, EStateTag newTags)
{
    // 착지
    if (TagAdded(oldTags, newTags, Tag_Grounded))
    {
        // 착지 이벤트 트리거
        abilitySystem->TriggerEvent(EGameEvent::Landed);
    }
    // 공중
    if (TagAdded(oldTags, newTags, Tag_Airborne))
    {
        activeGroundID = 0;
        activeGroundTop = -FLT_MAX;
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

void CCharacter::SetIsGrounded_Internal(bool inIsGrounded)
{
    bIsGrounded = inIsGrounded;
    if (bIsGrounded)
    {
        // 플랫폼 통과 해제
        ignoredPlatformID = 0;
    }
    else
    {
        // 활성 그라운드 해제
        activeGroundID = 0;
        activeGroundTop = -FLT_MAX;
        platformMinX = -FLT_MAX;
        platformMaxX = FLT_MAX;
    }
    
    if (rigidbody)
    {
        rigidbody->SetGrounded(bIsGrounded);
    }
}
