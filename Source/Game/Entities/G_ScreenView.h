// G_ScreenView.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

// A ScreenView entity is one that can be put into
// a mode where it's motion and position is constrained to
// the screen bounds, and can strafe left/right up/down

#pragma once

#include <Engine/World/Entity.h>
#include <Engine/Physics/Spring.h>
#include <Runtime/PushPack.h>

namespace world {

class G_ScreenView : public Entity
{
	E_DECL_BASE(Entity);
public:

	G_ScreenView();
	virtual ~G_ScreenView();

	RAD_DECLARE_READONLY_PROPERTY(G_ScreenView, spring, physics::Spring*);
	RAD_DECLARE_READONLY_PROPERTY(G_ScreenView, vertex, physics::SpringVertex*);

protected:

	virtual void TickPhysics(
		int frame, 
		float dt, 
		const xtime::TimeSlice &time
	);

	virtual void PushCallTable(lua_State *L);

private:

	ENT_DECL_GETSET(ScreenControlSpring);
	ENT_DECL_GETSET(ScreenControlVertex);
	ENT_DECL_GETSET(ScreenControlTarget);
	ENT_DECL_GETSET(ScreenControlClip);
	ENT_DECL_GETSET(ScreenControlTargetVelocity);
	ENT_DECL_GETSET(ScreenControlEnabled);
	ENT_DECL_GET(ScreenControlVertexVelocity);
	ENT_DECL_GET(ScreenControlVertexForce);
	ENT_DECL_GET(ScreenControlVertexLength);

	RAD_DECLARE_GET(spring, physics::Spring*) { return &const_cast<G_ScreenView*>(this)->m_spring; }
	RAD_DECLARE_GET(vertex, physics::SpringVertex*) { return &const_cast<G_ScreenView*>(this)->m_vertex; }
	
	physics::Spring m_spring;
	physics::SpringVertex m_vertex;
	Vec3 m_target;
	Vec3 m_clip;
	Vec3 m_targetVel;
	bool m_enabled;
	bool m_first;
};

} // world

#include <Runtime/PopPack.h>

