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
	
	level.antivirusSpiderSpawnRate = 25
	level.antivirusSpiderSpeed = 4
	
	level.goal = { "symbol_a", "symbol_b", "symbol_c", "symbol_d" }

	level.board = {  { x=0, y=0, img="cell_green" }
		, { x=10, y=3, img="cell_green" }
		, { x=5, y=5, img="cell_green" }
		, { x=3, y=6, img="cell_a" }		
		, { x=5, y=6, img="cell_b" }
		, { x=7, y=6, img="cell_d" }
		, { x=9, y=6, img="cell_e" }

		, { x=0, y=9, img="mark_start" }
		, { x=self.INDEX_MAX_X-1, y=1, img="mark_end" }				
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
	self.PLAYER_SPEED = 60
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE	

	self:CreateBoards()	
	
	-- define structure: self.state
	local level = self.db.levels[1][1] -- load appropriate level based on skill + difficulty
	self.state = { }	
	self.state.heading = { }
	self.state.lastHeading = { }
	self.state.victory = { }
	self.state.current = { }	
	self.state.heading.x = 0
	self.state.heading.y = 0
	self.state.lastHeading.x = 0
	self.state.lastHeading.y = 0
	self.state.gameOver = false
	self.state.currentMove = 1
	self.state.gameOverTimer = 5	
	self.state.level = level
	self.state.spawnTimer = level.antivirusSpiderSpawnRate	
	self.state.antivirusSpawnTimer = level.antivirusSpiderSpawnRate
	
	-- define structure self.widgets
	self.widgets = {}
	self.widgets.goal = { }
	self.widgets.board = { }		
	self.widgets.lines = { }
	self.widgets.spiders = { }
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=UI.EatInput})
	World.SetRootWidget(TerminalPuzzles.UI_Layer, self.widgets.root)
	self.widgets.border = UI:CreateWidget("MatWidget", {rect={0,0,UI.screenWidth,UI.screenHeight}, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect={self.REFLEX_BOARD_OFFSET,self.REFLEX_BOARD_OFFSET,UI.screenWidth-self.REFLEX_BOARD_OFFSET*2,UI.screenHeight-self.REFLEX_BOARD_OFFSET*2}, material=self.gfx.board})
	UI:CenterWidget(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
	UI:CenterWidget(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
	self.widgets.root:AddChild(self.widgets.border)	
	self.widgets.root:AddChild(self.widgets.board)	
		
	COutLine(kC_Debug, "reflex.level.name=" .. self.state.level.name)
	-- goal step: center a,b,c,d list ot top of screen
	local counter = 0
	for i,v in ipairs(self.state.level.goal) do 
		local xo = self.REFLEX_CELL_SIZE/2 + self.REFLEX_CELL_SIZE * (i-1)		
		local goal = UI:CreateWidget("MatWidget", {rect={0,0,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v]})
		goal.state = self:CreateState(string.gsub(v,"symbol_","cell_"))
		--self.widgets.goal[counter] = goal
		self.widgets.root:AddChild(goal)
		UI:CenterWidget(goal, UI.screenWidth/2-(#self.state.level.goal)*self.REFLEX_CELL_SIZE/2+xo, self.REFLEX_CELL_SIZE)		
		counter = counter + 1
	end
	COutLine(kC_Debug, "Creating Board")	
	-- board step: board grid is x,y structure
	for i,v in ipairs(self.state.level.board) do
		local b = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v.img]})
		local index = self:ConvertCoordToIndex(v.x,v.y)
		b.state = self:CreateState(v.img)
		self.widgets.root:AddChild(b)
		self.widgets.board[index] = b		
		self:SetPositionByGrid(b,v.x,v.y)
		if (v.img == "mark_start") then
			local current = UI:CreateWidget("MatWidget", {rect={200,200,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_current})		
			current.state = self:CreateState("mark_current")
			self.widgets.current = current
			self.widgets.root:AddChild(current)
			self:SetPositionByGrid(current,v.x,v.y)	
			self.state.playerIndex = index
			current.state.startPos = self:GetPosition(current) 
			current.state.endPos = current.state.startPos
			self:SetLineSegmentPosition(current,current.state.startPos,current.state.endPos)			
			COutLine(kC_Debug,"current widget: x=%i, y=%i",v.x,v.y)	
		end
	end
	
	if (self.widgets.current == nil) then
		COutLine(kC_Debug,"mark_start NOT FOUND --> ERROR")	
	end
	
	COutLine(kC_Debug, "Board Completed")		
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
	self.gfx.mark_end = World.Load("Reflex-Game/reflex-mark-end_M")						
	self.gfx.mark_start = World.Load("Reflex-Game/reflex-mark-start_M")							
	self.gfx.symbol_a = World.Load("Reflex-Game/reflex-symbol-a_M")
	self.gfx.symbol_b = World.Load("Reflex-Game/reflex-symbol-b_M")
	self.gfx.symbol_c = World.Load("Reflex-Game/reflex-symbol-c_M")
	self.gfx.symbol_d = World.Load("Reflex-Game/reflex-symbol-d_M")

	self.typefaces = {}
	self.typefaces.BigText = World.Load("UI/TerminalPuzzlesBigFont_TF")				
end

function TerminalPuzzles.StartGame(self, gameType, gameCompleteCallback)
	self.gameCompleteCallback = gameCompleteCallback
end

function TerminalPuzzles.SetPositionByGrid(self,w,x,y)
	local xo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + x * self.REFLEX_CELL_SIZE
	local yo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + y * self.REFLEX_CELL_SIZE

	UI:CenterWidget(w,xo,yo)
	--COutLine(kC_Debug,"position line @ x=%.02f,y=%.02f",xo,yo)
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

function TerminalPuzzles.LerpVec2(self,v,heading,dt,speed)	
	local dx = heading.x * dt * speed
	local dy = heading.y * dt * speed
	
	local x = v.x + dx
	local y = v.y + dy
		
	local vec2 = { }
	vec2.x = x
	vec2.y = y
	
	return vec2	
end

function TerminalPuzzles.LerpWidget(self,widget,heading,dt,speed)	
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
		wv.x = 0
	end
	if (wv.x + width > UI.screenWidth) then
		wv.x = UI.screenWidth - half_width
	end
	if (wv.y < half_height) then
		wv.y = 0
	end	
	if (wv.y + height > UI.screenHeight) then
		wv.y = UI.screenHeight - half_height
	end
	
	return wv
end

function TerminalPuzzles.GetGridCellFromVec2(self,v)
	local x = (v.x - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE
	local y = (v.y - self.REFLEX_BOARD_OFFSET)/self.REFLEX_CELL_SIZE	
	local ix = math.floor(x)
	local iy = math.floor(y)

	local vec2 = { }
	vec2.x = ix
	vec2.y = iy
	
	return vec2
end

function TerminalPuzzles.GetGridCell(self,widget)
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

function TerminalPuzzles.IsGridCellOnBoard(self,x,y)
	if (x >= 0 and y >= 0 and x < self.INDEX_MAX_X and y < self.INDEX_MAX_Y) then
		return true
	end
		
	return false
end

function TerminalPuzzles.ConvertCoordToIndex(self,x,y)
	return bit.bor(bit.lshift(y, 16), x)
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

function TerminalPuzzles.GetPosition(self,w)
	local r = w:Rect()
	local vec2 = { } 
	vec2.x = r[1] + r[3]/2
	vec2.y = r[2] + r[4]/2	
	return vec2
end


function TerminalPuzzles.CreateState(self,architype)
	local state = { }
	state.architype = architype
	return state
end

function TerminalPuzzles.SetLineSegmentPosition(self,line,startPos,endPos)
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

function TerminalPuzzles.Think(self,dt)
	-- TODO: -djr, -joe this should be in seconds but is currently in milliseconds (fix in main branch)
	dt = dt / 1000

	if (self.state.gameOver) then
		self.state.gameOverTimer = self.state.gameOverTimer - dt
		if (self.state.gameOverTimer < 0) then
			self.state.gameOverTimer = 0
			self.gameCompleteCallback();
		end
		
		if (self.widgets.bigTextLabel == nil) then 
			self.widgets.bigTextLabel = UI:CreateWidget("TextLabel", {rect={0, 0, 0, 0}, typeface=self.typefaces.BigText})
			self.widgets.root:AddChild(self.widgets.bigTextLabel)		
		end
		--self.widgets.bigTextLabel:SetText(string.fromat("%i",self.state.gameOverTimer))
		self.widgets.bigTextLabel:SetText("Game Over")
		UI:CenterLabel(self.widgets.bigTextLabel)
				
		-- game over never advances past here
		return
	end

	if (self.state.heading.x == 0 and self.state.heading.y == 0) then
		-- NO HEADING: Game hasn't started		
		return
	end
	
	if (self.state.lastHeading.x == 0 and self.state.lastHeading.y == 0) then
		self.state.lastHeading.x = self.state.heading.x
		self.state.lastHeading.y = self.state.heading.y
	end
	
	if (self.widgets.current.heading == nil) then
		self.widgets.current.heading = self.state.heading
	end

	local currentPos = self:LerpVec2(self.widgets.current.state.endPos,self.state.heading,dt,self.PLAYER_SPEED)
	--COutLine(kC_Debug,"currentPos: x=%.02f, y=%.02f",currentPos.x,currentPos.y)	
	local currentGrid = self:GetGridCellFromVec2(currentPos)
	--COutLine(kC_Debug,"currentGrid: x=%i, y=%i",currentGrid.x,currentGrid.y)	
	if (self:IsGridCellOnBoard(currentGrid.x,currentGrid.y)) then
		--COutLine(kC_Debug,"isOnBoard @ currentGrid: x=%i, y=%i",currentGrid.x,currentGrid.y)
		currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y)
		self.widgets.current.state.endPos = currentPos
		self:SetLineSegmentPosition(self.widgets.current,self.widgets.current.state.startPos,self.widgets.current.state.endPos)
		
		-- detect change of direction
		if (self.state.lastHeading.x ~= self.state.heading.x or self.state.lastHeading.y ~= self.state.heading.y) then
			COutLine(kC_Debug,"newLineSegment @ currentGrid: x=%i, y=%i",currentPos.x,currentPos.y)
			local line = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_line_v})
			line.state = self:CreateState("mark_line_v")
			self.widgets.lines[self.state.playerIndex] = line
			self.widgets.current = line
			self.widgets.root:AddChild(line)		
						
			line.state.startPos = currentPos
			line.state.endPos = line.state.startPos
			self:SetLineSegmentPosition(line,line.state.startPos,line.state.endPos)
			
			self.state.lastHeading.x = self.state.heading.x
			self.state.lastHeading.y = self.state.heading.y
		end
		
--[[
		UI:CenterWidget(self.widgets.current,currentPos.x,currentPos.y)	

		if (self.widgets.lines[self.state.playerIndex] == nil) then	
			COutLine(kC_Debug,"line added: x=%i, y=%i",currentGrid.x,currentGrid.y)	
			-- TODO: -djr change this out with a rectangle stretch. create a new rect on a direction change
			local line = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_line_v})
			line.state = self:CreateState("mark_line_v")
			self.widgets.root:AddChild(line)		
			self.widgets.lines[self.state.playerIndex] = line
			self:SetPositionByGrid(line,currentGrid.x,currentGrid.y)
		end
]]		
	end		
	
	local pieceAtPlayer = self.widgets.board[self.state.playerIndex]	
	if (pieceAtPlayer) then
		COutLine(kC_Debug,"pieceAtPlayer found")
		if (pieceAtPlayer.state.architype =="mark_end") then
			COutLine(kC_Debug,"Game Over Detected")
			self.state.gameOver = true
			return
		end	
	end
	
	--COutLine(kC_Debug,"antivirusSpawnTimer=%i, dt=%f, rate=%i",self.state.antivirusSpawnTimer,dt,self.state.level.antivirusSpiderSpawnRate)
	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
	if (self.state.antivirusSpawnTimer < 0) then
		self.state.antivirusSpawnTimer = self.state.level.antivirusSpiderSpawnRate		
		local x = math.random(self.INDEX_MAX_X)-1
		local y = math.random(self.INDEX_MAX_Y)-1		
		local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.antivirus_spider})
		self.widgets.root:AddChild(spider)	
		table.insert(self.widgets.spiders,spider)
		self:SetPositionByGrid(spider,x,y)		
		spider.state = self:CreateState("antivirus_spider")
		spider.state.heading = self:Vec2Normal(math.random() * 2 - 1,math.random() * 2 - 1)
		if (spider.state.heading.x == 0 and spider.state.heading.y == 0) then -- failsafe
			spider.state.heading.x = 1
		end
		COutLine(kC_Debug,"spawnedSpider @ currentGrid: x=%i, y=%i, heading = %.04f,%.04f",x,y,spider.heading.x,spider.heading.y)
	end	
	
	--COutLine(kC_Debug,"Spider move")
	for i,v in ipairs(self.widgets.spiders) do	
		local pos = v:Rect()
		--COutLine(kC_Debug,"pos @ : x=%i, y=%i",pos[1],pos[2])			
		local nextPos = self:LerpWidget(v,v.state.heading,dt,self.state.level.antivirusSpiderSpeed)
		local nextGrid = self:GetGridCellFromVec2(nextPos)
		if (not self:IsGridCellOnBoard(nextGrid.x,nextGrid.y)) then
			table.remove(self.widgets.spiders,i)
			self.widgets.root:RemoveChild(v)
			COutLine(kC_Debug,"remove spider @ : x=%i, y=%i",nextPos.x,nextPos.y)			
		else
			UI:CenterWidget(v,nextPos.x,nextPos.y)
			--COutLine(kC_Debug,"move spider to: x=%.02f, y=%.02f",nextPos.x,nextPos.y)		
			-- TODO: detect spider crossing a line segment
		end	
	end	
end

terminal_puzzles = TerminalPuzzles
