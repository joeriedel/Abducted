// G_ScreenView.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#include "G_ScreenView.h"
#include <Engine/World/World.h>
#include <algorithm>
#undef max

namespace world {

G_ScreenView::G_ScreenView() : 
E_CONSTRUCT_BASE,
m_enabled(false),
m_first(true),
m_target(Vec3::Zero),
m_targetVel(Vec3::Zero),
m_clip(Vec3::Zero)
{
}

G_ScreenView::~G_ScreenView()
{
}

ENT_GETSET(G_ScreenView, ScreenControlSpring, physics::Spring, m_spring);
ENT_GETSET(G_ScreenView, ScreenControlVertex, physics::SpringVertex, m_vertex);
ENT_GETSET(G_ScreenView, ScreenControlTarget, Vec3, m_target);
ENT_GETSET(G_ScreenView, ScreenControlClip, Vec3, m_clip);
ENT_GETSET(G_ScreenView, ScreenControlTargetVelocity, Vec3, m_targetVel);
ENT_GETSET(G_ScreenView, ScreenControlEnabled, bool, m_enabled);
ENT_GET(G_ScreenView, ScreenControlVertexVelocity, Vec3, m_vertex.vel);
ENT_GET(G_ScreenView, ScreenControlVertexForce, Vec3, m_vertex.force);
ENT_GET(G_ScreenView, ScreenControlVertexLength, float, m_vertex.length);

void G_ScreenView::TickPhysics(
	int frame, 
	float dt, 
	const xtime::TimeSlice &time
)
{
	if (!m_enabled)
	{ // no screen controls
		Entity::TickPhysics(frame, dt, time);
		return;
	}

	Entity::Ref parent = m_ps.parent.lock();
	if (parent)
	{
		TickOther(*parent, frame, dt, time);
		m_ps.origin = parent->ps->worldPos;
		m_ps.originAngles = parent->ps->worldAngles;
	}

	m_target += m_targetVel*dt;
	for (int i = 0; i < 3; ++i)
		m_target[i] = math::Clamp(m_target[i], -m_clip[i], m_clip[i]);

	m_vertex.Update(dt, m_target, m_spring);

	m_ps.worldAngles = WrapAngles(m_ps.originAngles + m_ps.angles.pos);
	m_ps.cameraAngles = m_ps.worldAngles;

	Mat4 m = Mat4::Rotation(QuatFromAngles(Vec3(0, 0, m_ps.worldAngles[2])));
	Vec3 fwd = Vec3(1, 0, 0) * m;
	Vec3 up, left;
	FrameVecs(fwd, up, left);

	m_ps.pos = (left*m_vertex.pos[0])+(up*m_vertex.pos[1])+(fwd*m_vertex.pos[2]);
	m_ps.cameraPos = m_ps.origin;
	Move(true, true);
}

void G_ScreenView::PushCallTable(lua_State *L)
{
	Entity::PushCallTable(L);
	LUART_REGISTER_GETSET(L, ScreenControlSpring);
	LUART_REGISTER_GETSET(L, ScreenControlVertex);
	LUART_REGISTER_GETSET(L, ScreenControlTarget);
	LUART_REGISTER_GETSET(L, ScreenControlClip);
	LUART_REGISTER_GETSET(L, ScreenControlTargetVelocity);
	LUART_REGISTER_GETSET(L, ScreenControlEnabled);
	LUART_REGISTER_GET(L, ScreenControlVertexVelocity);
	LUART_REGISTER_GET(L, ScreenControlVertexForce);
	LUART_REGISTER_GET(L, ScreenControlVertexLength);
}

} // world
