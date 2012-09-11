// MaxScene.h
// Copyright (c) 2010 Sunside Inc., All Rights Reserved
// Author: Joe Riedel
// See Radiance/LICENSE for licensing terms.

#pragma once

#include "System.h"
#include <vector>

struct MaxScene
{
	std::vector<CRenderMesh::Ref> meshes;
	vec3 mins;
	vec3 maxs;
	vec3 origin;

	// not const & incase t is a member
	void Translate(vec3 t);

	bool Load(const char *filename);
};
