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
	self.radius = NumberForString(self.keys.radius, 25)
	self.delay = NumberForString(self.keys.delay, 0)
	self.probability = NumberForString(self.keys.probability, 0)
	self.enabled = self.enabled
	self.active = 0

	self:Go()

end

function BugSpawner.Go(self)
	if (not self.enabled) then
		return
	end
	
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
	
	World.TempSpawn(keys)
	
	self.active = self.active + 1
	
	if (self.active >= self.count) then
		self.think = nil
	end
	
end

function BugSpawner.FindSpawnPosition(self)

	if (self.floorPosition.floor == -1) then
		return self.floorPosition
	end
	
	if (self.radius < 1) then
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
	
		local distance = random() * self.radius
		local angle = random() * pi * 2
		local c = x + (cos(angle) * distance)
		local s = y + (sin(angle) * distance)
		
		fp = ClipToFloor(
			self.floorPosition.floor,
			{c, s, z+512},
			{c, s, z-512}
		)
	
	end

	return fp
	
end

function BugSpawner.NotifyDead(self)

	self.active = self.active - 1
	if (self.active < self.count) then
		self:Go()
	end

end

function BugSpawner.OnEvent(cmd, args)
	if (cmd == "enable") then
		self.enabled = true
		self:Go()
	elseif (cmd == "disable") then
		self.enabled = false
		self.think = nil
	end
end

info_bug_spawner = BugSpawner

--[[---------------------------------------------------------------------------
	Bug
-----------------------------------------------------------------------------]]

Bug = Entity:New()
Bug.kMoveSpeed = 70
Bug.kAccel = 50
Bug.kFriction = 300
Bug.kZigZagSize = {0, 10}
Bug.kZigZagDist = {50, 100} -- (min, min+d)

function Bug.Spawn(self)

	COutLine(kC_Debug, "Bug:Spawn")
	Entity.Spawn(self)
	
	if (BoolForString(self.keys.group, false)) then
		self.model = LoadModel("Characters/Buggroup12")
	else
		self.model = LoadModel("Characters/Bugtestmesh1")
	end
	
	self.model.dm = self:AttachDrawModel(self.model)
	self:SetMins({-24, -24, -24})
	self:SetMaxs({ 24,  24,  24})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
		
	if (self.model.BlendToState) then
		self.model:BlendToState("crawling")
	else
		self.model.dm:SetAngles({0,0,180})
	end
	
	self:SetSnapTurnAngles({360, 360, 50})
	
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
	
	self:SetClassBits(kEntityClass_Monster)
	self:SetOccupantType(kOccupantType_BBox)
	self:EnableFlags(kPhysicsFlag_Friction, true)
	self:SetMoveType(kMoveType_Floor)
	
	self:SetMaxGroundSpeed(Bug.kMoveSpeed)
	self:SetAccel({Bug.kAccel, 0, 0}) -- <-- How fast the player accelerates (units per second).
	self:SetGroundFriction(Bug.kFriction)
	
	self:SpawnFloorPosition()
	
	if (self.keys.bug_waypoint) then
		self.curWaypoint = 0
		self.waypoints = World.FindEntityTargets(self.keys.bug_waypoint)
	end
	
	self.think = Bug.Wander
	self:SetNextThink(0.1)
	
end

function Bug.Wander(self)

	self:SelectNode()
	self.think = nil
	
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
	self:ZigZag()
	
end

function Bug.ZigZag(self)

	if (self.curZigZag >= #self.zigZags) then
		self:SelectNode()
		return
	end
	
	self.curZigZag = self.curZigZag + 1
	self:ExecuteMove(self.zigZags[self.curZigZag])
	
end

function Bug.ZigZagToTarget(self, targetPos)
	local pos = self:FloorPosition()
	local seekVec = VecSub(targetPos.pos, pos.pos)
	local seekDist = nil
	
	local vecScale = VecScale
	local vecAdd = VecAdd
	local rand = FloatRand
	local min = Min
	local zigZagDist = Bug.kZigZagDist
	local zigZagSize = Bug.kZigZagSize
	local clipToFloor = World.ClipToFloor
	
	seekVec, seekDist = VecNorm(seekVec)
	local crossVec = VecCross(seekVec, kZAxis)
	
	self.zigZags = {}
	
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
			{fwd[1], fwd[2], fwd[3]+512},
			{fwd[1], fwd[2], fwd[3]-512}
		)
		
		if (target == nil) then
			-- don't zig zag anymore
			break
		end
		
		table.insert(self.zigZags, target)
	end
	
	self.curZigZag = 0
	table.insert(self.zigZags, targetPos)
end

function Bug.ExecuteMove(self, targetPos)
	local moveCommand = World.CreateFloorMove(self:FloorPosition(), targetPos)
	if (moveCommand == nil) then
		return false
	end
	
	self:SetDesiredMove(moveCommand)
	self:EnableFlags(kPhysicsFlag_Friction, false)
	return true
end

function Bug.OnFloorMoveComplete(self)
	self:ZigZag()
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
