-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Entity:New()
PlayerPawn.kWalkSpeed = 80
PlayerPawn.kAutoDecelDistance = 5
PlayerPawn.kFriction = 1000
PlayerPawn.kRunSpeed = 160
PlayerPawn.kShieldSpeed = 1
PlayerPawn.kShieldAccel = 1
PlayerPawn.kAccel = 400
PlayerPawn.HandBone = "Girl_RArmPalm"
PlayerPawn.PulseBeamScale = 1/120
PlayerPawn.PulseKillRadius = 50
PlayerPawn.GodMode = false

PlayerPawn.AnimationStates = {
	default = {
	-- any animations *not* listed here will pass-through unaltered
	-- example, idle isn't listed here, so "idle" will just become "idle"
		OnSelect = function()
			HUD:EnableAll()
		end,
		bbox = {mins = {-28, -28, 0}, maxs = {28, 28, 128}}
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
		arm_default_flyin = "arm_limp_flyin",
		arm_default_flyout = "arm_limp_flyout",
		arm_pose_standing = "arm_pose_limp",
		puzzle_default_choice = "puzzle_limp_choice",
		speedScale = 0.7,
		tapAdjust = 48, -- CheckTappedOn
		bbox = {mins = {-48, -48, 0}, maxs = {48, 48, 104}},
		canRun = false,
		OnSelect = function()
			HUD:Enable({"arm", "shield", "manipulate"})
		end,
	},
	limpscrunch = {
		idle = "limp_scrunched_idle",
		walk = "limp_scrunched_forward",
		manipulate_idle = "limpmanidle",
		maniplate_left = "limpmanleft",
		manipulate_right = "limpmanright",
		manipulate_up = "limpmanup",
		manipulate_down = "limpmandown",
		arm_default_flyin = "arm_limp_flyin",
		arm_default_flyout = "arm_limp_flyout",
		arm_pose_standing = "arm_pose_limp",
		puzzle_default_choice = "puzzle_limp_choice",
		speedScale = 0.4,
		tapAdjust = 48, -- CheckTappedOn
		bbox = {mins = {-32, -32, 0}, maxs = {32, 32, 90}},
		canRun = false,
		OnSelect = function()
			HUD:Enable({"arm", "shield", "manipulate"})
		end,
	},
	walkfast = {
		walk = "walkfast",
		speedScale = 2.2,
		OnSelect = function()
			HUD:EnableAll()
		end
	}
}

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	self:SetLightInteractionFlags(kLightInteractionFlag_Player)
	self:SetCameraShift({0,0,64})
	
	MakeAnimatable(self)
	
	self.animState = StringForString(self.keys.initial_state, "default")
	self.visible = true
	
	self.model = World.Load("Characters/Eve")
	self.model:SetRootController("BlendToController")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetPos({0, 0, 0}) -- on floor
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
	self.shield.dm:SetPos({0, 0, 0}) -- on floor
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
			pos = {0, 0, 68}, -- relative to drawmodel
			size = {148, 158},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.shieldSprite.dm:BlendTo({0,0,0,0}, 0)
	
	self.shieldSounds = {
		Activate = World.LoadSound("Audio/AFX_ShieldActivate"),
		Hum = World.LoadSound("Audio/AFX_ShieldHum"),
		Deactivate = World.LoadSound("Audio/AFX_ShieldDeactivate"),
		Spark = World.LoadSound("Audio/AFX_ShieldImpactLight", 3)
	}
	
	self.shieldSounds.Hum:SetLoop(true)
	
	self.shieldSoundPaused = false
	
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
	
	self.pulseSounds = {
		Hum = World.LoadSound("Audio/AFX_PulseCycleLoop"),
		Explode = World.LoadSound("Audio/AFX_ShieldImpactAggressive"),
		Fire = World.LoadSound("Audio/AFX_PulseEnergyImpact")
	}
	
	self.pulseSounds.Hum:SetLoop(true)
	
	self.bugSounds = {
		Stun = World.LoadSound("Audio/EFX_IdleBugSwarm_rev1"),
		Eaten = World.LoadSound("Audio/EFX_IdleBugSwarm_rev1"),
		Kill = World.LoadSound("Audio/EFX_ManyBugSquish_rev1")
	}
	
	-- Angles > than these get snapped immediately
	-- The last number is Z angle, which is the player facing.
	self:SetSnapTurnAngles({360, 360, 160})

	local angle = NumberForString(self.keys.angle, 0)
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	angleVertex.mass = 5
	angleVertex.drag[1] = 7
	angleVertex.drag[2] = 7
	angleVertex.friction = 0.5
	self:SetAngles(angleVertex)
	self:SetTargetAngles(angleVertex.pos)
	
	local spring = self:AngleSpring()
	spring.elasticity = 240 -- <-- larger numbers means she turns faster
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
	
	World.gameTimers:Add(f, 0.15)
	
	f = function ()
		self.shield.dm:BlendTo({1,1,1,1}, 1)
		self.shieldSprite.dm:BlendTo({1,1,1,1}, 0.7)
	end
	
	World.gameTimers:Add(f, 0.05)
	
	self:SetSpeeds()
	self:PlayShieldSound()
end

function PlayerPawn.EndShield(self)

	self.shieldActive = false
	self.shield.dm:BlendTo({0,0,0,0}, 0.15)
	self.shield.dm:ScaleTo({1.07,1.07,1.07}, 0.1)
	self.shieldSprite.dm:BlendTo({0,0,0,0}, 0.15)
	
	local f = function()
		self.shield.dm:ScaleTo({0,0,0,0}, 0.2)
	end
	
	World.gameTimers:Add(f, 0.1)
	
	self:SetSpeeds()
	self:StopShieldSound()
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
	self.pulseSounds.Hum:FadeVolume(0, 0)
	self.pulseSounds.Hum:FadeVolume(1, 0.1)
	self.pulseSounds.Hum:Play(kSoundChannel_FX, 0)
end

function PlayerPawn.EndPulse(self)
	self.disableAnimTick = false
	self.pulseSounds.Hum:FadeOutAndStop(0.1)
end

function PlayerPawn.PulseLight(self, pos)

	local light = World.CreateDynamicLight()
	light:SetDiffuseColor({0.5, 1, 1})
	light:SetSpecularColor({0.5, 1, 1})
	light:SetRadius(400)
	light:SetIntensity(2)
	light:SetStyle(bit.bor(kLightStyle_Diffuse, kLightStyle_Specular))
	light:SetInteractionFlags(kLightInteractionFlag_All)
	light:SetPos(pos)
	light:Link()
	
	-- animate
	local steps = {
		{ intensity = 0, time = 0 },
		{ intensity = 3, time = 0.05 },
		{ intensity = 0, time = 0.05 }
	}
	
	light:AnimateIntensity(steps, false)
	
	local f = function()
		light:Unlink()
	end
	
	World.gameTimers:Add(f, 0.5)
	
end

function PlayerPawn.PulseDamage(self, pos)

	local radius = PlayerSkills:PulseKillRadius()
	
	-- kill in a radius
	local radiusBox = {radius, radius, radius}
	local mins = VecSub(pos, radiusBox) 
	local maxs = VecAdd(pos, radiusBox)
	
	local targets = World.BBoxTouching(mins, maxs, kEntityClass_Monster)
	
	local bugs = false
	
	if (targets) then
		for k,v in pairs(targets) do
			if (v.PulseKill) then
				if (not bugs) then
					if (v.keys.classname == "info_bug") then
						bugs = true
					end
				end
				v:PulseKill()
			end
		end
	end
	
	if (bugs) then
		self.bugSounds.Kill:Play(kSoundChannel_FX, 0)
	end
end

function PlayerPawn.FirePulse(self, target, normal)
	self:PulseLight(VecAdd(target, VecScale(normal, 32)))
	self:PulseDamage(target)
	
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
	
	World.gameTimers:Add(f, 0.1)
	
	f = function()
		self:EndPulse()
	end
	
	self.pulseActive = false
	self.pulseSounds.Hum:Stop()
	self.pulseSounds.Fire:Play(kSoundChannel_FX, 0)
	
	local anim = self:LookupAnimation("pulse_fire")
	self:PlayAnim(anim , self.model).Seq(f)
	HUD:RechargePulse()
	
end

function PlayerPawn.DischargePulse(self)

	if (math.random() < 0.5) then
		self:PulseExplode()
		return true, false
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
		return false, true
	end
	
	self.pulseActive = false
	self:EndPulse()
	return false, false
end

function PlayerPawn.PulseExplode(self)
	self.pulseActive = false
	self.pulseSounds.Hum:Stop()
	self.pulseSounds.Explode:Play(kSoundChannel_FX, 0)
	
	local angle = self:TargetAngles()[3]
	local fwd = RotateVecZ({0,0,1}, angle)
	local pos = VecAdd(self:WorldPos(), VecAdd(VecScale(fwd, 64), {0,0,72}))
	
	self:PulseLight(pos)
	self:PulseDamage(self:WorldPos())
	
	self:Kill()
end

function PlayerPawn.Kill(self, instigator, killMessage)
	if (self.dead or PlayerPawn.GodMode) then
		return
	end
	self.dead = true
	
	self.customAnim = false
	self.disableAnimTick = false
	
	if (self.shieldActive) then
		self:EndShield()
	end
	self:SetMoveType(kMoveType_None)
	self:PlayAnim(self:LookupAnimation("death"), self.model)
	
	self:AbortBugAction()
	
	Game.entity:PlayerDied(killMessage)
end

function PlayerPawn.CheckTappedOn(self, e)
	return false
end

function PlayerPawn.OnEvent(self, cmd, args)
	COutLineEvent("PlayerPawn", "@player", cmd, args)
	
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
		self:Show(false)
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
		if (fp) then
			self:SetFloorPosition(fp)
			self:SetDesiredMove(nil)
			
			if (facing) then
				self:SetFacing(facing)
			end
		else
			COutLine(kC_Debug, "ERROR: PlayerPawn, waypoint '%s' doesn't have a valid floor position.", userId)
		end
	else
		COutLine(kC_Debug, "ERROR: PlayerPawn(teleport) there is no waypoint with a userid of '%s'.", userId)
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
				else
					self.validFloorPosition = false
					error("PlayerPawn.PlayCinematicAnim: walked off the floor.")
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

function PlayerPawn.BugStun(self, callback)

	if (self.bugStun) then
		return
	end
	
	self:SetDesiredMove(nil)
	
	if (Abducted.entity.manipulate) then
		Abducted.entity:EndManipulate()
	end
	
	if (Abducted.entity.pulse) then
		Abducted.entity:EndPulse()
	end
	
	local anim = self:LookupAnimation("bug_stun")

	self.disableAnimTick = true
	self.customMove = true
	self.state = nil
	self.bugStun = true
	self.bugStunCallback = callback
	self:Stop()
	
	self.bugSounds.Stun:Play(kSoundChannel_FX, 0)
	
	local blend = self:PlayAnim(anim, self.model)
	if (blend) then
		local f = function()
			if (not self.dead) then
				self.bugStun = false
				self.disableAnimTick = false
				self.customMove = false
				self.bugStunCallback = nil
				self:EnableFlags(kPhysicsFlag_Friction, false) -- resume walking
				self:TickPhysics()
				if (callback) then
					callback()
				end
			end
		end
		blend.Seq(f)
	end
end

function PlayerPawn.BugStomp(self, callback)

	if (self.stomping) then
		if (self.stompChain) then
			table.insert(self.stompChain, callback)
		end
		return
	end
	
	if (Abducted.entity.manipulate) then
		Abducted.entity:EndManipulate()
		self:SetDesiredMove(nil)
	end
	
	if (Abducted.entity.pulse) then
		Abducted.entity:EndPulse()
		self:SetDesiredMove(nil)
	end

	local anim = self:LookupAnimation("bug_squish")

	self.disableAnimTick = true
	self.customMove = true
	self.state = nil
	self:Stop()
	
	self.stomping = true
	self.stompChain = {callback}
	
	local blend = self:PlayAnim(anim, self.model)
	if (blend) then
		local f = function()
			if (not self.dead) then
				self.stomping = false
				self.disableAnimTick = false
				self.customMove = false
				self:EnableFlags(kPhysicsFlag_Friction, false) -- resume walking
				self:TickPhysics()
			end
		end
		blend.Seq(f)
		
		f = function(pawn, tag)
			if (tag == "@squish") then
				for k,v in pairs(self.stompChain) do
					v()
				end
				self.stompChain = nil
			end
		end
		blend.OnTag = f
	end
end

function PlayerPawn.AbortBugAction(self)

	if (self.stompChain) then
		for k,v in pairs(self.stompChain) do
			v()
		end
		self.stompChain = nil
	end
	
	if (self.bugStunCallback) then
		self.bugStunCallback()
		self.bugStunCallback = nil
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

function PlayerPawn.EnterArm(self)

	Abducted.entity.eatInput = true
	
	local anim = self:LookupAnimation("arm_pose_standing")
	local cameraMove = self:LookupAnimation("arm_default_flyin")

	local callbacks = {
		OnTag = function(self, tag)
			if (tag == "@arm_transition") then
				Arm:Start("chat")
			else
				World.PostEvent(tag)
			end
		end
	}
	
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)
	
	self.state = nil
	self.disableAnimTick = true
	self.model:BlendToState(anim)
	self:Stop()
	HUD:SetVisible(false)
end

function PlayerPawn.ExitArm(self)

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end,
		OnComplete = function()
			Abducted.entity.eatInput = false
			HUD:SetVisible(true)
		end
	}

	self.disableAnimTick = false
	local cameraMove = self:LookupAnimation("arm_default_flyout")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)

end

function PlayerPawn.EnterTerminal(self, terminal)

	local fp = self:FloorPosition()
	
	if (fp.floor == -1) then
		COutLine(kC_Debug, "ERROR: PlayerPawn.EngageTerminal: player must be on a floor!")
		return
	end
	
	-- move us relative to the terminal
	local terminalPos = terminal:WorldPos()
	local angle = terminal:Angles().pos[3]
	local targetPos = RotateVecZ({140,0,0}, angle - 90)
	
	targetPos = VecAdd(targetPos, terminalPos)
	fp = self:FindFloor(targetPos)
	if (fp == nil) then
		COutLine(kC_Debug, "ERROR: PlayerPawn.EngageTerminal: terminal->player position is not on a floor!")
		return
	end
	
	self:SetFloorPosition(fp)
	self.validFloorPosition = true
	
	self:SetFacing(angle + 180 - 90)
	
	local anim = self:LookupAnimation("arm_pose_standing")
	local cameraMove = self:LookupAnimation("puzzle_default_choice")
	
	World.PlayCinematic(cameraMove, bit.bor(kCinematicFlag_Loop, kCinematicFlag_AnimateCamera), 0, self)
	
	self.state = nil
	self.disableAnimTick = true
	self.model:BlendToState(anim)
	self:Stop()
	HUD:SetVisible(false)
	
	local f = function()
		terminal:ShowUI()
	end
	
	World.gameTimers:Add(f, 0.2)
end

function PlayerPawn.EnterHackGame(self, terminal)
	local cameraMove = self:LookupAnimation("puzzle_default_choice")
	World.StopCinematic(cameraMove)
	
	local callbacks = {
		OnTag = function(self, tag)
			if (tag == "@puzzle_transition") then
				terminal:DoHackGame()
			else
				World.PostEvent(tag)
			end
		end
	}
	
	cameraMove = self:LookupAnimation("puzzle_default_hack_flyin")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)
	
end

function PlayerPawn.LeaveHackGame(self, terminal, result)

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end,
		OnComplete = function()
			self.disableAnimTick = false
			TerminalScreen.Active = nil
			Abducted.entity.eatInput = false
			HUD:SetVisible(true)
			terminal:PostHackEvents(result)
			if (result) then
				Abducted.entity:VisibleCheckpoint()
			else
				terminal:PopupUI()
				terminal:CheckSignalDowngrade()
			end
		end
	}

	cameraMove = self:LookupAnimation("puzzle_default_hack_flyout")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)
end

function PlayerPawn.EnterSolveGame(self, terminal)
	local cameraMove = self:LookupAnimation("puzzle_default_choice")
	World.StopCinematic(cameraMove)
	
	local callbacks = {
		OnTag = function(self, tag)
			if (tag == "@puzzle_transition") then
				terminal:DoSolveGame()
			else
				World.PostEvent(tag)
			end
		end
	}
	
	cameraMove = self:LookupAnimation("puzzle_default_solve_flyin")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)
	
end

function PlayerPawn.LeaveSolveGame(self, terminal)

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end,
		OnComplete = function()
			self.disableAnimTick = false
			TerminalScreen.Active = nil
			Abducted.entity.eatInput = false
			HUD:SetVisible(true)
			terminal:PostSolveEvents(terminal)
			if (result) then
				Abducted.entity:VisibleCheckpoint()
			else
				terminal:PopupUI()
			end
		end
	}

	cameraMove = self:LookupAnimation("puzzle_default_solve_flyout")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)
end

function PlayerPawn.LeaveTerminal(self)
	local cameraMove = self:LookupAnimation("puzzle_default_choice")
	World.StopCinematic(cameraMove)
	self.disableAnimTick = false
	TerminalScreen.Active:PopupUI()
	TerminalScreen.Active = nil
	Abducted.entity.eatInput = false
	HUD:SetVisible(true)
end

function PlayerPawn.ShowShield(self, show)
	if (show and self.shieldActive) then
		self.shield.dm:SetVisbile(show)
		self.shieldSprite.dm:SetVisible(show)
		self:PauseShieldSound(false)
	elseif (not show) then
		self.shield.dm:SetVisible(false)
		self.shieldSprite.dm:SetVisible(false)
		self:PauseShieldSound(true)
	end
end

function PlayerPawn.PlayShieldSound(self)

	self.activeShieldSound = self.shieldSounds.Hum
	self.activeShieldSound:FadeVolume(0, 0)
	self.activeShieldSound:Play(kSoundChannel_FX, 0)
	self.activeShieldSound:FadeVolume(1, 0.3)
	
	self.shieldSounds.Activate:Play(kSoundChannel_FX, 0)
	self:ActivateShieldSpark()
	self:ActivateShieldHum2()
end

function PlayerPawn.ActivateShieldSpark(self)
	local f = function()
		self:DoShieldSpark()
	end
	
	self.shieldFlavorTimer = World.gameTimers:Add(f, FloatRand(1, 10))
end

function PlayerPawn.DoShieldSpark(self)
	local f = function()
		self:DoShieldSpark()
	end
	
	local num = IntRand(1, 3)
	self.shieldSounds.Spark:Play(kSoundChannel_FX, 0)
	
	if (num > 1) then
		local spark = function()
			if (self.shieldActive) then
				self.shieldSounds.Spark:Play(kSoundChannel_FX, 0)
			end
		end
		
		local t = 0
		
		for i=2,num do
			local x = FloatRand(0.2, 0.7)
			World.gameTimers:Add(spark, x+t)
			t = t + x
		end
	end
	
	self.shieldFlavorTimer = World.gameTimers:Add(f, FloatRand(num, 7))
end

function PlayerPawn.ActivateShieldHum2(self)
	local f = function()
		self:DoShieldHum2()
	end
	
	self.shieldFlavorTimer2 = World.gameTimers:Add(f, FloatRand(4, 10))
end

function PlayerPawn.DoShieldHum2(self)
	local f = function()
		self:DoShieldHum2()
	end
	self.shieldSounds.Deactivate:Play(kSoundChannel_FX, 0)
	self.shieldFlavorTimer2 = World.gameTimers:Add(f, FloatRand(4, 10))
end

function PlayerPawn.PauseShieldSound(self, pause)
	if (self.shieldSoundPaused == pause) then
		return
	end
	
	if (self.activeShieldSound) then
		if (self.shieldFlavorTimer) then
			self.shieldFlavorTimer:Clean()
			self.shieldFlavorTimer = nil
		end
		if (self.shieldFlavorTimer2) then
			self.shieldFlavorTimer2:Clean()
			self.shieldFlavorTimer2 = nil
		end
		self.shieldSoundPaused = pause
		self.activeShieldSound:Pause(pause)
		self.shieldSounds.Deactivate:Stop()
		if (not pause) then
			self:ActivateShieldSpark()
			self:ActivateShieldHum2()
		end
	end
end

function PlayerPawn.StopShieldSound(self)
	self.shieldSoundPaused = false
	if (self.shieldFlavorTimer) then
		self.shieldFlavorTimer:Clean()
		self.shieldFlavorTimer = nil
	end
	if (self.shieldFlavorTimer2) then
		self.shieldFlavorTimer2:Clean()
		self.shieldFlavorTimer2 = nil
	end
	if (self.activeShieldSound) then
		self.activeShieldSound:Stop()
		self.activeShieldSound = nil
		self.shieldSounds.Deactivate:Play(kSoundChannel_FX, 0)
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
	
	self.pulseSounds.Hum:Stop()
	
	if (self.activeShieldSound) then
		self.activeShieldSound:Stop()
		self.activeshieldSound = nil
	end
	self.shieldSoundPaused = false
	
	if (self.shieldFlavorTimer) then
		self.shieldFlavorTimer:Clean()
		self.shieldFlavorTimer = nil
	end
	if (self.shieldFlavorTimer2) then
		self.shieldFlavorTimer2:Clean()
		self.shieldFlavorTimer2 = nil
	end
		
	if (state.shieldActive == "true") then
		self:BeginShield()
	else
		self.shieldActive = false
		self.shield.dm:BlendTo({0,0,0,0}, 0)
		self.shieldSprite.dm:BlendTo({0,0,0,0}, 0)
	end
	
	self:Show(state.visible == "true")
	
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
	
	self.animState = nil
	self:SelectAnimState(state.animState)
	
	local anim = self:LookupAnimation("idle")
	if (anim) then
		self.model:BlendImmediate(anim)
	end
	
	self:Link()
end

info_player_start = PlayerPawn
