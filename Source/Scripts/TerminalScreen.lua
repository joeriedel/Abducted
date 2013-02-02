-- TerminalScreen.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TerminalScreen = Entity:New()
TerminalScreen.MaxActivateDistance = 100
TerminalScreen.MaxTouchDistancePct = 1/8
TerminalScreen.TouchPosShift = {0,0,48}
TerminalScreen.PopupEntity = nil
TerminalScreen.Objects = {}

function TerminalScreen.Spawn(self)
	Entity.Spawn(self)
	
	self.model = LoadModel(self.keys.model)
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetScale(VecForString(self.keys.scale, {1,1,1}))

	self.enabled = BoolForString(self.keys.enabled, false)
	self.active = false
	self.popup = false
	
	table.insert(TerminalScreen.Objects, self)
end

function TerminalScreen.CheckTouch(self, x, y)
	if (not (self.enabled and self.active)) then
		return false
	end
	
	return CheckWorldTouch(
		VecAdd(self:WorldPos(), TerminalScreen.TouchPosShift), 
		x, 
		y, 
		TerminalScreen.MaxTouchDistance
	)
end

function TerminalScreen.CheckProximity(self, playerPos)
	if (not self.enabled) then
		return false
	end
	
	local kMaxDist = UI.systemScreen.diagonal * TerminalScreen.MaxTouchDistancePct
	
	local dd = VecSub(playerPos, self:WorldPos())
	dd = VecMag(dd)
	
	return (dd <= kMaxDist)
end

function TerminalScreen.CheckActive(self, playerPos)

	local activate = self:CheckProximity(playerPos)
	self:Activate(activate)

end

function TerminalScreen.Activate(self, activate)

	if (self.active == activate) then
		return
	end
	
	self.active = activate

end

function TerminalScreen.Touched(self)
	if (self.popup) then
		return
	end
	
	self.popup = true
	TerminalScreen.PopupEntity = self
	TerminalScreen.CancelUI() -- cancel any active UI
end

function TerminalScreen.CancelUI()

end

function TerminalScreen.Touch(e)

	if (not Input.IsTouchBegin(e)) then
		return false
	end

	local best
	local bestDist
	
	for k,v in pairs(TerminalScreen.Objects) do
		local r, dd = v:CheckTouch(e.data[1], e.data[2])
		if (r) then
			if ((bestDist==nil) or (dd < bestDist)) then
				best = v
				bestDist = dd
			end
		end
	end
	
	if (best) then
		best:Touched()
	end
	
	return (best ~= nil)
end

function TerminalScreen.CheckActivate(playerPos)

	for k,v in pairs(TerminalScreen.Objects) do
		v:CheckActive(playerPos)
	end

end

function TerminalScreen.UpdateUI()
	if (World.paused) then
		return
	end
	
end

function TerminalScreen.StaticInit()

	local f = function()
		TerminalScreen.CheckActivate(World.playerPawn:WorldPos())
	end
	
	TerminalScreen.Timer = World.gameTimers:Add(f, 0.33)
end