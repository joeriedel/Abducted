-- Bugs.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

--[[---------------------------------------------------------------------------
	Bug Spawner
-----------------------------------------------------------------------------]]

BugSpawner = Entity:New()

function BugSpawner.Spawn(self)

	COutLine(kC_Debug, "BugSpawner:Spawn")
	Entity.Spawn(self)

	self.floorPosition = self:SpawnFloorPosition()
	
	self.enabled = BoolForString(self.keys.enabled, false)
	self.count = NumberForString(self.keys.count, 1)
	self.maxCount = NumberForString(self.keys.max_count, 0)
	self.radius = NumberForString(self.keys.radius, 0)
	self.spawn_radius = NumberForString(self.keys.spawn_radius, 25)
	self.delay = NumberForString(self.keys.delay, 0)
	self.probability = NumberForString(self.keys.probability, 0)
	self.enabled = self.enabled
	self.continuous = StringForString(self.keys.mode, "singleshot") == "continuous"
	self.radiusTarget = self
	self.aggression = NumberForString(self.keys.aggression, 1)
	if (self.aggression <= 0) then
		self.aggression = 1
	end
	self.active = 0
	self.total = 0
	self.bugs = LL_New()
	
	self:Precache("Characters/Buggroup10")
	self:Precache("FX/bug_guts_group01")
	self:Precache("Characters/Bug1")
	self:Precache("FX/bug_guts_lone01")
	
	self.sounds = {}
	
	if (self.keys.spawn_sound) then
		self.sounds.Spawn = World.LoadSound(self.keys.spawn_sound)
		self:AttachSound(self.sounds.Spawn)
	end
	
	if (self.keys.respawn_sound) then
		self.sounds.Respawn = World.LoadSound(self.keys.respawn_sound)
		self:AttachSound(self.sounds.Respawn)
	end
	
	if (self.enabled) then
		if (self.sounds.Spawn) then
			self.sounds.Spawn:Play(kSoundChannel_FX, 0)
		end
		self:Go()
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

function BugSpawner.PostSpawn(self)

	if (self.keys.radius_targetname) then
		local target = World.FindEntityTargets(self.keys.radius_targetname)
		if (target) then
			self.radiusTarget = target[1]
		end
	end
end

function BugSpawner.Go(self)
	if (not self.validFloorPosition) then
		return
	end
	
	if (self.active < self.count) then
		self.think = BugSpawner.Think
		self:SetNextThink(self.delay)
	end
end

function BugSpawner.Think(self)
	
	local keys = {}
	keys.classname = "info_bug"
	
	local fp = self:FindSpawnPosition()
	
	keys.floorNum = tostring(fp.floor)
	keys.floorTri = tostring(fp.tri)
	keys.origin = Vec3ToString(fp.pos)
	keys.bug_waypoint = self.keys.bug_waypoint
	keys.group = tostring(math.random() < self.probability)
	keys.aggression = tostring(self.aggression)
	
	if (self.keys.intro_anim) then
		keys.intro_anim = self.keys.intro_anim
	end
	
	if (self.keys.intro_angle) then
		keys.intro_angle = self.keys.intro_angle
	end
	
	local bug = World.AsyncTempSpawn(keys)
	bug.listItem = LL_Append(self.bugs, {bug=bug})
	bug.spawner = self
	
	self.active = self.active + 1
	self.total = self.total + 1
	
	if (((self.maxCount > 0) and (self.total >= self.maxCount)) or (self.active >= self.count)) then
		self.think = nil
	end
	
	COutLine(kC_Debug, "Spawned Bug")
end

function BugSpawner.FindSpawnPosition(self)

	if (self.floorPosition.floor == -1) then
		return self.floorPosition
	end
	
	if (self.keys.intro_anim or (self.spawn_radius < 1)) then
		return self.floorPosition
	end
	
	local fp = nil
	local x = self.floorPosition.pos[1]
	local y = self.floorPosition.pos[2]
	local z = self.floorPosition.pos[3]
	
	local cos = math.cos
	local sin = math.sin
	local random = math.random
	local pi = math.pi
	local ClipToFloor = World.ClipToFloor
	
	while (fp == nil) do
	
		local distance = random() * self.spawn_radius
		local angle = random() * pi * 2
		local c = x + (cos(angle) * distance)
		local s = y + (sin(angle) * distance)
		
		fp = ClipToFloor(
			self.floorPosition.floor,
			{c, s, z+32},
			{c, s, z-512}
		)
	
	end

	return fp
	
end

function BugSpawner.NotifyDead(self, bug)

	self:NotifyRemove(bug)

	self.active = self.active - 1
	if ((self.maxCount < 1) or (self.total < self.maxCount)) then
		if (self.active < self.count) then
			if (self.continuous and self.enabled) then
				if (self.sounds.Respawn) then
					self.sounds.Respawn:Play(kSoundChannel_FX, 0)
				end
				self:Go()
			end
		end
	end
	
end

function BugSpawner.NotifyRemove(self, bug)
	if (bug.listItem) then
		LL_Remove(self.bugs, bug.listItem)
		bug.listItem = nil
	end
end

function BugSpawner.OnEvent(self, cmd, args)
	COutLineEvent("BugSpawner", self.keys.targetname, cmd, args)
	
	if (cmd == "enable") then
		self.enabled = true
		if (self.sounds.Spawn) then
			self.sounds.Spawn:Play(kSoundChannel_FX, 0)
		end
		self:Go()
	elseif (cmd == "disable") then
		self.enabled = false
		self.think = nil
	end
end

function BugSpawner.SaveState(self)
	local state = {
		enabled = tostring(self.enabled)
	}
	return state
end

function BugSpawner.LoadState(self, state)
	self.enabled = state.enabled == "true"
	self.active = 0
	self.total = 0
	
	if (self.enabled) then
		self:Go()
	end
end

info_bug_spawner = BugSpawner

--[[---------------------------------------------------------------------------
	Bug
-----------------------------------------------------------------------------]]

Bug = Entity:New()
Bug.KillMessages = { "BUG_KILLED_MESSAGE1" }
Bug.DebugMessages = false
Bug.Health = 25

function Bug.Spawn(self)

	COutLine(kC_Debug, "Bug:Spawn")
	Entity.Spawn(self)
	
	self:SetLightInteractionFlags(kLightInteractionFlag_Objects)
		
	self.aggression = NumberForString(self.keys.aggression, 1)
	
	if (BoolForString(self.keys.group, false)) then
		self.model = LoadModel("Characters/Buggroup10")
		self.guts = LoadModel("FX/bug_guts_group01")
		self.group = true
		self.moveSpeed = 90
		self.accel = 1000
		self.friction = 1000
		self.traceMoveStep = 10
		self.moveRange = {600, 1000}
		self.turnRange = {-15, 15}
		self.wanderHangOutTime = {0.2, 0.8}
		self.playerDistance = {150, 250, 600}
		self.playerSeekAttackDistance = 210 * self.aggression
		self.playerAttackDistance = 75
		self.zigZagSize = {10, 15}
		self.zigZagDist = {100, 150}
	else
		self.model = LoadModel("Characters/Bug1")
		self.guts = LoadModel("FX/bug_guts_lone01")
		self.moveSpeed = 90
		self.accel = 1000
		self.friction = 1000
		self.traceMoveStep = 10
		self.moveRange = {200, 400}
		self.turnRange = {-30, 30}
		self.wanderHangOutTime = {0.1, 0.2}
		self.playerDistance = {180, 350, 600}
		self.playerSeekAttackDistance = 210
		self.playerAttackDistance = 30 * self.aggression
		self.zigZagSize = {15, 25}
		self.zigZagDist = {40, 80}
	end
	
	self.health = Bug.Health
	self.stompDistance = 35
	
	self.sounds = {}
	self.sounds.Squish = World.LoadSound("Audio/EFX_SingleBugSquish_rev1")
	self:AttachSound(self.sounds.Squish)
	
	self.model.dm = self:AttachDrawModel(self.model)
	self:SetMins({-24, -24, 0})
	self:SetMaxs({ 24,  24, 32})
	self:SetShadowMins(self:Mins())
	self:SetShadowMaxs(self:Maxs())
	self.model.dm:SetBounds(self:Mins(), self:Maxs())

	self.guts.dm = self:AttachDrawModel(self.guts)
	self.guts.dm:SetBounds(VecAdd(self:Mins(), {0,0,2}), self:Maxs()) -- avoid clipping problems
	self.guts.dm:SetVisible(false)
	
	self.model.dm:SetAngles({0,0,180})
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
	
	self:SetClassBits(kEntityClass_Monster)
	self:SetOccupantType(kOccupantType_BBox)
	self:EnableFlags(kPhysicsFlag_Friction, true)
	self:SetMoveType(kMoveType_Floor)
	
	self:SetMaxGroundSpeed(self.moveSpeed)
	self:SetAccel({self.accel, 0, 0}) -- <-- How fast the player accelerates (units per second).
	self:SetGroundFriction(self.friction)
	
	local fp = self:SpawnFloorPosition()
	self.floor = fp.floor
	
	if (self.keys.bug_waypoint) then
		self.curWaypoint = 0
		self.waypoints = World.FindEntityTargets(self.keys.bug_waypoint)
	end
	
	local doBugBrain = function()
		self.think = Bug.BugBrain
		self:SetNextThink(0.1)
		if (self.model.BlendImmediate and self.group) then
			self.model:BlendImmediate("crawling")
		end
	end
	
	if (self.keys.intro_anim) then
		self:SetFacing(NumberForString(self.keys.intro_angle, 0))
		local callbacks = {
			OnEndFrame = function()
				doBugBrain()
			end
		}
		self.model:BlendImmediate(self.keys.intro_anim, nil, false, self, callbacks)
	else
		doBugBrain()
	end
end

function Bug.BugBrain(self)

	if (not self.spawner) then
		return -- spawner not set yet
	end

	if (self:CheckStomp()) then
		return
	end
	
	if (World.playerPawn:CheckPowerBubbleKill(self, self:WorldPos())) then
		return
	end
	
	if (not self:SeekSpawner()) then
		self:SeekPlayer() -- keep player within range OR attack
	end
	
	self:AvoidOtherBugs()
	
	if (self.action == nil) then
		self.busy = false
		self.action = Bug.Wander
	end
	
	self:action()
end

function Bug.Wander(self)

	if (self.nextWanderTime and (self.nextWanderTime > Game.time)) then
		return
	end
	
	self.nextWanderTime = nil
	self:UpdateRandomMove(Bug.WanderMoveFinished)

end

function Bug.WanderMoveFinished(self)
	self.busy = false
	self.action = Bug.Wander
	if (not self.group) then
		self.nextWanderTime = Game.time + FloatRand(self.wanderHangOutTime[1], self.wanderHangOutTime[2])
	end
end

function Bug.UpdateRandomMove(self, moveCompleteCallback)
	if (self.busy) then
		return
	end
	
	for i=0,3 do
		for k=1,2 do
			local d = FloatRand(self.moveRange[1], self.moveRange[2])
			local angle = FloatRand(self.angle+self.turnRange[1], self.angle+self.turnRange[2])
			angle = angle + (i*90)
				
			angle = WrapAngle(angle)
					
			local fp, d = self:TraceMove(d, angle, true, self.traceMoveStep)
			
			if (fp) then
				if (self:ZigZagToTarget(fp)) then
					self.angle = angle
					self.busy = true
					self.floorMoveCallback = moveCompleteCallback
					if (Bug.DebugMessages) then
						COutLine(kC_Debug, "Bug.RandomMove")
					end
					return -- success
				elseif (self:ExecuteMove(fp)) then
					self.angle = angle
					self.busy = true
					self.floorMoveCallback = moveCompleteCallback
					if (Bug.DebugMessages) then
						COutLine(kC_Debug, "Bug.RandomMove")
					end
					return -- success
				else
					COutLine(kC_Error, "ERROR: Bug.UpdateRandomMove - CreateFloorMove() failed.")
					self:Stop()
					break
				end
			end
		end
	end
	
	COutLine(kC_Error, "ERROR: Bug.UpdateRandomMove - can't find anywhere to go.")
	self:Stop()
	
end

function Bug.AvoidOtherBugs(self)

end

function Bug.PlayerInSpawnerRadius(self)

	local playerFP = World.playerPawn:FloorPosition()
	
	if ((playerFP == nil) or (self.floor ~= playerFP.floor)) then
		return false -- don't seek player when on a different floor
	end
	
	if (self.spawner.radius < 1) then
		return true
	end
	
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, self.spawner.radiusTarget:WorldPos())
	local v, d = VecNorm(v)

	return d <= self.spawner.radius
end

function Bug.SeekPlayer(self)

	if (not self:PlayerInSpawnerRadius()) then
		self.action = nil
		return
	end
	
	if (((not World.playerPawn.stomping) and (self.group or (not World.playerPawn.bugStun))) and self:SeekPlayerAttack()) then
		return
	end
	
	if (World.playerPawn.dead or (not self.group)) then	
		if (self:RunAwayFromPlayer()) then
			return
		end
	end
	
	self:SeekPlayerDistance()
end

function Bug.SeekSpawner(self)

	if (self.spawner.radius < 1) then
		return false
	end
	
	if (self.action == Bug.SeekSpawnerAction) then
		return true
	end
	
	-- too far?
	local selfPos = self:WorldPos()
	
	local v = VecSub(self.spawner.radiusTarget:WorldPos(), selfPos)
	local v, d = VecNorm(v)
	
	if (d <= self.spawner.radius) then
		return false
	end
	
	-- we need to get closer, run at the spawner
	
	self.busy = false
	
	local runDistance = d - (self.spawner.radius * math.random(0.5, 0.7))
		
	if (self:RunToDistance(runDistance, v)) then
		self.angle = LookAngles(v)[3]
		self.action = Bug.SeekSpawnerAction
		if (Bug.DebugMessages) then
			COutLine(kC_Debug, "Bug.SeekSpawner")
		end
		return true
	end
	
	return false

end

function Bug.SeekSpawnerAction(self)
	if (not self.busy) then
		self.action = nil
	end
end

function Bug.SeekPlayerMoveFinished(self)
	self.busy = false
end

function Bug.SeekPlayerDistance(self)

	if (self.action == Bug.SeekPlayerDistanceAction) then
		return true
	end
	
	-- too far?
	local selfPos = self:WorldPos()
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, self.spawner:WorldPos())
	local v, d = VecNorm(v)
	
	if (d < self.playerDistance[3]) then
		return false
	end
	
	-- we need to get closer, run at the player
	
	self.busy = false
	
	local runDistance = d - self.playerDistance[2]
		
	if (self:RunToDistance(runDistance, v)) then
		self.angle = LookAngles(v)[3]
		self.action = Bug.SeekPlayerDistanceAction
		if (Bug.DebugMessages) then
			COutLine(kC_Debug, "Bug.SeekPlayerDistance")
		end
		return true
	end
	
	return false
end

function Bug.SeekPlayerDistanceAction(self)
	if (not self.busy) then
		self.action = nil
	end
end

function Bug.RunAwayFromPlayer(self)
	if (self.action == Bug.RunAwayFromPlayerAction) then
		return true
	end
	
	-- too close?
	local selfPos = self:WorldPos()
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, selfPos)
	local v, d = VecNorm(v)
	
	if (d > self.playerDistance[1]) then
		return false
	end
	
	-- are we already heading away?
	local myForward = RotateVecZ({1,0,0}, self.angle)
	if (VecDot(myForward, v) < 0.3) then
		return false -- already moving away
	end
	
	local runAngle = LookAngles(v)[3]
	
	if (World.playerPawn.dead) then
		if (math.random() < 0.5) then
			runAngle = runAngle - 90
		else
			runAngle = runAngle + 90
		end
	else
		if (math.random() < 0.5) then
			runAngle = runAngle - FloatRand(110, 140)
		else
			runAngle = runAngle + FloatRand(110, 140)
		end
	end
	
	runAngle = WrapAngle(runAngle)
	
	local runDistance = self.playerDistance[2] - d
	if (self:RunToDistance(runDistance, runAngle)) then
		self.angle = runAngle
		self.action = Bug.RunAwayFromPlayerAction
		if (Bug.DebugMessages) then
			COutLine(kC_Debug, "Bug.RunAwayFromPlayer")
		end
		return true
	end
	
	return false
end

function Bug.RunAwayFromPlayerAction(self)
	if (not self.busy) then
		self.action = nil
	end
end

function Bug.SeekPlayerAttack(self)
	if (World.playerPawn.dead) then
		if (self.action == nil) then	
			self.eating = false
		end
		if (self.eating) then
			return true
		end
		return false
	end
	
	if (self.attackCooldown and (self.attackCooldown > Game.time)) then
		return false
	end
	
	if (World.playerPawn.shieldActive) then
		if (not self.group) then
			-- single bugs are too stupid they are gonna get zapped
			if (not World.playerPawn.powerBubble) then
				return false -- can't pass shield
			end
		else
			return false
		end
	end
		
	-- close enough to seek attack?
	local selfPos = self:WorldPos()
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, selfPos)
	local v, d = VecNorm(v)
	
	local playerAngle = World.playerPawn:TargetAngles()[3]
	local playerFwd = RotateVecZ({1,0,0}, playerAngle)
	
	if (self.action ~= Bug.SeekPlayerAttackAction) then
		if (d > self.playerSeekAttackDistance) then
			return false
		end
		
		if (not self.group) then
			if (VecDot(v, playerFwd) < 0.6) then -- not behind enough
				return false
			end
		end
	else
		if (self.group) then
			self:CheckGroupAttack()
		else
			local dd = VecDot(v, playerFwd)
			
			if (self.nextAttackCheck and (self.nextAttackCheck <= Game.time)) then
				if (dd < 0) then -- they turned to face us run away!
					return false
				end
			end
			
			self.nextAttackCheck = Game.time + 1
			self:CheckAttack(d, dd, playerPos, playerAngle)
		end
		return true -- keep attacking
	end
	
	-- attack!
	
	self.busy = false
	self.nextAttackCheck = Game.time + 1
		
	if (self:RunToDistance(d, v)) then
		self.angle = LookAngles(v)[3]
		self.action = Bug.SeekPlayerAttackAction
		if (Bug.DebugMessages) then
			COutLine(kC_Debug, "Bug.SeekPlayerAttack")
		end
		return true
	end
		
	return false
end

function Bug.SeekPlayerAttackAction(self)
	if (not self.busy) then
		self.action = nil
		self.eating = false
	end
end


function Bug.RunToDistance(self, distance, zangle)
	local fp = self:TraceMove(distance, zangle, true, self.traceMoveStep)
	if (fp) then
--		local moveCommand = World.CreateFloorMove(self:FloorPosition(), fp)
		if (self:ZigZagToTarget(fp)) then
			self.busy = true
			self.floorMoveCallback = Bug.SeekPlayerMoveFinished
--			self:SetDesiredMove(moveCommand)
--			self:EnableFlags(kPhysicsFlag_Friction, false)
			return true
		end
	end
	
	return false
end

function Bug.CheckAttack(self, d, dd, playerPos, playerAngle)
	
	if (World.playerPawn.dead) then
		return false
	end
	
	if (World.playerPawn.stomping or World.playerPawn.bugStun or (not World.playerPawn.visible)) then
		return false
	end
	
	if (self.attackCooldown and (self.attackCooldown > Game.time)) then
		return false
	end
	
	if (d > self.playerAttackDistance) then
		return false
	end
	
	if (dd < 0.707) then
		return false -- on front of player
	end
	
	local fp = self:FloorPosition()
	
	local callback = function()
		if (World.playerPawn.dead) then
			self:Remove()
		else
			self.think = Bug.BugBrain
			self.attacking = false
			self:SetNextThink(0.1)
			self.attackCooldown = Game.time + 5
			self:SetMoveType(kMoveType_Floor)
			self:SetFloorPosition(fp)
			self:EnableFlags(kPhysicsFlag_Friction, false)
			self.model:BlendImmediate("idle")
			self:SetFacing(180)
			self:Link()
		end
	end

	self.think = nil
	self.attacking = true
	self:Stop()
	self:SetDesiredMove(nil)
	self:SetMoveType(kMoveType_None)
	self:SetOrigin(playerPos)
	self:SetFacing(playerAngle)
	self:Link()
	
	self.model:BlendToState("bug_stun", nil, true)
	
	World.playerPawn:BugStun(callback)
	return true
	
end

function Bug.CheckGroupAttack(self)
	if (World.playerPawn.dead or World.playerPawn.shieldActive or (not World.playerPawn.visible) or PlayerPawn.GodMode) then
		return false
	end
	
	-- close enough to seek attack?
	local selfPos = self:WorldPos()
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, selfPos)
	local v, d = VecNorm(v)
	
	if (d > self.playerAttackDistance) then
		return false
	end
	
	local killMessage = Bug.KillMessages[IntRand(1, #Bug.KillMessages)]
	World.playerPawn:Damage(50, self, killMessage)
	World.playerPawn.bugSounds.Eaten:Play(kSoundChannel_FX, 0)
	
	-- crawl over her body
	local callbacks = {
		OnEndFrame = function()
			self.model:BlendToState("crawling")
		end
	}
	
	self.model:BlendToState("eat_eve", nil, true, self, callbacks)
	self.busy = true
	self.eating = true
	self.action = Bug.SeekPlayerAttackAction
	self:RunToDistance(self.moveSpeed * 4, self.angle) -- keep going well past player
		
	return true
end

function Bug.CheckStomp(self)

	if (World.playerPawn.dead) then
		return false
	end
	
	if (self.group) then
		return false
	end

	-- stomp, ouchie!
	local selfPos = self:WorldPos()
	local playerPos = World.playerPawn:WorldPos()
	
	local v = VecSub(playerPos, selfPos)
	local v, d = VecNorm(v)
	
	if (d > self.stompDistance) then
		return false
	end
	
	local playerAngle = World.playerPawn:TargetAngles()[3]
	local playerFwd = RotateVecZ({1,0,0}, playerAngle)
	
	if (VecDot(playerFwd, v) > 0) then
		return false -- on back of player
	end

	local f = function()
		self:Damage(Bug.Health, World.playerPawn)
	end

	self.think = nil
	self:Stop()
	World.playerPawn:BugStomp(f)
	return true
end

function Bug.SeekWaypoints(self)

	self:SelectNode()
	self.think = nil
	self.floorMoveCallback = Bug.SelectNode
	
end

function Bug.SelectNode(self)

	if (not self.validFloorPosition) then
		return
	end
	
	if (self.waypoints == nil) then
		return
	end
	
	local x = self.curWaypoint
	while (x == self.curWaypoint) do
	
		local size = #self.waypoints
		if (size < 2) then
			return -- not cool
		end
		
		x = IntRand(1, size)
	
	end
	
	self.curWaypoint = x
	
	self:ZigZagToTarget(self.waypoints[self.curWaypoint].floorPosition)
	
end

function Bug.ZigZagToTarget(self, targetPos)
	local pos = self:FloorPosition()
	local seekVec = VecSub(targetPos.pos, pos.pos)
	local seekDist = nil
	
	local vecScale = VecScale
	local vecAdd = VecAdd
	local rand = FloatRand
	local min = Min
	local zigZagDist = self.zigZagDist
	local zigZagSize = self.zigZagSize
	local clipToFloor = World.ClipToFloor
	
	seekVec, seekDist = VecNorm(seekVec)
	local crossVec = VecCross(seekVec, kZAxis)
	
	local zigZags = { pos }
	
	local minDist = zigZagDist[1] * 1.5
	local ofs = 0
	local flip = nil
	
	while (seekDist > minDist) do
		local maxDist = Min(zigZagDist[2], seekDist-zigZagDist[1])
		local d = rand(0, maxDist) + zigZagDist[1]
		local zag = rand(zigZagSize[1], zigZagSize[2])
		
		if (flip == nil) then
			flip = math.random() < 0.5
		end
		
		if (flip) then
			zag = -zag
			flip = false
		else
			flip = true
		end
		
		seekDist = seekDist - d
				
		local fwd = vecScale(seekVec, d+ofs)
		ofs = ofs + d
		
		fwd = vecAdd(pos.pos, fwd)
		zag = vecScale(crossVec, zag)
		fwd = vecAdd(fwd, zag)
		
		local target = clipToFloor(
			pos.floor,
			{fwd[1], fwd[2], fwd[3]+32},
			{fwd[1], fwd[2], fwd[3]-512}
		)
		
		if (target == nil) then
			-- don't zig zag anymore
			break
		end
		
		table.insert(zigZags, target)
	end
	
	table.insert(zigZags, targetPos)
	return self:ExecuteMove(zigZags)
end

function Bug.ExecuteMove(self, targetPos)
	local moveCommand
	
	if (type(targetPos) == "table") then
		moveCommand = World.CreateFloorMoveSeq(targetPos)
	else
		moveCommand = World.CreateFloorMove(self:FloorPosition(), targetPos)
	end
	
	if (moveCommand == nil) then
		return false
	end
	
	self:SetDesiredMove(moveCommand)
	self:EnableFlags(kPhysicsFlag_Friction, false)
	return true
end

function Bug.OnFloorMoveComplete(self)
	if (self.floorMoveCallback) then
		self:floorMoveCallback()
	end
end

function Bug.Start(self)
	self.busy = false
	self:EnableFlags(kPhysicsFlag_Friction, false)
end

function Bug.Stop(self)
	self.busy = false
	self:EnableFlags(kPhysicsFlag_Friction, true)
end

function Bug.Despawn(self)
	if (self.spawner) then
		self.spawner:NotifyDead(self)
	end	
	self:Remove()	
end

function Bug.Damage(self, damage, instigator)

	if (self.dead) then
		return
	end
	
	self.health = self.health - damage
	if (self.health <= 0) then
		self:SetMoveType(kMoveType_None)
		self.model.dm:SetVisible(false)
		self.guts.dm:SetVisible(true)
		self.think = nil
		self.dead = true
		self.sounds.Squish:Play(kSoundChannel_FX, 0)
		
		if (self.spawner) then
			self.spawner:NotifyDead(self)
		end
		
		local f = function()
			self:Remove()
		end
		
		World.gameTimers:Add(f, 20) -- remove us in 20 seconds
		
		if (self.group) then
			GameDB:SquishedBugs(IntRand(8, 12))
		else
			GameDB:SquishedBugs(1)
		end
	end
end

function Bug.PulseDamage(self, damage)
	self.health = self.health - damage
	
	if (self.health <= 0) then
		-- messy!
		self.dead = true
		self.think = nil
		self:Stop()
		self:SetMoveType(kMoveType_None)
		self.sounds.Squish:Play(kSoundChannel_FX, 0)
		
		if (self.group) then
			self.model:BlendToState("pulsedeath")
			GameDB:KilledBugs(IntRand(8, 12))
		else
			self.model.dm:SetVisible(false)
			GameDB:KilledBugs(1)
		end
		
		self.guts.dm:SetVisible(true)
		
		if (self.spawner) then
			self.spawner:NotifyDead(self)
		end
		
		local f = function()
			self:Remove()
		end
		
		World.gameTimers:Add(f, 20) -- remove us in 20 seconds
	end
end

function Bug.Remove(self)
	if (self.spawner) then
		self.spawner:NotifyRemove(self)
	end
	
	self:SetMoveType(kMoveType_None)
	self.model.dm:SetVisible(false)
	self.guts.dm:SetVisible(false)
	self.think = nil
	self.dead = true
	self:Delete() -- mark for gc
end

info_bug = Bug

--[[---------------------------------------------------------------------------
	Bug Waypoint
-----------------------------------------------------------------------------]]

BugWaypoint = Entity:New()

function BugWaypoint.Spawn(self)
	Entity.Spawn(self)
	self.floorPosition = self:SpawnFloorPosition()
end

info_bug_waypoint = BugWaypoint
