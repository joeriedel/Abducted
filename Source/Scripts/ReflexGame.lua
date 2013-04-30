-- ReflexGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Dave Reese
-- See Abducted/LICENSE for licensing terms

ReflexGame = Class:New()
ReflexGame.active = false

function ReflexGame.DebugStart(self)
    ReflexGame:InitGame("reflex-game: debug_start", 1, 1)
    ReflexGame:ShowBoard(true)
    ReflexGame:StartGame()
end

function ReflexGame.Spawn(self)
	ReflexGame.entity = self
		
	self:LoadMaterials()
	self:InitUI()
	
	-- Note: InitUI sets widgets.root to be invisible (hides the whole board)

end

function ReflexGame.ShowBoard(self, show)
	self = ReflexGame.entity
	-- NOTE: show board is called *after* InitGame
	-- InitGame should get the board ready to be seen
	self.widgets.root:SetVisible(show)
	World.DrawUIOnly(show) -- < disable/enable 3D rendering
end

function ReflexGame.InitGame(self, gameType, playerSkill, terminalSkill)
	self = ReflexGame.entity
	-- InitGame: prep the board to be shown with ShowBoard
	-- but we should not start until StartGame is called.
end

function ReflexGame.StartGame(self, gameCompleteCallback)
	self = ReflexGame.entity
	
	ReflexGame.active = true
	self.gameCompleteCallback = gameCompleteCallback
	
	self.think = ReflexGame.Think
	self:SetNextThink(0)
	
	World.SetEnabledGestures(kIG_Line)
	World.FlushInput(true)
end

function ReflexGame.EndGame(self, result)

	
	World.SetEnabledGestures(0)
	World.FlushInput(true)
	
	self.think = nil
	self.gameCompleteCallback(result);

end

function ReflexGame.ResetGame(self)
	self = ReflexGame.entity
	-- clean up game-board, get ready for another ShowBoard/StartGame call sometime in the future.
	-- NOTE: the terminal puzzle UI is hidden right now
	ReflexGame.active = false
end

function ReflexGame.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.antivirusSpiderSpawnRate = 5
	level.antivirusSpiderSpeed = 40
    level.blackholeSpeed = 40
    level.lineTimerEnabledEnabledTimer = 5
    level.time = 120
	
	level.board = {
		-- row 0
		-- row 1
		{ x=0, y=1, img="mark_start" }	
		, { x=0, y=0, img="cell_green" }
		, { x=3, y=1, img="cell_green" }
		, { x=11, y=1, img="cell_green" }		
		-- row 2
		, { x=3, y=2, img="cell_green" }
		, { x=4, y=2, img="cell_02" }
		, { x=11, y=2, img="cell_green" }		
		, { x=12, y=2, img="cell_green" }				
		-- row 3
		, { x=5, y=3, img="cell_green" }
		-- row 4
		, { x=12, y=4, img="cell_green" }
		, { x=13, y=4, img="cell_04" }
		-- row 5
		, { x=6, y=5, img="cell_green" }
		, { x=13, y=5, img="cell_03" }
		-- row 6
		, { x=2, y=6, img="cell_green" }
		, { x=3, y=6, img="cell_green" }
		-- row 7
		, { x=3, y=7, img="cell_01" }		
		, { x=16, y=7, img="mark_end" }
		-- row 8
		-- row 9
        , { x=8, y=4, img="blackhole", heading={ 0,1 } }
        -- row 10
		-- row 11
		-- row 12
		-- row 13
		-- row 14
		}
	
	return level
end

function ReflexGame.CreateLevel1x2(self)
	local level = {}
	
	level.name = "1x2"	

	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=9, y=3, img="cell_green" }
		, { x=7, y=5, img="cell_green" }
		}
	
	return level
end

function ReflexGame.CreateLevel1x3(self)
	local level = {}
	
	level.name = "1x3"	
	
	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=6, y=3, img="cell_green" }
		, { x=3, y=5, img="cell_green" }
		}
	
	return level
end

function ReflexGame.CreateBoards(self)
	self.db = { }
	self.db.levels = { }    
	
	self.db.levels = {  { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }               
		, { self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3() }                               
		}
end

function ReflexGame.OnInputEvent(self,e)
	self = ReflexGame.entity
	
	if (e.type == kI_KeyDown) then
		--COutLine(kC_Debug,"key=%i",e.data[1])
		if (e.data[1] == kKeyCode_I) then
			--COutLine(kC_Debug,"moving widget")
			self.state.heading.x = 0
			self.state.heading.y = -1
			--UI:MoveWidgetByCenter(self.widgets.current,UI.screenWidth/2, UI.screenHeight/2)
			return true
		end
		if (e.data[1] == kKeyCode_K) then
			self.state.heading.x = 0
			self.state.heading.y = 1
			return true
		end		
		if (e.data[1] == kKeyCode_J) then
			self.state.heading.x = -1
			self.state.heading.y = 0
			return true
		end				
		if (e.data[1] == kKeyCode_L) then
			self.state.heading.x = 1
			self.state.heading.y = 0
			return true
		end						
	end
	
	return false
end

function ReflexGame.OnInputGesture(self,g)
	self = ReflexGame.entity
	
	if (g.id ~= kIG_Line) then
		return true
	end
	
	-- left?
	if (g.args[1] > 0.707) then
		self.state.heading.x = 1
		self.state.heading.y = 0
	elseif (g.args[1] < -0.707) then -- right?
		self.state.heading.x = -1
		self.state.heading.y = 0
	elseif (g.args[2] > 0.707) then -- down
		self.state.heading.x = 0
		self.state.heading.y = 1
	else -- up
		self.state.heading.x = 0
		self.state.heading.y = -1
	end
	
	return true
end

function ReflexGame.InitUI(self)
	-- constants
	self.REFLEX_CELL_SIZE = {67*UI.identityScale[1], 67*UI.identityScale[1] }
    self.BLACKHOLE_SIZE = {100*UI.identityScale[1], 100*UI.identityScale[1]}
	self.REFLEX_BOARD_OFFSET = {self.screen[1], self.screen[2]}
	self.INDEX_MAX_X = 16
	self.INDEX_MAX_Y = 9
    self.LINE_SPAWN_TIME = 1
	self.PLAYER_SPEED = 100
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + 0 * self.REFLEX_CELL_SIZE[1]
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + 0 * self.REFLEX_CELL_SIZE[2]
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE[1]
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE[2]	

	self:CreateBoards()	
	
	-- define structure: self.state
	local level = self.db.levels[1][1] -- load appropriate level based on skill + difficulty
	self.state = { }	
	self.state.heading = { }
	self.state.lastHeading = { }
	self.state.victory = { }
	self.state.current = {}
	self.state.path = {}
	self.state.pathByCell = { }
	self.state.heading.x = 0
	self.state.heading.y = 0
	self.state.lastHeading.x = 0
	self.state.lastHeading.y = 0
	self.state.gameOver = false
	self.state.currentMove = 1
	self.state.gameOverTimer = 2	
	self.state.victory = false
	self.state.level = level
    self.state.timeLeft = level.time
	self.state.spawnTimer = level.antivirusSpiderSpawnRate	
	self.state.antivirusSpawnTimer = level.antivirusSpiderSpawnRate
    self.state.lineTimerEnabledTimer = level.lineTimerEnabledEnabledTimer
    self.state.lineTimer = 0
	self.state.goalCounter = 1
    self.state.lineIndex = 1
    self.state.fadeInBoardTimer = 2
    self.state.swipeToMoveTimer = 1
    self.state.touchWhenReadyTimer = 3

	-- define structure self.widgets
	self.widgets = {}
	self.widgets.goals = { }
	self.widgets.board = { }		
	self.widgets.lines = { }
	self.widgets.spiders = { }
    self.widgets.blackholes = { }
	self.widgets.grid = {}
	self.widgets.cells = {}

	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=ReflexGame.OnInputEvent, OnInputGesture=ReflexGame.OnInputGesture})
	World.SetRootWidget(UI.kLayer_TerminalPuzzles, self.widgets.root)
	self.widgets.root:SetOpaqueLayerInput(true) -- no input goes past this
	
	self.widgets.root:SetVisible(false)

	self.widgets.border = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.board})
--	UI:MoveWidgetByCenter(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
--	UI:MoveWidgetByCenter(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
	self.widgets.root:AddChild(self.widgets.border)
	self.widgets.root:AddChild(self.widgets.board)

	COutLine(kC_Debug, "reflex.level.name=" .. self.state.level.name)

	COutLine(kC_Debug, "Creating Board")	
	-- board step: board grid is x,y structure
	for i,v in ipairs(self.state.level.board) do

        local objectTable = self.widgets.cells
        local objectSize = { self.REFLEX_CELL_SIZE[1], self.REFLEX_CELL_SIZE[2] }

        if (self.gfx[v.img] == self.gfx.blackhole) then
            objectTable = self.widgets.blackholes
            objectSize = { self.BLACKHOLE_SIZE[1], self.BLACKHOLE_SIZE[2] }
        end

		local b = UI:CreateWidget("MatWidget", {rect={0,0,objectSize[1],objectSize[2]}, material=self.gfx[v.img]})
		local index = self:ConvertCoordToIndex(v.x,v.y)
		b.state = self:CreateState(v.img,v)
        b:BlendTo({1,1,1,0}, 0)
		self.widgets.root:AddChild(b)
        if (objectTable == self.widgets.cells) then
		    self.widgets.grid[index] = b
        end
        table.insert(objectTable,b)
		self:SetPositionByGrid(b,v.x,v.y)
		if (v.img == "mark_start") then
            local player = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.mark_current})
            player.state = self:CreateState("player",v)
            self.widgets.player = player
            self.widgets.root:AddChild(player)
            self:SetPositionByGrid(player,v.x,v.y)

			local current = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.mark_line_v})
			current.state = self:CreateState("mark_current",v)
			self.widgets.current = current
			self.widgets.root:AddChild(current)
			self:SetPositionByGrid(current,v.x,v.y)
            current.state.lastCell = v
			current.state.startPos = self:GetPosition(current) 
			current.state.endPos = current.state.startPos
			table.insert(self.widgets.lines,current)
			self:SetLineSegmentPosition(current,current.state.startPos,current.state.endPos)			
			COutLine(kC_Debug,"current widget: x=%i, y=%i",v.x,v.y)	
		end
    end

    self.widgets.timeLeftLabel = UI:CreateWidget("TextLabel", {rect={80, 35, UI.screenWidth*.15,UI.screenHeight*.15}, typeface=self.typefaces.TimerText})
    self.widgets.root:AddChild(self.widgets.timeLeftLabel)

    self.widgets.touchWhenReadyLabel = UI:CreateWidget("TextLabel", {rect={ 320, UI.screenHeight/2 - 65, UI.screenWidth, UI.screenHeight/2 + 20}, typeface=self.typefaces.TouchWhenReadyText})
    self.widgets.touchWhenReadyLabel:SetText("Touch when ready")
    self.widgets.touchWhenReadyLabel:BlendTo({1,1,1,0}, 0)
    self.widgets.root:AddChild(self.widgets.touchWhenReadyLabel)

    self.widgets.swipeToMoveLabel = UI:CreateWidget("TextLabel", {rect={ 60, 500, 500, 100}, typeface=self.typefaces.SwipeToMoveText})
    self.widgets.swipeToMoveLabel:SetText("Swipe to move - Collect the Glyphs")
    self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, 0)
    self.widgets.root:AddChild(self.widgets.swipeToMoveLabel)

    self:UpdateHud()

    if (self.widgets.current == nil) then
		COutLine(kC_Debug,"mark_start NOT FOUND --> ERROR")	
	end
	
	COutLine(kC_Debug, "Board Completed")		
end

function ReflexGame.LoadMaterials(self)
	
	self.gfx = {}
	self.gfx.blackhole = World.Load("Puzzles/reflex-blackhole1_M");
	self.gfx.antivirus_spider = World.Load("Puzzles/AlienICE_M")
    self.gfx.blue_glow = World.Load("Puzzles/reflex-blueglow1_M")
    self.gfx.board = World.Load("Puzzles/reflex-checkerboard1_M")
	self.gfx.border = World.Load("UI/arm_screen1_M")

    self.gfx.mark_current = World.Load("Puzzles/reflex-player1_M")
    self.gfx.mark_line_v = World.Load("Puzzles/reflex-goal1_M")
    self.gfx.mark_line_h = self.gfx.mark_line_v
    self.gfx.mark_end = World.Load("Puzzles/reflex-goal1_M")

    self.gfx.cell_green = World.Load("Puzzles/reflex-block1_M")

    self.gfx.cell_01 = World.Load("Puzzles/glyph01_M")
    self.gfx.cell_02 = World.Load("Puzzles/glyph02_M")
    self.gfx.cell_03 = World.Load("Puzzles/glyph03_M")
    self.gfx.cell_04 = World.Load("Puzzles/glyph04_M")
    self.gfx.cell_05 = World.Load("Puzzles/glyph05_M")
    self.gfx.cell_06 = World.Load("Puzzles/glyph06_M")
    self.gfx.cell_07 = World.Load("Puzzles/glyph07_M")
    self.gfx.cell_08 = World.Load("Puzzles/glyph08_M")
    self.gfx.cell_09 = World.Load("Puzzles/glyph09_M")
    self.gfx.cell_10 = World.Load("Puzzles/glyph10_M")
    self.gfx.cell_11 = World.Load("Puzzles/glyph11_M")
    self.gfx.cell_12 = World.Load("Puzzles/glyph12_M")
    self.gfx.cell_13 = World.Load("Puzzles/glyph13_M")
    self.gfx.cell_14 = World.Load("Puzzles/glyph14_M")
    self.gfx.cell_15 = World.Load("Puzzles/glyph15_M")
    self.gfx.cell_16 = World.Load("Puzzles/glyph16_M")
    self.gfx.cell_17 = World.Load("Puzzles/glyph17_M")
    self.gfx.cell_18 = World.Load("Puzzles/glyph18_M")
    self.gfx.cell_19 = World.Load("Puzzles/glyph19_M")
    self.gfx.cell_20 = World.Load("Puzzles/glyph20_M")
    self.gfx.cell_21 = World.Load("Puzzles/glyph21_M")
    self.gfx.cell_22 = World.Load("Puzzles/glyph22_M")
    self.gfx.cell_23 = World.Load("Puzzles/glyph23_M")
    self.gfx.cell_24 = World.Load("Puzzles/glyph24_M")
    self.gfx.cell_25 = World.Load("Puzzles/glyph25_M")
    self.gfx.cell_26 = World.Load("Puzzles/glyph26_M")
    self.gfx.cell_27 = World.Load("Puzzles/glyph27_M")

    self.gfx.mark_start = nil
	self.typefaces = {}
	self.typefaces.BigText = World.Load("UI/TerminalPuzzlesBigFont_TF")
    self.typefaces.TimerText = World.Load("UI/TerminalPuzzlesBigFont_TF")
    self.typefaces.TouchWhenReadyText = World.Load("UI/TerminalPuzzlesBigFont_TF")
    self.typefaces.SwipeToMoveText = World.Load("UI/TerminalPuzzlesBigFont_TF")

	local xScale = UI.screenWidth / 1280
	local yScale = UI.screenHeight / 720
	
	-- the border is authored to be a 16:9 image packed in a square image, adjust for this
	
	local region = (1 - UI.yAspect) / 2
	local inset  = region * UI.screenWidth
	
	self.magicBoardRect = {0, -inset, UI.screenWidth, UI.screenHeight+inset*2}
	
	local wideRegion = (1 - (9/16)) / 2
	local wideInset = wideRegion * 1280 * xScale
	
	if (UI.systemScreen.aspect == "4x3") then
		wideInset = wideInset * 0.91 -- wtf?
	end
	
	self.screen = {
		1280 * 0.05156 * xScale,
		(720 * 0.07644 * yScale) + (wideInset-inset)*yScale,
		0,
		0
	}
	
	self.screen[3] = UI.screenWidth - (self.screen[1]*2)
	self.screen[4] = UI.screenHeight - (self.screen[2]*2)
end

function ReflexGame.GetPositionByGrid(self,x,y)
    local v = { }
    v.x = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + x * self.REFLEX_CELL_SIZE[1]
    v.y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + y * self.REFLEX_CELL_SIZE[2]
    return v
end


function ReflexGame.SetPositionByGrid(self,w,x,y)
    local v = self:GetPositionByGrid(x,y)
	UI:MoveWidgetByCenter(w,v.x,v.y)
	--COutLine(kC_Debug,"position line @ x=%.02f,y=%.02f",xo,yo)
end

function ReflexGame.Vec2Normal(self,x,y)
	local n = math.sqrt(x * x + y * y)
	--COutLine(kC_Debug,"Vec2Normal: x=%.02f, y=%.02f, n = %f",x,y,n)	
	local vec2 = { }
	vec2.x = 0
	vec2.y = 0
	if (n > 0) then
		vec2.x = x / n
		vec2.y = y / n
	end
	return vec2
end

function ReflexGame.LerpVec2(self,v,heading,dt,speed)	
	local dx = heading.x * dt * speed
	local dy = heading.y * dt * speed
	
	local x = v.x + dx
	local y = v.y + dy
		
	local vec2 = { }
	vec2.x = x
	vec2.y = y
	
	return vec2	
end

function ReflexGame.LerpWidget(self,widget,heading,dt,speed,constrain)
	local r = widget:Rect()
	
	local width = r[3]
	local height = r[4]
	local half_width = width/2
	local half_height = height/2

	local wv = { }
	wv.x = r[1] + half_width
	wv.y = r[2] + half_height

	local o = self:LerpVec2(wv,heading,dt,speed)

    if (constrain) then
        if (o.x < half_width) then
            o.x = 0
        end
        if (o.x + width > UI.screenWidth) then
            o.x = UI.screenWidth - half_width
        end
        if (o.y < half_height) then
            o.y = 0
        end
        if (o.y + height > UI.screenHeight) then
            o.y = UI.screenHeight - half_height
        end
	end
	return o
end

function ReflexGame.GetGridCellFromVec2(self,v)
	local x = (v.x - self.REFLEX_BOARD_OFFSET[1])/self.REFLEX_CELL_SIZE[1]
	local y = (v.y - self.REFLEX_BOARD_OFFSET[2])/self.REFLEX_CELL_SIZE[2]
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function ReflexGame.GetGridCell(self,widget)
	local pos = self:GetPosition(widget)

	local x = (pos.x - self.REFLEX_BOARD_OFFSET[2])/self.REFLEX_CELL_SIZE[1]
	local y = (pos.y - self.REFLEX_BOARD_OFFSET[2])/self.REFLEX_CELL_SIZE[2]
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function ReflexGame.IsGridCellOnBoard(self,x,y)
	if (x >= 0 and y >= 0 and x < self.INDEX_MAX_X and y < self.INDEX_MAX_Y) then
		return true
	end
		
	return false
end

function ReflexGame.ConvertCoordToIndex(self,x,y)
	return bit.bor(bit.lshift(y, 16), x)
end

function ReflexGame.ConstrainPointToBoard(self,x,y)
	if (x < self.COORD_MIN_X) then
		x = self.COORD_MIN_X
	end
	if (x >= self.COORD_MAX_X) then
		x = self.COORD_MAX_X - 1
	end
	if (y < self.COORD_MIN_Y) then
		y = self.COORD_MIN_Y
	end		
	if (y >= self.COORD_MAX_Y) then
		y = self.COORD_MAX_Y - 1
	end	

	local vec2 = { }
	vec2.x = x
	vec2.y = y
	return vec2			
end

function ReflexGame.GetPosition(self,w)
	local r = w:Rect()
	local vec2 = { } 
	vec2.x = r[1] + r[3]/2
	vec2.y = r[2] + r[4]/2	
	return vec2
end


function ReflexGame.CreateState(self,architype,ref)
	local state = { }
	state.architype = architype
    state.ref = ref
	return state
end

function ReflexGame.SetLineSegmentPosition(self,line,startPos,endPos)
	--COutLine(kC_Debug,"SetLineSegment start/end @ start=%i,%i, end=%i,%i",startPos.x,startPos.y,endPos.x,endPos.y)		

	local x = startPos.x
	local y = startPos.y
	
	local xx = endPos.x
	local yy = endPos.y
	
	if (x > xx) then
		x = endPos.x
		xx = startPos.x
	end
	
	if (y > yy) then
		y = endPos.y
		yy = startPos.y
	end 
	
	local width = xx - x
	if (width == 0) then
		width = 5
	end
	
	local height = yy - y
	if (height == 0) then
		height = 5
	end
	
	local r = { }
	r[1] = x
	r[2] = y
	r[3] = width
	r[4] = height
	
	--COutLine(kC_Debug,"SetLineSegment @ x=%i, y=%i, width=%i, height=%i",x,y,width,height)		
	line:SetRect(r)
end

function ReflexGame.CollideWithHazard(self,x,y)
    local v2 = {}
    v2.x = x
    v2.y = y
    local cell = self:GetGridCellFromVec2(v2)

    for i,k in pairs(self.widgets.blackholes) do
        local v = self:GetGridCell(k)
        if (v.x == cell.x and v.y == cell.y) then
            return true
        end
    end

    for i,k in pairs(self.widgets.spiders) do
        local v = self:GetGridCell(k)
        if (v.x == cell.x and v.y == cell.y) then
            return true
        end
    end
	
	return false
end

function ReflexGame.CollideWithLine(self,x,y,ignore)
    local count = #self.widgets.lines
    if (ignore) then
        count = count - 2
    end

    for i, k in pairs(self.widgets.lines) do
        local r = k:Rect()
        if (i < count and x >= r[1] and x < r[1]+r[3] and y >= r[2] and y < r[2] + r[4]) then
            return true
        end
    end

    return false
end

function ReflexGame.CollideWithBoard(self,x,y,isPlayer)
	if (x < self.COORD_MIN_X) then
		COutLine(kC_Debug,"CollideWihtBoard found min X @ x=%i, y=%i",x,y)		
		return true
	end
	if (x >= self.COORD_MAX_X) then
		COutLine(kC_Debug,"CollideWihtBoard found max X @ x=%i, y=%i",x,y)			
		return true
	end
	if (y < self.COORD_MIN_Y) then
		COutLine(kC_Debug,"CollideWihtBoard found min Y @ x=%i, y=%i",x,y)			
		return true
	end		
	if (y >= self.COORD_MAX_Y) then
		COutLine(kC_Debug,"CollideWihtBoard found max Y @ x=%i, y=%i",x,y)			
		return true
	end	
	
	local screenCoord = { }
	screenCoord.x = x
	screenCoord.y = y
	local v = self:GetGridCellFromVec2(screenCoord)
	local index = self:ConvertCoordToIndex(v.x,v.y)
	
	local piece = self.widgets.grid[index]		
	if (piece == nil) then
		return false
	end
	
	if (isPlayer) then
		if (piece.state.architype == "mark_end" or piece.state.architype == "mark_start") then
			return false
		end
		
		if (piece.state.architype == "cell_01" 
			or piece.state.architype == "cell_02" 
			or piece.state.architype == "cell_03" 
			or piece.state.architype == "cell_04" 
		) then
			return false
		end
	end
	
	COutLine(kC_Debug,"CollideWihtBoard found Piece @ x=%i, y=%i, type=%s",x,y,piece.state.architype)			
	return true
end

function ReflexGame.UpdateHud(self)
    local minutes = math.floor(self.state.timeLeft / 60)
    local seconds = self.state.timeLeft - (minutes * 60)
    local text = string.format("%i:%02i",minutes,seconds)
    self.widgets.timeLeftLabel:SetText(text)
end

function ReflexGame.Think(self,dt)
	if (self.state.gameOver) then
		self.state.gameOverTimer = self.state.gameOverTimer - dt
		if (self.state.gameOverTimer < 0) then
			self.state.gameOverTimer = 0
			self:EndGame("win")
			return
		end
		
		if (self.widgets.bigTextLabel == nil) then 
			self.widgets.bigTextLabel = UI:CreateWidget("TextLabel", {rect={0, 0, UI.screenWidth,UI.screenHeight}, typeface=self.typefaces.BigText})
			self.widgets.root:AddChild(self.widgets.bigTextLabel)		
		end
		--self.widgets.bigTextLabel:SetText(string.fromat("%i",self.state.gameOverTimer))
		if (self.state.victory) then
			self.widgets.bigTextLabel:SetText("You Win!")
		else
			self.widgets.bigTextLabel:SetText("You Lose!")
		end
		UI:CenterLabel(self.widgets.bigTextLabel, UI.fullscreenRect)
				
		-- game over never advances past here
		return
    end

    if (self.state.swipeToMoveTimer > 0) then
        self.state.swipeToMoveTimer = self.state.swipeToMoveTimer - dt
        if (self.state.swipeToMoveTimer <= 0) then
            self.widgets.swipeToMoveLabel:BlendTo({1,1,1,1}, .5)
        end
    end

    if (self.state.fadeInBoardTimer > 0) then
        self.state.fadeInBoardTimer = self.state.fadeInBoardTimer - dt
        if (self.state.fadeInBoardTimer <= 0) then
            for i,k in pairs(self.widgets.cells) do
                k:BlendTo({1,1,1,1}, 2)
            end
            for i,k in pairs(self.widgets.blackholes) do
                k:BlendTo({1,1,1,1}, 2)
            end
        end
        return
    end

    if (self.state.touchWhenReadyTimer > 0) then
        self.state.touchWhenReadyTimer = self.state.touchWhenReadyTimer - dt
        if (self.state.touchWhenReadyTimer <= 0) then
            self.widgets.touchWhenReadyLabel:BlendTo({1,1,1,1}, 2)
        end
    end

	if (self.state.heading.x == 0 and self.state.heading.y == 0) then
		-- NO HEADING: Game hasn't started		
		return
    end

    if (not self.state.labelFadeOut) then
        self.widgets.touchWhenReadyLabel:BlendTo({1,1,1,0}, .5)
        self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, .5)
        self.state.labelFadeOut = true
    end

	if (self.state.lastHeading.x == 0 and self.state.lastHeading.y == 0) then
		self.state.lastHeading.x = self.state.heading.x
		self.state.lastHeading.y = self.state.heading.y
	end
	
	if (self.widgets.current.heading == nil) then
		self.widgets.current.heading = self.state.heading
	end

	local currentPos = self:LerpVec2(self.widgets.current.state.endPos,self.state.lastHeading,dt,self.PLAYER_SPEED)
	if (self:CollideWithBoard(currentPos.x,currentPos.y,true) or self:CollideWithHazard(currentPos.x,currentPos.y)) then
		COutLine(kC_Debug,"GameOver player collided with board @ : x=%i, y=%i",currentPos.x,currentPos.y)			
		self.state.gameOver = true
		return
    end

	--COutLine(kC_Debug,"currentPos: x=%.02f, y=%.02f",currentPos.x,currentPos.y)
	currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y)
	self.widgets.current.state.endPos = currentPos
	self:SetLineSegmentPosition(self.widgets.current,self.widgets.current.state.startPos,self.widgets.current.state.endPos)
    UI:MoveWidgetByCenter(self.widgets.player,currentPos.x,currentPos.y)
    local nextCell = self:GetGridCellFromVec2(currentPos)
    if (nextCell.x ~= self.widgets.current.state.lastCell.x or nextCell.y ~= self.widgets.current.state.lastCell.y) then
        self.widgets.current.state.lastCell = nextCell
        local index = self:ConvertCoordToIndex(nextCell.x,nextCell.y)
        if (self.state.pathByCell[index] == nil) then
            self.state.pathByCell[index] = nextCell
            self.state.path[#self.state.path+1] = nextCell
        end
    end

	-- detect change of direction
	if (self.state.lastHeading.x ~= self.state.heading.x or self.state.lastHeading.y ~= self.state.heading.y) then
        currentPos = self:GetPositionByGrid(nextCell.x,nextCell.y)
        self:SetPositionByGrid(self.widgets.player,currentPos.x,currentPos.y)
        self.widgets.current.state.endPos = currentPos
        self:SetLineSegmentPosition(self.widgets.current,self.widgets.current.state.startPos,self.widgets.current.state.endPos)
        local angle = 0
        if (self.state.heading.x < 0) then
            angle = 180
        elseif (self.state.heading.y < 0) then
            angle = 270
        elseif (self.state.heading.y > 0) then
            angle = 90
        end
        self.widgets.player:RotateTo({self.REFLEX_CELL_SIZE[1]/2, self.REFLEX_CELL_SIZE[2]/2, angle}, {0, 0, .05}, true)
        -- so the first arg there is a Vec3, or just a [3] array
        -- cx, cy are the coordinates to rotate around

		local oldR = self.widgets.current:Rect()
		COutLine(kC_Debug,"oldLineSegment @ x=%i, y=%i, width=%i, height=%i",oldR[1],oldR[2],oldR[3],oldR[4])			
		COutLine(kC_Debug,"newLineSegment @ currentPos: x=%i, y=%i",currentPos.x,currentPos.y)
		local line = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.mark_line_v})
		line.state = self:CreateState("mark_line_v")
		table.insert(self.widgets.lines,line)
		self.widgets.current = line
		self.widgets.root:AddChild(line)

        line.state.lastCell = nextCell
		line.state.startPos = currentPos
		line.state.endPos = line.state.startPos
		self:SetLineSegmentPosition(line,line.state.startPos,line.state.endPos)
		
		self.state.lastHeading.x = self.state.heading.x
		self.state.lastHeading.y = self.state.heading.y
    end

    self.state.lineTimerEnabledTimer = self.state.lineTimerEnabledTimer - dt
    if (self.state.lineTimerEnabledTimer <= 0) then
        self.state.lineTimer = self.state.lineTimer + dt
        if (self.state.lineTimer >= self.LINE_SPAWN_TIME) then
            self.state.lineTimer = 0

            local v = self.state.path[self.state.lineIndex]
            local index = self:ConvertCoordToIndex(v.x,v.y)
            local b = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.cell_green})
            b.state = self:CreateState("cell_green")
            self.widgets.root:AddChild(b)
            self.widgets.grid[index] = b
            self:SetPositionByGrid(b,v.x,v.y)
            b:ScaleTo({0,0}, {0,0})
            b:ScaleTo({1,1}, {1,1})
            self.state.lineIndex = self.state.lineIndex + 1
        end
    end

    self.state.timeLeft = self.state.timeLeft - dt
    self:UpdateHud()

	local playerGridCell = self:GetGridCellFromVec2(currentPos)
	local playerIndex = self:ConvertCoordToIndex(playerGridCell.x,playerGridCell.y)	 

	local pieceAtPlayer = self.widgets.grid[playerIndex]	
	if (pieceAtPlayer or self.state.timeLeft <= 0) then
		if (self.state.goalCounter < #self.widgets.goals) then
			local goalPiece = self.widgets.goals[self.state.goalCounter]
			if (goalPiece.state.architype == pieceAtPlayer.state.architype) then
				COutLine(kC_Debug,"Goal accomplished: x=%s",goalPiece.state.architype)	
				self.state.goalCounter = self.state.goalCounter + 1
				-- TODO: -djr Question: How do they want to indicate that you activated a box, just
				-- swap out the cell with another cell?				
			end
		end		
		
		if (pieceAtPlayer.state.architype == "mark_end" and self.state.goalCounter >= #self.widgets.goals) then
			COutLine(kC_Debug,"Game Over Detected")
			self.state.gameOver = true
			self.state.victory = true
			return
		end

        if (self.state.timeLeft <= 0) then
            COutLine(kC_Debug,"Game Over Detected (time ran out)")
            self.state.gameOver = true
            self.state.victory = false
            return
        end
    end
-- this isn't how it works now
--	if (self:CollideWithLine(currentPos.x,currentPos.y,true)) then
--		COutLine(kC_Debug,"Game Over - collided with own line")
--		self.state.gameOver = true
--		return
--	end
	
	--COutLine(kC_Debug,"antivirusSpawnTimer=%i, dt=%f, rate=%i",self.state.antivirusSpawnTimer,dt,self.state.level.antivirusSpiderSpawnRate)
	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
	if (self.state.antivirusSpawnTimer < 0) then
		self.state.antivirusSpawnTimer = self.state.level.antivirusSpiderSpawnRate		
		local x = math.random(self.INDEX_MAX_X)-1
		local y = math.random(self.INDEX_MAX_Y)-1		
		local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.antivirus_spider})
		self.widgets.root:AddChild(spider)	
		table.insert(self.widgets.spiders,spider)
		self:SetPositionByGrid(spider,x,y)		
		spider.state = self:CreateState("antivirus_spider")
		spider.state.heading = self:Vec2Normal(math.random() * 2 - 1,math.random() * 2 - 1)
		if (spider.state.heading.x == 0 and spider.state.heading.y == 0) then -- failsafe
			spider.state.heading.x = 1
		end
		COutLine(kC_Debug,"spawnedSpider @ grid: x=%i, y=%i, heading = %.04f,%.04f",x,y,spider.state.heading.x,spider.state.heading.y)
    end

    --COutLine(kC_Debug,"Blackhole move")
    for i,k in pairs(self.widgets.blackholes) do
        if (k.state.heading == null) then
            k.state.heading = {}
            k.state.heading.x = k.state.ref.heading[1]
            k.state.heading.y = k.state.ref.heading[2]
            COutLine(kC_Debug,"blackhole heading @ : x=%i, y=%i",k.state.heading.x,k.state.heading.y)
        end

        local pos = k:Rect()
--        COutLine(kC_Debug,"pos @ : x=%i, y=%i, dt=%.04f, heading = %.04f,%.04f, speed=%i",pos[1]+pos[3]/2,pos[2]+pos[4]/2,dt,k.state.heading.x,k.state.heading.y,self.state.level.antivirusSpiderSpeed)
        local nextPos = self:LerpWidget(k,k.state.heading,dt,self.state.level.blackholeSpeed,false)
        if (self:CollideWithBoard(nextPos.x,nextPos.y,false)) then
            k.state.heading.x = -k.state.heading.x
            k.state.heading.y = -k.state.heading.y
            nextPos = self:LerpWidget(k,k.state.heading,2*dt,self.state.level.blackholeSpeed,false)
            COutLine(kC_Debug,"blackhole bounce @ : x=%i, y=%i",nextPos.x,nextPos.y)
        end
        UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
    end

    --COutLine(kC_Debug,"Spider move")
	for i,k in pairs(self.widgets.spiders) do	
		local pos = k:Rect()
--		COutLine(kC_Debug,"pos @ : x=%i, y=%i, dt=%.04f, heading = %.04f,%.04f, speed=%i",pos[1]+pos[3]/2,pos[2]+pos[4]/2,dt,k.state.heading.x,k.state.heading.y,self.state.level.antivirusSpiderSpeed)
		local nextPos = self:LerpWidget(k,k.state.heading,dt,self.state.level.antivirusSpiderSpeed,false)
        UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
        if (self:CollideWithBoard(nextPos.x,nextPos.y,false)) then
			table.remove(self.widgets.spiders,i)
			self.widgets.root:RemoveChild(k)
			COutLine(kC_Debug,"remove spider @ : x=%i, y=%i",nextPos.x,nextPos.y)
-- this isn't how it works now
--		else
--			UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
--			--COutLine(kC_Debug,"move spider to: x=%.02f, y=%.02f",nextPos.x,nextPos.y)
--			-- TODO: detect spider crossing a line segment
--			if (self:CollideWithLine(nextPos.x,nextPos.y,false)) then
--				COutLine(kC_Debug,"Game Over - spider crossed player line")
--				self.state.gameOver = true
--				return
--			end
		end	
	end	
end

reflex_game = ReflexGame
