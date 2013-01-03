-- Player.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Player = Class:New()

function Player.Spawn(self)
	COutLine(kC_Debug, "Player:Spawn")
	Entity.Spawn(self)
	
	self.model = World.Load("Characters/HumanFemale")
	self.model:SetRootController("BlendToController")
	self.model:BlendToState("manipulate_idle")
	self:AttachDrawModel(self.model)
	self:SetDrawModelScale(self.model, {0.4, 0.4, 0.4}) -- temp art
	self:SetMins({-24, -24, -48})
	self:SetMaxs({ 24,  24,  48})
	self:SetDrawModelBounds(self.model, self:Mins(), self:Maxs())
	self:SetCameraShift({0, 0, 70}) -- camera looks here
	
	local angle = NumberForString(self.keys.angle, 0)
	
	local angleVertex = self:Angles()
	angleVertex.pos = {0, 0, angle}
	self:SetAngles(angleVertex)
	
	self:SetOccupantType(kOccupantType_BBox)
	self:Link() -- kMoveType_None needs this
	
	World.playerPawn = self
	World.SetPlayerPawn(self)
end

info_player_start = Player
