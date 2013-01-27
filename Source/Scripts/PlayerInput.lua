-- PlayerInput.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerInput = Class:New()
PlayerInput.Enabled = true

function PlayerInput.Spawn(self)
	self:Load()
end

function PlayerInput.Load(self)
	self.sfx = {}
	self.sfx.PlayerCommand = World.LoadSound("Audio/player_command", 4)
end

function PlayerInput.OnInputEvent(self, e)
	if (not self.Enabled) then
		return false
	end
	
	e = UI:MapInputEvent(e)
	
	if (Input.IsTouchBegin(e)) then
		UI:ShowFinger(true, 0.25)
		if (World.playerPawn:CheckTappedOn(e.original)) then
			World.playerPawn:Stop()
		else
			self:TapWaypoint(e.original.data[1], e.original.data[2])
		end
	elseif (Input.IsTouchEnd(e)) then
		UI:ShowFinger(false, 0.5)
	end
	
	if (Input.IsTouchEvent(e)) then
		UI:PlaceFinger(e.data[1], e.data[2])
	end
	
	return false
end

function PlayerInput.OnInputGesture(self, g)
	if (not self.Enabled) then
		return false
	end
	
	return false
end

function PlayerInput.TapWaypoint(self, x, y)
	local waypoint = World.PickWaypoint(x, y,  350)
	if (waypoint) then
		if (World.playerPawn:MoveToWaypoint(waypoint)) then
			self.sfx.PlayerCommand:Play(kSoundChannel_UI, 0)
		end
	end
end

