-- ReflexGame.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms


function ReflexGame.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.antivirusSpiderSpawnRate = {7, 13}
	level.antivirusSpiderSpeed = {50, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 5 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.1
    level.time = 120
	
	level.board = {
		-- row 0
		{ x=0, y=0, img="blocker_green" }
		, { x=1, y=0, img="blocker_green" }
		, { x=8, y=0, img="blackhole", heading={ 0,1 } }
		-- row 1
		-- row 2	
		-- row 3
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=16, y=4, img="mark_end" }
		, { x=8, y=4, img="cell_01" }
		-- row 5
		-- row 6
		, { x=14, y=6, img="blocker_green" }
		, { x=15, y=6, img="blocker_green" }
		, { x=16, y=6, img="blocker_green" }
		-- row 7
		, { x=14, y=7, img="blocker_green" }
		, { x=15, y=7, img="blocker_green" }
		, { x=16, y=7, img="blocker_green" }
		-- row 8
		, { x=0, y=8, img="blocker_green" }
		, { x=1, y=8, img="blocker_green" }
		, { x=2, y=8, img="blocker_green" }
		, { x=3, y=8, img="blocker_green" }
		, { x=4, y=8, img="blocker_green" }
		, { x=5, y=8, img="blocker_green" }
		, { x=6, y=8, img="blocker_green" }
		, { x=7, y=8, img="blocker_green" }
		, { x=8, y=8, img="blocker_green" }
		, { x=9, y=8, img="blocker_green" }
		, { x=10, y=8, img="blocker_green" }
		, { x=11, y=8, img="blocker_green" }
		, { x=12, y=8, img="blocker_green" }
		, { x=13, y=8, img="blocker_green" }
		, { x=14, y=8, img="blocker_green" }
		, { x=15, y=8, img="blocker_green" }
		, { x=16, y=8, img="blocker_green" }
		}
	
	return level
end

function ReflexGame.CreateLevel1x2(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel1x3(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel1x4(self)
	return self:CreateLevel1x1()
end

------- level 2's

function ReflexGame.CreateLevel2x1(self)
	local level = {}
	
	level.name = "2x1"
	
	level.antivirusSpiderSpawnRate = {6, 12}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.1
    level.time = 100
	
	level.board = {
		-- row 0
		{ x=0, y=0, img="blocker_green" }
		, { x=1, y=0, img="blocker_green" }
		-- row 1
		-- row 2
		, { x=7, y=2, img="blocker_green" }	
		, { x=16, y=2, img="mark_end" }	
		-- row 3
		, { x=6, y=3, img="blocker_green" }
		, { x=5, y=3, img="cell_02" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=5, y=4, img="blocker_green" }
		, { x=10, y=4, img="blocker_green" }
		, { x=16, y=4, img="blackhole", heading={ 1,0 } }
		-- row 5
		, { x=9, y=5, img="blocker_green" }
		-- row 6
		, { x=8, y=6, img="blocker_green" }
		, { x=9, y=6, img="cell_03" }
		, { x=14, y=6, img="blocker_green" }
		, { x=15, y=6, img="blocker_green" }
		, { x=16, y=6, img="blocker_green" }
		-- row 7
		, { x=14, y=7, img="blocker_green" }
		, { x=15, y=7, img="blocker_green" }
		, { x=16, y=7, img="blocker_green" }
		-- row 8
		, { x=14, y=8, img="blocker_green" }
		, { x=15, y=8, img="blocker_green" }
		, { x=16, y=8, img="blocker_green" }
		}
		
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel2x2(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel2x3(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel2x4(self)
	return self:CreateLevel1x1()
end

------- level 3's

function ReflexGame.CreateLevel3x1(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel3x2(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel3x3(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel3x4(self)
	return self:CreateLevel1x1()
end

------- level 4's

function ReflexGame.CreateLevel4x1(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel4x2(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel4x3(self)
	return self:CreateLevel1x1()
end

function ReflexGame.CreateLevel4x4(self)
	return self:CreateLevel1x1()
end