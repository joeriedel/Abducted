// G_SkModel.cpp
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Crow/LICENSE for licensing terms.

#include "G_SkModel.h"

namespace world {

D_SkModel::Ref D_SkModel::New(const r::SkMesh::Ref &mesh)
{
	return D_SkModel::Ref(new (ZWorld) G_SkModel(mesh));
}

G_SkModel::G_SkModel(const r::SkMesh::Ref &mesh) : D_SkModel(mesh)
{
}

bool G_SkModel::SetRootController(lua_State *Lerr, Entity *entity, const char *string)
{
	return D_SkModel::SetRootController(Lerr, entity, string);
}

} // world

