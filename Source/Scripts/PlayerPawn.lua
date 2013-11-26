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
PlayerPawn.GodMode = false
PlayerPawn.kMaxShieldDamage = 100
PlayerPawn.PulseSmokePPS = 7

PlayerPawn.DeathSounds = {
	"Audio/VO_Eve_BigPain01",
	"Audio/VO_Eve_BigPain02",
	"Audio/VO_Eve_Scream01",
	"Audio/VO_Eve_Scream02",
	"Audio/VO_Eve_Scream03",
	"Audio/VO_Eve_Scream04",
}

PlayerPawn.FallDeathSounds = {
	"Audio/VO_Eve_Scream05"
}

PlayerPawn.PainSounds = {
	"Audio/VO_Eve_Pain01",
	"Audio/VO_Eve_Pain02",
	"Audio/VO_Eve_Pain03",
	"Audio/VO_Eve_Pain04",
	"Audio/VO_Eve_Pain05",
	"Audio/VO_Eve_Pain06",
	"Audio/VO_Eve_Pain07"
}

PlayerPawn.SurpriseSounds = {
	"Audio/VO_Eve_Surprise01",
	"Audio/VO_Eve_Surprise02",
	"Audio/VO_Eve_Surprise03",
	"Audio/VO_Eve_Surprise04",
	"Audio/VO_Eve_Surprise05",
	"Audio/VO_Eve_Surprise06",
	"Audio/VO_Eve_Surprise07",
	"Audio/VO_Eve_What02",
	"Audio/VO_Eve_What05"
}

PlayerPawn.GruntSounds = {
	"Audio/VO_Eve_Grunt01",
	"Audio/VO_Eve_Grunt03",
	"Audio/VO_Eve_Grunt04"
}

PlayerPawn.AnimationStates = {
	default = {
	-- any animations *not* listed here will pass-through unaltered
	-- example, idle isn't listed here, so "idle" will just become "idle"
		OnSelect = function()
			HUD:EnableAll()
		end,
		bbox = {mins = {-18, -18, 0}, maxs = {18, 18, 128}},
		shadowBox = {mins = {-68, -68, 0}, maxs = {68, 68, 128}},
		cameraShift = {0, 0, 32}
	},
	limp = {
		idle = "limpidle",
		walk = "limpcrawl",
		death = "limpdeath",
		manipulate_idle = "limpmanidle",
		manipulate_left = "limpmanleft",
		manipulate_right = "limpmanright",
		manipulate_up = "limpmanup",
		manipulate_down = "limpmandown",
		pulse_idle = "limpmanidle",
		pulse_fire = "limpmanup",
		pulse_overload1 = "limp_pulse_overload1",
		pulse_overload2 = "limp_pulse_overload2",
		pulse_overload3 = "limp_pulse_overload3",
		arm_default_flyin = "arm_limp_flyin",
		arm_default_flyout = "arm_limp_flyout",
		arm_pose_standing = "arm_pose_limp",
		puzzle_default_choice = "puzzle_limp_choice",
		smack_metadata = "limpmanup",
		speedScale = 0.05,
		tapAdjust = 48, -- CheckTappedOn
		bbox = {mins = {-40, -40, 0}, maxs = {40, 40, 90}},
		canRun = false,
		OnSelect = function()
			HUD:EnableAll()
		end,
		cameraShift = {0, 0, -32}
	},
	limpscrunch = {
		idle = "limp_scrunched_idle",
		walk = "limp_scrunched_forward",
		manipulate_idle = "limpmanidle",
		manipulate_left = "limpmanleft",
		manipulate_right = "limpmanright",
		manipulate_up = "limpmanup",
		manipulate_down = "limpmandown",
		pulse_idle = "limpmanidle",
		pulse_fire = "limpmanup",
		pulse_overload1 = "limp_pulse_overload1",
		pulse_overload2 = "limp_pulse_overload2",
		pulse_overload3 = "limp_pulse_overload3",
		arm_default_flyin = "arm_limp_flyin",
		arm_default_flyout = "arm_limp_flyout",
		arm_pose_standing = "arm_pose_limp",
		puzzle_default_choice = "puzzle_limp_choice",
		smack_metadata = "limpmanup",
		speedScale = 0.4,
		tapAdjust = 48, -- CheckTappedOn
		bbox = {mins = {-32, -32, 0}, maxs = {32, 32, 90}},
		canRun = false,
		OnSelect = function()
			HUD:EnableAll()
		end,
		cameraShift = {0, 0, -48}
	},
	walkfast = {
		walk = "walkfast",
		speedScale = 2.2,
		OnSelect = function()
			HUD:EnableAll()
		end
	},
	crawl = {
		idle = "limpidle",
		walk = "limpcrawl",
		death = "limpdeath",
		manipulate_idle = "limpmanidle",
		manipulate_left = "limpmanleft",
		manipulate_right = "limpmanright",
		manipulate_up = "limpmanup",
		manipulate_down = "limpmandown",
		pulse_idle = "limpmanidle",
		pulse_fire = "limpmanup",
		arm_default_flyin = "arm_limp_flyin",
		arm_default_flyout = "arm_limp_flyout",
		arm_pose_standing = "arm_pose_limp",
		puzzle_default_choice = "puzzle_limp_choice",
		smack_metadata = "limpmanup",
		pulse_overload1 = "limpmanidle",
		pulse_overload2 = "limpmanidle",
		pulse_overload3 = "limpmanidle",
		speedScale = 0.75,
		tapAdjust = 48, -- CheckTappedOn
		bbox = {mins = {-40, -40, 0}, maxs = {40, 40, 90}},
		canRun = false,
		OnSelect = function()
			HUD:EnableAll()
		end,
		cameraShift = {0, 0, 32}
	},
}

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	self:Precache("Objects/mine")
	self:Precache("Audio/mine_activate")
	self:Precache("Audio/mine_hum")
	self:Precache("Audio/mine_armed")
	self:Precache("Audio/mine_explode")
	self:Precache("Audio/mine_tripped")
	
	local loadSound = function (sound)
		sound = World.LoadSound(sound)
		self:AttachSound(sound)
		--sound:FadeVolume(0.7, 0)
		return sound
	end
	
	map(PlayerPawn.DeathSounds, loadSound)
	map(PlayerPawn.FallDeathSounds, loadSound)
	map(PlayerPawn.PainSounds, loadSound)
	map(PlayerPawn.SurpriseSounds, loadSound)
	map(PlayerPawn.GruntSounds, loadSound)
	
	self.charDBStatus = StringForString(self.keys.health_status, "good")
	
	self.mineDrop = World.LoadSound("Audio/mine_drop")
	self:AttachSound(self.mineDrop)
	
	self.mineMaterials = {
		Default = World.Load("Objects/mine_M"),
		Armed = World.Load("Objects/mine_armed_M"),
		Tripped = World.Load("Objects/mine_tripped_M")
	}
	
	self.arm_M = World.Load("Characters/armscreen1_M")
	self.armSignaled_M = World.Load("Characters/armscreen2_M")
	
	self.numActiveMines = 0
	
	self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	self:SetLightInteractionFlags(kLightInteractionFlag_Player)
		
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
	if (bbox == nil) then
		bbox = PlayerPawn.AnimationStates.default.bbox
	end
	
	local shadowBox = set.shadowBox
	if (shadowBox == nil) then
		shadowBox = PlayerPawn.AnimationStates.default.shadowBox
	end
	
	local cameraShift = set.cameraShift
	if (cameraShift == nil) then
		cameraShift = PlayerPawn.AnimationStates.default.cameraShift
	end
	
	self:SetMins(bbox.mins)
	self:SetMaxs(bbox.maxs)
	self:SetShadowMins(shadowBox.mins)
	self:SetShadowMaxs(shadowBox.maxs)
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	self:SetCameraShift(cameraShift)
	
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
	self.shieldSprite.dm:Skin()
	self.shieldSprite.dm:BlendTo({0,0,0,0}, 0)
	
	self.shieldSpriteSpark = World.CreateSpriteBatch(1, 1)
	self.shieldSpriteSpark.material = World.Load("FX/shieldspritespark_M")
	self.shieldSpriteSpark.dm = self:AttachDrawModel(self.shieldSpriteSpark, self.shieldSpriteSpark.material)
	self.shieldSpriteSpark.dm:SetBounds(self:Mins(), self:Maxs())
	self.shieldSpriteSpark.sprite = self.shieldSpriteSpark.dm:AllocateSprite()
	self.shieldSpriteSpark.dm:SetSpriteData(
		self.shieldSpriteSpark.sprite,
		{
			pos = {0, 0, 68}, -- relative to drawmodel
			size = {148, 158},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.shieldSpriteSpark.dm:Skin()
	self.shieldSpriteSpark.dm:BlendTo({0,0,0,0}, 0)
	
	self.shieldSpriteDamaged = World.CreateSpriteBatch(1, 1)
	self.shieldSpriteDamaged.material = World.Load("FX/shieldspritedamaged_M")
	self.shieldSpriteDamaged.dm = self:AttachDrawModel(self.shieldSpriteDamaged, self.shieldSpriteDamaged.material)
	self.shieldSpriteDamaged.dm:SetBounds(self:Mins(), self:Maxs())
	self.shieldSpriteDamaged.sprite = self.shieldSpriteDamaged.dm:AllocateSprite()
	self.shieldSpriteDamaged.dm:SetSpriteData(
		self.shieldSpriteDamaged.sprite,
		{
			pos = {0, 0, 68}, -- relative to drawmodel
			size = {148, 158},
			rgba = {1, 1, 1, 1},
			rot = 0
		}
	)
	self.shieldSpriteDamaged.dm:BlendTo({0,0,0,0}, 0)
	
	self.shieldSounds = {
		Activate = World.LoadSound("Audio/AFX_ShieldActivate"),
		Hum = World.LoadSound("Audio/AFX_ShieldHum"),
		Deactivate = World.LoadSound("Audio/AFX_ShieldDeactivate"),
		Spark = World.LoadSound("Audio/AFX_ShieldSpark", 3),
		ImpactLight = World.LoadSound("Audio/AFX_ShieldImpactLight", 2),
		ImpactHard = World.LoadSound("Audio/AFX_ShieldImpactAggressive")
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
		Fire = World.LoadSound("Audio/AFX_PulseEnergyImpact", 3),
		Overload1 = World.LoadSound("Audio/pulse_overload1"),
		Overload2 = World.LoadSound("Audio/pulse_overload2"),
		Overload3 = World.LoadSound("Audio/pulse_overload3"),
	}
	
	self.pulseSounds.Hum:SetLoop(true)
	self.pulseSounds.Overload1:SetLoop(true)
	self.pulseSounds.Overload2:SetLoop(true)
	self.pulseSounds.Overload3:SetLoop(true)
	
	self.pulseSmoke = World.Load("FX/pulsesmoke")
	self.pulseSmoke:SetMaxParticles(100)
	self.pulseSmoke.dm = self:AttachDrawModel(self.pulseSmoke)
	self.pulseSmoke.dm:SetBounds(self:Mins(), self:Maxs())
	self.pulseSmoke.dm:SetPositionMode(kParticleEmitterDrawModelPositionMode_World)
	self.pulseSmoke.dm:SetLocalDir({0,0,1})
	self.model.dm:AttachChildToBone(self.pulseSmoke.dm, self.model.handBone)
		
	self.pulseSparks = World.Load("FX/pulseimpactsparks")
	self.pulseSparks:SetMaxParticles(250)
	self.pulseSparks.dm = self:AttachDrawModel(self.pulseSparks)
	self.pulseSparks.dm:SetPositionMode(kParticleEmitterDrawModelPositionMode_World)
	self.pulseSparks.dm:SetBounds(self:Mins(), self:Maxs())
	
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
	
	self:Link()
	
	self:AddTickable(kTF_PostPhysics, function () PlayerPawn.TickPhysics(self) end)
	
	self.shieldActive = false
	self.manipulateActive = false
	self.pulseActive = false
	self.shieldAutoActivateTime = 0
	
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

function PlayerPawn.SwapModelTextures(model)

	if (GameDB.playerStyle ~= 1) then
	
		local swaps = {
			{ "Characters/hair1_M", string.format("Characters/hair%d_M", GameDB.playerStyle) },
			{ "Characters/femalehands1_M", string.format("Characters/femalehands%d_M", GameDB.playerStyle) },
			{ "Characters/femalehead1_M", string.format("Characters/femalehead%d_M", GameDB.playerStyle) },
			{ "Characters/liquidmetal_M", string.format("Characters/liquidmetal%d_M", GameDB.playerStyle) }
		}

		for k,v in pairs(swaps) do
		
			v[1] = World.Load(v[1])
			v[2] = World.Load(v[2])
			model.dm:ReplaceMaterial(v[1], v[2])
		
		end
	
	end

end

function PlayerPawn.PostSpawn(self)

	local state = self.animState
	self.animState = nil
	self:SelectAnimState(state)	

	-- player styles / portraits
	PlayerPawn.SwapModelTextures(self.model)
end

function PlayerPawn.PlaySoundGroup(self, group, probability, delay)

	if (math.random() <= probability) then
		local idx = IntRand(1, #group)
		if (delay) then
			local f = function()
				group[idx]:Play(kSoundChannel_FX, 0)
			end
			World.gameTimers:Add(f, delay)
		else
			group[idx]:Play(kSoundChannel_FX, 0)
		end
	end

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
		
		if (set) then
			local bbox = set.bbox
			if (not bbox) then
				bbox = PlayerPawn.AnimationStates.default.bbox
			end
			
			local shadowBox = set.shadowBox
			if (not shadowBox) then
				shadowBox = PlayerPawn.AnimationStates.default.shadowBox
			end
			
			local cameraShift = set.cameraShift
			if (cameraShift == nil) then
				cameraShift = PlayerPawn.AnimationStates.default.cameraShift
			end
			
			self:SetMins(bbox.mins)
			self:SetMaxs(bbox.maxs)
			self:SetShadowMins(shadowBox.mins)
			self:SetShadowMaxs(shadowBox.maxs)
			self.model.dm:SetBounds(self:Mins(), self:Maxs())
			self:SetCameraShift(cameraShift)
			
		end
		
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
	self.powerBubble = false
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
	self.powerBubble = false
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

function PlayerPawn.ShowShieldDamage(self)

	self.shieldSpriteDamaged.dm:BlendTo({1,1,1,1}, 0.1)
	
	local f = function()
		self.shieldSpriteDamaged.dm:BlendTo({0,0,0,0}, 0.15)
	end

	World.gameTimers:Add(f, 0.5)

end

function PlayerPawn.ShieldSpark(self)

	if (self.shieldSparkTimer) then
		self.shieldSparkTimer:Clean()
	end

	self.shieldSpriteSpark.dm:BlendTo({1,1,1,1}, 0.1)

	local f = function()
		self.shieldSpriteSpark.dm:BlendTo({0,0,0,0}, 0.1)
	end

	self.shieldSparkTimer = World.gameTimers:Add(f, 0.1)
end

function PlayerPawn.Stop(self)
	self:EnableFlags(kPhysicsFlag_Friction, true)
end

function PlayerPawn.LookAt(self, pos)

	local v = VecSub(pos, self:WorldPos())
	self:SetFacing(LookAngles(VecNorm(v))[3])

end

function PlayerPawn.BeginPulse(self, dischargeTime)
	self.pulseActive = true
	self.disableAnimTick = true
	self.state = nil
	local anim = self:LookupAnimation("pulse_idle")
	self:PlayAnim(anim, self.model)
	self:Stop()
	self.pulseSounds.Hum:FadeVolume(0, 0)
	self.pulseSounds.Hum:FadeVolume(1, 0.1)
	self.pulseSounds.Hum:Play(kSoundChannel_FX, 0)
	self:StartPulseOverload(dischargeTime)
end

function PlayerPawn.EndPulse(self)
	self.disableAnimTick = false
	self.pulseSounds.Hum:FadeOutAndStop(0.1)
	self:EndPulseOverload()
end

function PlayerPawn.PulseLight(self, pos)

	local light = World.CreateDynamicLight()
	light:SetDiffuseColor({0.5, 1, 1})
	light:SetSpecularColor({0.5, 1, 1})
	light:SetRadius(400)
	light:SetIntensity(2)
	light:SetShadowWeight(2)
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

	local baseDamage = PlayerSkills:PulseDamage()
	local radius = PlayerSkills:PulseDamageRadius()
	local maxDamageRadius = radius*0.25
	local oneMinusMaxDamageRadius = radius-maxDamageRadius
	
	-- damage in a radius
	local radiusBox = {radius, radius, radius}
	local mins = VecSub(pos, radiusBox) 
	local maxs = VecAdd(pos, radiusBox)
	
	local targets = World.BBoxTouching(mins, maxs, kEntityClass_Monster)
	
	local bugs = false
		
	if (targets) then
		for k,v in pairs(targets) do
			if (v.PulseDamage) then
				if (not bugs) then
					if (v.keys.classname == "info_bug") then
						bugs = true
					end
				end
				
				local dd = VecMag(VecSub(pos, v:WorldPos()))
				local damage = baseDamage
				
				if (dd > maxDamageRadius) then
					damage = (oneMinusMaxDamageRadius-dd)/oneMinusMaxDamageRadius*baseDamage
				end
				
				if (damage > 0) then
					v:PulseDamage(damage)
				end
			end
		end
	end
	
	if (bugs) then
		self.bugSounds.Kill:Play(kSoundChannel_FX, 0)
	end
end

function PlayerPawn.StartPulseOverload(self, dischargeTime)

	local updateTime = dischargeTime / 4
	
	local f = function()
		self:UpdatePulseOverload()
	end
	
	self.pulseOverloadTimer = World.gameTimers:Add(f, updateTime, true)
	self:UpdatePulseOverload()

end

function PlayerPawn.UpdatePulseOverload(self)
	if (self.pulseOverloadStage == nil) then
		self.pulseOverloadStage = 1
		return
	end
	
	self.pulseOverloadStage = self.pulseOverloadStage + 1

	if (self.pulseOverloadStage == 2) then
	
		local anim = self:LookupAnimation("pulse_overload1")
		self:PlayAnim(anim, self.model)
		self.pulseSounds.Overload1:Play(kSoundChannel_FX, 0)
		
	elseif (self.pulseOverloadStage == 3) then
	
		local anim = self:LookupAnimation("pulse_overload2")
		self:PlayAnim(anim, self.model)
		self.pulseSounds.Overload1:Stop()
		self.pulseSounds.Overload2:Play(kSoundChannel_FX, 0)
		self.pulseSmoke:SetPPS(10)
		
	elseif (self.pulseOverloadStage == 4) then
	
		local anim = self:LookupAnimation("pulse_overload3")
		self:PlayAnim(anim, self.model)
		self.pulseSounds.Overload2:Stop()
		self.pulseSounds.Overload3:Play(kSoundChannel_FX, 0)
		self.pulseSmoke:SetPPS(25)
		
	end
end

function PlayerPawn.EndPulseOverload(self)

	self.pulseSounds.Overload1:Stop()
	self.pulseSounds.Overload2:Stop()
	self.pulseSounds.Overload3:Stop()
	self.pulseSmoke:SetPPS(0)

	self.pulseOverloadStage = nil
	
	if (self.pulseOverloadTimer) then
		self.pulseOverloadTimer:Clean()
		self.pulseOverloadTimer = nil
	end
end

function PlayerPawn.FirePulse(self, target, normal, sparks)
	local f = function()
		self:EndPulse()
	end
	
	self:EndPulseOverload()
	self.pulseSounds.Hum:Stop()
	self.pulseSounds.Fire:Play(kSoundChannel_FX, 0)
	self:InternalFirePulse(target, normal, sparks).Seq(f)
	self.pulseActive = false
	HUD:RechargePulse()
	
end

function PlayerPawn.RapidFirePulse(self, target, normal, sparks)
	self:EndPulseOverload()
	self.pulseSounds.Hum:Stop()
	self.pulseSounds.Fire:Play(kSoundChannel_FX, 0)
	self:InternalFirePulse(target, normal, sparks)
end

function PlayerPawn.InternalFirePulse(self, target, normal, sparks)
	if (Abducted.entity.pulseCount == 1) then -- only on first shot
		self:PulseLight(VecAdd(target, VecScale(normal, 32)))
	end
	
	if ((sparks==true) or (sparks==nil)) then
		self.pulseSparks.dm:SetWorldPos(target)
		self.pulseSparks.dm:SetLocalDir(normal)
		self.pulseSparks:Spawn(50)
	end
	
	World.viewController:BlendToLookTarget(
		target, -- target position to "look" at
		0.2, -- in time
		0.2, -- out time
		0, -- hold time
		0.6,  -- max weight (how much towards the target we look, 1 = all the way)
		1.4, -- smooth factor for in time, these serve to "tighten" or "loosen" the motion
		1.4, -- smooth factor for out time
		0  -- never cull this look target based on view angle
	)
	
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
	
	local anim = self:LookupAnimation("pulse_fire")
	return self:PlayAnim(anim, self.model)
	
end

function PlayerPawn.DischargePulse(self)

	self:EndPulseOverload()

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
		self:PulseDamage(trace.traceEnd)
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
	local fwd = RotateVecZ({1,0,0}, angle)
	local pos = VecAdd(self:WorldPos(), VecAdd(VecScale(fwd, 64), {0,0,72}))
	
	self:InternalFirePulse(pos, fwd, VecNeg(fwd))
	self:PulseDamage(self:WorldPos())
	
	if (self.shieldActive) then
		self:EndShield()
	end
	
	self:Kill()
end

function PlayerPawn.PowerBubble(self)

	local angle = self:TargetAngles()[3]
	local fwd = RotateVecZ({1,0,0}, angle)
	local pos = VecAdd(self:WorldPos(), VecAdd(VecScale(fwd, 64), {0,0,72}))
	
	self:FirePulse(pos, VecNeg(fwd))
	self:StartPowerBubble()
end

function PlayerPawn.CheckPowerBubbleKill(self, target, targetPos)
	if (self.powerBubble) then
		local pos = self:WorldPos()
		local dd = VecMag(VecSub(pos, targetPos))
		if (dd <= PlayerSkills:PowerBubbleZapRange()) then
			if (target.Kill) then
				target:Kill(self)
			end
			COutLine(kC_Debug, "PowerBubble - Zap!")
			self.powerBubbleZaps = self.powerBubbleZaps - 1
			if (self.powerBubbleZaps < 1) then
				self:EndPowerBubble()
			end
			return true
		end
	end
	
	return false
end

function PlayerPawn.StartPowerBubble(self)
	self.powerBubble = true
	self.powerBubbleZaps = PlayerSkills:PowerBubbleZapCount()
end

function PlayerPawn.EndPowerBubble(self)
	self.powerBubble = false
end

function PlayerPawn.Damage(self, damage, instigator, killMessage, specialCommand)
	if (self.dead) then
		return
	end
	
	if (instigator and instigator.isMine) then
		skill = PlayerSkills.Skills.Mines:CurrentLevel()
		if (skill == 2) then
			if (not self.shieldActive) then
			-- only stuns us
				self:PlayUninterruptable("damage_stun")
				self:PlaySoundGroup(PlayerPawn.PainSounds, 1)
				return
			end
		elseif (skill >= 3) then
			return -- we don't take damage from mines
		end
	end
	
	if (self.shieldActive) then
		if (damage <= PlayerPawn.kMaxShieldDamage) then
			self.shieldSounds.ImpactLight:Play(kSoundChannel_FX, 0)
			self:ShieldSpark()
			return
		end
		
		-- This damage amount overloaded our shield
		-- Play a flickering shield effect to show that it blocked the damage
		-- but it shorted out
		HUD:ExpireShield(PlayerSkills:MaxShieldTime())
		self.shieldSounds.ImpactHard:Play(kSoundChannel_FX, 0)
		self:ShowShieldDamage()
		self:ShieldSpark()
		
		if (damage < (PlayerPawn.kMaxShieldDamage*2)) then
		-- doesn't kill us
			self:PlayUninterruptable("damage_stun")
			self:PlaySoundGroup(PlayerPawn.PainSounds, 1)
			return
		end
	elseif ((PlayerSkills.Defender > 0) and (PlayerSkills:ShieldUnlocked())) then
		-- Auto activate shield defense
		local cd = PlayerSkills:ShieldAutoActivateCooldown()
		local elapsed = GameDB.realTime - self.shieldAutoActivateTime
		if ((elapsed >= cd) or (self.shieldAutoActivateTime == 0)) then
			self.shieldAutoActivateTime = GameDB.realTime
			if (damage <= PlayerPawn.kMaxShieldDamage) then
				self:BeginShield()
				self.shieldSounds.ImpactLight:Play(kSoundChannel_FX, 0)
				self:ShieldSpark()
				return
			end
			
			-- This damage amount overloaded our shield
			-- Play an flickering shield effect to show that it blocked the damage
			-- but it shorted out
			HUD:ExpireShield(PlayerSkills:MaxShieldTime())
			self.shieldSounds.ImpactHard:Play(kSoundChannel_FX, 0)
			self:ShowShieldDamage()
			self:ShieldSpark()
			
			if (damage < (PlayerPawn.kMaxShieldDamage*2)) then
			-- doesn't kill us
				self:PlayUninterruptable("damage_stun")
				self:PlaySoundGroup(PlayerPawn.PainSounds, 1)
				return
			end
		end
	end
	
	if (not PlayerPawn.GodMode) then
		self:Kill(instigator, killMessage, specialCommand)
	end
end

function PlayerPawn.Kill(self, instigator, killMessage, specialCommand)

	if (self.dead) then
		return
	end
	
	self.dead = true
	
	self.customAnim = false
	self.disableAnimTick = false
	
	self:SetMoveType(kMoveType_None)
	self:PlayAnim(self:LookupAnimation("death"), self.model)
	self:PlaySoundGroup(PlayerPawn.DeathSounds, 1)
	
	self:AbortBugAction()
	
	if (self.shieldActive) then
		self:EndShield()
	end
	
	Game.entity:PlayerDied(killMessage, specialCommand)
	PlayerInput:Flush()
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
	elseif (cmd == "set_health_status") then
		assert((args == "good") or (args == "bad") or (args == "stable"))
		self.charDBStatus = args
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
			self:Link()
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

function PlayerPawn.PlayUninterruptable(self, anim, callback)
	local anim = self:LookupAnimation(anim)

	self.disableAnimTick = true
	self.customMove = true
	self.state = nil
	self:Stop()
	
	local blend = self:PlayAnim(anim, self.model)
	if (blend) then
		local f = function()
			if (not self.dead) then
				self.disableAnimTick = false
				self.customMove = false
				self:TickPhysics()
			end
		end
		blend = blend.Seq(f)
		if (callback) then
			blend = blend.Seq(callback)
		end
		return blend
	end
	
	return nil
end

function PlayerPawn.PlayAnimSounds(self, anim)
	if ((anim == "climb_up_64") or (anim == "climb_up_96") or (anim == "climb_down_64") or (anim == "climb_down_96")) then
		self:PlaySoundGroup(PlayerPawn.GruntSounds, 0.75, 0.6)
	elseif ((anim == "jump128") or (anim == "jump196")) then
		self:PlaySoundGroup(PlayerPawn.GruntSounds, 1, 0.5)
	elseif (anim == "limp_dropdown_death") then
		self:PlaySoundGroup(PlayerPawn.FallDeathSounds, 1, 1)
	elseif (anim == "limp_dropdown_high") then
		self:PlaySoundGroup(PlayerPawn.DeathSounds, 1, 1)
	elseif (anim == "limp_dropdown_med") then
		self:PlaySoundGroup(PlayerPawn.DeathSounds, 1, 1)
	elseif (anim == "limp_dropdown_low") then
		self:PlaySoundGroup(PlayerPawn.DeathSounds, 1, 1)
	elseif (anim == "limp_ship_crawldown2") then
		self:PlaySoundGroup(PlayerPawn.DeathSounds, 1, 1)
	elseif (anim == "limp_ship_crawldown3") then
		self:PlaySoundGroup(PlayerPawn.DeathSounds, 1, 1)
	end
end

function PlayerPawn.PlayCinematicAnim(self, args)
	args = Tokenize(args)
	local anim = self:LookupAnimation(args[1])
	if (anim) then
		self:PlayAnimSounds(anim)
		
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
					self:Link()
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

function PlayerPawn.NumMines(self)
	return self.numActiveMines
end

function PlayerPawn.DropMine(self)
	self:Stop()
	
	self.mines = self.mines or {}
	self.numActiveMines = self.numActiveMines + 1
	
	self.mineDrop:Play(kSoundChannel_FX, 0)
	
	local pos = self:WorldPos()
	local floorZ = pos[3]
	pos[3] = pos[3] + 32
	
	local keys = {
		classname = "info_mine",
		origin = Vec3ToString(pos)
	}
	
	local mine = World.TempSpawn(keys)
	mine:DropToFloor(floorZ)
	
	table.insert(self.mines, mine)
	mine.idx = #self.mines
	
	return self.numActiveMines
end

function PlayerPawn.DetonateMines(self)
	self:Stop()
	local mines = self.mines
	self.mines = nil
	self.numActiveMines = 0
	
	for k,v in pairs(mines) do
		v:Trip()
	end
end

function PlayerPawn.RemoveMine(self, mine)
	if (self.mines) then
		self.numActiveMines = self.numActiveMines - 1
		table.remove(self.mines, mine.idx)
		if (self.numActiveMines == 0) then
			self.mines = nil
			HUD:NotifyZeroMines()
		end
	end
end

function PlayerPawn.SmackMetadata(self, callback)
	if (self.bugStun) then
		return false
	end
	if (Abducted.entity.manipulate) then
		return false
	end
	
	if (Abducted.entity.pulse) then
		return false
	end
	
	self:PlayUninterruptable("smack_metadata")
		
	if (callback) then
		World.globalTimers:Add(callback, 0.4)
	end
	
	return true
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
	
	self:PlaySoundGroup(PlayerPawn.PainSounds, 1)
	
	local f = function()
		if (not self.dead) then
			self.bugStun = false
			self.bugStunCallback = nil
			if (callback) then
				callback()
			end
		end
	end
	
	self.bugStun = true
	self.bugStunCallback = callback
	
	EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", "EVENT_LOG_BUG_CLIMB")
	
	local blend = self:PlayUninterruptable("bug_stun")
	if (blend) then
		blend.And(f)
	end
	
	self.bugSounds.Stun:Play(kSoundChannel_FX, 0)
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
	
	self:PlaySoundGroup(PlayerPawn.SurpriseSounds, 1)

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

function PlayerPawn.EnterArm(self, mode, dbTopic)

	if (self.bugStun or self.customMove) then
		return
	end
	
	self.bugStun = true
	self.oldGodMode = PlayerPawn.GodMode
	PlayerPawn.GodMode = true

	if (mode == nil) then
		mode = "chat"
	end
	
	Abducted.entity.eatInput = true
	
	local anim = self:LookupAnimation("arm_pose_standing")
	local cameraMove = self:LookupAnimation("arm_default_flyin")

	local callbacks = {
		OnTag = function(self, tag)
			if (tag == "@arm_transition") then
				Arm:Start(mode, dbTopic)
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
			self.bugStun = false
			PlayerPawn.GodMode = self.oldGodMode
		end
	}

	self.disableAnimTick = false
	local cameraMove = self:LookupAnimation("arm_default_flyout")
	World.PlayCinematic(cameraMove, kCinematicFlag_AnimateCamera, 0, self, Game.entity, callbacks)

end

function PlayerPawn.EnterTerminal(self, terminal)

	self.bugStun = true
	self.oldGodMode = PlayerPawn.GodMode
	PlayerPawn.GodMode = true
	
	-- move us relative to the terminal
	local terminalPos = terminal:WorldPos()
	local angle = terminal:Angles().pos[3]
	local targetPos = RotateVecZ({140,0,0}, angle - 90)
	
	targetPos = VecAdd(targetPos, terminalPos)
	local fp = self:FindFloor(targetPos)
	if (fp == nil) then
		COutLine(kC_Debug, "ERROR: PlayerPawn.EngageTerminal: terminal->player position is not on a floor!")
		self.bugStun = false
		PlayerPawn.GodMode = self.oldGodMode
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
			self.bugStun = false
			PlayerPawn.GodMode = self.oldGodMode
			terminal:PostHackEvents(result)
			if (result) then
				Achievements:HackedPuzzle(terminal.hackDifficulty)
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

function PlayerPawn.LeaveSolveGame(self, terminal, result)

	local callbacks = {
		OnTag = function(self, tag)
			World.PostEvent(tag)
		end,
		OnComplete = function()
			self.disableAnimTick = false
			TerminalScreen.Active = nil
			Abducted.entity.eatInput = false
			HUD:SetVisible(true)
			self.bugStun = false
			PlayerPawn.GodMode = self.oldGodMode
			if (result ~= "x") then
				terminal:PostSolveEvents(result == "w")
			end
			if (result == "w") then
				Achievements:SolvedPuzzle(#terminal.solveGlyphs)
				Abducted.entity:VisibleCheckpoint()
			else
				terminal:PopupUI()
				terminal:CheckSignalDowngrade()
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
	
	self.bugStun = false
	PlayerPawn.GodMode = self.oldGodMode
end

function PlayerPawn.ShowShield(self, show)
	self.shield.dm:SetVisible(show)
	self.shieldSprite.dm:SetVisible(show)
	self.shieldSpriteSpark.dm:SetVisible(show)
	self.shieldSpriteDamaged.dm:SetVisible(show)
	self:PauseShieldSound(not show)
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
	self:ShieldSpark()
	
	if (num > 1) then
		local spark = function()
			if (self.shieldActive) then
				self.shieldSounds.Spark:Play(kSoundChannel_FX, 0)
				self:ShieldSpark()
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
		self:PlayAnimSounds(anim)
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

function PlayerPawn.SignalArm(self, signal)

	if (signal) then
		self.model.dm:ReplaceMaterial(self.arm_M, self.armSignaled_M)
	else
		self.model.dm:ReplaceMaterial(self.armSignaled_M, self.arm_M)
	end

end

function PlayerPawn.SaveState(self)
	local fp = self:FloorPosition()
	if (fp.floor == -1) then
		error("PlayerPawn.SaveState: checkpoints not allowed when player is not on a floor!")
	end
--	if (self.customMove) then
--		error("PlayerPawn.SaveState: executing custom move, cannot save!")
--	end
	assert(not self.dead)
	
	local vertex = self:Angles()
	
	local state = {
		visible = tostring(self.visible),
		shieldActive = tostring(self.shieldActive),
		animState = self.animState,
		facing = tostring(vertex.pos[3]),
		sheidlAutoActiveTime = tostring(self.shieldAutoActivateTime),
		charDBStatus = tostring(self.charDBStatus)
	}
	
	self:SaveFloorPos(state)
	
	return state
end

function PlayerPawn.LoadState(self, state)
	
	self.dead = false
	self.disableAnimTick = false
	self.bugStun = false
	self.state = "idle"
	self.charDBStatus = state.charDBStatus
	self.mines = nil
		
	self.pulseSounds.Hum:Stop()
	self.shieldSpriteDamaged.dm:BlendTo({0,0,0,0}, 0)
	self.shieldSpriteSpark.dm:BlendTo({0,0,0,0}, 0)
	
	if (self.shieldSparkTimer) then
		self.shieldSparkTimer:Clean()
		self.shieldSparkTimer = nil
	end
	
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
	
	self.shieldAutoActivateTime = tonumber(state.shieldAutoActivateTime)
	self:Show(state.visible == "true")
--	self:SignalArm(HUD.armSignaled) -- done by HUD
	
	self:LoadFloorPos(state)
	
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
