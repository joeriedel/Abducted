-- TerminalPuzzles.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Dave Reese
-- See Abducted/LICENSE for licensing terms

TerminalPuzzles = Class:New()
TerminalPuzzles.UI_Layer = 5

function TerminalPuzzles.Spawn(self)
	TerminalPuzzles.entity = self
	
	self:LoadMaterials()
	self:InitUI()
	
	self.think = TerminalPuzzles.Think
	self:SetNextThink(0)
end

function TerminalPuzzles.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.goal = { "symbol_a", "symbol_b", "symbol_c", "symbol_d" }

	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=10, y=3, img="cell_green" }
		, { x=5, y=5, img="cell_green" }
		, { x=3, y=6, img="cell_a" }		
		, { x=5, y=6, img="cell_b" }
		, { x=7, y=6, img="cell_d" }
		, { x=9, y=6, img="cell_e" }
		}
	
	return level
end

function TerminalPuzzles.CreateLevel1x2(self)
	local level = {}
	
	level.name = "1x2"	
	
	level.goal = { "symbol_d", "symbol_b", "symbol_a", "symbol_c" }

	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=9, y=3, img="cell_green" }
		, { x=7, y=5, img="cell_green" }
		}
	
	return level
end

function TerminalPuzzles.CreateLevel1x3(self)
	local level = {}
	
	level.name = "1x3"	
	
	level.goal = { "symbol_b", "symbol_a", "symbol_c", "symbol_d" }

	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=6, y=3, img="cell_green" }
		, { x=3, y=5, img="cell_green" }
		}
	
	return level
end

function TerminalPuzzles.CreateBoards(self)
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

function TerminalPuzzles.GameIsActive()
	if (TerminalPuzzles.entity.state) then
		return true
	end
	
	return false
end

function TerminalPuzzles.OnInputEvent(self,e)
	if (e.type == kI_KeyDown) then
		--COutLine(kC_Debug,"key=%i",e.data[1])
		if (e.data[1] == kKeyCode_I) then
			--COutLine(kC_Debug,"moving widget")
			self.state.heading.x = 0
			self.state.heading.y = -1
			--UI:CenterWidget(self.widgets.current,UI.screenWidth/2, UI.screenHeight/2)
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
end

function tablelength(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

function TerminalPuzzles.InitUI(self)
	-- constants
	local REFLEX_CELL_SIZE = 40
	local REFLEX_BOARD_OFFSET = 80

	self:CreateBoards()	

	self.widgets = {}
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=UI.EatInput})
	World.SetRootWidget(TerminalPuzzles.UI_Layer, self.widgets.root)

	self.widgets.border = UI:CreateWidget("MatWidget", {rect={0,0,UI.screenWidth,UI.screenHeight}, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect={REFLEX_BOARD_OFFSET,REFLEX_BOARD_OFFSET,UI.screenWidth-REFLEX_BOARD_OFFSET*2,UI.screenHeight-REFLEX_BOARD_OFFSET*2}, material=self.gfx.board})
	
	UI:CenterWidget(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
	UI:CenterWidget(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
	
	self.widgets.root:AddChild(self.widgets.border)	
	self.widgets.root:AddChild(self.widgets.board)	
	
	self.state = { }
	self.state.heading = { }
	self.state.heading.x = 0
	self.state.heading.y = 0
	self.state.victory = { }
	self.state.current = { }
		
	-- load appropriate level based on skill + difficulty		
	local level = self.db.levels[1][1]
	COutLine(kC_Debug, "reflex.level.name=" .. level.name)
	-- goal step: center a,b,c,d list ot top of screen
	self.widgets.goal = { }
	for i,v in ipairs(level.goal) do 
		local xo = REFLEX_CELL_SIZE/2 + REFLEX_CELL_SIZE * (i-1)
		self.widgets.goal[v] = UI:CreateWidget("MatWidget", {rect={0,0,REFLEX_CELL_SIZE,REFLEX_CELL_SIZE}, material=self.gfx[v]})
		self.widgets.root:AddChild(self.widgets.goal[v])
		-- 120 = count(level.goal) * REFLEX_CELL_SIZE
		UI:CenterWidget(self.widgets.goal[v], UI.screenWidth/2-tablelength(level.goal)*REFLEX_CELL_SIZE/2+xo, REFLEX_CELL_SIZE)
		
		-- djr, work in progress
		local cell = { }
		cell.architype = string.gsub(v,"symbol_","cell_")
		table.insert(self.state.victory,cell)
	end
	-- board step: board grid is x,y structure
	self.widgets.board = { }	
	for i,v in ipairs(level.board) do
		local xo = REFLEX_BOARD_OFFSET + REFLEX_CELL_SIZE/2 + v.x * REFLEX_CELL_SIZE
		local yo = REFLEX_BOARD_OFFSET + REFLEX_CELL_SIZE/2 + v.y * REFLEX_CELL_SIZE
			
		self.widgets.board[i] = UI:CreateWidget("MatWidget", {rect={0,0,REFLEX_CELL_SIZE,REFLEX_CELL_SIZE}, material=self.gfx[v.img]})
		self.widgets.board[i].state = { }
		self.widgets.board[i].state.cell = v
		self.widgets.root:AddChild(self.widgets.board[i])
		UI:CenterWidget(self.widgets.board[i], xo, yo)
	end	
	
	self.state.currentMove = 1
	self.widgets.current = UI:CreateWidget("MatWidget", {rect={200,200,REFLEX_CELL_SIZE,REFLEX_CELL_SIZE}, material=self.gfx.mark_current})
	self.widgets.root:AddChild(self.widgets.current)
	
	-- ORIGINAL CODE
	--[[
	self.widgets = {}
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=UI.EatInput})
	World.SetRootWidget(TerminalPuzzles.UI_Layer, self.widgets.root)

	-- Make a border
	local xBorderPadding = UI.screenWidth * 0.1
	local yBorderPadding = UI.screenHeight * 0.1
	local xBorderWidth = UI.screenWidth * 0.05
	local yBorderWidth = xBorderWidth
	
	local horzRect = {0, 0, UI.screenWidth - xBorderPadding * 2, yBorderWidth}
	local vertRect = {0, 0, xBorderWidth, UI.screenHeight - yBorderPadding * 2}
	
	self.widgets.borderTop = UI:CreateWidget("MatWidget", {rect=horzRect, material=self.gfx.Border})
	self.widgets.borderBottom = UI:CreateWidget("MatWidget", {rect=horzRect, material=self.gfx.Border})
	self.widgets.borderLeft = UI:CreateWidget("MatWidget", {rect=vertRect, material=self.gfx.Border})
	self.widgets.borderRight = UI:CreateWidget("MatWidget", {rect=vertRect, material=self.gfx.Border})
	
	UI:HCenterWidget(self.widgets.borderTop, nil, yBorderPadding)
	UI:HCenterWidget(self.widgets.borderBottom, nil, 0)
	UI:VAlignWidget(self.widgets.borderBottom, nil, UI.screenHeight - yBorderPadding)
	
	UI:VCenterWidget(self.widgets.borderLeft, xBorderPadding, nil)
	UI:VCenterWidget(self.widgets.borderRight, xBorderPadding, nil)
	UI:RAlignWidget(self.widgets.borderRight, UI.screenWidth - xBorderPadding, nil)
	
	self.widgets.root:AddChild(self.widgets.borderTop)
	self.widgets.root:AddChild(self.widgets.borderBottom)
	self.widgets.root:AddChild(self.widgets.borderLeft)
	self.widgets.root:AddChild(self.widgets.borderRight)
	
	self.widgets.bigTextLabel = UI:CreateWidget("TextLabel", {rect={0, 0, 0, 0}, typeface=self.typefaces.BigText})
	self.widgets.bigTextLabel:SetText(StringTable.Get("TERMINAL_PUZZLES_TITLE"))
	UI:HCenterLabel(self.widgets.bigTextLabel, nil, 8)
	self.widgets.root:AddChild(self.widgets.bigTextLabel)
	
	--self.widgets.square = UI:CreateWidget("MatWidget", {rect={0, 0, 64, 64}, material=self.gfx.Border})
	--UI:CenterWidget(self.widgets.square)
	--self.widgets.root:AddChild(self.widgets.square)
	
	]]	
end

function TerminalPuzzles.LoadMaterials(self)

	--[[
		Asset loading:
		
			Restrictions: must be called from a THINK function. A think function
			is the entities .think function member.
			
			Spawn, PostSpawn are THINK functions.
			
			NOTE: Load methods are blocking by default for the entity which called
			them. Only the entity script which invoked the load is blocking waiting
			for it to complete, other entities and rendering is unaffected.
			
		World.Precache(path, async)
			path: path to resource to load
			
			async: optional paramter, false means function is blocking (as in
			engine tick will stall until it returns, only use if resource must
			be cached on that exact frame).
			
			Precaching does not return a usable resource, rather it returns
			an opaque reference. Precaching is used to keep a resource loaded
			that may be used (or loaded again) during gameplay. If the resource
			was not in memory the game would hitch or otherwise stall.
			
			Example: projectiles fired from a weapon. The projectile models
			would be precached so when they are fired their World.Load calls
			return immediately.
			
		World.Load(path, numInstances, async)
			path: path to resource to load
			
			numInstances: for sounds this determines how many instaces of the
			sounds can be played simultaneously.
			
			async: optional paramter, false means function is blocking (as in
			engine tick will stall until it returns, only use if resource must
			be cached on that exact frame).
	]]
	
	-- NOTE: materials used for UI widgets must have:
	--		DepthTest: false
	--		DepthWrite: false
	
	self.gfx = {}
--	self.gfx.antivirus_spider = World.Load("Reflex-Game/reflex-antivirus-spider_M")
	self.gfx.board = World.Load("Reflex-Game/reflex-board_M")
	self.gfx.border = World.Load("Reflex-Game/reflex-border_M")
	--[[
	self.gfx.cell_a = World.Load("Reflex-Game/reflex-cell-a_M")	
	self.gfx.cell_b = World.Load("Reflex-Game/reflex-cell-b_M")	
	self.gfx.cell_c = World.Load("Reflex-Game/reflex-cell-c_M")	
	self.gfx.cell_d = World.Load("Reflex-Game/reflex-cell-d_M")			
	]]
	self.gfx.cell_green = World.Load("Reflex-Game/reflex-cell-green_M")				

	self.gfx.mark_current = World.Load("Reflex-Game/reflex-mark-current_M")
	--[[
	self.gfx.mark_end = World.Load("Reflex-Game/reflex-mark-end_M")						
	self.gfx.mark_start = World.Load("Reflex-Game/reflex-mark-start_M")							
	]]
	self.gfx.symbol_a = World.Load("Reflex-Game/reflex-symbol-a_M")
	self.gfx.symbol_b = World.Load("Reflex-Game/reflex-symbol-b_M")
	self.gfx.symbol_c = World.Load("Reflex-Game/reflex-symbol-c_M")
	self.gfx.symbol_d = World.Load("Reflex-Game/reflex-symbol-d_M")
				
	-- ORIGINAL CODE
	--[[	
	self.gfx = {}

	self.gfx.Green = World.Load("UI/TerminalPuzzleGreen_M")
	self.gfx.Blue = World.Load("UI/TerminalPuzzleBlue_M")
	self.gfx.Border = World.Load("UI/TerminalPuzzleBorder_M")
	
	self.typefaces = {}
	self.typefaces.BigText = World.Load("UI/TerminalPuzzlesBigFont_TF")
	]]
end

function TerminalPuzzles.StartGame(self, gameType, gameCompleteCallback)
	self.gameCompleteCallback = gameCompleteCallback
end

function TerminalPuzzles.Think(self,dt)
	local r = self.widgets.current:Rect()

	local dx = self.state.heading.x * dt
	local dy = self.state.heading.y * dt
	
	r[1] = r[1] + r[3]/2 + dx
	r[2] = r[2] + r[4]/2 + dy
	
	if (r[1] < r[3]/2) then
		r[1] = 0
	end
	if (r[1] + r[3] > UI.screenWidth) then
		r[1] = UI.screenWidth - r[3]/2
	end
	if (r[2] < r[4]/2) then
		r[2] = 0
	end	
	if (r[2] + r[4] > UI.screenHeight) then
		r[2] = UI.screenWidth - r[4]/2
	end
			
	COutLine(kC_Debug,"x=%.02f,y=%.02f",r[1],r[2])
	UI:CenterWidget(self.widgets.current,r[1],r[2])
--	self.state.heading.x
	

--	local spiderKill = false
--	
--	-- determine current player pos
--	local px = 0
--	local py = 0
--	
--	local moveWasValid = true
--	for i,v in ipairs(level.board)  do
--		if v.state.cell.x == px and v.state.cell.y == py then
--			if v.state.moveIndex != 0 then
--				for i,v in ipairs(self.state.victory)
--					
--				end
--
--				v.state.moveIndex = self.state.currentMove
--				self.state.currentMove++
--			end
--			break
--		end
--	end
--
--	local pathComplete = false;	
--	for i,v in ipairs(level.board)  do
--		if v.cell.architype == 'antivirus_spider' then
--			-- move the spider to the player
--			--spikerKill = true
--		end
--		
--		if v.cell.architype == 'cell_a' 
--			or v.cell.architype == 'cell_b' 
--			or v.cell.architype == 'cell_c'
--			or v.cell.architype == 'cell_d' then
--			if v.state.moveIndex == 0 then
--				v.state.move = self.state.currentMove
--				self.state.currentMove = self.state.currentMove + 1
--			end
--		end		
--	end

	--self.gameCompleteCallback();
end

terminal_puzzles = TerminalPuzzles
