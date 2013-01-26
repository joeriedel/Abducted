-- PlayerPawn.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Entity:New()

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
	
	self:SetAccel({200, 0, 0}) -- <-- How fast the player accelerates (units per second).
	self:SetMaxGroundSpeed(100) -- <-- How fast the player moves (units per second).
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
	
	if (velocity > 0) then
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
	return true
end

function PlayerPawn.NotifyManipulate(self, enabled)
	
	
	if (enabled) then
		self.disableAnimTick = true
		self.state = nil
		self:PlayAnim("manipulate_idle", self.model)
		self:SetDesiredMove(nil)
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

info_player_start = PlayerPawn
