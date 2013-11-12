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
	self:SetLightingFlags(kObjectLightingFlag_CastShadows)
	
	self.mode = StringForString(self.keys.initial_state, "idle")
	self.model = LoadModel("Characters/Tormentor1")
	self:SetMotionSka(self.model)
	
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	self:SetClassBits(kEntityClass_Monster)
	self:SetOccupantType(kOccupantType_BBox)
	self:SetMoveType(kMoveType_Floor)
	self:Move(false)
	self:SetMins({-64, -128, 0})
	self:SetMaxs({64, 64, 220})
	self:SetShadowMins(self:Mins())
	self:SetShadowMaxs(self:Maxs())
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
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
	self.visible = BoolForString(self.keys.visible, true)
	
	if (self.visible) then
		self:SwitchModes()
	else
		self.model.dm:SetVisible(false)
	end
	
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
		if (self.visible) then
			self:SwitchModes()
		end
		return true
	elseif (cmd == "attack") then
		self.mode = "aggressive"
		if (self.visible) then
			self:SwitchModes()
		end
		return true
	elseif (cmd == "intro") then
		if (self.visible) then
			self:PlayCinematicAnim("cinematicintro1").Seq(function() self.didIntro = true end)
		end
		return true
	elseif (cmd == "teleport") then
		args = Tokenize(args)
		self:Teleport(args[1], tonumber(args[2]))
		return true
	elseif (cmd == "show") then
		if (not self.visible) then
			self.visible = true
			self:SwitchModes()
			self.model.dm:SetVisible(true)
		end
		return true
	elseif (cmd == "hide") then
		if (self.visible) then
			self.visible = false
			self.think = nil
			self.model.dm:SetVisible(false)
			if (self.attackDamageTimer) then
				self.attackDamageTimer:Clean()
				self.attackDamageTimer = nil
			end
		end
		return true
	elseif (cmd == "play") then
		if (self.visible) then
			self:PlayCinematicAnim(args)
		end
	end
	
	return false
end

function Tormentor.PlayCinematicAnim(self, args)
	args = Tokenize(args)
	
	local f = function()
		self:SetMoveType(kMoveType_Floor)
		
		if (args[2]) then
			self:Teleport(args[2], tonumber(args[3]))
		else
			local fp = self:FindFloor()
			if (fp) then
				self:SetFloorPosition(fp)
			else
				error("Tormentor.PlayCinematicAnim: walked off the floor.")
			end
		end
		
		self:SwitchModes()
	end
	
	self:SetMoveType(kMoveType_Ska)
	self.think = nil
	self:Move(false)
	self:SetDesiredMove(nil)
	
	local blend = self:PlayAnim(args[1], self.model)
	if (blend) then
		blend.Seq(f)
	else
		f()
	end
	
	return blend
end

function Tormentor.Teleport(self, userId, facing)
	local waypoints = World.WaypointsForUserId(userId)
	if (waypoints) then
		local fp = World.WaypointFloorPosition(waypoints[1])
		if (fp) then
			self:SetFloorPosition(fp)
			self:SetDesiredMove(nil)
			self.floor = fp
			
			if (facing) then
				self:SetFacing(facing)
			end
			
			if (self.attackDamageTimer) then
				self.attackDamageTimer:Clean()
				self.attackDamageTimer = nil
			end
			
			self:Move(false)
			self.think = nil
			
			if (self.visbile) then
				self:SwitchModes()
			end
			
		else
			COutLine(kC_Debug, "ERROR: Tormentor, waypoint '%s' doesn't have a valid floor position.", userId)
		end
	else
		COutLine(kC_Debug, "ERROR: Tormentor(teleport) there is no waypoint with a userid of '%s'.", userId)
	end
end

function Tormentor.SwitchModes(self)
	
	self.think = nil
	
	if (((self.didIntro == false) and BoolForString(self.keys.wait_for_intro, false)) or self.mode == "idle") then
		self:Move(false)
		self:PlayAnim("idle", self.model, false)
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
		self:PlayAnim("idle", self.model, false)
		self.think = Tormentor.SwitchModes
		self:SetNextThink(FloatRand(2, 4))
	end
	
	self:PlayAnim("pain", self.model, false).Seq(f)
end

function Tormentor.SeekPlayer(self)
	local fp = World.playerPawn:FloorPosition()
	if (fp and (fp.floor == self.floor.floor)) then
		self.think = Tormentor.SeekPlayerThink
		self.thinkTime = Game.time or 0
		self:SetNextThink(0)
		if (not self:CheckAttack()) then
			self:PlayAnim("run", self.model, false)
			self:SeekPlayerThink(true)
		end
	else
		self.think = Tormentor.SeekPlayer
		self:SetNextThink(1)
		self:PlayAnim("idle", self.model, false)
	end
end

function Tormentor.SeekPlayerThink(self, force)

	-- check player distance
	if (self:CheckAttack()) then
		return
	end
	
	if (force or (Game.time-self.thinkTime) > 1.5) then
		local fp = World.playerPawn:FloorPosition()
		local moved = false
			
		if (self.floor.floor > -1) then
			if (fp and (fp.floor == self.floor.floor)) then
				local moveCommand = World.CreateFloorMove(self:FloorPosition(), fp)
	
				if (moveCommand) then
					self:SetDesiredMove(moveCommand)
					moved = true
				end
			end
		end
		
		self.thinkTime = Game.time
		self:Move(moved)
		
		if (not moved) then
		-- go back to idle
			self.think = nil
			self:SwitchModes()
		end
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
	self:PlayAnim("swing", self.model).Seq("idle")
	
	local f = function()
		if (self:PlayerInAttackRange()) then
			World.playerPawn:Damage(PlayerPawn.kMaxShieldDamage*1.5, self, nil, self.keys.killed_player_command)
			if (World.playerPawn.dead) then
				local f = function()
					World.viewController:AddLookTarget(self, {0,0,160})
				end
				World.globalTimers:Add(f, 0.5)
			end
		end
	end
	
	self.attackDamageTimer = World.gameTimers:Add(f, 0.5)
	self.think = Tormentor.SwitchModes
	self:SetNextThink(3.5)
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
		
	if (self.attackDamageTimer) then
		self.attackDamageTimer:Clean()
		self.attackDamageTimer = nil
	end
	
	local vertex = self:Angles()
		
	local state = {
		mode = tostring(self.mode),
		didIntro = tostring(self.didIntro),
		facing = tostring(vertex.pos[3]),
		visible = tostring(self.visible)
	}
	
	self:SaveFloorPos(state)
	
	return state
end

function Tormentor.LoadState(self, state)
	
	self:LoadFloorPos(state)
	self:SetDesiredMove(nil)
	self:SetFacing(tonumber(state.facing))
	self:Link()
	
	self.mode = state.mode
	self.didIntro = state.didIntro == "true"
	self:SwitchModes()
	
	self.visible = state.visible == "true"
	self.model.dm:SetVisible(self.visible)
	
end

info_tormentor = Tormentor