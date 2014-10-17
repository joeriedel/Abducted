/*! \file G_ManipulatableObject.cpp
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include "G_ManipulatableObject.h"
#include <Engine/World/World.h>
#include <Engine/World/WorldDraw.h>
#include <Engine/MathUtils.h>

namespace world {

G_ManipulatableObject::G_ManipulatableObject() : E_CONSTRUCT_BASE, 
m_boneIdx(-1), m_enabled(false), m_touchClass(kEntityClassBits_Any) {
}

G_ManipulatableObject::~G_ManipulatableObject() {
}

int G_ManipulatableObject::Spawn(
	const Keys &keys,
	const xtime::TimeSlice &time,
	int flags
) {
	E_SPAWN_BASE();
	SetNextTick(0.f);
	return pkg::SR_Success;
}

void G_ManipulatableObject::Tick(
	int frame,
	float dt, 
	const xtime::TimeSlice &time
) {
	Entity::Ref target = m_autoFace.lock();
	if (target) {
		Vec3 v = target->ps->worldPos - this->ps->worldPos;
		v.Normalize();
		v = LookAngles(v);
		this->ps->targetAngles[2] = v[2];
		SeekAngles(dt);
	}

	Entity::Tick(frame, dt, time);
}

void G_ManipulatableObject::TickDrawModels(float dt) {
	Entity::TickDrawModels(dt);

	if (!m_enabled || !m_model) {
		m_touching.reset();
		return;
	}
	
	CheckDamage();
}

void G_ManipulatableObject::CheckDamage() {
	BBox bounds;
	GetTouchBounds(bounds);

#if defined(WORLD_DEBUG_DRAW)
	world->draw->DebugAddEntityBBox(bounds);
#endif

	Entity::Vec touching = world->BBoxTouching(bounds, m_touchClass);
	EntityBits wasTouched(m_touching);
	m_touching.reset();

	for (Entity::Vec::iterator it = touching.begin(); it != touching.end();) {
		const Entity::Ref &entity = *it;
		m_touching.set(entity->id);
		if (wasTouched.test(entity->id)) {
			it = touching.erase(it);
		} else {
			++it;
		}
	}

	if (!touching.empty()) {
		lua_State *L = world->lua->L;
		if (PushEntityCall(L, "OnDamage")) {
			
			lua_createtable(L, (int)touching.size(), 0);
			for (Entity::Vec::const_iterator it = touching.begin(); it != touching.end(); ++it) {
				const Entity::Ref &entity = *it;
				lua_pushinteger(L, (it-touching.begin())+1);
				entity->PushEntityFrame(L);
				lua_settable(L, -3);
			}

			world->lua->Call(L, "G_ManipulatableObject::CheckDamage()", 2, 0, 0);
		}
	}
}

void G_ManipulatableObject::GetTouchBounds(BBox &bounds) {
	Vec3 pos = m_model->WorldBonePos(m_boneIdx);
	bounds = m_bounds;
	bounds.Translate(pos);
}

void G_ManipulatableObject::PushCallTable(lua_State *L) {
	Entity::PushCallTable(L);
	lua_pushcfunction(L, lua_SetTouchBone);
	lua_setfield(L, -2, "SetTouchBone");
	lua_pushcfunction(L, lua_SetTouchClassBits);
	lua_setfield(L, -2, "SetTouchClassBits");
	lua_pushcfunction(L, lua_SetTouchBoneBounds);
	lua_setfield(L, -2, "SetTouchBoneBounds");
	lua_pushcfunction(L, lua_EnableTouch);
	lua_setfield(L, -2, "EnableTouch");
	lua_pushcfunction(L, lua_SetAutoFace);
	lua_setfield(L, -2, "SetAutoFace");
}

int G_ManipulatableObject::lua_SetTouchBone(lua_State *L) {
	G_ManipulatableObject *self = static_cast<G_ManipulatableObject*>(WorldLua::EntFramePtr(L, 1, true));
	self->m_model = lua::SharedPtr::Get<SkMeshDrawModel>(L, "SkDrawModel", 2, true);
	self->m_boneIdx = (int)luaL_checkinteger(L, 3);
	return 0;
}

int G_ManipulatableObject::lua_SetTouchClassBits(lua_State *L) {
	G_ManipulatableObject *self = static_cast<G_ManipulatableObject*>(WorldLua::EntFramePtr(L, 1, true));
	self->m_touchClass = (int)luaL_checkinteger(L, 2);
	return 0;
}

int G_ManipulatableObject::lua_SetTouchBoneBounds(lua_State *L) {
	G_ManipulatableObject *self = static_cast<G_ManipulatableObject*>(WorldLua::EntFramePtr(L, 1, true));
	self->m_bounds = BBox(
		lua::Marshal<Vec3>::Get(L, 2, true),
		lua::Marshal<Vec3>::Get(L, 3, true)
	);
	return 0;
}

int G_ManipulatableObject::lua_EnableTouch(lua_State *L) {
	G_ManipulatableObject *self = static_cast<G_ManipulatableObject*>(WorldLua::EntFramePtr(L, 1, true));
	self->m_enabled = lua_toboolean(L, 2) ? true : false;
	return 0;
}

int G_ManipulatableObject::lua_SetAutoFace(lua_State *L) {
	G_ManipulatableObject *self = static_cast<G_ManipulatableObject*>(WorldLua::EntFramePtr(L, 1, true));
	Entity *target = static_cast<Entity*>(WorldLua::EntFramePtr(L, 2, false));
	if (target) {
		self->m_autoFace = target->shared_from_this();
	} else {
		self->m_autoFace.reset();
	}
	return 0;
}

} // world

namespace spawn {

void *info_tentacle::Create() {
	return new (ZWorld) world::G_ManipulatableObject();
}

void *info_pylon::Create() {
	return new (ZWorld) world::G_ManipulatableObject();
}

void *info_custom_manipulatable::Create() {
	return new (ZWorld) world::G_ManipulatableObject();
}

} // spawn
