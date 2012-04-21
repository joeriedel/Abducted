// G_ScreenProjectile.cpp
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#include "G_ScreenProjectile.h"
#include <Engine/World/WorldLua.h>

namespace world {

G_ScreenProjectile::G_ScreenProjectile() : 
E_CONSTRUCT_BASE
{
	m_distance[0] = m_distance[1] = 0.f;
}

G_ScreenProjectile::~G_ScreenProjectile()
{
}

void G_ScreenProjectile::PushCallTable(lua_State *L)
{
	Entity::PushCallTable(L);
	lua_pushcfunction(L, lua_Fire);
	lua_setfield(L, -2, "NativeFire");
	LUART_REGISTER_GET(L, Distance);
}

void G_ScreenProjectile::Fire(const Vec3 &_start, const Vec3 &_end)
{
	Entity::Ref parent = m_ps.parent.lock();
	if (!parent)
		return;

	m_distance[0] = 0.f;
	m_distance[1] = (_end-_start).Magnitude();
	
	Mat4 m = Mat4::Rotation(QuatFromAngles(Vec3(0, 0, parent->ps->worldAngles[2])));
	Vec3 fwd = Vec3(1, 0, 0) * m;
	Vec3 up, left;
	FrameVecs(fwd, up, left);

	Vec3 start = _start-parent->ps->worldPos;
	Vec3 end   = _end-parent->ps->worldPos;
	end = math::Lerp(start, end, (m_distance[1]+16)/m_distance[1]);
	m_distance[1] += 16.f;

	m_start[0] = left.Dot(start);
	m_start[1] = up.Dot(start);
	m_start[2] = fwd.Dot(start);

	m_end[0] = left.Dot(end);
	m_end[1] = up.Dot(end);
	m_end[2] = fwd.Dot(end);
}

void G_ScreenProjectile::TickPhysics(
	int frame, 
	float dt, 
	const xtime::TimeSlice &time
)
{
	Entity::Ref parent = m_ps.parent.lock();
	if (!parent)
		return;

	TickOther(*parent, frame, dt, time);

	m_distance[0] += m_ps.velocity.Magnitude()*dt;

	Mat4 m = Mat4::Rotation(QuatFromAngles(Vec3(0, 0, parent->ps->worldAngles[2])));
	Vec3 fwd = Vec3(1, 0, 0) * m;
	Vec3 up, left;
	FrameVecs(fwd, up, left);

	Vec3 pos = math::Lerp(m_start, m_end, m_distance[0]/m_distance[1]);

	Vec3 start = parent->ps->worldPos+(left*m_start[0])+(up*m_start[1])+(fwd*m_start[2]);
	Vec3 end   = parent->ps->worldPos+(left*m_end[0])+(up*m_end[1])+(fwd*m_end[2]);
	
	m_ps.pos = parent->ps->worldPos+(left*pos[0])+(up*pos[1])+(fwd*pos[2]);
	m_ps.cameraPos = m_ps.pos;
	m_ps.angles.pos = LookAngles((end-start).Normalize());
	m_ps.worldAngles = WrapAngles(m_ps.originAngles + m_ps.angles.pos);
	Move(true, false);
}

int G_ScreenProjectile::lua_Fire(lua_State *L)
{
	G_ScreenProjectile *self = static_cast<G_ScreenProjectile*>(WorldLua::EntFramePtr(L, 1, true));
	Vec3 start = lua::Marshal<Vec3>::Get(L, 2, true);
	Vec3 end = lua::Marshal<Vec3>::Get(L, 3, true);

	self->Fire(start, end);

	return 0;
}

int G_ScreenProjectile::LUART_GETFN(Distance)(lua_State *L)
{
	G_ScreenProjectile *self = static_cast<G_ScreenProjectile*>(WorldLua::EntFramePtr(L, 1, true));
	lua_createtable(L, 2, 0);
	lua_pushinteger(L, 1);
	lua_pushnumber(L, self->m_distance[0]);
	lua_settable(L, -3);
	lua_pushinteger(L, 2);
	lua_pushnumber(L, self->m_distance[1]);
	lua_settable(L, -3);
	return 1;
}

} // world


namespace spawn {

void *info_projectile::Create()
{
	return new (ZWorld) world::G_ScreenProjectile();
}

} // spawn
