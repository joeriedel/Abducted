// G_VtModel.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Crow/LICENSE for licensing terms.

#include <Engine/World/Lua/D_VtModel.h>
#include <Runtime/PushPack.h>

namespace world {

class G_VtModel : public D_VtModel {
public:
	typedef boost::shared_ptr<G_VtModel> Ref;

	G_VtModel(const r::VtMesh::Ref &mesh);

	virtual bool SetRootController(lua_State *Lerr, Entity *entity, const char *string);
};

} // world

#include <Runtime/PopPack.h>
