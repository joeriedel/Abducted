-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Entity:New()
PlayerPawn.kWalkSpeed = 100
PlayerPawn.kAutoDecelDistance = 10
PlayerPawn.kFriction = 300
PlayerPawn.kRunSpeed = 200
PlayerPawn.kShieldSpeed = 0.5
PlayerPawn.kShieldAccel = 0.5
PlayerPawn.kAccel = 200
PlayerPawn.HandBone = "Hand_right"
PlayerPawn.PulseBeamScale = 1/120
PlayerPawn.GodMode = false

PlayerPawn.AnimationStates = {
	default = {
	-- any animations *not* listed here will pass-through unaltered
	-- example, idle isn't listed here, so "idle" will just become "idle"
		OnSelect = function()
			HUD:EnableAll()
		end,
		bbox = {mins = {-24, -24, -48}, maxs = {24, 24, 64}}
	},
	limp = {
		idle = "limpidle",
		walk = "limpcrawl",
		death = "limpdeath",
		manipulate_idle = "limpmanidle",
		maniplate_left = "limpmanleft",
		manipulate_right = "limpmanright",
		manipulate_up = "limpmanup",
		manipulate_down = "limpmandown",
		speedScale = 0.5,
		tapAdjust = -20, -- CheckTappedOn
		bbox = {mins = {-28, -28, -48}, maxs = {28, 28, 16}},
		canRun = false,
		OnSelect = function()
			HUD:Enable({"arm", "shield", "manipulate"})
		end
	}
}

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	MakeAnimatable(self)
	
	self.animState = StringForString(self.keys.initial_state, "default")
	self.visible = true
	
	self.model = World.Load("Characters/Eve")
	self.model:SetRootController("BlendToController")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetPos({0, 0, -48}) -- on floor
	self.model.handBone = self.model.dm:FindBone(PlayerPawn.HandBone)
	
	if (self.model.handBone == -1) then
		error("PlayerPawn: can't find bone named "..PlayerPawn.HandBone)
	end
	
	self:SetMotionSka(self.model)
	
	local set = PlayerPawn.AnimationStates[self.animState]
	local bbox = set.bbox
	if (not bbox) then
		bbox = PlayerPawn.AnimationStates.default.bbox
	end
	
	self:SetMins(bbox.mins)
	self:SetMaxs(bbox.maxs)
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
	-- shield mesh
	self.shield = World.Load("FX/shield1mesh")
	self.shield.dm = self:AttachDrawModel(self.shield)
	self.shield.dm:SetPos({0, 0, -48}) -- on floor
	self.shield.dm:BlendTo({0,0,0,0}, 0)
	self.shield.dm:ScaleTo({0,0,0}, 0)
	self.shield.dm:SetBounds(self:Mins(), self:Maxs())
	self.shieldSprite = World.CreateSpriteBatch(1, 1)
	self.shieldSprite.material = World.Load("FX/shieldsprite1_M")
	self.shieldSprite.dm = self:AttachDrawModel(self.shieldSprite, self.shieldSprite.material)
	self.shieldSprite.dm:SetBounds(self:Mins(), self:Maxs())
	self.shieldSprite.sprite = self.shieldSprite.dm:AllocateSprite()
	self.shieldSprite.dm:SetSpriteData(
		self.shieldSprite.sprite,
		{
			pos = {0, 0, 18}, -- relative to drawmodel
			size = {148, 158},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.shieldSprite.dm:BlendTo({0,0,0,0}, 0)
	
	-- pulse shot
	self.pulse = {
		World.Load("FX/pulsecone1mesh"),
		World.Load("FX/pulsebeam1mesh"),
		World.Load("FX/pulseimpact1mesh")
	}
	
	self.pulse.cone = self:AttachDrawModel(self.pulse[1])
	self.pulse.cone:SetBounds(self:Mins(), self:Maxs())
	self.pulse.beam = self:AttachDrawModel(self.pulse[2])
	self.pulse.beam:SetBounds(self:Mins(), self:Maxs())
	self.pulse.impact = self:AttachDrawModel(self.pulse[3])
	self.pulse.impact:SetBounds(self:Mins(), self:Maxs())
	self.pulse.cone:BlendTo({0,0,0,0}, 0)
	self.pulse.beam:BlendTo({0,0,0,0}, 0)
	self.pulse.impact:BlendTo({0,0,0,0}, 0)
	
	-- Angles > than these get snapped immediately
	-- The last number is Z angle, which is the player facing.
	self:SetSnapTurnAngles({360, 360, 60})

	local angle = NumberForString(self.keys.angle, 0)
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	angleVertex.mass = 5
	angleVertex.drag[1] = 7
	angleVertex.drag[2] = 7
	angleVertex.friction = 0.5
	self:SetAngles(angleVertex)
	
	local spring = self:AngleSpring()
	spring.elasticity = 160 -- <-- larger numbers means she turns faster
	self:SetAngleSpring(spring)
	self:SetGroundFriction(PlayerPawn.kFriction)
	self:SetAutoDecelDistance(PlayerPawn.kAutoDecelDistance)
	self:EnableFlags(kPhysicsFlag_Friction, true)
	self:SetMoveType(kMoveType_Floor)
	self:SetClassBits(kEntityClass_Player)
	self:SetOccupantType(kOccupantType_BBox)
	
	self:SpawnFloorPosition()
	
	if (self.validFloorPosition and (self.keys.floorNum)) then
	-- enable our starting floor by default
		Floors:SetFloorState(NumberForString(self.keys.floorNum), kFloorState_Enabled, nil)
	elseif (not self.validFloorPosition) then
		error("PlayerPawn must have a valid floor position (no valid starting waypoint or floor was found).")
	end
	
	World.playerPawn = self
	World.SetPlayerPawn(self)
	
	self:AddTickable(kTF_PostPhysics, function () PlayerPawn.TickPhysics(self) end)
	
	self.shieldActive = false
	self.manipulateActive = false
	self.pulseActive = false
	
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

function PlayerPawn.PostSpawn(self)

	local state = self.animState
	self.animState = nil
	self:SelectAnimState(state)	

end

function PlayerPawn.SelectAnimState(self, state)

	if (state ~= self.animState) then
		if (self.animState) then
			local set = PlayerPawn.AnimationStates[self.animState]
			if (set and set.OnDeselect) then
				set:OnDeselect(self)
			end
		end
		self.animState = state
		local set = PlayerPawn.AnimationStates[state]
		if (set and set.OnSelect) then
			set.OnSelect(self)
		end
		local bbox = set.bbox
		if (not bbox) then
			bbox = PlayerPawn.AnimationStates.default.bbox
		end
		
		self:SetMins(bbox.mins)
		self:SetMaxs(bbox.maxs)
		self.model.dm:SetBounds(self:Mins(), self:Maxs())
		self:SetSpeeds()
		self.state = nil -- force change
	end

end

function PlayerPawn.SetSpeeds(self)

	local speed
	local accel
	
	if (self.shieldActive) then
		speed = PlayerPawn.kWalkSpeed * PlayerPawn.kShieldSpeed
		accel = PlayerPawn.kAccel * PlayerPawn.kShieldAccel
	else
		speed = PlayerPawn.kWalkSpeed
		accel = PlayerPawn.kAccel
	end
	
	local set = PlayerPawn.AnimationStates[self.animState]
	if (set and set.speedScale) then
		speed = speed * set.speedScale
	--	accel = accel * set.speedScale
	end

	self:SetMaxGroundSpeed(speed)
	self:SetAccel({accel, 0, 0}) -- <-- How fast the player accelerates (units per second).

end

function PlayerPawn.LookupAnimation(self, name, animState)
	if (animState == nil) then
		animState = self.animState
	end
	
	local state = PlayerPawn.AnimationStates[animState]
	if ((state == nil) or (state[name] == nil)) then
		if (animState == "default") then
			return name
		end
		return self:LookupAnimation(name, "default")
	end
	
	return state[name]
end

function PlayerPawn.TickPhysics(self)
	if (self.disableAnimTick or self.dead) then
		return -- other logic controls us right now
	end
	
	local reqState
	local velocity = VecMag(self:Velocity())
--	COutLine(kC_Debug, "Velocity = %f", velocity)
--	COutLine(kC_Debug, "DistanceMoved = %f", self:DistanceMoved())
	
	if (velocity > 1) then
		reqState = "walk"
	else
		reqState = "idle"
	end
	
	if (self.state ~= reqState) then
		self.state = reqState
		local anim = self:LookupAnimation(reqState)
		if (anim) then
			self:PlayAnim(anim, self.model)
		end
	end
end

function PlayerPawn.MoveToFloorPosition(self, targetFloorPos)
	if (not self.validFloorPosition) then
		return false
	end
	
	local moveCommand = World.CreateFloorMove(self:FloorPosition(), targetFloorPos)
	if (moveCommand == nil) then
		return false
	end
	
	self:SetDesiredMove(moveCommand)
	self:EnableFlags(kPhysicsFlag_Friction, false)
	return true
end

function PlayerPawn.NotifyManipulate(self, enabled)
	
	if (enabled) then
		self.manipulateActive = true
		self.disableAnimTick = true
		self.state = nil
		local anim = self:LookupAnimation("manipulate_idle")
		self:PlayAnim(anim, self.model)
		self:Stop()
	else 
		self.manipulateActive = false
		if (self.manipulateDir == nil) then
			self:EndManipulate()
		end
	end
end

function PlayerPawn.ManipulateDir(self, dir)
	self.manipulateDir = dir
	
	local f = function()
		self:EndManipulate()
	end
	
	local anim = self:LookupAnimation("manipulate_"..dir)
	if (anim) then
		local blend = self:PlayAnim(anim, self.model)
		if (blend) then
			blend.Seq(f)
		else
			f()
		end
	end
end

function PlayerPawn.EndManipulate(self)
	self.manipulateDir = nil
	self.disableAnimTick = false
end

function PlayerPawn.BeginShield(self)

	self.shieldActive = true
	self.shield.dm:ScaleTo({1.07,1.07,1.07}, 0.1)
		
	local f = function()
		self.shield.dm:ScaleTo({1,1,1,1}, 0.1)
	end
	
	World.gameTimers:Add(f, 0.15, true)
	
	f = function ()
		self.shield.dm:BlendTo({1,1,1,1}, 1)
		self.shieldSprite.dm:BlendTo({1,1,1,1}, 0.7)
	end
	
	World.gameTimers:Add(f, 0.05, true)
	
	self:SetSpeeds()

end

function PlayerPawn.EndShield(self)

	self.shieldActive = false
	self.shield.dm:BlendTo({0,0,0,0}, 0.15)
	self.shield.dm:ScaleTo({1.07,1.07,1.07}, 0.1)
	self.shieldSprite.dm:BlendTo({0,0,0,0}, 0.15)
	
	local f = function()
		self.shield.dm:ScaleTo({0,0,0,0}, 0.2)
	end
	
	World.gameTimers:Add(f, 0.1, true)
	
	self:SetSpeeds()
	
end

function PlayerPawn.Stop(self)
	self:EnableFlags(kPhysicsFlag_Friction, true)
end

function PlayerPawn.BeginPulse(self)
	self.pulseActive = true
	self.disableAnimTick = true
	self.state = nil
	local anim = self:LookupAnimation("pulse_idle")
	self:PlayAnim(anim, self.model)
	self:Stop()
end

function PlayerPawn.EndPulse(self)
	self.disableAnimTick = false
end

function PlayerPawn.FirePulse(self, target, normal)

	local localPos = self.model.dm:BonePos(self.model.handBone)
	local start = self.model.dm:WorldBonePos(self.model.handBone)
	local ray = VecSub(target, start)
	local vec, distance = VecNorm(ray)
	local angles = LookAngles(vec)
	self:SetFacing(angles[3])
	
	local localTarget = WorldToLocal(target, self:WorldPos(), {0, 0, angles[3]})
	local fwd = VecSub(localTarget, localPos)
	local fwd = LookAngles(VecNorm(fwd))
		
	self.pulse.cone:BlendTo({1,1,1,1}, 0.1)
	self.pulse.cone:SetPos(localPos)
	self.pulse.cone:SetAngles(fwd)
	
	normal = WorldToLocal(VecNeg(normal), VecZero(), {0, 0, angles[3]})
	local impactAngles = LookAngles(normal)
	self.pulse.impact:BlendTo({1,1,1,1}, 0.1)
	self.pulse.impact:SetPos(localTarget)
	self.pulse.impact:SetAngles(impactAngles)
	
	self.pulse.beam:BlendTo({1,1,1,1}, 0.1)
	self.pulse.beam:SetPos(localPos)
	self.pulse.beam:SetAngles(fwd)
	self.pulse.beam:ScaleTo({distance * PlayerPawn.PulseBeamScale, 1, 1}, 0)
	
	local f = function()
		self.pulse.cone:BlendTo({0,0,0,0}, 0.1)
		self.pulse.impact:BlendTo({0,0,0,0}, 0.1)
		self.pulse.beam:BlendTo({0,0,0,0}, 0.1)
	end
	
	World.gameTimers:Add(f, 0.1, true)
	
	f = function()
		self:EndPulse()
	end
	
	self.pulseActive = false
	
	local anim = self:LookupAnimation("pulse_fire")
	self:PlayAnim(anim , self.model).Seq(f)
	HUD:RechargePulse()
	
end

function PlayerPawn.DischargePulse(self)

	if (math.random() < 0.5) then
		self:PulseExplode()
		return
	end
	
	local randomAngle = FloatRand(0, 359)
	local vec = ForwardVecFromAngles({0, 0, randomAngle})
	local ray = VecScale(vec, 4096)
	local pos = self:WorldPos()
	ray = VecAdd(ray, pos)
	
	local trace = {
		start = pos,
		_end = ray,
		contents = bit.bor(kContentsFlag_Solid, kContentsFlag_Clip)
	}
	
	trace = World.LineTrace(trace)
	
	if (trace and (not trace.startSolid)) then
		self:FirePulse(trace.traceEnd, trace.normal)
		return
	end
	
	self.pulseActive = false
	self:EndPulse()
end

function PlayerPawn.PulseExplode(self)
	self.pulseActive = false
	self:Kill()
end

function PlayerPawn.Kill(self)
	if (self.dead or PlayerPawn.GodMode) then
		return
	end
	self.dead = true
	if (self.shieldActive) then
		self:EndShield()
	end
	self:SetMoveType(kMoveType_None)
	self:PlayAnim("death", self.model)
	Game.entity:PlayerDied()
end

function PlayerPawn.CheckTappedOn(self, e)

	local pos = self:WorldPos()
	
	local set = PlayerPawn.AnimationStates[self.animState]
	
	if (set.tapAdjust) then
		pos = VecAdd(pos, {0, 0, set.tapAdjust})
	else
		pos = VecAdd(pos, {0, 0, 50})
	end
	
	local screen, r = World.Project(pos)
	if (r) then
		local dx = screen[1] - e.data[1]
		local dy = screen[2] - e.data[2]
		local dd = math.sqrt(dx*dx + dy*dy)
		local maxDist = UI.systemScreen.diagonal * 1/20
		if (dd <= maxDist) then
			return true
		end
	end
	
	return false
end

function PlayerPawn.OnEvent(self, cmd, args)
	COutLineEvent("PlayerPawn", cmd, args)
	
	if (cmd == "state") then
		if (args == nil) then
			COutLine(kC_Debug, "Error, animstate command requires an argument")
		else
			self:SelectAnimState(args)
		end
		return true
	elseif (cmd == "resetstate") then
		self:SelectAnimState("default")
		return true
	elseif (cmd == "show") then
		self:Show(true)
		return true
	elseif (cmd == "hide") then
		self:Hide(true)
		return true
	elseif (cmd == "kill") then
		self:Kill()
		return true
	elseif (cmd == "teleport") then
		args = Tokenize(args)
		self:Teleport(args[1], tonumber(args[2]))
		return true
	elseif (cmd == "set_facing") then
		self:SetFacing(tonumber(args[2]))
		return true
	elseif (cmd == "play") then
		self:PlayCinematicAnim(args)
		return true
	end
	
	return false
end

function PlayerPawn.Teleport(self, userId, facing)
	local waypoints = World.WaypointsForUserId(userId)
	if (waypoints) then
		self:Stop()
		local fp = World.WaypointFloorPosition(waypoints[1])
		self:SetFloorPosition(fp)
		self:SetDesiredMove(nil)
		
		if (facing) then
			self:SetFacing(facing)
		end
	end
end

function PlayerPawn.PlayCinematicAnim(self, args)
	args = Tokenize(args)
	local anim = self:LookupAnimation(args[1])
	if (anim) then
		local f = function()
			self:SetMoveType(kMoveType_Floor)
			self.disableAnimTick = false
			self.customMove = false
			
			if (args[2]) then
				self:Teleport(args[2], tonumber(args[3]))
			else
				local fp = self:FindFloor()
				if (fp) then
					self.validFloorPosition = true
					self:SetFloorPosition(fp)
					error("PlayerPawn.PlayCinematicAnim: walked off the floor.")
				else
					self.validFloorPosition = false
				end
			end
			
			self:TickPhysics() -- get us a new state immediately
		end
		self:SetMoveType(kMoveType_Ska)
		self.disableAnimTick = true
		self.customMove = true
		self.state = nil
		self:Stop()
		local blend = self:PlayAnim(anim, self.model)
		if (blend) then
			blend.Seq(f)
		else
			f()
		end
	end
end

function PlayerPawn.Show(self, show)

	if (show == self.visible) then
		return
	end
	
	self.visible = show
	self.model.dm:SetVisible(show)
	
	self:ShowShield(show)

end

function PlayerPawn.ShowShield(self, show)
	if (show and self.shieldActive) then
		self.shield.dm:SetVisbile(show)
		self.shieldSprite.dm:SetVisible(show)
	elseif (not show) then
		self.shield.dm:SetVisible(false)
		self.shieldSprite.dm:SetVisible(false)
	end
end

function PlayerPawn.CustomAnimMove(self, name)

	local f = function()
		self.disableAnimTick = false
		self.customMove = false
		self:TickPhysics() -- get us a new state immediately
		if (self.CustomMoveComplete) then
			self:CustomMoveComplete()
		end
	end
	
	local anim = self:LookupAnimation(name)
	if (anim) then
		self.disableAnimTick = true
		self.customMove = true
		self.state = nil
		self:EnableFlags(kPhysicsFlag_Friction, true)
		local blend = self:PlayAnim(anim, self.model)
		if (blend) then
			blend.Seq(f)
		else
			f()
		end
	end

end

function PlayerPawn.SaveState(self)
	local fp = self:FloorPosition()
	if (fp.floor == -1) then
		error("PlayerPawn.SaveState: checkpoints not allowed when player is not on a floor!")
	end
	if (self.customMove) then
		error("PlayerPawn.SaveState: executing custom move, cannot save!")
	end
	assert(not self.dead)
	
	local vertex = self:Angles()
	
	local state = {
		visible = tostring(self.visible),
		shieldActive = tostring(self.shieldActive),
		animState = self.animState,
		facing = tostring(vertex.pos[3]),
		pos = string.format("%d %d %d", fp.pos[1], fp.pos[2], fp.pos[3])
	}
	
	return state
end

function PlayerPawn.LoadState(self, state)
	
	self.dead = false
	self.disableAnimTick = false
	self.state = "idle"
	
	self:Show(state.visible == "true")
	
	if (state.shieldActive == "true") then
		self:BeginShield()
	else
		self.shieldActive = false
		self.shield.dm:BlendTo({0,0,0,0}, 0)
		self.shieldSprite.dm:BlendTo({0,0,0,0}, 0)
	end
	
	local pos = Vec3ForString(state.pos)
	local fp  = World.ClipToFloor(
		{pos[1], pos[2], pos[3] + 8},
		{pos[1], pos[2], pos[3] - 8}
	)
	
	assert(fp)
	self:SetFloorPosition(fp)
	self:SetDesiredMove(nil)
	self:SetMoveType(kMoveType_Floor)
	self:SetFacing(tonumber(state.facing))
	self:SelectAnimState(state.animState)
	
	local anim = self:LookupAnimation("idle")
	if (anim) then
		self.model:BlendImmediate(anim)
	end
	
	self:Link()
end

info_player_start = PlayerPawn
