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

function Bug.Spawn(self)

	COutLine(kC_Debug, "Bug:Spawn")
	Entity.Spawn(self)
	
	self.model = World.Load("Characters/Bugtestmesh1")
	self.model.dm = self:AttachDrawModel(self.model)
	self:SetMins({-24, -24, -24})
	self:SetMaxs({ 24,  24,  24})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	
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
	
	self:SetClassBits(kEntityClass_Monster)
	self:SetOccupantType(kOccupantType_BBox)
	
	self:Link()
end

info_bug = Bug
