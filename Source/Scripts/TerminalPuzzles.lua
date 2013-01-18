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
	
	level.antivirusSpiderSpawnRate = 5
	level.antivirusSpiderSpeed = 24
	
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

function TerminalPuzzles.InitUI(self)
	-- constants
	self.REFLEX_CELL_SIZE = 40
	self.REFLEX_BOARD_OFFSET = 80
	self.INDEX_MAX_X = 21
	self.INDEX_MAX_Y = 15
	self.PLAYER_SPEED = 1
	

	self:CreateBoards()	

	self.widgets = {}
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=UI.EatInput})
	World.SetRootWidget(TerminalPuzzles.UI_Layer, self.widgets.root)

	self.widgets.border = UI:CreateWidget("MatWidget", {rect={0,0,UI.screenWidth,UI.screenHeight}, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect={self.REFLEX_BOARD_OFFSET,self.REFLEX_BOARD_OFFSET,UI.screenWidth-self.REFLEX_BOARD_OFFSET*2,UI.screenHeight-self.REFLEX_BOARD_OFFSET*2}, material=self.gfx.board})
	
	UI:CenterWidget(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
	UI:CenterWidget(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
	
	self.widgets.root:AddChild(self.widgets.border)	
	self.widgets.root:AddChild(self.widgets.board)	
	
	self.state = { }
	-- load appropriate level based on skill + difficulty		
	self.state.level = self.db.levels[1][1]
	self.state.heading = { }
	self.state.heading.x = 0
	self.state.heading.y = 0
	self.state.victory = { }
	self.state.current = { }
	self.state.spawnTimer = self.state.level.antivirusSpiderSpawnRate
		
	COutLine(kC_Debug, "reflex.level.name=" .. self.state.level.name)
	-- goal step: center a,b,c,d list ot top of screen
	self.widgets.goal = { }
	for i,v in ipairs(self.state.level.goal) do 
		local xo = self.REFLEX_CELL_SIZE/2 + self.REFLEX_CELL_SIZE * (i-1)
		self.widgets.goal[v] = UI:CreateWidget("MatWidget", {rect={0,0,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v]})
		self.widgets.root:AddChild(self.widgets.goal[v])
		-- 120 = count(level.goal) * REFLEX_CELL_SIZE
		UI:CenterWidget(self.widgets.goal[v], UI.screenWidth/2-(#self.state.level.goal)*self.REFLEX_CELL_SIZE/2+xo, self.REFLEX_CELL_SIZE)
		
		-- djr, work in progress
		local cell = { }
		cell.architype = string.gsub(v,"symbol_","cell_")
		table.insert(self.state.victory,cell)
	end
	-- board step: board grid is x,y structure
	self.widgets.board = { }	
	for i,v in ipairs(self.state.level.board) do
		local xo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + v.x * self.REFLEX_CELL_SIZE
		local yo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + v.y * self.REFLEX_CELL_SIZE
			
		self.widgets.board[i] = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v.img]})
		self.widgets.board[i].state = { }
		self.widgets.board[i].state.cell = v
		self.widgets.root:AddChild(self.widgets.board[i])
		UI:CenterWidget(self.widgets.board[i], xo, yo)
	end	
	
	self.state.currentMove = 1
	self.widgets.current = UI:CreateWidget("MatWidget", {rect={200,200,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_current})
	self.widgets.root:AddChild(self.widgets.current)
	self.widgets.lines = { }
	
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE

	self.widgets.spiders = { }
	self.state.antivirusSpawnTimer = self.state.level.antivirusSpiderSpawnRate
	
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
	self.gfx.antivirus_spider = World.Load("Reflex-Game/reflex-antivirus-spider_M")
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
	self.gfx.mark_line_v = self.gfx.mark_current
	self.gfx.mark_line_h = self.gfx.mark_current	
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

function TerminalPuzzles.SetPosition(self,w,x,y)
	local xo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + x * self.REFLEX_CELL_SIZE
	local yo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + y * self.REFLEX_CELL_SIZE

	UI:CenterWidget(w,xo,yo)
	COutLine(kC_Debug,"position line @ x=%.02f,y=%.02f",xo,yo)
end

function TerminalPuzzles.Vec2Normal(self,x,y)
	local n = math.sqrt(x * x + y * y)
	local vec2 = { }
	vec2.x = 0
	vec2.y = 0
	if (n > 0) then
		vec2.x = x / n
		vec2.y = y / n
	end
	return vec2
end

function TerminalPuzzles.LerpWidget(self,widget,heading,dt,speed)	
	local r = widget:Rect()

	local dx = heading.x * dt * speed
	local dy = heading.y * dt * speed
	
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
		r[2] = UI.screenHeight - r[4]/2
	end
	
	local vec2 = { }
	vec2.x = r[1]
	vec2.y = r[2]
	
	return vec2	
end

function TerminalPuzzles.GetGridCell(self,widget)
	local r = widget:Rect()
	r[1] = r[1] + r[3]/2
	r[2] = r[2] + r[4]/2	

	local x = (r[1] - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE
	local y = (r[2] - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE	
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function TerminalPuzzles.IsGridCellOnBoard(self,x,y)
	if (x >= 0 and y >= 0 and x < self.INDEX_MAX_X and y < self.INDEX_MAX_Y) then
		return true
	end
		
	return false
end

function TerminalPuzzles.ConstrainPointToBoard(self,x,y)
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

function TerminalPuzzles.Think(self,dt)

	-- handle player movement
	local currentPos = self:LerpWidget(self.widgets.current,self.state.heading,dt,self.PLAYER_SPEED)
	--COutLine(kC_Debug,"currentPos: x=%.02f, y=%.02f",currentPos.x,currentPos.y)	
	local currentGrid = self:GetGridCell(self.widgets.current)
	currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y)
	--COutLine(kC_Debug,"currentGrid: x=%i, y=%i",currentGrid.x,currentGrid.y)	
	if (self:IsGridCellOnBoard(currentGrid.x,currentGrid.y)) then
		COutLine(kC_Debug,"isOnBoard @ currentGrid: x=%i, y=%i",currentGrid.x,currentGrid.y)
		currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y)
		UI:CenterWidget(self.widgets.current,currentPos.x,currentPos.y)	
		local index = bit.bor(bit.lshift(currentGrid.y, 16), currentGrid.x)
		if (self.widgets.lines[index] == nil) then	
			-- add some logic to select line type H, V or A1, A2, A3, A4 (different turns)
			local line = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_line_v})
			self.widgets.root:AddChild(line)		
			self.widgets.lines[index] = line
			self:SetPosition(line,currentGrid.x,currentGrid.y)
		end
	end		
	
	-- check for victory
	-- TODO: Error on side of player and set victory condition before failure conditions
	
	-- spider spawn timer, move, delete
	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
	if (self.state.antivirusSpawnTimer < 0) then
		self.state.antivirusSpawnTimer = self.state.level.antivirusSpiderSpawnRate		
		local x = math.random(self.INDEX_MAX_X)-1
		local y = math.random(self.INDEX_MAX_Y)-1		
		local index = bit.bor(bit.lshift(y, 16), x)		
		local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.antivirus_spider})
		self.widgets.root:AddChild(spider)	
		self.widgets.spiders[index] = spider
		self:SetPosition(spider,x,y)		
		COutLine(kC_Debug,"spawnedSpider @ currentGrid: x=%i, y=%i",x,y)
		spider.heading = self:Vec2Normal(math.random() * 2 - 1,math.random() * 2 - 1)
		if (spider.heading.x == 0 and spider.heading.y == 0) then -- failsafe
			spider.heading.x = 1
		end
	end	
	local spiderDeleteList = { }	
	for i,v in ipairs(self.widgets.spiders) do	
		local nextPos = self:LerpWidget(v,v.heading,dt,self.state.level.antivirusSpiderSpeed)		
		if (not self:IsGridCellOnBoard(nextPos.x,nextPos.y)) then
			table.insert(spiderDeleteList,v)
		else
			self:SetPosition(v,nextPos.x,nextPos.y)	
			
			-- TODO: detect failure condition
		end	
	end
	for i,v in ipairs(spiderDeleteList) do
		self.widgets.root:RemoveChild(v)
		table.remove(self.state.spiders,v)
	end
	
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
