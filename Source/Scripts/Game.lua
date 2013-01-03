-- Game.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Game = Class:New()

function Game.Spawn(self)
	Game.entity = self
end

function Game.PostSpawn(self)

end

game_code = Game

function World.BuiltIns()
	return {
		"game_code",
		"ui_code"
--		"terminal_puzzles"
	}
end