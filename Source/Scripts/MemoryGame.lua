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

MemoryGame.kLevelTime = 60

function MemoryGame.DebugStart(self)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	local f = function()
		local f = function()
			UI:BlendTo({1,1,1,1}, 0.3)
			local f = function()
				UI:BlendTo({0,0,0,0}, 0.3)
				MemoryGame:ShowBoard(false)
				MemoryGame:ResetGame()
				collectgarbage()
				Abducted.entity.eatInput = false
			end
			World.globalTimers:Add(f, 0.3)
		end
		MemoryGame:InitGame()
		UI:BlendTo({0,0,0,0}, 0.3)
		MemoryGame:ShowBoard(true)
		MemoryGame:StartGame({1,2,3}, "discover Bugs;award 6;message ARM_REWARD_MSG_POWER_RESTORED;unlock_topic LockedTest ARM_REWARD_LOCKEDTEST", f)
	end
	World.globalTimers:Add(f, 0.3)
end

function MemoryGame.Spawn(self)
	MemoryGame.entity = self

end

function MemoryGame.PostSpawn(self)

	if (Game.entity.type == "Map") then
		self:LoadMaterials()
		self:InitUI()
		-- Note: InitUI sets widgets.root to be invisbile (hides the whole board)
	end
	
end

function MemoryGame.ShowBoard(self, show)
	self = MemoryGame.entity
	-- NOTE: show board is called *after* InitGame
	-- InitGame should get the board ready to be seen
	self.widgets.root:SetVisible(show)
	self.widgets.root2:SetVisible(show)
	self.widgets.root3:SetVisible(show)
	
	if (show) then
		self.widgets.wavy:BlendTo({1,1,1,0}, 0)
		self.widgets.timeLeftLabel:BlendTo({1,1,1,0}, 0)
		self.widgets.highlight:SetVisible(false)
		self.widgets.button:BlendTo({1,1,1,0}, 0)
	end
	
	World.DrawUIOnly(show) -- < disable/enable 3D rendering
end

function MemoryGame.InitGame(self)
	self = MemoryGame.entity
	self.exiting = false
	-- InitGame: prep the board to be shown with ShowBoard
	-- but we should not start until StartGame is called.
end

function MemoryGame.StartGame(self, glyphs, actions, gameCompleteCallback)
	self = MemoryGame.entity
	
	MemoryGame.active = true
	self.gameCompleteCallback = gameCompleteCallback
	self.actions = actions
	self.goals = glyphs
	
	self:RestartGameState()
	self:Run()
	
end

function MemoryGame.RestartGameState(self)

	PuzzleScoreScreen:Unlink()
	
	self.shuffleGlyphs = nil
	
	self:CleanWidgets()
	self.widgets.goals = {}
	
	self.validGlyphs = {}
	
	for i=1,#self.glyphNums do
		local valid = true
		for k=1,#self.goals do
			if (self.goals[k] == self.glyphNums[i]) then
				valid = false
				break
			end
		end
		
		if (valid) then
			table.insert(self.validGlyphs, self.glyphNums[i])
		end
	end
	
	local invalidGlyph = self.goals[IntRand(1, #self.goals)]
	self.validGlyphs2 = {}
	
	for i=1,#self.glyphNums do
		if (self.glyphNums[i] ~= invalidGlyph) then
			table.insert(self.validGlyphs2, self.glyphNums[i])
		end
	end
		
	-- define structure: self.state
    self.state = {}    
    self.state.gameOver = false
    self.state.gameOverTimer = 2
    self.state.victory = false
    self.state.timeLeft = MemoryGame.kLevelTime
    self.state.phase = kMGPhase_AnimateOpening
    self.state.phaseTimer = 2
    self.state.phaseShuffles = self.PHASE1_BOARD_SHUFFLE_MAX
    self.state.phaseGlyphsCounter = 0
    
    self:ChangeLevel(self:CreateBoardSmall())
    
    self:UpdateHud()
    
    self.widgets.timeLeftLabel:BlendTo({1,1,1,0}, 0)
    self.widgets.wavy:BlendTo({1,1,1,0}, 0)
    self.widgets.highlight:SetVisible(false)
    self.widgets.button:BlendTo({1,1,1,0}, 0)
    
    local highlightWidth = self.GOAL_SIZE * #self.goals
    self.widgets.highlight:SetRect({
		(UI.screenWidth - highlightWidth)/2,
		self.screen[2],
		highlightWidth,
		self.GOAL_SIZE
	})
	
end

function MemoryGame.Run(self)
    
    local f = function()
		self.widgets.timeLeftLabel:BlendTo({1,1,1,1}, 0.5)
	end
	
	World.globalTimers:Add(f, 0.5)
	
	self.think = MemoryGame.Think
	self:SetNextThink(0)
	
	World.FlushInput(true)
	Abducted.entity.eatInput = false
end

function MemoryGame.EndGame(self, result)
	self = MemoryGame.entity
	self.think = nil
	self.exiting = true
	
	if (self.transitionTimer) then
		self.transitionTimer:Clean()
		self.transitionTimer = nil
	end
	
	World.FlushInput(true)
	
	if (self.gameCompleteCallback) then
		self.gameCompleteCallback(result)
	end
end

function MemoryGame.ResetGame(self)
	self = MemoryGame.entity
	PuzzleScoreScreen:Unlink()
	self:CleanWidgets()
	collectgarbage()
	-- clean up game-board, get ready for another ShowBoard/StartGame call sometime in the future.
	-- NOTE: the terminal puzzle UI is hidden right now
	MemoryGame.active = false
end

function MemoryGame.DoQuit(self)
	if (not self.exiting) then
		self:EndGame("f")
	end
end

function MemoryGame.DoRetry(self)
	if (self.exiting) then
		return
	end
	
	self.widgets.black:BlendTo({0,0,0,1}, 0.3)
	Abducted.entity.eatInput = true
	
	local f = function()
		Abducted.entity.eatInput = false
		self:RestartGameState()
		self.widgets.black:BlendTo({0,0,0,0}, 0.3)
		
		local f = function()
			self:Run()
		end
		
		World.globalTimers:Add(f, 0.3)
	end
	
	World.globalTimers:Add(f, 0.3)
end

function MemoryGame.QuitButtonPressed(self)
	self:EndGame("x")	
end

function MemoryGame.CreateBoard(self,level)
	self:CleanBoard()
	collectgarbage()
	
	self.widgets.board = {}
	self.widgets.grid = {}
		
	for iy=1,level.INDEX_MAX_Y do
        for ix=1,level.INDEX_MAX_X do
			local pos = self:GetPositionByGrid(ix,iy)
			local tile = self:CreateTile(pos.x,pos.y)
            table.insert(self.widgets.board,{x=ix, y=iy, tile=tile})
            local index = self:ConvertCoordToIndex(ix, iy)
            self.widgets.grid[index] = tile
        end
    end

end

function MemoryGame.PlaceGoalCmds(self, disjoint)

	if (disjoint) then
	
		local cmds = {}
		local indices = {}
		
		for i=1,#self.goals do
		
			while (true) do
				local x = IntRand(1, self.INDEX_MAX_X)
				local y = IntRand(1, self.INDEX_MAX_Y)
				local index = self:ConvertCoordToIndex(x,y)
				
				if (not indices[index]) then
					indices[index] = true
					
					local tile = self.widgets.grid[self:ConvertCoordToIndex(x,y)]
					table.insert(cmds, {x=x, y=y, tile=tile})
					break
				end
			end
			
		end
		
		return cmds
	
	end

	-- find a spot for us
	local dir = math.random() < 0.5
	local xflip = math.random() < 0.5
	local yflip = math.random() < 0.5
	
	local x,y
	if (dir) then
		x = IntRand(1, self.INDEX_MAX_X - #self.goals + 1)
		y = IntRand(1, self.INDEX_MAX_Y)
		
		if (xflip) then
			-- flip horizontal
			x = self.INDEX_MAX_X - (x - 1) - (#self.goals - 1)
			x = Max(1, x)
		end
		
		if (yflip) then
			-- flip vertical
			y = self.INDEX_MAX_Y - (y - 1)
		end
	else
		x = IntRand(1, self.INDEX_MAX_X)
		y = IntRand(1, self.INDEX_MAX_Y - #self.goals + 1)
		
		if (xflip) then
			-- flip horizontal
			x = self.INDEX_MAX_X - (x - 1)
		end
		
		if (yflip) then
			-- flip vertical
			y = self.INDEX_MAX_Y - (y - 1) - (#self.goals - 1)
			y = Max(1, y)
		end
	end
			
	local cmds = {}
	
	for i=1,#self.goals do
	
		local index = nil
		
		local tx, ty
		
		if (dir) then -- horizontal
			
			if (hflip) then -- right to left
				tx = x+#self.goals-i
				ty = y
			else
				tx = x+i-1
				ty = y
			end
			
		else
		
			if (vflip) then -- bottom to top
				tx = x
				ty = y+#self.goals-i
			else
				tx = x
				ty = y+i-1
			end
			
		end
		
		local tile = self.widgets.grid[self:ConvertCoordToIndex(tx,ty)]
		
		table.insert(cmds, {x=tx, y=ty, tile=tile})
	end

	return cmds
end

function MemoryGame.PlaceGoal(self, disjoint)

	local cmds = self:PlaceGoalCmds(disjoint)
	
	for k,v in pairs(cmds) do
	
		local index = self:ConvertCoordToIndex(v.x,v.y)
		local tile = self.widgets.grid[index]
	
		tile.goal = true
		tile.id = self.goals[k]
		self.widgets.goals[k].x = v.x
		self.widgets.goals[k].y = v.y
		
		self:SetFullTileMaterial(tile, self.gfx[string.format("cell_%02i", self.goals[k])])
	
	end

end

function MemoryGame.ShuffleBoard(self, validGlyphs)

	local i = 1
	local glyphs = ShuffleArray(validGlyphs)

    for k,v in pairs(self.widgets.grid) do
		if (i > #validGlyphs) then
			i = 1
			glyphs = ShuffleArray(validGlyphs)
		end
		local tile = v
		local gfx = glyphs[i]
		tile.id = gfx
		self:SetFullTileMaterial(tile, self.gfx[string.format("cell_%02i", gfx)])
		i = i + 1
    end
end

function MemoryGame.GenShuffleCommands(self)

	local numPiecesToShuffle = IntRand(12, 40)
		
	local cmds = {}
	local touched = {}
		
	for i=1,numPiecesToShuffle do
	
		local x = IntRand(1, self.INDEX_MAX_X)
		local y = IntRand(1, self.INDEX_MAX_Y)
		
		local index = self:ConvertCoordToIndex(x, y)
		if (not touched[index]) then
			local tile = self.widgets.grid[index]
			
			if (not tile.busy) then
				if (tile.goal) then
					if (self.phaseGlyphsCounter == 0) then -- can't move goal if selected
						-- goal piece, move entire goal
						local goalCmds = self:PlaceGoalCmds()
						
						for gk,gv in pairs(goalCmds) do
							local goal = self.widgets.goals[gk]
							local gindex = self:ConvertCoordToIndex(goal.x, goal.y)
							local gtile = self.widgets.grid[gindex]
													
							table.insert(cmds, {x=gv.x, y=gv.y, goal=gk}) -- make goal
							table.insert(cmds, {tile=gtile}) -- change tile
							
							touched[self:ConvertCoordToIndex(gv.x, gv.y)] = true
							touched[gindex] = true
						end
					end
				else
					touched[index] = true
					table.insert(cmds, {tile=tile})
				end
			end
		end
	
	end
	
	return cmds
	
end

function MemoryGame.ChallengShuffle(self)

	local cmds = self:GenShuffleCommands()
	local tiles = {}
	
	if (self.shuffleGlyphs == nil) then
		self.shuffleGlyphs = ShuffleArray(self.validGlyphs2)
		self.shuffleIdx = 1
	end
	
	for k,v in pairs(cmds) do
	
		if (self.shuffleIdx > #self.shuffleGlyphs) then
			self.shuffleGlyphs = ShuffleArray(self.validGlyphs2)
			self.shuffleIdx = 1
		end
		
		local glyph = self.shuffleGlyphs[self.shuffleIdx]
		local gfx = self.gfx[string.format("cell_%02i", glyph)]
		self.shuffleIdx = self.shuffleIdx + 1
		
		local tile = v.tile
		
		if (v.goal) then -- make tile a goal
			tile = self.widgets.grid[self:ConvertCoordToIndex(v.x,v.y)]
			tile.goal = true
			tile.id = glyph
			tile.sel:SetMaterial(gfx)
		else
			tile.goal = false
			tile.id = glyph
			tile.sel:SetMaterial(gfx)
		end
		
		table.insert(tiles, {tile=tile})
	
	end
	
	self:DoRedEffect(tiles)

end

function MemoryGame.CleanWidgets(self)
	self:CleanBoard()
	
	for i,k in pairs(self.widgets.goals) do
		self.widgets.root:RemoveChild(k.base)
		k.base:Unmap()
		self.widgets.root:RemoveChild(k.piece)
		k.piece:Unmap()
	end
	
	self.widgets.goals = nil
	collectgarbage()
end

function MemoryGame.CleanBoard(self)
	if (self.widgets.board) then
		-- reset the board for next draw
		for i,k in pairs(self.widgets.board) do
			self.widgets.root:RemoveChild(k.tile.tile)
			k.tile.tile:Unmap()
			self.widgets.root:RemoveChild(k.tile.sel)
			k.tile.sel:Unmap()
			self.widgets.root:RemoveChild(k.tile.scale)
			k.tile.scale:Unmap()
			self.widgets.root:RemoveChild(k.tile.red)
			k.tile.red:Unmap()
		end
		
		self.widgets.grid = nil
		self.widgets.board = nil
	end
end

function MemoryGame.CreateBoardSmall(self)

	local level = {}

    level.name = "Small"
    level.INDEX_MAX_X = self.SMALL_INDEX_MAX_X
    level.INDEX_MAX_Y = self.SMALL_INDEX_MAX_Y
    level.CELL_SCALE = self.SMALL_CELL_SCALE
    level.CELL_SPACE = self.SMALL_CELL_SPACE
    
    return level
end

function MemoryGame.CreateBoardLarge(self,legend)
	
	local level = {}

    level.name = "Large"
    level.INDEX_MAX_X = self.LARGE_INDEX_MAX_X
    level.INDEX_MAX_Y = self.LARGE_INDEX_MAX_Y
    level.CELL_SCALE = self.LARGE_CELL_SCALE
    level.CELL_SPACE = self.LARGE_CELL_SPACE
    level.board = {}

    return level
end

function MemoryGame.OnInputEvent(self,e)
    self = MemoryGame.entity

    COutLine(kC_Debug,"OnInputEvent - call")

    if (not Input.IsTouchBegin(e)) then
        return false
    end
    
	if (not (self.state.phase == kMGPhase_DiscoverPattern or self.state.phase == kMGPhase_FindPattern)) then
		return true
	end

    COutLine(kC_Debug,"OnInputEvent - begin")

    local best
    local x = e.data[1]
    local y = e.data[2]
    for k,v in pairs(self.widgets.board) do
        COutLine(kC_Debug,"OnInputEvent - widget")
        local r = v.tile.tile:Rect()
        if (x >= r[1] and x <= (r[1] + r[3]) and y >= r[2] and y <= (r[2] + r[4])) then
            best = v
            break
        end
    end

    if (best) then
        COutLine(kC_Debug,"OnInputEvent - found")

        e = UI:MapInputEvent(e)
       
		if (self.state.phaseGlyphsCounter < #self.goals) then
			COutLine(kC_Debug,"begin glyph code")
			local idx = self.state.phaseGlyphsCounter + 1
			local gfx = self.goals[idx]
			if (best.tile.goal and (best.tile.id == gfx)) then
				if (not best.tile.busy) then
					best.tile.busy = true
					if (self.state.phase == kMGPhase_DiscoverPattern) then
						self.widgets.goals[idx].piece:BlendTo({1,1,1,1}, 0.1)
					end
					best.tile.tile:BlendTo({1,1,1,1}, 0.2)
					best.tile.sel:BlendTo({1,1,1,1}, 0.2)
					self.state.phaseGlyphsCounter = idx
				end
			else
				self:DoRedEffect({best}, true)
			end
		end
    end

    return true
end

function MemoryGame.ChangeLevel(self,level)
    
    self.REFLEX_BOARD_OFFSET = {self.boardRect[1], self.boardRect[2]}
    self.REFLEX_CELL_SIZE = {(level.CELL_SCALE+level.CELL_SPACE[1])*UI.identityScale[1], (level.CELL_SCALE+level.CELL_SPACE[2])*UI.identityScale[1] }
    self.REFLEX_TILE_SIZE = {(level.CELL_SCALE)*UI.identityScale[1], (level.CELL_SCALE)*UI.identityScale[1] }
    self.INDEX_MAX_X = level.INDEX_MAX_X
    self.INDEX_MAX_Y = level.INDEX_MAX_Y
    self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + 0 * self.REFLEX_CELL_SIZE[1]
    self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + 0 * self.REFLEX_CELL_SIZE[2]
    self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE[1]
    self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE[2]

    self:CreateBoard(level)
    
end

function MemoryGame.InitUI(self)
    -- constants
    self.SMALL_CELL_SCALE = 130
    self.SMALL_INDEX_MAX_X = 8
    self.SMALL_INDEX_MAX_Y = 4
    self.SMALL_CELL_SPACE  = {12, 0}

    self.LARGE_CELL_SCALE = 87
    self.LARGE_INDEX_MAX_X = 12
    self.LARGE_INDEX_MAX_Y = 6
    self.LARGE_CELL_SPACE = {8, 0}
    
    self.GOAL_SIZE = 130 * UI.identityScale[1]

    self.PHASE0_LEGEND_REVEAL_TIMER = 0.7
    self.PHASE1_BOARD_SHUFFLE_TIMER = .125
    self.PHASE1_BOARD_SHUFFLE_MAX = 10

    self.NUM_GLPYHS_TO_FIND = 3

    self.NUM_GLYPHS = 27
    
    self.glyphNums = {}
    for i=1,self.NUM_GLYPHS do
		table.insert(self.glyphNums, i)
    end
    
    self.boardRect = {
		self.screen[1],
		self.screen[2] + (self.SMALL_CELL_SCALE+8)*UI.identityScale[1],
		self.screen[3],
		0
	}
	
	self.boardRect[4] = self.screen[2] + self.screen[4] - self.boardRect[2]

    -- define structure self.widgets
    self.widgets = {}
    self.widgets.goals = { }
    self.widgets.board = { }
    self.widgets.grid = {}

    self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=MemoryGame.OnInputEvent})
    World.SetRootWidget(UI.kLayer_SolveGame, self.widgets.root)
    self.widgets.root:SetOpaqueLayerInput(true) -- no input goes past this

    self.widgets.root:SetVisible(false)

    self.widgets.background = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.board})
    self.widgets.root:AddChild(self.widgets.background)

	self.widgets.wavy = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.wavy})
	self.widgets.root:AddChild(self.widgets.wavy)
	self.widgets.wavy:SetClipRect(self.boardRect)
	
	self.widgets.highlight = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=self.gfx.highlight})
	self.widgets.root:AddChild(self.widgets.highlight)
	self.widgets.highlight:SetVisible(false)
	self.widgets.highlight:SetHAlign(kHorizontalAlign_Center)
	self.widgets.highlight:SetVAlign(kVerticalAlign_Center)
	
	local buttonWidth = self.SMALL_CELL_SCALE * 1.5
	local buttonHeight = self.SMALL_CELL_SCALE * 0.5
	
	self.widgets.button = UI:CreateStylePushButton(
		{
			self.screen[1]+self.screen[3]-buttonWidth-16*UI.identityScale[1],
			self.screen[2]+(self.SMALL_CELL_SCALE-buttonHeight)/3,
			buttonWidth,
			buttonHeight
		},
		function () self:QuitButtonPressed() end,
		{fontSize="small",highlight={on={0,0,0,0}}},
		self.widgets.root
	)
	
	local text = StringTable.Get("TERMINAL_LOGOUT")
	UI:LineWrapCenterText(
		self.widgets.button.label,
		nil,
		nil,
		0,
		text
	)
	
	self.widgets.button:BlendTo({1,1,1,0},0)

    COutLine(kC_Debug, "Creating Board")
    -- board step: board grid is x,y structure

    self.widgets.timeLeftLabel = UI:CreateWidget("TextLabel", {rect={self.screen[1]+(self.SMALL_CELL_SCALE/2)*UI.identityScale[1], 0, UI.screenWidth*.15,UI.screenHeight*.15}, typeface=self.typefaces.TimerText})
    self.widgets.root:AddChild(self.widgets.timeLeftLabel)
    
    self.widgets.root2 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
    World.SetRootWidget(UI.kLayer_SolveGame2, self.widgets.root2)
    self.widgets.root2:SetVisible(false)
    
    self.widgets.root3 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
    World.SetRootWidget(UI.kLayer_SolveGame3, self.widgets.root3)
    self.widgets.root3:SetVisible(false)
    
    self.widgets.black = UI:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=UI.gfx.Solid})
	self.widgets.root3:AddChild(self.widgets.black)
	self.widgets.black:BlendTo({0,0,0,0}, 0)

    COutLine(kC_Debug, "Board Completed")
end

function MemoryGame.LoadMaterials(self)
    self.gfx = {}
    self.gfx.blackhole = World.Load("Puzzles/reflex-blackhole1_M");
    self.gfx.antivirus_spider = World.Load("Puzzles/AlienICE_M")
    self.gfx.blue_glow = World.Load("Puzzles/reflex-blueglow1_M")
    self.gfx.board = World.Load("UI/terminal_screen1_M")
    self.gfx.wavy = World.Load("UI/distortion_pattern1_M")
    self.gfx.highlight = World.Load("UI/arm_buttons_overbright_M")

    self.gfx.mark_current = World.Load("Puzzles/reflex-player1_M")
    self.gfx.mark_line_v = World.Load("Puzzles/reflex-goal1_M")
    self.gfx.mark_line_h = self.gfx.mark_line_v
    self.gfx.mark_end = World.Load("Puzzles/reflex-goal1_M")

    self.gfx.blocker_green = World.Load("UI/blue_blank1_M")

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
    self.typefaces.BigText = World.Load("UI/TerminalPuzzlesFont_TF")
    self.typefaces.TimerText = World.Load("UI/TerminalPuzzlesTimeFont_TF")
    self.typefaces.TouchWhenReadyText = World.Load("UI/TerminalPuzzlesFont_TF")
    self.typefaces.SwipeToMoveText = World.Load("UI/TerminalPuzzlesFont_TF")

    local xScale = UI.screenWidth / 1280
    local yScale = UI.screenHeight / 720

    -- the border is authored to be a 16:9 image packed in a square image, adjust for this

    local region = (1 - UI.yAspect) / 2
    local inset  = region * UI.screenWidth

    self.magicBoardRect = {0, -inset, UI.screenWidth, UI.screenHeight+inset*2}

    local wideRegion = (1 - (9/16)) / 2
    local wideInset = wideRegion * 1280 * xScale

    if (UI.systemScreen.aspect == "3x2") then
        wideInset = wideInset * 1.1
    end

    self.screen = {
        1280 * 0.05156 * xScale,
        (wideInset-inset)*yScale,
        0,
        0
    }

    self.screen[3] = UI.screenWidth - (self.screen[1]*2)
    self.screen[4] = UI.screenHeight - (self.screen[2]*2)
end

function MemoryGame.GetPositionByGrid(self,x,y)
    local v = { }
    v.x = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1]/2 + (x-1) * self.REFLEX_CELL_SIZE[1]
    v.y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2]/2 + (y-1) * self.REFLEX_CELL_SIZE[2]
    return v
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

function MemoryGame.CreateTile(self,x,y)

	x = x-self.REFLEX_TILE_SIZE[1]/2
	y = y-self.REFLEX_TILE_SIZE[2]/2
	
	local tile = {x=x,y=y}
    local base = UI:CreateWidget("MatWidget", {rect={x,y,self.REFLEX_TILE_SIZE[1],self.REFLEX_TILE_SIZE[2]}})
    local sel = UI:CreateWidget("MatWidget", {rect={x,y,self.REFLEX_TILE_SIZE[1],self.REFLEX_TILE_SIZE[2]}})
    local scale = UI:CreateWidget("MatWidget", {rect={x,y,self.REFLEX_TILE_SIZE[1],self.REFLEX_TILE_SIZE[2]}})
    local red = UI:CreateWidget("MatWidget", {rect={x,y,self.REFLEX_TILE_SIZE[1],self.REFLEX_TILE_SIZE[2]}})
    
    scale:SetHAlign(kHorizontalAlign_Center)
    scale:SetVAlign(kHorizontalAlign_Center)
    
    tile.tile = base
    tile.sel = sel
    tile.scale = scale
    tile.red = red
    
    base:BlendTo({1,1,1,0}, 0)
    sel:BlendTo({1,1,1,0}, 0)
    scale:BlendTo({1,1,1,0}, 0)
    red:BlendTo({1,0,0,0}, 0)
    
    self.widgets.root:AddChild(tile.red)
    self.widgets.root:AddChild(tile.tile)
    self.widgets.root:AddChild(tile.sel)
    self.widgets.root:AddChild(tile.scale)
    
    return tile
end

function MemoryGame.CreateGoal(self,gfx,x,y)

	x = x-self.GOAL_SIZE/2
	y = y-self.GOAL_SIZE/2
	
	local goal = {}
	
	local base = UI:CreateWidget("MatWidget", {rect={x,y,self.GOAL_SIZE,self.GOAL_SIZE}, material=self.gfx.blocker_green})
    local piece = UI:CreateWidget("MatWidget", {rect={x,y,self.GOAL_SIZE,self.GOAL_SIZE}, material=self.gfx[string.format("cell_%02i", gfx)]})
    
    goal.base = base
	goal.piece = piece
	
	base:BlendTo({1,1,1,0}, 0)
	piece:BlendTo({1,1,1,0}, 0)
	
	self.widgets.root:AddChild(base)
	self.widgets.root:AddChild(piece)
	
	return goal
end

function MemoryGame.SetTileMaterial(self, tile, material)

	tile.tile:SetMaterial(material)
	tile.sel:SetMaterial(material)

end

function MemoryGame.SetFullTileMaterial(self, tile, material)

	tile.tile:SetMaterial(material)
	tile.sel:SetMaterial(material)
	tile.scale:SetMaterial(material)
	tile.red:SetMaterial(material)
	
end

function MemoryGame.UpdateHud(self)
    local minutes = math.floor(math.ceil(self.state.timeLeft) / 60)
    local seconds = math.ceil(self.state.timeLeft) - (minutes * 60)
    local text = string.format("%01i:%02i",minutes,seconds)
    UI:SetLabelText(self.widgets.timeLeftLabel, text, {1,1})
    UI:SizeLabelToContents(self.widgets.timeLeftLabel)
    UI:VCenterLabel(self.widgets.timeLeftLabel, {self.screen[1], self.screen[2], self.screen[3], self.SMALL_CELL_SCALE*UI.identityScale[1]})
end

function MemoryGame.BlendBoard(self, blend, time, sel)

	for k,v in pairs(self.widgets.board) do
		v.tile.tile:BlendTo(blend, time)
		if (sel) then
			v.tile.sel:BlendTo(blend, time)
		end
	end

end

function MemoryGame.DoInterferenceEffect(self, tiles)

	for k,v in pairs(tiles) do
		
		v.tile.busy = true
		v.tile.scale:BlendTo({1,1,1,0}, 0)
		v.tile.scale:ScaleTo({1,1.5}, {0,0})
		v.tile.scale:BlendTo({1,1,1,0.5}, 0.1)
		
	end
	
	local f = function()
		for k,v in pairs(tiles) do
			v.tile.scale:BlendTo({1,1,1,0}, 0.3)
			v.tile.busy = false
		end
	end
	
	World.globalTimers:Add(f, 0.2)

end

function MemoryGame.DoRedEffect(self, tiles, forError)

	for k,v in pairs(tiles) do
	
		local shiftX = FloatRand(0.1, 0.2) * self.REFLEX_CELL_SIZE[1]
		local shiftY = FloatRand(0.1, 0.2) * self.REFLEX_CELL_SIZE[1]
		
		if (math.random() < 0.5) then
			shiftX = -shiftX
		end
		
		if (math.random() < 0.5) then
			shiftY = -shiftY
		end
		
		local rect = v.tile.tile:Rect()
		
		rect[1] = rect[1] + shiftX
		rect[2] = rect[2] + shiftY
		
		v.tile.busy = true
		
		v.tile.red:SetRect(rect)
		v.tile.red:BlendTo({1,0,0,1}, 0.1)
		v.tile.tile:BlendTo({1,1,1,0.1}, 0.05)
			
	end
	
	local f = function()
	
		for k,v in pairs(tiles) do
			
			v.tile.red:BlendTo({1,0,0,0}, 0.5)
			
			if (forError) then
				v.tile.busy = false
				v.tile.tile:BlendTo({1,1,1,0.4}, 1)
			else
				v.tile.sel:BlendTo({1,1,1,0.4}, 1)
				v.tile.tile:BlendTo({1,1,1,0}, 1)
				v.tile.sel,v.tile.tile = v.tile.tile,v.tile.sel
			end
		
		end
		
		if (not forError) then
			local f = function()
				for k,v in pairs(tiles) do
					v.tile.busy = false
					v.tile.sel:SetMaterial(self.gfx[string.format("cell_%02i", v.tile.id)])
				end
			end
			
			World.globalTimers:Add(f, 1)
		end
	
	end
	
	World.globalTimers:Add(f, 0.15)

end

function MemoryGame.DoPhase0(self,dt)
    self.state.phaseTimer = self.state.phaseTimer - dt
    if (self.state.phaseTimer > 0) then
        return
    end

--    COutLine(kC_Debug,"Phase0 - Legend/Goal Count: %i/%i",#self.state.level.legend,#self.widgets.goals)
    if (#self.widgets.goals >= #self.goals) then
        self.state.phase = kMGPhase_ShuffleSmallTiles
        self.state.phaseTimer = self.PHASE1_BOARD_SHUFFLE_TIMER
        self:BlendBoard({1,1,1,0.4}, 0)
        self.widgets.button:BlendTo({1,1,1,1}, 0.3)
        return
    end

    local gfx = self.goals[#self.widgets.goals+1]
    self.state.phaseTimer = self.PHASE0_LEGEND_REVEAL_TIMER
    local b = self:CreateGoal(
		gfx,
		(UI.screenWidth - (#self.goals*self.GOAL_SIZE))/2 + (#self.widgets.goals*self.GOAL_SIZE) + self.GOAL_SIZE/2,
		self.screen[2]+self.GOAL_SIZE/2
	)
	
    b.base:BlendTo({1,1,1,0}, 0)
    b.base:BlendTo({1,1,1,1}, 0.2)
    table.insert(self.widgets.goals,b)
end

function MemoryGame.DoPhase1(self,dt)
    self.state.phaseTimer = self.state.phaseTimer - dt
    if (self.state.phaseTimer > 0) then
        return
    end

	self:ShuffleBoard(self.validGlyphs)
	
    self.state.phaseShuffles = self.state.phaseShuffles - 1
    if (self.state.phaseShuffles <= 0) then
        self.state.phase = kMGPhase_DiscoverPattern
        self:PlaceGoal(true)
        self:DoInterferenceEffect(self.widgets.board)
        return
    end

    self.state.phaseTimer = self.PHASE1_BOARD_SHUFFLE_TIMER
	
end

function MemoryGame.DoPhase2(self,dt)
    if (self.state.phaseGlyphsCounter >= #self.widgets.goals) then
        self.state.phase = kMGPhase_AnimateTransition
        self.state.phaseTimer = 1
        self.state.phaseGlyphsCounter = 0
        
        local f = function()
			self:DoInterferenceEffect(self.widgets.board)
			
			local f = function()
				self:BlendBoard({1,1,1,0}, 0.5, true)
				
				local f = function()
				
					self.widgets.highlight:SetVisible(true)
					self.widgets.highlight:ScaleTo({0,1},{0,0})
					self.widgets.highlight:ScaleTo({1,1},{0.8,0})
				
					local f = function()
					
						for k,v in pairs(self.widgets.goals) do
							v.base:BlendTo({1,1,1,0}, 0.8)
						end
					
						local f = function()
							self:ChangeLevel(self:CreateBoardLarge())
							self:ShuffleBoard(self.validGlyphs2)
							self:PlaceGoal()
							self:BlendBoard({1,1,1,0.4}, 1)
							self:DoInterferenceEffect(self.widgets.board)
							
							local f = function()
								self.state.phase = kMGPhase_FindPattern
								self.state.phaseTimer = FloatRand(5, 12)
								self.widgets.wavy:BlendTo({1,1,1,1}, 1)
							end
							self.transitionTimer = nil
							World.globalTimers:Add(f, 1)
						end
					
						self.transitionTimer = World.globalTimers:Add(f, 1)
					end
					
					self.transitionTimer = World.globalTimers:Add(f, 1)
				end
				
				self.transitionTimer = World.globalTimers:Add(f, 0.6)
			end
			
			self.transitionTimer = World.globalTimers:Add(f, 0.2)
        end
        
        self.transitionTimer = World.globalTimers:Add(f, 0.7)
    end
end

function MemoryGame.DoPhase3(self,dt)
end

function MemoryGame.DoPhase4(self,dt)
    if (self.state.phaseGlyphsCounter >= #self.widgets.goals) then
        self.state.phase = kMGPhase_GameOver
        self.state.gameOver = true
        self.state.victory = true
        self.widgets.wavy:BlendTo({1,1,1,0}, 0.2)
        self.widgets.button:BlendTo({1,1,1,0}, 0.2)
        return
    end
    
    self.state.phaseTimer = self.state.phaseTimer - dt
    if (self.state.phaseTimer > 0) then
        return
    end
    
    self.state.phaseTimer = FloatRand(5, 12)
    self:ChallengShuffle()
end

function MemoryGame.Think(self,dt)
	if (self.state.gameOver) then
		self.think = nil
		if (self.state.victory) then
			local f = function()
				PuzzleScoreScreen:DoSuccessScreen(
					self.widgets.root2,
					self.actions,
					function()
						self:EndGame("w")
					end
				)
			end
			World.globalTimers:Add(f, 1)
		else
			PuzzleScoreScreen:DoRetryQuitScreen(
				self.widgets.root2,
				function ()
					self:DoRetry()
				end,
				function ()
					self:DoQuit()
				end
			)
		end
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

    if (self.state.phase > kMGPhase_ShuffleSmallTiles) then
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
