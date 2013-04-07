-- TouchTrigger.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TouchTrigger = Entity:New()

function TouchTrigger.Spawn(self)

	-- TouchTrigger is primarily implemented in C++. Engine side code however
	-- doesn't know about our game specific classbits so we set those here.
	
	local triggerTypes = StringForString(self.keys.types, "all")
	if (triggerTypes == "all") then
		self:SetTouchClassBits(kEntityClass_Any)
	elseif (triggerTypes == "player") then
		self:SetTouchClassBits(kEntityClass_Player)
	else
		assert(triggerTypes == "monsters")
		self:SetTouchClassBits(kEntityClass_Monster)
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

function TouchTrigger.SaveState(self)

	local instigator = self:Instigator()
	if (instigator) then
		instigator = instigator.uid
	else
		instigator = -1
	end
	
	local state = {
		instigator = tostring(instigator),
		enabled = tostring(self:Enabled())
	}

	return state
	
end

function TouchTrigger.LoadState(self, state)

	local instigator = tonumber(state.instigator)

	if (instigator ~= -1) then
		instigator = World.FindEntityUID(instigator)
	end
	
	self:SetInstigator(instigator)
	self:SetEnabled(state.enabled == "true")
	
end

info_touch_trigger = TouchTrigger
