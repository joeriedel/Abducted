// G_SkModel.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Crow/LICENSE for licensing terms.

#include <Engine/World/Lua/D_SkModel.h>
#include <Runtime/PushPack.h>

namespace world {

class G_SkModel : public D_SkModel
{
public:
	typedef boost::shared_ptr<G_SkModel> Ref;

	G_SkModel(const r::SkMesh::Ref &mesh);

	virtual bool SetRootController(lua_State *Lerr, Entity *entity, const char *string);
};

} // world

#include <Runtime/PopPack.h>
