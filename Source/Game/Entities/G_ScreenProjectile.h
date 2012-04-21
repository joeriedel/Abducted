// G_ScreenProjectile.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#pragma once

#include <Engine/World/Entity.h>
#include <Runtime/PushPack.h>

namespace world {

class G_ScreenProjectile : public Entity
{
	E_DECL_BASE(Entity);
public:

	G_ScreenProjectile();
	virtual ~G_ScreenProjectile();

	void Fire(const Vec3 &start, const Vec3 &end);

protected:
	
	virtual void PushCallTable(lua_State *L);

	virtual void TickPhysics(
		int frame, 
		float dt, 
		const xtime::TimeSlice &time
	);

private:

	ENT_DECL_GET(Distance);

	RAD_DECLARE_GET(start, const Vec3&) { return m_start; }
	RAD_DECLARE_SET(start, const Vec3&) { m_start = value; }
	RAD_DECLARE_GET(end, const Vec3&) { return m_end; }
	RAD_DECLARE_SET(end, const Vec3&) { m_end = value; }

	static int lua_Fire(lua_State *L);

	Vec3 m_start;
	Vec3 m_end;
	float m_distance[2];
	
};

} // world

E_DECL_SPAWN(RADNULL_API, info_projectile)

#include <Runtime/PopPack.h>
