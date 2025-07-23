﻿#include "Player.hpp"

#include "GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"

Player::Player(Game* owner): Entity(owner)
{
    m_camera         = new Camera();
    m_camera->m_mode = eMode_Perspective;
    m_camera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
}

Player::~Player()
{
    POINTER_SAFE_DELETE(m_camera)
}

void Player::Update(float deltaSeconds)
{
    Vec2 cursorDelta = g_theInput->GetCursorClientDelta();
    //printf(Stringf("%f %f \n", cursorDelta.x, cursorDelta.y).c_str());


    m_orientation.m_yawDegrees += -cursorDelta.x * 0.125f;
    m_orientation.m_pitchDegrees += -cursorDelta.y * 0.125f;

    const XboxController& controller = g_theInput->GetController(0);
    float                 speed      = 2.0f;

    Vec2  leftStickPos  = controller.GetLeftStick().GetPosition();
    Vec2  rightStickPos = controller.GetRightStick().GetPosition();
    float leftStickMag  = controller.GetLeftStick().GetMagnitude();
    float rightStickMag = controller.GetRightStick().GetMagnitude();
    float leftTrigger   = controller.GetLeftTrigger();
    float rightTrigger  = controller.GetRightTrigger();

    if (rightStickMag > 0.f)
    {
        m_orientation.m_yawDegrees += -(rightStickPos * speed * rightStickMag * 0.125f).x;
        m_orientation.m_pitchDegrees += -(rightStickPos * speed * rightStickMag * 0.125f).y;
    }

    if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT) || controller.IsButtonDown(XBOX_BUTTON_A))
    {
        speed *= 10.f;
    }

    m_orientation.m_rollDegrees += leftTrigger * 0.125f * deltaSeconds * speed;
    m_orientation.m_rollDegrees -= rightTrigger * 0.125f * deltaSeconds * speed;


    if (g_theInput->IsKeyDown('Q'))
    {
        m_orientation.m_rollDegrees += 0.125f;
    }

    if (g_theInput->IsKeyDown('E'))
    {
        m_orientation.m_rollDegrees -= 0.125f;
    }


    //m_orientation.m_yawDegrees   = GetClamped(m_orientation.m_yawDegrees, -85.f, 85.f);
    m_orientation.m_pitchDegrees = GetClamped(m_orientation.m_pitchDegrees, -85.f, 85.f);
    m_orientation.m_rollDegrees  = GetClamped(m_orientation.m_rollDegrees, -45.f, 45.f);


    Vec3 forward, left, up;
    m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);

    //printf("x: %f, y: %f\n", leftStickPos.x, leftStickPos.y);

    m_position += (leftStickPos * speed * leftStickMag * deltaSeconds).y * forward;
    m_position += -(leftStickPos * speed * leftStickMag * deltaSeconds).x * left;

    if (g_theInput->IsKeyDown('W'))
    {
        m_position += forward * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('S'))
    {
        m_position -= forward * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('A'))
    {
        m_position += left * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('D'))
    {
        m_position -= left * speed * deltaSeconds;
    }

    if (g_theInput->IsKeyDown('Z') || controller.IsButtonDown(XBOX_BUTTON_RS))
    {
        m_position.z -= deltaSeconds * speed;
    }

    if (g_theInput->IsKeyDown('C') || controller.IsButtonDown(XBOX_BUTTON_LS))
    {
        m_position.z += deltaSeconds * speed;
    }

    m_camera->SetPerspectiveView(2.0f, 60.f, 0.1f, 100.f);
    Mat44 ndcMatrix;
    ndcMatrix.SetIJK3D(Vec3(0, 0, 1), Vec3(-1, 0, 0), Vec3(0, 1, 0));

    m_camera->SetPosition(m_position);
    m_camera->SetOrientation(m_orientation);


    m_camera->SetCameraToRenderTransform(ndcMatrix);
    
    Entity::Update(deltaSeconds);
}

void Player::Render() const
{
    g_theRenderer->BeginCamera(*m_camera);
    g_theRenderer->EndCamera(*m_camera);
}
