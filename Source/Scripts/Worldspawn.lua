-- Worldspawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Worldspawn = Entity:New()

function Worldspawn.Spawn(self)
	COutLine(kC_Debug, "Worldspawn:Spawn")
	World.worldspawn = self
	World.SetWorldspawn(self)
	if self.keys.farClip then
		World.SetCameraFarClip(tonumber(self.keys.farClip))
	end
	
	-- enable cloud storage?
	World.cloudEnabled = Persistence.ReadBool(Globals, "icloud", true)
	SaveGame:EnableCloudStorage(World.cloudEnabled)
		
end

function Worldspawn.PostSpawn(self)	
	GameNetwork.LogEvent("Loaded ("..self.keys.mappath..")")
	World.PlayCinematic("intro", kCF_AnimateCamera, 0)
end

function Worldspawn.OnEvent(self, cmd, args)
	return false
end

worldspawn = Worldspawn
