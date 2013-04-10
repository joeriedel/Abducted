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
	
	Game.entity:Initialize(StringForString(self.keys.type, "Map"))
	
	if (Game.entity.type == "Map") then
		Arm:LoadChats(self.keys.arm_chats)
		
		if (StringForString(self.keys.arm_state, "unlocked") == "locked") then
			Arm:ChatLockout()
		end
	end
end

function Worldspawn.PostSpawn(self)
	GameNetwork.LogEvent("Loaded ("..self.keys.mappath..")")
	Game.entity:PostSpawn()
end

function Worldspawn.OnEvent(self, cmd, args)
	return Game.entity:OnEvent(cmd, args)
end

worldspawn = Worldspawn
