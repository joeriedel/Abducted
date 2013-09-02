-- MemoryGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Dave Reese
-- See Abducted/LICENSE for licensing terms

MemoryGame = Class:New()
MemoryGame.active = false

kMGPhase_AnimateOpening = 0
kMGPhase_ShuffleSmallTiles = 1
kMGPhase_DiscoverPattern = 2
kMGPhase_AnimateTransition = 3
kMGPhase_FindPattern = 4
kMGPhase_GameOver = 99

function MemoryGame.DebugStart(self)
    MemoryGame:InitGame("memory-game: debug_start", 1, 1)
    MemoryGame:ShowBoard(true)
    MemoryGame:StartGame()
end

function MemoryGame.FlattenList(self,list)
    local temp = {}
    for i,v in ipairs(list) do
        table.insert(temp,v)
    end
    return temp
end

function MemoryGame.Spawn(self)
	MemoryGame.entity = self
		
	self:LoadMaterials()
	self:InitUI()
	
	-- Note: InitUI sets widgets.root to be invisbile (hides the whole board)

end

function MemoryGame.ShowBoard(self, show)
	self = MemoryGame.entity
	-- NOTE: show board is called *after* InitGame
	-- InitGame should get the board ready to be seen
	self.widgets.root:SetVisible(show)
	World.DrawUIOnly(show) -- < disable/enable 3D rendering
end

function MemoryGame.InitGame(self, gameType, playerSkill, terminalSkill)
	self = MemoryGame.entity
	-- InitGame: prep the board to be shown with ShowBoard
	-- but we should not start until StartGame is called.
end

function MemoryGame.StartGame(self, gameCompleteCallback)
	self = MemoryGame.entity
	
	MemoryGame.active = true
	self.gameCompleteCallback = gameCompleteCallback
	
	self.think = MemoryGame.Think
	self:SetNextThink(0)

    World.SetEnabledGestures(kIG_Line)
	World.FlushInput(true)
end

function MemoryGame.EndGame(self, result)

	
	World.SetEnabledGestures(0)
	World.FlushInput(true)
	
	self.think = nil
	self.gameCompleteCallback(result);

end

function MemoryGame.ResetGame(self)
	self = MemoryGame.entity
	-- clean up game-board, get ready for another ShowBoard/StartGame call sometime in the future.
	-- NOTE: the terminal puzzle UI is hidden right now
	MemoryGame.active = false
end

function MemoryGame.RandomizeBoard(self,level,legend)
    local all = {}
    for iy=1,level.INDEX_MAX_Y do
        for ix=1,level.INDEX_MAX_X do
            local gi = math.random(self.NUM_GLYPHS)
            local name = string.format("cell_%02i",gi)
            table.insert(level.board,{x=ix, y=iy, img=name})
            all[gi] = name
        end
    end

    if (not legend) then
        local all_array = self:FlattenList(all)

--        COutLine(kC_Debug,"All Count: %i",#all_array)
        while (#level.legend < 3) do
            local i = math.random(#all_array)
            local v = all_array[i]
            table.insert(level.legend,v)
            table.remove(all_array,i)
            all_array = self:FlattenList(all_array)
        end
    else
        level.legend = legend

        local x = math.random(level.INDEX_MAX_X)
        local y = math.random(level.INDEX_MAX_Y)

        if (math.random(100) < 50) then
            if (x >= level.INDEX_MAX_X - #legend) then
                x = level.INDEX_MAX_X - #legend
            end

            -- suppose we could do diagonal
            local counter = 1
            for xi=x,x + #legend-1 do
                for i,v in ipairs(level.board) do
                    if (v.x == xi and v.y == y) then
--                    COutLine(kC_Debug,"phase 2 - legend imprint: index: %i and x,y: %i, %i:",index,xi,y)
                        local refImage = legend[counter]
                        counter = counter + 1
                        v.img = refImage
                        break
                    end
                end
            end
        else
            if (y > level.INDEX_MAX_Y - #legend) then
                y = level.INDEX_MAX_Y - #legend
            end

            -- suppose we could do diagonal
            local counter = 1
            for yi=y,y + #legend-1 do
                for i,v in ipairs(level.board) do
                    if (v.x == x and v.y == yi) then
--                    COutLine(kC_Debug,"phase 2 - legend imprint: index: %i and x,y: %i, %i:",index,xi,y)
                        local refImage = legend[counter]
                        counter = counter + 1
                        v.img = refImage
                        break
                    end
                end
            end
        end
    end
end

function MemoryGame.ShuffleBoard(self)
    local newPieces = {}

    for iy=1,self.state.level.INDEX_MAX_Y do
        for ix=1,self.state.level.INDEX_MAX_X do
            local i = math.random(#self.state.level.board)
            local v = self.state.level.board[i]
--            COutLine(kC_Debug,"b: %i, i: %i",#self.state.level.board,i)
            local bv = {x=ix, y=iy, img=v.img }
            local index = self:ConvertCoordToIndex(ix,iy)
            local w = self.widgets.grid[index]
            w:SetMaterial(self.gfx[bv.img])
            w.state = self:CreateState(bv.img,bv)
            table.insert(newPieces,bv)
            table.remove(self.state.level.board,i)
            self.state.level.board = self:FlattenList(self.state.level.board)
        end
    end
    self.state.level.board = newPieces
end

function MemoryGame.CreateBoardSmall(self)
    local level = {}

    level.name = "Small"
    level.time = 120
    level.INDEX_MAX_X = self.SMALL_INDEX_MAX_X
    level.INDEX_MAX_Y = self.SMALL_INDEX_MAX_Y
    level.CELL_SCALE = self.SMALL_CELL_SCALE
    level.board = {}
    level.legend = {}

    self:RandomizeBoard(level,nil)

    return level
end

function MemoryGame.CreateBoardLarge(self,legend)
    local level = {}

    level.name = "Large"
    level.time = 120
    level.INDEX_MAX_X = self.LARGE_INDEX_MAX_X
    level.INDEX_MAX_Y = self.LARGE_INDEX_MAX_Y
    level.CELL_SCALE = self.LARGE_CELL_SCALE
    level.board = {}
    level.legend = {}

    self:RandomizeBoard(level,legend)

    return level
end


function MemoryGame.CreateBoards(self)
    self.db = { }
    self.db.levels = { }

    self.db.levels = {  { self:CreateBoardSmall(), self:CreateBoardSmall() }
    }
end

--function MemoryGame.CheckTouch(self, v, x, y)
--    return CheckWorldTouch(
--        v:WorldPos(),
--        x,
--        y,
--        (.75 * UI.systemScreen.diagonal)
--    )
--end

function MemoryGame.OnInputEvent(self,e)
    self = MemoryGame.entity

    COutLine(kC_Debug,"OnInputEvent - call")

    if (not Input.IsTouchBegin(e)) then
        return false
    end

    COutLine(kC_Debug,"OnInputEvent - begin")

    local best
    local x = e.data[1]
    local y = e.data[2]
    for k,v in ipairs(self.widgets.cells) do
        COutLine(kC_Debug,"OnInputEvent - widget")
        local r = v:Rect()
        if (x >= r[1] and x <= (r[1] + r[3]) and y >= r[2] and y <= (r[2] + r[4])) then
            best = v
        end
    end

    if (best) then
        COutLine(kC_Debug,"OnInputEvent - found")

        e = UI:MapInputEvent(e)
        if (self.state.phase == kMGPhase_DiscoverPattern or self.state.phase == kMGPhase_FindPattern) then
            if (self.state.phaseGlyphsCounter < #self.widgets.goals) then
                COutLine(kC_Debug,"begin glph code")
                local glyphsCounter = self.state.phaseGlyphsCounter + 1
                local refImage = self.state.level.legend[glyphsCounter]
                if (self.state.phase == kMGPhase_DiscoverPattern) then
                    -- we just assume the input is correct since legend glyphs are created inside game right now
                    local w = self.widgets.goals[glyphsCounter]
                    w:SetMaterial(self.gfx[refImage])
                    self.state.phaseGlyphsCounter = glyphsCounter
                elseif (self.state.phase == kMGPhase_FindPattern) then
                    local handled = false
                    COutLine(kC_Debug,"identify click: %s",best.state.architype)
                    if (best.state.architype == refImage) then
                        COutLine(kC_Debug,"matched click: %s",best.state.architype)
                        self.state.phaseGlyphsCounter = glyphsCounter
                        UI:AckFinger(e.data)
                        handled = true
                    end

                    if (not handled) then
                        -- some kind of reset here (watch video send email)
                        UI:AckFinger(e.data)
                    end
                end
            end
        end
    end

    return (best ~= nil)
end

--function MemoryGame.OnInputGesture(self,g)
--	self = MemoryGame.entity

--  djr, disabling this for now (joe added this i think)
--	if (g.id ~= kIG_Line) then
--		return true
--	end
--
--	-- left?
--	if (g.args[1] > 0.707) then
--		self.state.heading.x = 1
--		self.state.heading.y = 0
--	elseif (g.args[1] < -0.707) then -- right?
--		self.state.heading.x = -1
--		self.state.heading.y = 0
--	elseif (g.args[2] > 0.707) then -- down
--		self.state.heading.x = 0
--		self.state.heading.y = 1
--	else -- up
--		self.state.heading.x = 0
--		self.state.heading.y = -1
--	end
--
--	return true
--end

function MemoryGame.ChangeLevel(self,level)
    self.state.level = level
    COutLine(kC_Debug, "memory.level.name=" .. self.state.level.name)

    self.REFLEX_BOARD_OFFSET = {self.screen[1] -  level.CELL_SCALE - 20, self.screen[2] - level.CELL_SCALE - 20 + 100}
    self.REFLEX_CELL_SIZE = {level.CELL_SCALE*UI.identityScale[1], level.CELL_SCALE*UI.identityScale[1] }
    self.INDEX_MAX_X = level.INDEX_MAX_X
    self.INDEX_MAX_Y = level.INDEX_MAX_Y
    self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + 0 * self.REFLEX_CELL_SIZE[1]
    self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + 0 * self.REFLEX_CELL_SIZE[2]
    self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE[1]
    self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE[2]

    -- reset the board for next draw
    for i,k in ipairs(self.widgets.cells) do
        self.widgets.root:RemoveChild(k)
    end
    self.widgets.grid = {}
    self.widgets.cells = {}
end

function MemoryGame.InitUI(self)
    -- constants
    self.SMALL_CELL_SCALE = 150
    self.SMALL_INDEX_MAX_X = 8
    self.SMALL_INDEX_MAX_Y = 4

    self.LARGE_CELL_SCALE = 90
    self.LARGE_INDEX_MAX_X = 12
    self.LARGE_INDEX_MAX_Y = 6

    self.PHASE0_LEGEND_REVEAL_TIMER = 1
    self.PHASE1_BOARD_SHUFFLE_TIMER = .125
    self.PHASE1_BOARD_SHUFFLE_MAX = 48

    self.NUM_GLPYHS_TO_FIND = 3

    self.NUM_GLYPHS = 27

    self:CreateBoards()
    -- define structure: self.state
    local level = self.db.levels[1][1] -- load appropriate level based on skill + difficulty
    self.state = {}
    self.state.level = level
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
    self.state.timeLeft = level.time
    self.state.phase = kMGPhase_AnimateOpening
    self.state.phaseTimer = self.PHASE0_LEGEND_REVEAL_TIMER
    self.state.phaseShuffles = self.PHASE1_BOARD_SHUFFLE_MAX
    self.state.phaseGlyphsCounter = 0

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
    self.widgets.grid = {}
    self.widgets.cells = {}

    self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=MemoryGame.OnInputEvent, OnInputGesture=ReflexGame.OnInputGesture})
    World.SetRootWidget(UI.kLayer_TerminalPuzzles, self.widgets.root)
    self.widgets.root:SetOpaqueLayerInput(true) -- no input goes past this

    self.widgets.root:SetVisible(false)

    self.widgets.border = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.border})
    self.widgets.board = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.board})
    --	UI:MoveWidgetByCenter(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
    --	UI:MoveWidgetByCenter(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
    self.widgets.root:AddChild(self.widgets.border)
    self.widgets.root:AddChild(self.widgets.board)

    self:ChangeLevel(level)

    COutLine(kC_Debug, "Creating Board")
    -- board step: board grid is x,y structure

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

function MemoryGame.LoadMaterials(self)
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

    self.gfx.blocker_green = World.Load("Puzzles/reflex-block1_M")

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

function MemoryGame.GetPositionByGrid(self,x,y)
    local v = { }
    v.x = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + x * self.REFLEX_CELL_SIZE[1]
    v.y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + y * self.REFLEX_CELL_SIZE[2]
    return v
end


function MemoryGame.SetPositionByGrid(self,w,x,y)
    local v = self:GetPositionByGrid(x,y)
    UI:MoveWidgetByCenter(w,v.x,v.y)
    --COutLine(kC_Debug,"position line @ x=%.02f,y=%.02f",xo,yo)
end

function MemoryGame.Vec2Normal(self,x,y)
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

function MemoryGame.LerpVec2(self,v,heading,dt,speed)	
	local dx = heading.x * dt * speed
	local dy = heading.y * dt * speed
	
	local x = v.x + dx
	local y = v.y + dy
		
	local vec2 = { }
	vec2.x = x
	vec2.y = y
	
	return vec2	
end

function MemoryGame.LerpWidget(self,widget,heading,dt,speed)	
	local r = widget:Rect()
	
	local width = r[3]
	local height = r[4]
	local half_width = width/2
	local half_height = height/2

	local wv = { }
	wv.x = r[1] + half_width
	wv.y = r[2] + half_height

	local o = self:LerpVec2(wv,heading,dt,speed)

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
	
	return o
end

function MemoryGame.GetGridCellFromVec2(self,v)
	local x = (v.x - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE
	local y = (v.y - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE	
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function MemoryGame.GetGridCell(self,widget)
	local pos = self:GetPosition(widget)

	local x = (pos.x - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE
	local y = (pos.y - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE	
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function MemoryGame.ConvertCoordToIndex(self,x,y)
	return bit.bor(bit.lshift(y, 16), x)
end

function MemoryGame.GetPosition(self,w)
	local r = w:Rect()
	local vec2 = { } 
	vec2.x = r[1] + r[3]/2
	vec2.y = r[2] + r[4]/2	
	return vec2
end


function MemoryGame.CreateState(self,architype,ref)
    local state = { }
    state.architype = architype
    state.ref = ref
    return state
end

function MemoryGame.UpdateHud(self)
    local minutes = math.floor(self.state.timeLeft / 60)
    local seconds = self.state.timeLeft - (minutes * 60)
    local text = string.format("%i:%02i",minutes,seconds)
    self.widgets.timeLeftLabel:SetText(text)
end

function MemoryGame.DrawBoard(self)
    local objectSize = { self.REFLEX_CELL_SIZE[1], self.REFLEX_CELL_SIZE[2] }
    for i,v in ipairs(self.state.level.board) do
        local b = UI:CreateWidget("MatWidget", {rect={0,0,objectSize[1],objectSize[2]}, material=self.gfx[v.img]})
        local index = self:ConvertCoordToIndex(v.x,v.y)
        b.state = self:CreateState(v.img,v)
        -- -djr animate cells in eventually
        --        b:BlendTo({1,1,1,0}, 0)
        self.widgets.root:AddChild(b)
        self.widgets.grid[index] = b
        table.insert(self.widgets.cells,b)
        self:SetPositionByGrid(b,v.x,v.y)
        --        if (string.find(b.state.architype,"cell_") ~= nil) then
        --            table.insert(self.widgets.goals,b)
        --        end
    end
end

function MemoryGame.DoPhase0(self,dt)
    self.state.phaseTimer = self.state.phaseTimer - dt
    if (self.state.phaseTimer > 0) then
        return
    end

--    COutLine(kC_Debug,"Phase0 - Legend/Goal Count: %i/%i",#self.state.level.legend,#self.widgets.goals)
    if (#self.widgets.goals >= #self.state.level.legend) then
        self:DrawBoard()
        self.state.phase = kMGPhase_ShuffleSmallTiles
        self.state.phaseTimer = self.PHASE1_BOARD_SHUFFLE_TIMER
        return
    end

    local v = self.state.level.legend[#self.widgets.goals+1]
    self.state.phaseTimer = self.PHASE0_LEGEND_REVEAL_TIMER
    local b = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.blocker_green})
    b.state = self:CreateState(v,nil)
    self.widgets.root:AddChild(b)
    UI:MoveWidgetByCenter(b,UI.screenWidth/2 - (#self.state.level.legend * self.REFLEX_CELL_SIZE[1])/2 + #self.widgets.goals * self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]/2)
    -- -djr animate cells in eventually
    --        b:BlendTo({1,1,1,0}, 0)
    table.insert(self.widgets.goals,b)
end

function MemoryGame.DoPhase1(self,dt)
    self.state.phaseTimer = self.state.phaseTimer - dt
    if (self.state.phaseTimer > 0) then
        return
    end

    self.state.phaseShuffles = self.state.phaseShuffles - 1
    if (self.state.phaseShuffles <= 0) then
        self.state.phase = kMGPhase_DiscoverPattern
        return
    end

    self.state.phaseTimer = self.PHASE1_BOARD_SHUFFLE_TIMER;

    self:ShuffleBoard(self.state.level)
end

function MemoryGame.DoPhase2(self,dt)
    if (self.state.phaseGlyphsCounter >= #self.widgets.goals) then
        self.state.phase = kMGPhase_AnimateTransition
        self.state.phaseGlyphsCounter = 0
        local level = self:CreateBoardLarge(self.state.level.legend)
        self:ChangeLevel(level)
        -- djr, animate this once it works (see video)
        self:DrawBoard()
    end
end

function MemoryGame.DoPhase3(self,dt)
    -- make it a fancy animation
    self.state.phase = kMGPhase_FindPattern
end

function MemoryGame.DoPhase4(self,dt)
    if (self.state.phaseGlyphsCounter >= #self.widgets.goals) then
        self.state.phase = kMGPhase_GameOver
        self.state.gameOver = true
        self.state.victory = true
    end
end

function MemoryGame.Think(self,dt)
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
		return
    end

    if (self.state.phase == kMGPhase_AnimateOpening) then
        self:DoPhase0(dt)
    elseif (self.state.phase == kMGPhase_ShuffleSmallTiles) then
        self:DoPhase1(dt)
    elseif (self.state.phase == kMGPhase_DiscoverPattern) then
        self:DoPhase2(dt)
    elseif (self.state.phase == kMGPhase_AnimateTransition) then
        self:DoPhase3(dt)
    elseif (self.state.phase == kMGPhase_FindPattern) then
        self:DoPhase4(dt)
    end

    self:UpdateHud()

    if (self.state.phase > kMGPhase_AnimateOpening) then
        self.state.timeLeft = self.state.timeLeft - dt
        if (self.state.timeLeft <= 0) then
            self.state.timeLeft = 0
            self.state.phase = kMGPhase_GameOver
            self.state.gameOver = true
            self.state.victory = false
        end
    end
end

memory_game = MemoryGame
