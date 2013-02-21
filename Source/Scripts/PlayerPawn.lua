-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Entity:New()
PlayerPawn.kWalkSpeed = 100
PlayerPawn.kAutoDecelDistance = 10
PlayerPawn.kFriction = 300
PlayerPawn.kRunSpeed = 200
PlayerPawn.kAccel = 200
PlayerPawn.HandBone = "right_hand"
PlayerPawn.PulseBeamScale = 1/120

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	MakeAnimatable(self)
	
	self.model = World.Load("Characters/HumanFemale")
	self.model:SetRootController("BlendToController")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:ScaleTo({0.4, 0.4, 0.4}, 0) -- temp art
	self.model.dm:SetMotionScale(2.5) -- temp art
	self.model.dm:SetPos({0, 0, -48}) -- on floor
	self.model.handBone = self.model.dm:FindBone(PlayerPawn.HandBone)
	
	if (self.model.handBone == -1) then
		error("PlayerPawn: can't find bone named %s", PlayerPawn.HandBone)
	end
	
	self:SetMins({-24, -24, -48})
	self:SetMaxs({ 24,  24,  48})
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
	self:SetMaxGroundSpeed(PlayerPawn.kWalkSpeed)
	self:SetAccel({PlayerPawn.kAccel, 0, 0}) -- <-- How fast the player accelerates (units per second).
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
	end
	
	World.playerPawn = self
	World.SetPlayerPawn(self)
	
	self:AddTickable(kTF_PostPhysics, function () PlayerPawn.TickPhysics(self) end)
end

function PlayerPawn.TickPhysics(self)
	if (self.disableAnimTick) then
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
		self:PlayAnim(reqState, self.model)
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
		self.disableAnimTick = true
		self.state = nil
		self:PlayAnim("manipulate_idle", self.model)
		self:Stop()
	else 
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
	
	self:PlayAnim("manipulate_"..dir, self.model).Seq(f)
	
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
	
end

function PlayerPawn.Stop(self)
	self:EnableFlags(kPhysicsFlag_Friction, true)
end

function PlayerPawn.BeginPulse(self)
	self.disableAnimTick = true
	self.state = nil
	self:PlayAnim("pulse_idle", self.model)
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
	
	self:PlayAnim("pulse_fire", self.model).Seq(f)
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
		self:FirePulse(trace.traceEnd)
		return
	end
	
	self:EndPulse()
end

function PlayerPawn.PulseExplode(self)
	self.dead = true
	self:PlayAnim("death", self.model)
	Game.entity:PlayerDied()
end

function PlayerPawn.SetFacing(self, zAngle)
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0,  zAngle}
	self:SetAngles(angleVertex)
end

function PlayerPawn.CheckTappedOn(self, e)

	local pos = self:WorldPos()
	pos = VecAdd(pos, {0, 0, 50})
	
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

info_player_start = PlayerPawn
