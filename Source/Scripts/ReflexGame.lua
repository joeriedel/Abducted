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
	
end

function ReflexGame.PostSpawn(self)

	if (Game.entity.type == "Map") then
		self:LoadMaterials()
		self:InitUI()
		-- Note: InitUI sets widgets.root to be invisible (hides the whole board)
	end
	
end

function ReflexGame.ShowBoard(self, show)
	self = ReflexGame.entity
	-- NOTE: show board is called *after* InitGame
	-- InitGame should get the board ready to be seen
	self.widgets.root:SetVisible(show)
	self.widgets.root2:SetVisible(show)
	self.widgets.root3:SetVisible(show)
	World.SetDrawUIOnly(show) -- < disable/enable 3D rendering
end

function ReflexGame.InitGame(self, playerSkill, terminalSkill)
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
	
	if (UI.mode == kGameUIMode_Mobile) then
		World.SetEnabledGestures(kIG_Line)
	end
	
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

function ReflexGame.LoadLevels(self)
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
	return self.eatInput
end

function ReflexGame.HandleDPadEvents(self, widget, e)
	local press = false
	
	if (Input.IsTouchBegin(e)) then
		if (widget.touch == nil) then
			press = true
			widget:SetMaterial(self.gfx.RightArrowPressed)
			widget:SetCapture(true)
			widget.touch = e.touch
			self.sfx.dpad:Play(kSoundChannel_UI, 0)
		end
	elseif (widget.touch and Input.IsTouchEnd(e, widget.touch)) then
		widget.touch = nil
		widget:SetMaterial(self.gfx.RightArrow)
		widget:SetCapture(false)
	end
	
	return press
end

function ReflexGame.DPadUp(widget, e)
	self = ReflexGame.entity
	if (self:HandleDPadEvents(widget, e)) then
		self.state.heading.x = 0
		self.state.heading.y = -1
	end
	return true
end

function ReflexGame.DPadDown(widget, e)
	self = ReflexGame.entity
	if (self:HandleDPadEvents(widget, e)) then
		self.state.heading.x = 0
		self.state.heading.y = 1
	end
	return true
end

function ReflexGame.DPadLeft(widget, e)
	self = ReflexGame.entity
	if (self:HandleDPadEvents(widget, e)) then
		self.state.heading.x = -1
		self.state.heading.y = 0
	end
	return true
end

function ReflexGame.DPadRight(widget, e)
	self = ReflexGame.entity
	if (self:HandleDPadEvents(widget, e)) then
		self.state.heading.x = 1
		self.state.heading.y = 0
	end
	return true
end

function ReflexGame.LoadMaterials(self)
	
	self.gfx = {}
	self.gfx.blackhole = World.Load("Puzzles/reflex-blackhole1_M");
	self.gfx.antivirus_spider = World.Load("Puzzles/AlienICE_M")
    self.gfx.blue_glow = World.Load("Puzzles/reflex-blueglow1_M")
    self.gfx.board = World.Load("Puzzles/reflex-checkerboard1_M")
	self.gfx.border = World.Load("UI/arm_screen1_M")

    self.gfx.mark_current = World.Load("Puzzles/reflex-player1_M")
    self.gfx.mark_line_v = World.Load("Puzzles/trail_M")
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
    
    self.gfx.RightArrow = World.Load("UI/right_arrow_M")
    self.gfx.RightArrowPressed = World.Load("UI/right_arrow_pressed_M")
	self.gfx.PlayerParticles = World.Load("Puzzles/playerparticles_M")
	self.gfx.GoalParticles = World.Load("Puzzles/goalparticles_M")
	self.gfx.GoalOpen = World.Load("Puzzles/reflex-goal_open_M")
	
    self.gfx.mark_start = nil
	self.typefaces = {}
    self.typefaces.TimerText = World.Load("UI/TerminalPuzzlesBigFont_TF")
    self.typefaces.SwipeToMoveText = World.Load("UI/TerminalPuzzlesBigFont_TF")
    
    self.sfx = {}
    self.sfx.dpad = World.Load("Audio/dpad")

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
		(720 * 0.07430 * yScale) + (wideInset-inset)*yScale,
		0,
		0
	}
	
	self.screen[3] = UI.screenWidth - (self.screen[1]*2)
	self.screen[4] = UI.screenHeight - (self.screen[2]*2)
end


function ReflexGame.InitUI(self)
	-- constants
	self.REFLEX_CELL_SIZE = {67*UI.identityScale[1], 67*UI.identityScale[1] }
    self.BLACKHOLE_SIZE = {self.REFLEX_CELL_SIZE[1]*1.5, self.REFLEX_CELL_SIZE[2]*1.5}
    self.BLACKHOLE_WIDGET_SIZE = {self.REFLEX_CELL_SIZE[1]*3, self.REFLEX_CELL_SIZE[2]*3}
    self.SPIDER_SIZE = {self.REFLEX_CELL_SIZE[1]*0.9, self.REFLEX_CELL_SIZE[2]*0.9}
    self.SPIDER_WIDGET_SIZE = {self.REFLEX_CELL_SIZE[1]*2, self.REFLEX_CELL_SIZE[2]*2}
	self.REFLEX_BOARD_OFFSET = {self.screen[1], self.screen[2]}
	self.INDEX_MAX_X = 16
	self.INDEX_MAX_Y = 8
    self.LINE_SPAWN_TIME = 1
	self.PLAYER_SPEED = 100
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET[1]
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET[2]
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1] + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE[1]
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2] + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE[2]	

	self:LoadLevels()	
		
	-- define structure self.widgets
	self.widgets = {}
	
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=ReflexGame.OnInputEvent})
	World.SetRootWidget(UI.kLayer_TerminalPuzzles, self.widgets.root)
	self.widgets.root:SetOpaqueLayerInput(true) -- no input goes past this
	
	self.widgets.root:SetVisible(false)
	
	self.widgets.root2 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=ReflexGame.OnInputEvent})
	World.SetRootWidget(UI.kLayer_TerminalPuzzles2, self.widgets.root2)
	
	self.widgets.root2:SetVisible(false)
	
	self.widgets.root3 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=ReflexGame.OnInputEvent})
	World.SetRootWidget(UI.kLayer_TerminalPuzzles3, self.widgets.root3)
	
	self.widgets.root3:SetVisible(false)

	self.widgets.border = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.board})
	self.widgets.root:AddChild(self.widgets.border)
	self.widgets.root:AddChild(self.widgets.board)
	
	self:CreateBoard()
end

function ReflexGame.DeInitUI(self)
	if (self.widgets) then
		if (self.widgets.goals) then
			for k,v in pairs(self.widgets.goals) do
				self.widgets.root2:RemoveChild(v.particles)
				v.particles:Unmap()
				v.particles = nil
			end
			self.widgets.goals = nil
		end
		
		if (self.widgets.grid) then
			for k,v in pairs(self.widgets.grid) do
				self.widgets.root:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.grid = nil
		end
		if (self.widgets.lines) then
			for k,v in pairs(self.widgets.lines) do
				self.widgets.root:RemoveChild(v)
				v:Unmap()
			end
		end
		if (self.widgets.blackholes) then
			for k,v in pairs(self.widgets.blackholes) do
				self.widgets.root2:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.blackholes = nil
		end
		if (self.widgets.spiders) then
			for k,v in pairs(self.widgets.spiders) do
				self.widgets.root2:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.spiders = nil
		end
		if (self.widgets.player) then
			self.widgets.root2:RemoveChild(self.widgets.player)
			self.widgets.player:Unmap()
			self.widgets.player = nil
		end
		
		if (self.widgets.playerParticles) then
			self.widgets.root2:RemoveChild(self.widgets.playerParticles)
			self.widgets.playerParticles:Unmap()
			self.widgets.playerParticles = nil
		end
		
		self.widgets.portal = nil
		
		collectgarbage()
	end
end

function ReflexGame.CreateBoard(self)
	self:DeInitUI()
	
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
	self.state.antivirusSpawnTimer = FloatRand(level.antivirusSpiderSpawnRate[1], level.antivirusSpiderSpawnRate[2])
    self.state.lineTimerEnabledTimer = level.blockChaseTime
    self.state.lineTimer = 0
	self.state.goalCounter = 0
    self.state.lineIndex = 1
    self.state.fadeInBoardTimer = 2
    self.state.swipeToMoveTimer = 1
	
	self.widgets.goals = { }
	self.widgets.board = { }		
	self.widgets.lines = { }
	self.widgets.spiders = { }
    self.widgets.blackholes = { }
	self.widgets.grid = {}
	self.widgets.cells = {}
	self.widgets.portal = nil
	
	COutLine(kC_Debug, "reflex.level.name=" .. self.state.level.name)

	COutLine(kC_Debug, "Creating Board")	
	-- board step: board grid is x,y structure
	for i,v in ipairs(self.state.level.board) do

        local objectTable = self.widgets.cells
        local objectSize = { self.REFLEX_CELL_SIZE[1], self.REFLEX_CELL_SIZE[2] }
		local layer = self.widgets.root
		
        if (self.gfx[v.img] == self.gfx.blackhole) then
            objectTable = self.widgets.blackholes
            objectSize = { self.BLACKHOLE_WIDGET_SIZE[1], self.BLACKHOLE_WIDGET_SIZE[2] }
			layer = self.widgets.root2
        end

		local b = UI:CreateWidget("MatWidget", {rect={0,0,objectSize[1],objectSize[2]}, material=self.gfx[v.img]})
		local index = self:ConvertCoordToIndex(v.x,v.y)
		b.state = self:CreateState(v.img,v)
        b:BlendTo({1,1,1,0}, 0)
		layer:AddChild(b)
        if (objectTable == self.widgets.cells) then
		    self.widgets.grid[index] = b
        end
        table.insert(objectTable,b)
		self:SetPositionByGrid(b,v.x,v.y,objectTable==self.widgets.blackholes)
		if (v.img == "mark_start") then
            local player = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.mark_current})
            player.state = self:CreateState("player",v)
            self.widgets.player = player
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
        if (v.img == "mark_end") then
			self.widgets.portal = b
        end
        if (string.find(b.state.architype,"cell_") ~= nil) then
            table.insert(self.widgets.goals,b)
		end
    end
    
    if (self.widgets.player) then
		self.widgets.root2:AddChild(self.widgets.player) -- on top of blackholes
    end
    
    self.widgets.playerParticles = UI:CreateWidget("MatWidget", {rect={0,0,310*UI.identityScale[1],310*UI.identityScale[1]}, material=self.gfx.PlayerParticles})
	self.widgets.root2:AddChild(self.widgets.playerParticles)
	self.widgets.playerParticles:SetVisible(false)
	
	for k,v in pairs(self.widgets.goals) do
		local w = UI:CreateWidget("MatWidget", {rect={0,0,310*UI.identityScale[1],310*UI.identityScale[1]}, material=self.gfx.GoalParticles})
		w:SetVisible(false)
		v.particles = w
	end
    
    if (UI.mode == kGameUIMode_Mobile) then
		self.widgets.dpad = {}
		
		local dpadButtonSize = 64*UI.identityScale[1]
		local dpadSize = dpadButtonSize*3
		local dpadPad = 16*UI.identityScale[1]
		local dpadLeft = UI.screenWidth-dpadSize-(48*UI.identityScale[1])
		local dpadTop = UI.screenHeight-dpadSize-(72*UI.identityScale[1])
		
		self.widgets.dpad.left = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft-dpadPad, dpadTop+dpadButtonSize, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow, OnInputEvent=ReflexGame.DPadLeft}
		)
		
		self.widgets.dpad.left:RotateTo({dpadButtonSize/2, dpadButtonSize/2, 180}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.left)
		
		self.widgets.dpad.right = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadSize-dpadButtonSize+dpadPad, dpadTop+dpadButtonSize, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow, OnInputEvent=ReflexGame.DPadRight}
		)
				
		self.widgets.root3:AddChild(self.widgets.dpad.right)
		
		self.widgets.dpad.up = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadButtonSize, dpadTop-dpadPad, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow, OnInputEvent=ReflexGame.DPadUp}
		)
		
		self.widgets.dpad.up:RotateTo({dpadButtonSize/2, dpadButtonSize/2, -90}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.up)
		
		self.widgets.dpad.down = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadButtonSize, dpadTop+dpadSize-dpadButtonSize+dpadPad, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow, OnInputEvent=ReflexGame.DPadDown}
		)
		
		self.widgets.dpad.down:RotateTo({dpadButtonSize/2, dpadButtonSize/2, 90}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.down)
		
	end
	
    self.widgets.timeLeftLabel = UI:CreateWidget("TextLabel", {rect={80*UI.identityScale[1], 35*UI.identityScale[2], 8, 8}, typeface=self.typefaces.TimerText})
    self.widgets.root3:AddChild(self.widgets.timeLeftLabel)

    self.widgets.swipeToMoveLabel = UI:CreateWidget("TextLabel", {rect={ 0, 0, 8, 8}, typeface=self.typefaces.SwipeToMoveText})
	UI:SetLabelText(self.widgets.swipeToMoveLabel, StringTable.Get("TAP_DPAD_TO_MOVE"))
    UI:SizeLabelToContents(self.widgets.swipeToMoveLabel)
    UI:CenterLabel(self.widgets.swipeToMoveLabel, UI.fullscreenRect)
    self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, 0)
    self.widgets.root3:AddChild(self.widgets.swipeToMoveLabel)

    self:UpdateHud()

    if (self.widgets.current == nil) then
		COutLine(kC_Debug,"mark_start NOT FOUND --> ERROR")	
	end
	
	COutLine(kC_Debug, "Board Completed")		
end

function ReflexGame.GetPositionByGrid(self,x,y)
    local v = { }
    v.x = self.REFLEX_BOARD_OFFSET[1] + ((self.REFLEX_CELL_SIZE[1]-1)/2) + x * self.REFLEX_CELL_SIZE[1]
    v.y = self.REFLEX_BOARD_OFFSET[2] + ((self.REFLEX_CELL_SIZE[2]-1)/2) + y * self.REFLEX_CELL_SIZE[2]
    return v
end


function ReflexGame.SetPositionByGrid(self,w,x,y,shift)
    local v = self:GetPositionByGrid(x,y)
    if (shift) then
		v.x = v.x + ((self.REFLEX_CELL_SIZE[1]-1)/2)
		v.y = v.y + ((self.REFLEX_CELL_SIZE[1]-1)/2)
	end
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
	local x = (v.x-self.REFLEX_BOARD_OFFSET[1])/self.REFLEX_CELL_SIZE[1]
	local y = (v.y-self.REFLEX_BOARD_OFFSET[2])/self.REFLEX_CELL_SIZE[2]
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function ReflexGame.GetGridCell(self,widget)
	local pos = self:GetPosition(widget)
	return self:GetGridCellFromVec2(pos)
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

function ReflexGame.ConstrainPointToBoard(self,x,y,w,h)
	w = w/2
	h = h/2
	
	if ((x-w) < self.COORD_MIN_X) then
		x = self.COORD_MIN_X+w
	end
	if ((x+w) >= self.COORD_MAX_X) then
		x = self.COORD_MAX_X-w-1
	end
	if ((y-h) < self.COORD_MIN_Y) then
		y = self.COORD_MIN_Y+h
	end		
	if ((y+h) >= self.COORD_MAX_Y) then
		y = self.COORD_MAX_Y-h-1
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
		y = y-(height/2)
		if (startPos.x < endPos.x) then
			width = width + height/2
		else
			x = x-height/2
		end
	else
		x = x-(width/2)
		if (startPos.y < endPos.y) then
			height = height + width/2
		else
			y = y-width/2
		end
	end
	
	local r = { }
	r[1] = x
	r[2] = y
	r[3] = width
	r[4] = height
	
	line:SetRect(r)
end

function ReflexGame.CollideWithSymbol(self,x,y)
    local v2 = {}
    v2.x = x
    v2.y = y
    local cell = self:GetGridCellFromVec2(v2)
    local index = self:ConvertCoordToIndex(cell.x,cell.y)

    local piece = self.widgets.grid[index]
    if (piece ~= nil) then
        if (string.find(piece.state.architype,"cell_") ~= nil) then
            -- -djr do effect
            self:ExplodeGoal(piece)
            self.widgets.grid[index] = nil
            self.widgets.root:RemoveChild(piece)
            self.state.goalCounter = self.state.goalCounter + 1
            if (self.state.goalCounter == #self.widgets.goals) then
				self.widgets.portal:SetMaterial(self.gfx.GoalOpen)
            end
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

function ReflexGame.RectCellBounds(self,x,y,w,h)

	w = w/2
	h = h/2
	
	local min = self:GetGridCellFromVec2({x=x-w,y=y-h})
	local max = self:GetGridCellFromVec2({x=x+w,y=y+h})
	
	return min,max

end

function ReflexGame.ClipToBoard(self,x,y,w,h)

	w = w/2
	h = h/2

	if ((x-w) < self.COORD_MIN_X) then
		return self:ConstrainPointToBoard(x,y,w,h), 1
	end
	if ((x+w) >= self.COORD_MAX_X) then
		return self:ConstrainPointToBoard(x,y,w,h), 2
	end
	if ((y-h) < self.COORD_MIN_Y) then
		return self:ConstrainPointToBoard(x,y,w,h), 3
	end		
	if ((y+h) >= self.COORD_MAX_Y) then
		return self:ConstrainPointToBoard(x,y,w,h), 4
	end	
	
	return nil
end

function ReflexGame.CollidePlayerWithBoard(self,x,y)

	if (x < self.COORD_MIN_X) then
		COutLine(kC_Debug,"CollidePlayerWithBoard found min X @ x=%i, y=%i",x,y)		
		return true
	end
	if (x >= self.COORD_MAX_X) then
		COutLine(kC_Debug,"CollidePlayerWithBoard found max X @ x=%i, y=%i",x,y)			
		return true
	end
	if (y < self.COORD_MIN_Y) then
		COutLine(kC_Debug,"CollidePlayerWithBoard found min Y @ x=%i, y=%i",x,y)			
		return true
	end		
	if (y >= self.COORD_MAX_Y) then
		COutLine(kC_Debug,"CollidePlayerWithBoard found max Y @ x=%i, y=%i",x,y)			
		return true
	end	
	
	local screenCoord = { x = x, y = y }
	local v = self:GetGridCellFromVec2(screenCoord)
			
	local index = self:ConvertCoordToIndex(v.x,v.y)
			
	local piece = self.widgets.grid[index]		
	if (piece ~= nil) then
		if (piece.state.architype == "mark_end" or piece.state.architype == "mark_start") then
			return false
		end

		if (string.find(piece.state.architype,"cell_") ~= nil) then
			return false
		end
				
		COutLine(kC_Debug,"CollidePlayerWithBoard found Piece @ x=%i, y=%i, type=%s",x,y,piece.state.architype)			
		return true
	end
	
	return false
end

function ReflexGame.SuckupPieces(self,x,y,w,h)

end

function ReflexGame.SuckupPlayer(self,x,y)
	local tr = self.widgets.player:Rect()
	tr[1] = tr[1] + x - (tr[1]+(tr[3]/2))
	tr[2] = tr[2] + y - (tr[2]+(tr[4]/2))
	self.widgets.player:MoveTo(tr, {0.7,0.7})
	self.widgets.player:ScaleTo({0,0}, {0.7,0.7})
	self.widgets.player:BlendTo({1,1,1,0}, 0.7)
	--self.widgets.player:RotateTo({self.REFLEX_CELL_SIZE[1]/8, self.REFLEX_CELL_SIZE[2]/4, 360*5}, {0,0,4})
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

	if (self.state.heading.x == 0 and self.state.heading.y == 0) then
		-- NO HEADING: Game hasn't started		
		return
    end

    if (not self.state.labelFadeOut) then
		self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, .5)
        self.state.labelFadeOut = true
    end

	local firstMove = false
	if (self.state.lastHeading.x == 0 and self.state.lastHeading.y == 0) then
		self.state.lastHeading.x = self.state.heading.x
		self.state.lastHeading.y = self.state.heading.y
		firstMove = true
	end
	
	if (self.widgets.current.heading == nil) then
		self.widgets.current.heading = self.state.heading
    end

	local currentPos = self:LerpVec2(self.widgets.current.state.endPos,self.state.lastHeading,dt,self.PLAYER_SPEED)
    self:CollideWithSymbol(currentPos.x,currentPos.y)

	if (self:CollidePlayerWithBoard(currentPos.x,currentPos.y)) then
		COutLine(kC_Debug,"GameOver player collided with board @ : x=%i, y=%i",currentPos.x,currentPos.y)			
		self.state.gameOver = true
		self:ExplodePlayer()
		return
    end

	--COutLine(kC_Debug,"currentPos: x=%.02f, y=%.02f",currentPos.x,currentPos.y)
	currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y,0,0)
	self.widgets.current.state.endPos = currentPos
	self:SetLineSegmentPosition(self.widgets.current,self.widgets.current.state.startPos,self.widgets.current.state.endPos)
    UI:MoveWidgetByCenter(self.widgets.player,currentPos.x,currentPos.y)
    local nextCell = self:GetGridCellFromVec2(currentPos)
    if ((nextCell.x ~= self.widgets.current.state.lastCell.x) or (nextCell.y ~= self.widgets.current.state.lastCell.y) or firstMove) then
        self.widgets.current.state.lastCell = nextCell
        local index = self:ConvertCoordToIndex(nextCell.x,nextCell.y)
        if (self.state.pathByCell[index] == nil) then
            self.state.pathByCell[index] = nextCell
            self.state.path[#self.state.path+1] = nextCell
        end
    end

	if (self.state.lastHeading.x ~= self.state.heading.x or self.state.lastHeading.y ~= self.state.heading.y) then
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
	elseif (firstMove) then
		local angle = 0
		if (self.state.heading.x < 0) then
			angle = 180
		elseif (self.state.heading.y < 0) then
			angle = 270
		elseif (self.state.heading.y > 0) then
			angle = 90
		end
		self.widgets.player:RotateTo({self.REFLEX_CELL_SIZE[1]/2, self.REFLEX_CELL_SIZE[2]/2, angle}, {0, 0, .05}, true)
    end

    self.state.lineTimerEnabledTimer = self.state.lineTimerEnabledTimer - dt
    if (self.state.lineTimerEnabledTimer <= 0) then
        self.state.lineTimer = self.state.lineTimer + dt
        if (self.state.lineTimer >= self.LINE_SPAWN_TIME) then
            self.state.lineTimer = 0

            local v = self.state.path[self.state.lineIndex]
            local index = self:ConvertCoordToIndex(v.x,v.y)
            local b = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.blocker_green})
            b.state = self:CreateState("blocker_green")
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
		if (pieceAtPlayer.state.architype == "mark_end" and self.state.goalCounter >= #self.widgets.goals) then
			COutLine(kC_Debug,"Game Over Detected")
			self.state.gameOver = true
            self.state.victory = true
            local pos = self:GetPosition(pieceAtPlayer)
            self:SuckupPlayer(pos.x, pos.y)
			return
		end

        if (self.state.timeLeft <= 0) then
            COutLine(kC_Debug,"Game Over Detected (time ran out)")
            self.state.gameOver = true
            self.state.victory = false
            return
        end
    end
    
    for i,k in pairs(self.widgets.spiders) do
		self:SpiderThink(i, k, dt)
	end	
	
	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
	if (self.state.antivirusSpawnTimer < 0) then
		self.state.antivirusSpawnTimer = FloatRand(self.state.level.antivirusSpiderSpawnRate[1], self.state.level.antivirusSpiderSpawnRate[2])		
		local x = math.random(self.INDEX_MAX_X)-1
		local y = math.random(self.INDEX_MAX_Y)-1		
		local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.SPIDER_WIDGET_SIZE[1],self.SPIDER_WIDGET_SIZE[2]}, material=self.gfx.antivirus_spider})
		self.widgets.root2:AddChild(spider)	
		table.insert(self.widgets.spiders,spider)
		self:SetPositionByGrid(spider,x,y)		
		spider.state = self:CreateState("antivirus_spider")
		spider.state.lifetime = FloatRand(self.state.level.antivirusSpiderLifetime[1], self.state.level.antivirusSpiderLifetime[2])
		self:SpiderPickHeading(spider)
		COutLine(kC_Debug,"spawnedSpider @ grid: x=%i, y=%i, heading = %.04f,%.04f",x,y,spider.state.heading.x,spider.state.heading.y)
    end

    for i,k in pairs(self.widgets.blackholes) do
        if (k.state.heading == null) then
            k.state.heading = {}
            k.state.heading.x = k.state.ref.heading[1]
            k.state.heading.y = k.state.ref.heading[2]
            k.state.headingTime = FloatRand(1.7, 10)
            k.state.speed = FloatRand(self.state.level.blackholeSpeed[1], self.state.level.blackholeSpeed[2])
            COutLine(kC_Debug,"blackhole heading @ : x=%i, y=%i",k.state.heading.x,k.state.heading.y)
		else
			k.state.headingTime = k.state.headingTime - dt
        end

        local pos = k:Rect()
        local nextPos = self:LerpWidget(k,k.state.heading,dt,k.state.speed,false)
        if ((k.state.headingTime < 0) or (self:ClipToBoard(nextPos.x,nextPos.y,self.BLACKHOLE_SIZE[1],self.BLACKHOLE_SIZE[2]))) then
            k.state.heading.x = -k.state.heading.x
            k.state.heading.y = -k.state.heading.y
            k.state.headingTime = FloatRand(1.7, 10)
            nextPos = self:LerpWidget(k,k.state.heading,2*dt,k.state.speed,false)
            COutLine(kC_Debug,"blackhole bounce @ : x=%i, y=%i",nextPos.x,nextPos.y)
        end
        UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
        self:SuckupPieces(nextPos.x,nextPos.y,self.BLACKHOLE_SIZE[1],self.BLACKHOLE_SIZE[2])
        if (not self.state.gameOver) then
			if (self:CheckTouchPlayer(nextPos.x,nextPos.y,self.BLACKHOLE_SIZE[1],self.BLACKHOLE_SIZE[2])) then
				self:SuckupPlayer(nextPos.x, nextPos.y)
				self.state.gameOver = true
				COutLine(kC_Debug, "Player eaten by blackhole.")
			end
		end
    end
end

function ReflexGame.SpiderThink(self, index, spider, dt)
	if (spider.state.lifetime < dt) then
		table.remove(self.widgets.spiders,index)
		self.widgets.root2:RemoveChild(spider)
		return
	else
		spider.state.lifetime = spider.state.lifetime - dt
	end
	
	local pos = self:GetPosition(spider)
	local gridRange = self.REFLEX_CELL_SIZE[1] * self.state.level.antivirusSpiderSeekPlayerRange
	local seekPlayer = self:CheckTouchPlayer(pos.x, pos.y, gridRange, gridRange)
	
	if (seekPlayer) then
	
		--COutLine(kC_Debug, "Spider seeking player")
		-- seek the player
		spider.state.headingTime = 0
		spider.state.heading = self:GetHeadingTowardsPlayer(pos.x,pos.y)
	
	else
	
		if (spider.state.headingTime < dt) then
			self:SpiderPickHeading(spider)
		else
			spider.state.headingTime = spider.state.headingTime - dt
		end
	
	end
	
	local pos = spider:Rect()
	local nextPos = self:LerpWidget(spider,spider.state.heading,dt,spider.state.speed,false)
	local clip, edge = self:ClipToBoard(nextPos.x,nextPos.y,self.SPIDER_SIZE[1],self.SPIDER_SIZE[2])
	
	if (clip) then
		nextPos = clip
		
		if (edge == 1 or edge == 2) then
			spider.state.heading.x = -spider.state.heading.x
		elseif (edge == 3 or edge == 4) then
			spider.state.heading.y = -spider.state.heading.y
		end
	end	
	
	UI:MoveWidgetByCenter(spider,nextPos.x,nextPos.y)
	
	if (not self.state.gameOver) then
		if (self:CheckTouchPlayer(nextPos.x, nextPos.y, self.SPIDER_SIZE[1],self.SPIDER_SIZE[2])) then
			COutLine(kC_Debug, "Player eaten by spider.")
			self.state.gameOver = true
			self:ExplodePlayer()
		end
	end
end

function ReflexGame.SpiderPickHeading(self, spider)
	spider.state.heading = self:Vec2Normal(math.random() * 2 - 1,math.random() * 2 - 1)
	spider.state.speed = FloatRand(self.state.level.antivirusSpiderSpeed[1], self.state.level.antivirusSpiderSpeed[2])
	spider.state.headingTime = FloatRand(self.state.level.antivirusSpiderHeadingTime[1], self.state.level.antivirusSpiderHeadingTime[2])
	if (spider.state.heading.x == 0 and spider.state.heading.y == 0) then -- failsafe
		spider.state.heading.x = 1
	end
end

function ReflexGame.ExplodePlayer(self)

	self.widgets.player:SetVisible(false)
	UI:MoveWidgetByCenter(self.widgets.playerParticles,self.widgets.current.state.endPos.x,self.widgets.current.state.endPos.y)
	self.widgets.root2:AddChild(self.widgets.playerParticles)
	self.widgets.playerParticles:SetVisible(true)
	self.widgets.playerParticles:ScaleTo({0,0}, {0,0})
	self.widgets.playerParticles:ScaleTo({1,1}, {1,0.7})
	
	local f = function()
		self.widgets.playerParticles:BlendTo({1,1,1,0}, 0.3)
	end
	
	World.globalTimers:Add(f, 0.4)
end

function ReflexGame.ExplodeGoal(self, widget)
	local r = widget:Rect()
	r[1] = r[1] + (r[3]/2)
	r[2] = r[2] + (r[4]/2)
	UI:MoveWidgetByCenter(widget.particles, r[1], r[2])
	widget.particles:SetVisible(true)
	self.widgets.root2:AddChild(widget.particles)
	
	widget.particles:ScaleTo({0,0}, {0,0})
	widget.particles:ScaleTo({1,1}, {1,0.7})
	
	local f = function()
		widget.particles:BlendTo({1,1,1,0}, 0.3)
	end
	
	World.globalTimers:Add(f, 0.4)
end

function ReflexGame.CheckTouchPlayer(self,x,y,w,h)
	local touchRect = {x-(w/2), y-(h/2),x+(w/2), y+(h/2)}
			   
	local playerRect = {self.widgets.current.state.endPos.x-(self.REFLEX_CELL_SIZE[1]/2), 
						self.widgets.current.state.endPos.y-(self.REFLEX_CELL_SIZE[2]/2),
						self.widgets.current.state.endPos.x+(self.REFLEX_CELL_SIZE[1]/2), 
						self.widgets.current.state.endPos.y+(self.REFLEX_CELL_SIZE[2]/2)}
						
	local disjointX = (touchRect[3] <= playerRect[1]) or (playerRect[3] <= touchRect[1])
	local disjointY = (touchRect[4] <= playerRect[2]) or (playerRect[4] <= touchRect[2])
	
	return not (disjointX or disjointY)
end

function ReflexGame.GetHeadingTowardsPlayer(self,x,y)
	local dx = self.widgets.current.state.endPos.x - x
	local dy = self.widgets.current.state.endPos.y - y
	return self:Vec2Normal(dx,dy)
end

reflex_game = ReflexGame
