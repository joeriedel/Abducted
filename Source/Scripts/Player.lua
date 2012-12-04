-- Player.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Player = Class:New()

function Player.Spawn(self)
	World.playerPawn = self
	World.SetPlayerPawn(self)
end

info_player_start = Player
