/*! \file G_ManipulatableObject.h
	\copyright Copyright (c) 2012 Sunside Inc., All Rights Reserved.
	\copyright See Radiance/LICENSE for licensing terms.
	\author Joe Riedel
	\ingroup game
*/

#include <Engine/World/Entity.h>
#include <Runtime/PushPack.h>

namespace world {

class G_ManipulatableObject : public Entity {
	E_DECL_BASE(Entity);
public:
	
	G_ManipulatableObject();
	virtual ~G_ManipulatableObject();

protected:

	virtual void PushCallTable(lua_State *L);
	virtual int Spawn(
		const Keys &keys,
		const xtime::TimeSlice &time,
		int flags
	);
	virtual void Tick(
		int frame,
		float dt, 
		const xtime::TimeSlice &time
	);
	virtual void TickDrawModels(float dt);

private:

	void CheckDamage();
	void GetTouchBounds(BBox &bounds);

	static int lua_SetTouchBone(lua_State *L);
	static int lua_SetTouchClassBits(lua_State *L);
	static int lua_SetTouchBoneBounds(lua_State *L);
	static int lua_EnableTouch(lua_State *L);
	static int lua_SetAutoFace(lua_State *L);
	
	EntityBits m_touching;
	BBox m_bounds;
	Entity::WRef m_autoFace;
	SkMeshDrawModel::Ref m_model;
	int m_boneIdx;
	int m_touchClass;
	bool m_enabled;
};

}

E_DECL_SPAWN(RADNULL_API, info_tentacle)
E_DECL_SPAWN(RADNULL_API, info_pylon)

#include <Runtime/PopPack.h>
