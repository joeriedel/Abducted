-- Player.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

PlayerPawn = Class:New()

function PlayerPawn.Spawn(self)
	COutLine(kC_Debug, "PlayerPawn:Spawn")
	Entity.Spawn(self)
	
	
	local x = Tokenize("ab cd ef 1.2f4 \"testing with spaces\" bb yy")
	
	self.model = World.Load("Characters/HumanFemale")
	self.model:SetRootController("BlendToController")
	self.model:BlendToState("idle")
	self:AttachDrawModel(self.model)
	self:SetDrawModelScale(self.model, {0.4, 0.4, 0.4}) -- temp art
	self:SetMins({-24, -24, -48+64})
	self:SetMaxs({ 24,  24,  48+64})
	self:SetDrawModelBounds(self.model, self:Mins(), self:Maxs())
	self:SetCameraShift({0, 0, 90}) -- camera looks here
	
	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	self:SetAngles(angleVertex)
	
	self:SetClassBits(kEntityClass_Player)
	self:SetOccupantType(kOccupantType_BBox)
	
	if (self.keys.start_waypoint) then
		local waypoints = World.WaypointsForUserId(self.keys.start_waypoint)
		if (waypoints == nil) then
			error("PlayerPawn: unable to find starting waypoint named "..self.keys.start_waypoint)
		end
		
		self.floorPosition = World.WaypointFloorPosition(waypoints[1])
		self:SetOrigin(self.floorPosition.pos)
	end
	
	self:Link() -- kMoveType_None needs this
	
	World.playerPawn = self
	World.SetPlayerPawn(self)
end

info_player_start = PlayerPawn
