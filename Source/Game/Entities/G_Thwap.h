/*! \file G_Thwap.h
	\copyright Copyright (c) 2010 Sunside Inc., All Rights Reserved.
	\copyright See Crow/LICENSE for licensing terms.
*/

#pragma once

#include <Engine/World/Entity.h>
#include <Runtime/PushPack.h>

namespace world {

class G_Thwap : public Entity
{
public:

	G_Thwap();
	virtual ~G_Thwap();

protected:

	virtual void Tick(
		int frame,
		float dt,
		const xtime::TimeSlice &time
	);

private:

	bool m_touching;
};

} // world

E_DECL_SPAWN(RADNULL_API, x_thwap)
E_DECL_SPAWN(RADNULL_API, y_thwap)
E_DECL_SPAWN(RADNULL_API, z_thwap)

#include <Runtime/PopPack.h>
