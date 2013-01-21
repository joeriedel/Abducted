-- TouchTrigger.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TouchTrigger = Entity:New()

function TouchTrigger.Spawn(self)

	-- TouchTrigger is primarily implemented in C++. Engine side code however
	-- doesn't know about our game specific classbits so we set those here.
	
	local triggerTypes = StringForKey(self.keys.types, "all")
	if (triggerTypes == "all") then
		self:SetTouchClassBits(kEntityClassBits_All)
	elseif (triggerTypes == "player") then
		self:SetTouchClassBits(kEntityClass_Player)
	else
		assert(triggerTypes == "monsters")
		self:SetTouchClassBits(kEntityClass_Monster)
	end

end

info_touch_trigger = TouchTrigger
