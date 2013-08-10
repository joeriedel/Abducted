-- ReflexGame.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms


function ReflexGame.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.antivirusSpiderSpawnRate = {5, 10}
	level.antivirusSpiderSpeed = {70, 100}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 5 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1 -- delay before blocks start eating player line
    level.blockGrowTime = 1
    level.time = 40
	
	level.board = {
		-- row 0
		{ x=16, y=0, img="mark_end" }
		-- row 1
		, { x=0, y=1, img="mark_start" }	
		, { x=3, y=1, img="blocker_green" }
		, { x=9, y=1, img="blocker_green" }	
		, { x=11, y=1, img="blocker_green" }
		-- row 2
		, { x=3, y=2, img="blocker_green" }
		, { x=4, y=2, img="cell_02" }
		, { x=8, y=2, img="blocker_green" }	
		, { x=11, y=2, img="blocker_green" }		
		, { x=12, y=2, img="blocker_green" }	
		-- row 3
		, { x=5, y=3, img="blocker_green" }
		-- row 4
		, { x=12, y=4, img="blocker_green" }
		, { x=13, y=4, img="cell_04" }
		, { x=8, y=4, img="blackhole", heading={ 0,1 } }
		-- row 5
		, { x=6, y=5, img="blocker_green" }
		, { x=13, y=5, img="cell_03" }
		-- row 6
		, { x=2, y=6, img="blocker_green" }
		, { x=3, y=6, img="blocker_green" }
		, { x=8, y=6, img="blocker_green" }
		-- row 7
		, { x=3, y=7, img="cell_01" }
		, { x=9, y=7, img="blocker_green" }
		-- row 8
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