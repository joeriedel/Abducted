-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Entity:New()
PlayerPawn.kWalkSpeed = 100
PlayerPawn.kAutoDecelDistance = 15
PlayerPawn.kFriction = 300
PlayerPawn.kRunSpeed = 200
PlayerPawn.kAccel = 200

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	MakeAnimatable(self)
	
	self.model = World.Load("Characters/HumanFemale")
	self.model:SetRootController("BlendToController")
	self.model.dm = self:AttachDrawModel(self.model)
	self.model.dm:SetScale({0.4, 0.4, 0.4}) -- temp art
	self.model.dm:SetMotionScale(2.5) -- temp art
	self.model.dm:SetAngles({0, -90, 180})
	self.model.dm:SetPos({0, 0, 60})
	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self.model.dm:SetBounds(self:Mins(), self:Maxs())
	self:SetCameraShift({0, 0, 50}) -- camera looks here
	
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
	
	if (self.keys.start_waypoint) then
		local waypoints = World.WaypointsForUserId(self.keys.start_waypoint)
		if (waypoints == nil) then
			error("PlayerPawn: unable to find starting waypoint named "..self.keys.start_waypoint)
		end
		
		local floorPosition = World.WaypointFloorPosition(waypoints[1])
		self.floorPositionInit = true
		self:SetFloorPosition(floorPosition) -- for moves
		self:SetOrigin(floorPosition.pos)
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

function PlayerPawn.MoveToWaypoint(self, waypointNum)
	if (not self.floorPositionInit) then
		return false
	end
	
	local targetFloorPos = World.WaypointFloorPosition(waypointNum)
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

function PlayerPawn.Stop(self)
	self:EnableFlags(kPhysicsFlag_Friction, true)
end

function PlayerPawn.CheckTappedOn(self, e)

	local pos = self:WorldPos()
	pos = VecAdd(pos, {0, 0, 60})
	
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
