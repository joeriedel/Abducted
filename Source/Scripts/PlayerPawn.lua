-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Class:New()

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	self.model = World.Load("Characters/HumanFemale")
	self.model:SetRootController("BlendToController")
	self:AttachDrawModel(self.model)
	self:SetDrawModelScale(self.model, {0.4, 0.4, 0.4}) -- temp art
	self:SetDrawModelMotionScale(self.model, 2.5) -- temp art
	self:SetDrawModelAngles(self.model, {0, -90, 180})
	self:SetDrawModelPos(self.model, {0, 0, 64})
	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self:SetDrawModelBounds(self.model, self:Mins(), self:Maxs())
	self:SetCameraShift({0, 0, 90}) -- camera looks here
	
	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	angleVertex.mass = 5 -- how much mass the crow has (how much he resists changes in direction)
	angleVertex.drag[1] = 7 -- energy lost when crow is being pulled
	angleVertex.drag[2] = 7 -- energy lost per second when the crow is in position
	-- friction effects
	angleVertex.friction = 0.5
	self:SetAngles(angleVertex)
	
	local spring = self:AngleSpring()
	spring.elasticity = 40 -- larger numbers mean the spring is stiffer
	self:SetAngleSpring(spring)
	
	self:SetAccel({80, 0, 0})
	self:SetMaxGroundSpeed(80)
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
	local reqState
	local velocity = VecMag(self:Velocity())
--	COutLine(kC_Debug, "Velocity = %f", velocity)
--	COutLine(kC_Debug, "DistanceMoved = %f", self:DistanceMoved())
	
	if (velocity > 0) then
		reqState = "walk"
	else
		reqState = "idle"
	end
	
	if (self.state ~= reqState) then
		self.state = reqState
		self.model:BlendToState(reqState)
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
	return true
end

info_player_start = PlayerPawn
