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
	
	if ((Game.entity.type == "Map") or (Game.entity.type == "Cinematic")) then
		Arm:LoadChats(self.keys.arm_chats)
		
		if (StringForString(self.keys.arm_state, "unlocked") == "locked") then
			Arm:ChatLockout()
		end
		
		local abilities = NumberForString(self.keys.starting_abilities, 15)
	
		if (BitTest(abilities, 1)) then
			PlayerSkills:UnlockArm()
		end
		
		if (BitTest(abilities, 2)) then
			PlayerSkills:UnlockManipulate()
		end
		
		if (BitTest(abilities, 4)) then
			PlayerSkills:UnlockShield()
		end
		
		if (BitTest(abilities, 8)) then
			PlayerSkills:UnlockPulse()
		end
	end
	
end

function Worldspawn.PostSpawn(self)
	GameNetwork.LogEvent("Loaded ("..self.keys.mappath..")")
end

function Worldspawn.OnEvent(self, cmd, args)
	return Game.entity:OnEvent(cmd, args)
end

worldspawn = Worldspawn
