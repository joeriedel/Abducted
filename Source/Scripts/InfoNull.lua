-- InfoNull.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

InfoNull = Entity:New()

function InfoNull.Spawn(self)

	COutLine(kC_Debug, "InfoNull:Spawn")
	
	self:SpawnFloorPosition()

end

info_null = InfoNull
