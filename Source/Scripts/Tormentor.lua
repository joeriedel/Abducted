-- Tormentor.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Tormentor = Entity:New()
Tormentor.List = LL_New()
Tormentor.PlayerAttackDistance = 140
Tormentor.Acceleration = 300
Tormentor.MoveSpeed = 300
Tormentor.Friction = 2000
-- Percent of total screen
Tormentor.TouchDistance = 0.1

function Tormentor.Spawn(self)

	COutLine(kC_Debug, "Tormentor:Spawn")
	Entity.Spawn(self)

	MakeAnimatable(self)
	
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
	
	self.mode = StringForString(self.keys.initial_state, "idle")
	self.model = LoadModel("Characters/Tormentor1")
	
	self.model.dm = self:AttachDrawModel(self.model)
	
	self:SetClassBits(kEntityClass_Monster)
	self:SetOccupantType(kOccupantType_BBox)
	self:SetMoveType(kMoveType_Floor)
	self:Move(false)
	self:SetMins({-64, -64, 0})
	self:SetMaxs({64, 64, 256})
	self:SetShadowMins(self:Mins())
	self:SetShadowMaxs(self:Maxs())
	self:SetMaxGroundSpeed(Tormentor.MoveSpeed)
	self:SetAccel({Tormentor.Acceleration, 0, 0})
	self:SetGroundFriction(Tormentor.Friction)
	
	self:SetSnapTurnAngles({360, 360, 180})
	
	local angle = NumberForString(self.keys.angle, 0)
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	angleVertex.mass = 5
	angleVertex.drag[1] = 7
	angleVertex.drag[2] = 7
	angleVertex.friction = 0.5
	self:SetAngles(angleVertex)
	self.angle = angle
	
	local spring = self:AngleSpring()
	spring.elasticity = 120 -- <-- larger numbers means she turns faster
	self:SetAngleSpring(spring)
	
	self.floor = self:SpawnFloorPosition()
	
	self.listItem = LL_Append(Tormentor.List, {x=self})
	
	self.didIntro = false
	
	self:SwitchModes()
	
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

function Tormentor.OnEvent(self, cmd, args)
	COutLineEvent(self.keys.targetname, "Tormentor", cmd, args)
	
	if (cmd == "idle") then
		self.mode = cmd
		self:SwitchModes()
		return true
	elseif (cmd == "attack") then
		self.mode = "aggressive"
		self:SwitchModes()
		return true
	elseif (cmd == "intro") then
		self:PlayAnim("cinematicintro1", self.model).Seq(function() self.didIntro = true end).And(Tormentor.SwitchModes)
		return true
	end
	
	return false
end

function Tormentor.SwitchModes(self)
	
	self.think = nil
	
	if (((self.didIntro == false) and BoolForString(self.keys.wait_for_intro, false)) or self.mode == "idle") then
		self:Move(false)
		self:PlayAnim("idle", self.model)
	else
		self:SeekPlayer()
	end
	
end

function Tormentor.Stun(self)

	if (self.attackDamageTimer) then
		self.attackDamageTimer:Clean()
		self.attackDamageTimer = nil
	end
	
	self:Move(false)
	
	self.think = nil
	
	local f = function()
		self:PlayAnim("idle", self.model)
		self.think = Tormentor.SwitchModes
		self:SetNextThink(FloatRand(2, 4))
	end
	
	self:PlayAnim("pain", self.model).Seq(f)
end

function Tormentor.SeekPlayer(self)
	self.think = Tormentor.SeekPlayerThink
	self.thinkTime = Game.time or 0
	self:SetNextThink(0)
	if (not self:CheckAttack()) then
		self:PlayAnim("run", self.model)
		self:SeekPlayerThink(true)
	end
end

function Tormentor.SeekPlayerThink(self, force)

	-- check player distance
	if (self:CheckAttack()) then
		return
	end
	
	if (force or (Game.time-self.thinkTime) > 1.5) then
		local fp = World.playerPawn:FloorPosition()
		local moveCommand = World.CreateFloorMove(self:FloorPosition(), fp)
	
		if (moveCommand) then
			self:SetDesiredMove(moveCommand)
			self:Move(true)
		end
		
		self.thinkTime = Game.time
	end
end

function Tormentor.PlayerInAttackRange(self)
	local dist = VecMag(VecSub(World.playerPawn:WorldPos(), self:WorldPos()))
	return dist < Tormentor.PlayerAttackDistance
end

function Tormentor.CheckAttack(self)
	if (self:PlayerInAttackRange()) then
		self:AttackPlayer()
		return true
	end
	
	return false
end

function Tormentor.AttackPlayer(self)
	if (World.playerPawn.dead) then
		self.mode = "idle"
		self:SwitchModes()
		return
	end
	
	self.think = nil
	self:Move(false)
	self:PlayAnim("swing", self.model).Seq(Tormentor.SwitchModes)
	
	local f = function()
		if (self:PlayerInAttackRange()) then
			World.playerPawn:Kill(self, nil, self.keys.killed_player_command)
		end
	end
	
	self.attackDamageTimer = World.gameTimers:Add(f, 0.5)
end

function Tormentor.Move(self, move)
	self:EnableFlags(kPhysicsFlag_Friction, not move)
end

function Tormentor.ShotWithPulse(self)
	self:Stun()
end

function Tormentor.CheckPulseTargets(mx, my)
	local x = LL_Head(Tormentor.List)
	while (x) do
		if(x.x:CheckPulseTarget(mx, my)) then
			return x.x
		end
		x = LL_Next(x)
	end
	
	return nil
end

function Tormentor.CheckPulseTarget(self, x, y)
	self.pos = VecAdd(self:WorldPos(), {0, 0, 128})
	local p, r = World.Project(self.pos)
	if (not r) then
		return false
	end
	
	p = UI:MapToUI(p)
	
	local dx = p[1]-x
	local dy = p[2]-y
	local dd = math.sqrt(dx*dx+dy*dy)
	if (dd <= UI.screenDiagonal*Discovery.TouchDistance) then
		return true
	end
	
	return false
end

function Tormentor.SaveState(self)
	local fp = self:FloorPosition()
	
	if (self.attackDamageTimer) then
		self.attackDamageTimer:Clean()
		self.attackDamageTimer = nil
	end
	
	local vertex = self:Angles()
		
	local state = {
		mode = tostring(self.mode),
		didIntro = tostring(self.didIntro),
		facing = tostring(vertex.pos[3]),
		pos = string.format("%d %d %d", fp.pos[1], fp.pos[2], fp.pos[3])
	}
	
	return state
end

function Tormentor.LoadState(self, state)
	
	local pos = Vec3ForString(state.pos)
	local fp  = World.ClipToFloor(
		{pos[1], pos[2], pos[3] + 8},
		{pos[1], pos[2], pos[3] - 8}
	)
	
	assert(fp)
	self:SetFloorPosition(fp)
	self:SetDesiredMove(nil)
	self:SetFacing(tonumber(state.facing))
	self:Link()
	
	self.mode = state.mode
	self.didIntro = state.didIntro == "true"
	self:SwitchModes()
	
end

info_tormentor = Tormentor