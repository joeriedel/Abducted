-- ReflexGame.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms


function ReflexGame.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.antivirusSpiderSpawnRate = {6, 12}
	level.antivirusSpiderSpeed = {50, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 5 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 100
	
	level.board = {
		-- row 0
		-- row 1
		-- row 2
		{ x=5, y=2, img="blocker_green" }
		, { x=6, y=2, img="blocker_green" }
		, { x=10, y=2, img="blocker_green" }
		, { x=11, y=2, img="blocker_green" }	
		-- row 3
		, { x=5, y=3, img="blocker_green" }
		, { x=11, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=5, y=4, img="blocker_green" }
		, { x=8, y=4, img="cell_01" }
		, { x=11, y=4, img="blocker_green" }
		, { x=16, y=4, img="mark_end" }
		-- row 5
		, { x=5, y=5, img="blocker_green" }
		, { x=11, y=5, img="blocker_green" }
		-- row 6
		, { x=5, y=6, img="blocker_green" }
		, { x=6, y=6, img="blocker_green" }
		, { x=10, y=6, img="blocker_green" }
		, { x=11, y=6, img="blocker_green" }
		-- row 7
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
	local level = {}
	
	level.name = "2x1"
	
	level.antivirusSpiderSpawnRate = {6, 12}
	level.antivirusSpiderSpeed = {50, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 80
	
	level.board = {
		-- row 0
		{ x=9, y=0, img="blocker_green" }
		, { x=12, y=0, img="blocker_green" }
		-- row 1
		, { x=6, y=1, img="blocker_green" }
		, { x=7, y=1, img="cell_02" }
		, { x=9, y=1, img="blocker_green" }
		, { x=12, y=1, img="cell_03" }
		-- row 2
		, { x=3, y=2, img="blocker_green" }
		, { x=6, y=2, img="blocker_green" }
		, { x=9, y=2, img="blocker_green" }
		-- row 3
		, { x=3, y=3, img="blocker_green" }
		, { x=6, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=12, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=3, y=4, img="blocker_green" }
		, { x=6, y=4, img="blocker_green" }
		, { x=9, y=4, img="blocker_green" }
		, { x=12, y=4, img="blocker_green" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=3, y=5, img="blocker_green" }
		, { x=6, y=5, img="blocker_green" }
		, { x=9, y=5, img="blocker_green" }
		-- row 6
		, { x=3, y=6, img="blocker_green" }
		, { x=6, y=6, img="blocker_green" }
		-- row 7
		, { x=3, y=7, img="blocker_green" }
		, { x=6, y=7, img="blocker_green" }
		, { x=12, y=7, img="blocker_green" }
		-- row 8
		, { x=3, y=8, img="blocker_green" }
		, { x=9, y=8, img="blocker_green" }
		, { x=12, y=8, img="blocker_green" }
		}
		
	return level
end

function ReflexGame.CreateLevel2x2(self)
	local level = {}
	
	level.name = "2x2"
	
	level.antivirusSpiderSpawnRate = {5, 10}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 80
	
	level.board = {
		-- row 0
		{ x=6, y=0, img="blocker_green" }
		, { x=12, y=0, img="blocker_green" }
		-- row 1
		, { x=6, y=1, img="cell_04" }
		, { x=9, y=1, img="cell_05" }
		, { x=12, y=1, img="blocker_green" }
		-- row 2
		, { x=0, y=2, img="blackhole", heading={ 1,0 } }
		, { x=5, y=2, img="blocker_green" }
		, { x=6, y=2, img="blocker_green" }
		, { x=7, y=2, img="blocker_green" }
		, { x=8, y=2, img="blocker_green" }
		, { x=9, y=2, img="blocker_green" }
		, { x=12, y=2, img="blocker_green" }
		-- row 3
		, { x=5, y=3, img="blocker_green" }
		, { x=6, y=3, img="blocker_green" }
		, { x=7, y=3, img="blocker_green" }
		, { x=8, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=12, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=2, y=4, img="blocker_green" }
		, { x=3, y=4, img="blocker_green" }
		, { x=5, y=4, img="blocker_green" }
		, { x=6, y=4, img="blocker_green" }
		, { x=7, y=4, img="blocker_green" }
		, { x=8, y=4, img="blocker_green" }
		, { x=9, y=4, img="blocker_green" }
		, { x=12, y=4, img="blocker_green" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=12, y=5, img="blocker_green" }
		-- row 6
		, { x=12, y=6, img="blocker_green" }
		-- row 7
		, { x=12, y=7, img="blocker_green" }
		-- row 8
		, { x=12, y=8, img="blocker_green" }
		}
	return level
end

function ReflexGame.CreateLevel2x3(self)
	local level = {}
	
	level.name = "2x3"
	
	level.antivirusSpiderSpawnRate = {5, 10}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 80
	
	level.board = {
		-- row 0
		{ x=13, y=0, img="blocker_green" }
		-- row 1
		, { x=11, y=1, img="blocker_green" }
		, { x=13, y=1, img="blocker_green" }
		-- row 2
		, { x=2, y=2, img="blocker_green" }
		, { x=3, y=2, img="blocker_green" }
		, { x=4, y=2, img="blocker_green" }
		, { x=13, y=2, img="blocker_green" }
		-- row 3
		, { x=2, y=3, img="blocker_green" }
		, { x=6, y=3, img="blocker_green" }
		, { x=7, y=3, img="blocker_green" }
		, { x=8, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=11, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=2, y=4, img="blocker_green" }
		, { x=3, y=4, img="cell_06" }
		, { x=6, y=4, img="blocker_green" }
		, { x=7, y=4, img="blocker_green" }
		, { x=8, y=4, img="blocker_green" }
		, { x=9, y=4, img="blocker_green" }
		, { x=10, y=4, img="cell_07" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=2, y=5, img="blocker_green" }
		, { x=6, y=5, img="blocker_green" }
		, { x=7, y=5, img="blocker_green" }
		, { x=8, y=5, img="blocker_green" }
		, { x=9, y=5, img="blocker_green" }
		, { x=11, y=5, img="blocker_green" }
		-- row 6
		, { x=2, y=6, img="blocker_green" }
		, { x=3, y=6, img="blocker_green" }
		, { x=4, y=6, img="blocker_green" }
		, { x=13, y=6, img="blocker_green" }
		-- row 7
		, { x=7, y=7, img="blackhole", heading={ 0,1 } }
		, { x=11, y=7, img="blocker_green" }
		, { x=13, y=7, img="blocker_green" }
		-- row 8
		, { x=13, y=8, img="blocker_green" }
		}
	return level
end

function ReflexGame.CreateLevel2x4(self)
	return self:CreateLevel2x1()
end

------- level 3's

function ReflexGame.CreateLevel3x1(self)
	local level = {}
	
	level.name = "3x1"
	
	level.antivirusSpiderSpawnRate = {4, 8}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 60
	
	level.board = {
		-- row 0
		{ x=4, y=0, img="blocker_green" }
		, { x=7, y=0, img="blocker_green" }
		, { x=8, y=0, img="blocker_green" }
		, { x=9, y=0, img="blocker_green" }
		, { x=12, y=0, img="blocker_green" }
		-- row 1
		, { x=4, y=1, img="cell_08" }
		-- row 2
		, { x=4, y=2, img="blocker_green" }
		, { x=9, y=2, img="blocker_green" }
		, { x=12, y=2, img="blocker_green" }
		-- row 3
		, { x=4, y=3, img="blocker_green" }
		, { x=7, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=12, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=7, y=4, img="blocker_green" }
		, { x=8, y=4, img="cell_09" }
		, { x=9, y=4, img="blocker_green" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=4, y=5, img="blocker_green" }
		, { x=7, y=5, img="blocker_green" }
		, { x=9, y=5, img="blocker_green" }
		, { x=12, y=5, img="blocker_green" }
		-- row 6
		, { x=4, y=6, img="blocker_green" }
		, { x=7, y=6, img="blocker_green" }
		, { x=12, y=6, img="blocker_green" }
		-- row 7
		, { x=10, y=7, img="blackhole", heading={ 0,1 } }
		, { x=10, y=7, img="cell_10" }
		-- row 8
		, { x=4, y=8, img="blocker_green" }
		, { x=7, y=8, img="blocker_green" }
		, { x=8, y=8, img="blocker_green" }
		, { x=9, y=8, img="blocker_green" }
		, { x=12, y=8, img="blocker_green" }
		}
	return level
end

function ReflexGame.CreateLevel3x2(self)
	local level = {}
	
	level.name = "3x2"
	
	level.antivirusSpiderSpawnRate = {4, 8}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 60
	
	level.board = {
		-- row 0
		{ x=6, y=0, img="blocker_green" }
		, { x=11, y=0, img="blocker_green" }
		-- row 1
		, { x=6, y=1, img="blocker_green" }
		, { x=11, y=1, img="blocker_green" }
		, { x=15, y=1, img="blackhole", heading={ 1,0 } }
		-- row 2
		, { x=3, y=2, img="cell_11" }
		, { x=6, y=2, img="blocker_green" }
		, { x=11, y=2, img="blocker_green" }
		-- row 3
		, { x=0, y=3, img="blocker_green" }
		, { x=1, y=3, img="blocker_green" }
		, { x=2, y=3, img="blocker_green" }
		, { x=3, y=3, img="blocker_green" }
		, { x=8, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=11, y=3, img="blocker_green" }
		, { x=12, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=3, y=4, img="blocker_green" }
		, { x=8, y=4, img="blocker_green" }
		, { x=9, y=4, img="blocker_green" }
		, { x=12, y=4, img="cell_12" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=3, y=5, img="blocker_green" }
		, { x=8, y=5, img="blocker_green" }
		, { x=9, y=5, img="blocker_green" }
		, { x=11, y=5, img="blocker_green" }
		, { x=12, y=5, img="blocker_green" }
		-- row 6
		, { x=3, y=6, img="blocker_green" }
		, { x=5, y=6, img="blocker_green" }
		, { x=6, y=6, img="blocker_green" }
		, { x=8, y=6, img="blocker_green" }
		, { x=9, y=6, img="blocker_green" }
		, { x=11, y=6, img="blocker_green" }
		-- row 7
		, { x=3, y=7, img="cell_13" }
		, { x=6, y=7, img="blocker_green" }
		, { x=7, y=7, img="blocker_green" }
		, { x=8, y=7, img="blocker_green" }
		, { x=9, y=7, img="blocker_green" }
		, { x=10, y=7, img="blocker_green" }
		, { x=11, y=7, img="blocker_green" }
		-- row 8
		, { x=3, y=8, img="blocker_green" }
		, { x=6, y=8, img="blocker_green" }
		, { x=7, y=8, img="blocker_green" }
		, { x=8, y=8, img="blocker_green" }
		, { x=9, y=8, img="blocker_green" }
		, { x=10, y=8, img="blocker_green" }
		, { x=11, y=8, img="blocker_green" }
		}
	return level
end

function ReflexGame.CreateLevel3x3(self)
	local level = {}
	
	level.name = "3x3"
	
	level.antivirusSpiderSpawnRate = {4, 8}
	level.antivirusSpiderSpeed = {60, 90}
	level.antivirusSpiderLifetime = {300, 300}
	level.antivirusSpiderHeadingTime = {3, 5}
	level.antivirusSpiderSeekPlayerRange = 6 -- grid squares
    level.blackholeSpeed = {50, 50}
    level.blockChaseTime = 1.1 -- delay before blocks start eating player line
    level.blockGrowTime = 1.05
    level.time = 60
	
	level.board = {
		-- row 0
		{ x=7, y=0, img="blocker_green" }
		, { x=8, y=0, img="blocker_green" }
		, { x=9, y=0, img="blocker_green" }
		, { x=14, y=0, img="blackhole", heading={ 0,1 } }
		-- row 1
		, { x=1, y=1, img="blocker_green" }
		, { x=2, y=1, img="blocker_green" }
		, { x=3, y=1, img="blocker_green" }
		, { x=8, y=1, img="blocker_green" }
		-- row 2
		, { x=1, y=2, img="blocker_green" }
		, { x=11, y=2, img="blocker_green" }
		, { x=12, y=2, img="blocker_green" }
		, { x=13, y=2, img="blocker_green" }
		, { x=14, y=2, img="blocker_green" }
		, { x=15, y=2, img="blocker_green" }
		-- row 3
		, { x=6, y=3, img="blocker_green" }
		, { x=7, y=3, img="blocker_green" }
		, { x=9, y=3, img="blocker_green" }
		, { x=10, y=3, img="blocker_green" }
		-- row 4
		, { x=0, y=4, img="mark_start" }
		, { x=5, y=4, img="cell_14" }
		, { x=6, y=4, img="blocker_green" }
		, { x=7, y=4, img="cell_15" }
		, { x=10, y=4, img="blocker_green" }
		, { x=11, y=4, img="blocker_green" }
		, { x=12, y=4, img="blocker_green" }
		, { x=13, y=4, img="cell_16" }
		, { x=14, y=4, img="blocker_green" }
		, { x=15, y=4, img="blocker_green" }
		, { x=16, y=4, img="mark_end" }	
		-- row 5
		, { x=6, y=5, img="blocker_green" }
		, { x=7, y=5, img="blocker_green" }
		, { x=9, y=5, img="blocker_green" }
		, { x=10, y=5, img="blocker_green" }
		-- row 6
		, { x=1, y=6, img="blocker_green" }
		, { x=11, y=6, img="blocker_green" }
		, { x=12, y=6, img="blocker_green" }
		, { x=13, y=6, img="blocker_green" }
		, { x=14, y=6, img="blocker_green" }
		, { x=15, y=6, img="blocker_green" }
		-- row 7
		, { x=1, y=7, img="blocker_green" }
		, { x=2, y=7, img="blocker_green" }
		, { x=3, y=7, img="blocker_green" }
		, { x=8, y=7, img="blocker_green" }
		, { x=11, y=7, img="blackhole", heading={ 0,1 } }
		-- row 8
		, { x=7, y=8, img="blocker_green" }
		, { x=8, y=8, img="blocker_green" }
		, { x=9, y=8, img="blocker_green" }
		}
	return level
end

function ReflexGame.CreateLevel3x4(self)
	return self:CreateLevel3x1()
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