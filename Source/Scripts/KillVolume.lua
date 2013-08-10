-- KillVolume.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

KillVolume = Entity:New()

function KillVolume.Spawn(self)

	self.enabled = BoolForString(self.keys.enabled, false)
	
	local triggerTypes = StringForString(self.keys.types, "all")
	if (triggerTypes == "all") then
		self.classbits = kEntityClass_Any
	elseif (triggerTypes == "player") then
		self.classbits = kEntityClass_Player
	else
		assert(triggerTypes == "monsters")
		self.classbits = kEntityClass_Monster
	end
	
	if (self.enabled) then
		self.think = KillVolume.KillThink
		self:SetNextThink(1/4)
	end

	local io = {
		Save = function()
			return self:SaveState()
		end,
		Load = function(s, x)
			return self:LoadState(x)
		end
	}
	
	GameDB.PersistentObjects[self.keys.uuid] = io
	
end

function KillVolume.Enable(self, enable)

	if (self.enabled ~= enable) then
		self.enabled = enable
		if (self.enabled) then
			self.think = KillVolume.KillThink
			self:SetNextThink(1/4)
			self:KillThink()
		end
	end

end

function KillVolume.OnEvent(self, cmd, args)
	COutLineEvent("KillVolume", self.keys.targetname, cmd, args)
	
	if (cmd == "enable") then
		self:Enable(true)
		return true
	elseif (cmd == "disable") then
		self:Enable(false)
		return true
	elseif (cmd == "trigger") then
		self:KillThink()
		return true
	end
	
	return false
end

function KillVolume.KillThink(self)

	local touching = self:GetTouching(self.classbits)
	
	if (touching) then
		for i=1,#touching do
			local entity = touching[i]
			if (entity.Kill and (not entity.dead)) then
				local msg = nil
				if (entity:ClassBits() == kEntityClass_Player) then
					msg = self.keys.killed_player_msg
				end
				entity:Kill(self, msg)
			end
		end
	end

end

function KillVolume.SaveState(self)

	local state = {
		enabled = tostring(self.enabled)
	}

	return state
	
end

function KillVolume.LoadState(self, state)

	self.enabled = state.enabled == "true"

	if (self.enabled) then
		self.think = KillVolume.KillThink
		self:SetNextThink(1/4)
	else
		self.think = nil
	end
end

info_kill_volume = KillVolume