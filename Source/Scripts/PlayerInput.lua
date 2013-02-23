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
		return false, false
	end
	
	if (World.playerPawn.dead) then
		return false, false
	end
	
	local action = false
	
	e = UI:MapInputEvent(e)
	
	if (Input.IsTouchBegin(e) and (self.touch == nil)) then
		self.touch = e.touch
		UI:ShowFinger(true, 0.25)
		if (not World.playerPawn.customMove) then
			if (Game.entity.pulse) then
				action = self:TapPulse(e.original.data[1], e.original.data[2])
			else
				if (World.playerPawn:CheckTappedOn(e.original)) then
					World.playerPawn:Stop()
					action = true
				else
					action = self:TapMove(e.original.data[1], e.original.data[2])
				end
			end
		end
	elseif (self.touch ~= e.touch) then
		return false, false
	elseif (Input.IsTouchEnd(e, self.touch)) then
		self.touch = nil
		UI:ShowFinger(false, 0.5)
	end
	
	if (Input.IsTouchEvent(e)) then
		UI:PlaceFinger(e.data[1], e.data[2])
	end
	
	return false, action
end

function PlayerInput.OnInputGesture(self, g)
	if (not self.Enabled) then
		return false
	end
	
	return false
end

function PlayerInput.TapPulse(self, x, y)

	local a = World.Unproject({x, y, 0})
	local b = World.Unproject({x, y, 1})
	
	local trace = {
		start = a,
		_end = b,
		contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
	}
	
	trace = World.LineTrace(trace)
	if (trace and not (trace.startSolid)) then
		Game.entity:FirePulse(trace.traceEnd, trace.normal)
	end
	
end

function PlayerInput.TapMove(self, x, y)

	local a = World.Unproject({x, y, 0})
	local b = World.Unproject({x, y, 1})
	
	local trace = {
		start = a,
		_end = b,
		contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
	}
	
	trace = World.LineTrace(trace)
		
	local dist = nil
	if (trace and not (trace.startSolid)) then
		dist = VecMag(VecSub(trace.traceEnd, World.CameraPos()))
	end

	if (self:TapFloor(a, b, dist)) then
		return true
	end
	
	if (trace) then
		-- see if we can cast downward and hit a floor
		a = VecAdd(trace.traceEnd, VecScale(trace.normal, 32))
		b = VecAdd(a, {0, 0, -512})
		
		local targetFloorPos = World.ClipToFloor(a, b)
		if (targetFloorPos) then
			if (World.playerPawn:MoveToFloorPosition(targetFloorPos)) then
				self.sfx.PlayerCommand:Play(kSoundChannel_UI, 0)
				return true
			end
		end
	end

	return self:TapWaypoint(x, y, dist)
end

function PlayerInput.TapFloor(self, a, b, dist)
	local targetFloorPos = World.ClipToFloor(a, b)
	if (targetFloorPos) then
		local dd = VecMag(VecSub(targetFloorPos.pos, World.CameraPos()))
		if ((dist == nil) or (dd < dist)) then
			if (World.playerPawn:MoveToFloorPosition(targetFloorPos)) then
				self.sfx.PlayerCommand:Play(kSoundChannel_UI, 0)
				return true
			end
		end
	end
	
	return false
end

function PlayerInput.TapWaypoint(self, x, y)
	-- PickWaypoint(x, y, radiusOnScreen, dropDistance)
	-- dropDistance: all points that are within radiusOnScreen are
	-- collected and sorted front to back based on distance from the
	-- camera. And points that are more than dropDistance from the
	-- closest candidate waypoint are removed from consideration.
	--
	-- ZERO means no points are ever dropped.
	local waypoint = World.PickWaypoint(x, y,  350, 0) -- NOTE: does per-waypoint waypoint->camera line trace
	if (waypoint) then
		local targetFloorPos = World.WaypointFloorPosition(waypoint)
		if (targetFloorPos) then
			if (World.playerPawn:MoveToFloorPosition(targetFloorPos)) then
				self.sfx.PlayerCommand:Play(kSoundChannel_UI, 0)
				return true
			end
		end
	end
	
	return false
end

