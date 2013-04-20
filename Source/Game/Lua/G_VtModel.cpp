// G_VtModel.cpp
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Crow/LICENSE for licensing terms.

#include "G_VtModel.h"

namespace world {

D_VtModel::Ref D_VtModel::New(const r::VtMesh::Ref &mesh) {
	return D_VtModel::Ref(new (ZWorld) G_VtModel(mesh));
}

G_VtModel::G_VtModel(const r::VtMesh::Ref &mesh) : D_VtModel(mesh) {
}

bool G_VtModel::SetRootController(lua_State *Lerr, Entity *entity, const char *string) {
	return D_VtModel::SetRootController(Lerr, entity, string);
}

} // world

