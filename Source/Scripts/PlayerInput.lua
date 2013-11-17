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
				action = self:TapPulse(e)
			else
				if ((UI.mode == kGameUIMode_Mobile) and World.playerPawn:CheckTappedOn(e.original)) then
					World.playerPawn:Stop()
					action = true
				elseif (Discovery.CheckTouch(e)) then
					action = true
					self.touch = nil
					UI:ShowFinger(true, 0)
					UI:ShowFinger(false, 0.5)
					World.FlushInput(true)
					World.playerPawn:Stop()
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

function PlayerInput.Flush(self)
	
	UI:ShowFinger(false, 0.5)
	self.touch = nil

end

function PlayerInput.TapPulse(self, e)

	if (self:TapTarget(e)) then
		return true
	end

	local a = World.Unproject({e.original.data[1], e.original.data[2], 0})
	local b = World.Unproject({e.original.data[1], e.original.data[2], 1})
	
	local trace = {
		start = a,
		_end = b,
		contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
	}
	
	trace = World.LineTrace(trace)
	if (trace and not (trace.startSolid)) then
		COutLine(kC_Debug, "Pulse - shooting world!")
		
		local playerTrace = {
			start = VecAdd(World.playerPawn:WorldPos(), World.playerPawn:CameraShift()),
			_end = trace.traceEnd,
			contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
		}
		
		playerTrace = World.LineTrace(playerTrace) or trace
		Game.entity:FirePulse(playerTrace.traceEnd, playerTrace.normal)
		return true
	else
		if (trace) then
			COutLine(kC_Debug, "Pulse - trace was startSolid, error.")
		else
			COutLine(kC_Debug, "Pulse - nothing hit.")
		end
	end
	
	return false
	
end

function PlayerInput.TapTarget(self, e)

	local playerPos = VecAdd(World.playerPawn:WorldPos(), World.playerPawn:CameraShift())
	
	local target1 = Metadata.CheckPulseTargets(e)
	local target2 = Tormentor.CheckPulseTargets(e)
	local target3 = ManipulatableObject.CheckPulseTargets(e)
	local target = nil
	
	if (target1 and target2) then
		-- choose closest
		local dist1 = VecMag(VecSub(playerPos, target1.pos))
		local dist2 = VecMag(VecSub(playerPos, target2.pos))
		
		if (dist1 <= dist2) then
			target = target1
		else
			target = target2
		end
	else
		target = target1 or target2
	end
	
	if (target and target3) then
		-- choose closest
		local dist1 = VecMag(VecSub(playerPos, target.pos))
		local dist2 = VecMag(VecSub(playerPos, target3.pos))
		
		if (dist2 < dist1) then
			target = target3
		end
	else
		target = target or target3
	end
	
	if (target) then
		
		-- did we hit something first?
		local trace = {
			start = playerPos,
			_end = target.pos,
			contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
		}
	
		if (World.LineTrace(trace) == nil) then
			COutLine(kC_Debug, "Pulse - targeting object!")
			local normal = VecSub(trace.start, trace._end)
			normal = VecNorm(normal)
			Game.entity:FirePulse(target.pos, normal, false)
			target:ShotWithPulse()
			return true
		else
			COutLine(kC_Debug, "Pulse - was going to shoot object but hit collision mesh first!")
		end
	end
	
	return false

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
		COutLine(kC_Debug, "PlayerInput: Tapped on floor - precise (1).")
		return true
	end
	
	if (self:TapWaypoint(x, y, 50)) then
		COutLine(kC_Debug, "PlayerInput: Tapped on waypoint - precise (2).")
		return true
	end	
	
	-- see if we can cast downward and hit a floor
	if (trace) then
		local d = 16
		while (d <= 224) do
			a = VecAdd(trace.traceEnd, VecScale(trace.normal, d))
			b = VecAdd(a, {0, 0, -512})
			
			local targetFloorPos = World.ClipToFloor(a, b)
			if (targetFloorPos) then
			
				-- is this point on screen?
				local p,r = World.Project(targetFloorPos.pos)
				if (r) then
				
					if ((p[1] > 0) and (p[1] < UI.systemScreen.width) and
						(p[2] > 0) and (p[2] < UI.systemScreen.height)) then
			
						-- on screen
						COutLine(kC_Debug, "PlayerInput: Tapped on clip-mesh, floor found below (3).")
						
						if (World.playerPawn:MoveToFloorPosition(targetFloorPos)) then
							self.sfx.PlayerCommand:Play(kSoundChannel_UI, 0)
							return true
						end
						
						COutLine(kC_Debug, "PlayerInput: Player could not generate path to floor!")
						break
					
					end
				end
			end
			
			d = d + 16
		end
	end
	
	if (self:TapWaypoint(x, y, 100*UI.identityScale[1])) then
		COutLine(kC_Debug, "PlayerInput: Tapped on waypoint - unprecise (4).")
		return true
	end
	
	COutLine(kC_Debug, "PlayerInput: no point.")
	
	return false
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

function PlayerInput.TapWaypoint(self, x, y, radius)
	-- PickWaypoint(x, y, radiusOnScreen, dropDistance)
	-- dropDistance: all points that are within radiusOnScreen are
	-- collected and sorted front to back based on distance from the
	-- camera. And points that are more than dropDistance from the
	-- closest candidate waypoint are removed from consideration.
	--
	-- ZERO means no points are ever dropped.
	local waypoint = World.PickWaypoint(x, y,  radius, 0) -- NOTE: does per-waypoint waypoint->camera line trace
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

