-- MemoryGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Dave Reese
-- See Abducted/LICENSE for licensing terms

MemoryGame = Class:New()
MemoryGame.active = false

kMGPhase_DiscoverPattern = 0
kMGPhase_MatchTiles = 1

function MemoryGame.DebugStart(self)
    MemoryGame:InitGame("memory-game: debug_start", 1, 1)
    MemoryGame:ShowBoard(true)
    MemoryGame:StartGame()
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

function MemoryGame.CreateLevel1x1(self)
	local level = {}
	
	level.name = "1x1"
	
	level.state = kMGPhase_DiscoverPattern;
	level.rows = 2
	
	level.goal = { "symbol_a", "symbol_b", "symbol_c", "symbol_d" }

	level.board = {
        "symbol_a"
        , "symbol_b"
        , "symbol_c"
        , "symbol_d"
		}
	
	return level
end

function MemoryGame.CreateBoards(self)
	self.db = { }
	self.db.levels = { }    

    -- djr, these are just using the same map system as reflex
	self.db.levels = {  { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		, { self:CreateLevel1x1(), self:CreateLevel1x1(), self:CreateLevel1x1() }
		}
end

function MemoryGame.OnInputEvent(self,e)
	self = MemoryGame.entity

-- djr, disabling this for now
--	if (e.type == kI_KeyDown) then
--		--COutLine(kC_Debug,"key=%i",e.data[1])
--		if (e.data[1] == kKeyCode_I) then
--			--COutLine(kC_Debug,"moving widget")
--			self.state.heading.x = 0
--			self.state.heading.y = -1
--			--UI:MoveWidgetByCenter(self.widgets.current,UI.screenWidth/2, UI.screenHeight/2)
--			return true
--		end
--		if (e.data[1] == kKeyCode_K) then
--			self.state.heading.x = 0
--			self.state.heading.y = 1
--			return true
--		end
--		if (e.data[1] == kKeyCode_J) then
--			self.state.heading.x = -1
--			self.state.heading.y = 0
--			return true
--		end
--		if (e.data[1] == kKeyCode_L) then
--			self.state.heading.x = 1
--			self.state.heading.y = 0
--			return true
--		end
--	end
	
	return false
end

function MemoryGame.OnInputGesture(self,g)
	self = MemoryGame.entity

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
	
	return true
end

function MemoryGame.InitUI(self)
	-- constants
	self.REFLEX_CELL_SIZE = 60
	self.REFLEX_BOARD_OFFSET =80
	self.INDEX_MAX_X = 21
	self.INDEX_MAX_Y = 15
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + 0 * self.REFLEX_CELL_SIZE
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE	

	self:CreateBoards()	
	
	-- define structure: self.state
	local level = self.db.levels[1][1] -- load appropriate level based on skill + difficulty
	self.state = { }	
	self.state.victory = { }
	self.state.current = { }	
	self.state.gameOver = false
	self.state.currentMove = 1
	self.state.gameOverTimer = 2	
	self.state.victory = false
	self.state.level = level
	self.state.goalCounter = 1
	
	-- define structure self.widgets
	self.widgets = {}
	self.widgets.goals = { }
	self.widgets.board = { }		
	self.widgets.grid = { }
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=MemoryGame.OnInputEvent, OnInputGesture=MemoryGame.OnInputGesture})
	World.SetRootWidget(UI.kLayer_TerminalPuzzles, self.widgets.root)
	
	self.widgets.root:SetVisible(false)

-- djr, border might not be needed
--	self.widgets.border = UI:CreateWidget("MatWidget", {rect={0,0,UI.screenWidth,UI.screenHeight}, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect={0,0,UI.screenWidth,UI.screenHeight}, material=self.gfx.board})
	UI:MoveWidgetByCenter(self.widgets.board, UI.screenWidth/2, UI.screenHeight/2)
-- djr, border might not be needed
--	UI:MoveWidgetByCenter(self.widgets.border, UI.screenWidth/2, UI.screenHeight/2)
-- djr, border might not be needed
--	self.widgets.root:AddChild(self.widgets.border)
	self.widgets.root:AddChild(self.widgets.board)

	COutLine(kC_Debug, "memory.level.name=" .. self.state.level.name)
	for i,v in ipairs(self.state.level.goal) do 
		local xo = self.REFLEX_CELL_SIZE/2 + self.REFLEX_CELL_SIZE * (i-1)		
		local goal = UI:CreateWidget("MatWidget", {rect={0,0,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v]})
		goal.state = self:CreateState(v)
		table.insert(self.widgets.goals,goal)
		self.widgets.root:AddChild(goal)
		UI:MoveWidgetByCenter(goal, UI.screenWidth/2-(#self.state.level.goal)*self.REFLEX_CELL_SIZE/2+xo, self.REFLEX_CELL_SIZE)		
	end
	COutLine(kC_Debug, "Creating Board")	
	-- board step: board grid is x,y structure
--    for yo = 0, yo < 2, yo = yo + 1 do
--        for xo = 0, xo < self.INDEX_MAX_X, xo = xo + 1 do
--            local v = self.state.level.board[random(#self.state.level.board)];
--            local b = UI:CreateWidget("MatWidget", {rect={0,0,	self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx[v]})
--            b.state = self:CreateState(v.img)
--            self.widgets.root:AddChild(b)
--            self.widgets.grid[index] = b
--            self:SetPositionByGrid(b,v.x,v.y)
--        end
--    end

	COutLine(kC_Debug, "Board Completed")		
end

function MemoryGame.LoadMaterials(self)
	
	self.gfx = {}
--	self.gfx.antivirus_spider = World.Load("Reflex-Game/reflex-antivirus-spider_M")
	self.gfx.board = World.Load("Memory-Game/memory-board_M")
-- djr, might not need the border seperate from the board
--	self.gfx.border = World.Load("Memory-Game/memory-border_M")

--	self.gfx.cell_a = World.Load("Reflex-Game/reflex-cell-a_M")
--	self.gfx.cell_b = World.Load("Reflex-Game/reflex-cell-b_M")
--	self.gfx.cell_c = World.Load("Reflex-Game/reflex-cell-c_M")
--	self.gfx.cell_d = World.Load("Reflex-Game/reflex-cell-d_M")
--
--	self.gfx.cell_green = World.Load("Reflex-Game/reflex-cell-green_M")
--
--	self.gfx.mark_current = World.Load("Reflex-Game/reflex-mark-current_M")
--	self.gfx.mark_line_v = self.gfx.mark_current
--	self.gfx.mark_line_h = self.gfx.mark_current
--	self.gfx.mark_end = World.Load("Reflex-Game/reflex-mark-end_M")
--	self.gfx.mark_start = World.Load("Reflex-Game/reflex-mark-start_M")

--	self.gfx.symbol_a = World.Load("Memory-Game/memory-symbol-a_M")
--	self.gfx.symbol_b = World.Load("Memory-Game/memory-symbol-b_M")
--	self.gfx.symbol_c = World.Load("Memory-Game/memory-symbol-c_M")
--	self.gfx.symbol_d = World.Load("Memory-Game/memory-symbol-d_M")
	
	self.typefaces = {}
	self.typefaces.BigText = World.Load("UI/TerminalPuzzlesBigFont_TF")				
end

function MemoryGame.SetPositionByGrid(self,w,x,y)
	local xo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + x * self.REFLEX_CELL_SIZE
	local yo = self.REFLEX_BOARD_OFFSET + self.REFLEX_CELL_SIZE/2 + y * self.REFLEX_CELL_SIZE

	UI:MoveWidgetByCenter(w,xo,yo)
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

function MemoryGame.IsGridCellOnBoard(self,x,y)
	if (x >= 0 and y >= 0 and x < self.INDEX_MAX_X and y < self.INDEX_MAX_Y) then
		return true
	end
		
	return false
end

function MemoryGame.ConvertCoordToIndex(self,x,y)
	return bit.bor(bit.lshift(y, 16), x)
end

function MemoryGame.ConstrainPointToBoard(self,x,y)
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

function MemoryGame.GetPosition(self,w)
	local r = w:Rect()
	local vec2 = { } 
	vec2.x = r[1] + r[3]/2
	vec2.y = r[2] + r[4]/2	
	return vec2
end


function MemoryGame.CreateState(self,architype)
	local state = { }
	state.architype = architype
	return state
end

function MemoryGame.SetLineSegmentPosition(self,line,startPos,endPos)
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

function MemoryGame.CollideWithLine(self,x,y,ignore)
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

function MemoryGame.CollideWithBoard(self,x,y,isPlayer)
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

--  djr, this isn't relevant
--	if (isPlayer) then
--		if (piece.state.architype == "mark_end" or piece.state.architype == "mark_start") then
--			return false
--		end
--
--		if (piece.state.architype == "cell_a"
--			or piece.state.architype == "cell_b"
--			or piece.state.architype == "cell_c"
--			or piece.state.architype == "cell_d"
--		) then
--			return false
--		end
--	end
	
	COutLine(kC_Debug,"CollideWihtBoard found Piece @ x=%i, y=%i, type=%s",x,y,piece.state.architype)			
	return true
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
				
		-- game over never advances past here
		return
	end

--  djr, this isn't relevant
--	if (self.state.heading.x == 0 and self.state.heading.y == 0) then
--		-- NO HEADING: Game hasn't started
--		return
--	end
--
--	if (self.state.lastHeading.x == 0 and self.state.lastHeading.y == 0) then
--		self.state.lastHeading.x = self.state.heading.x
--		self.state.lastHeading.y = self.state.heading.y
--	end
--
--	if (self.widgets.current.heading == nil) then
--		self.widgets.current.heading = self.state.heading
--	end
--
--	local currentPos = self:LerpVec2(self.widgets.current.state.endPos,self.state.lastHeading,dt,self.PLAYER_SPEED)
--	if (self:CollideWithBoard(currentPos.x,currentPos.y,true)) then
--		COutLine(kC_Debug,"GameOver player collided with board @ : x=%i, y=%i",currentPos.x,currentPos.y)
--		self.state.gameOver = true
--		return
--	end
--
--	--COutLine(kC_Debug,"currentPos: x=%.02f, y=%.02f",currentPos.x,currentPos.y)
--	currentPos = self:ConstrainPointToBoard(currentPos.x,currentPos.y)
--	self.widgets.current.state.endPos = currentPos
--	self:SetLineSegmentPosition(self.widgets.current,self.widgets.current.state.startPos,self.widgets.current.state.endPos)
--
--	-- detect change of direction
--	if (self.state.lastHeading.x ~= self.state.heading.x or self.state.lastHeading.y ~= self.state.heading.y) then
--		local oldR = self.widgets.current:Rect()
--		COutLine(kC_Debug,"oldLineSegment @ x=%i, y=%i, width=%i, height=%i",oldR[1],oldR[2],oldR[3],oldR[4])
--		COutLine(kC_Debug,"newLineSegment @ currentPos: x=%i, y=%i",currentPos.x,currentPos.y)
--		local line = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.mark_line_v})
--		line.state = self:CreateState("mark_line_v")
--		table.insert(self.widgets.lines,line)
--		self.widgets.current = line
--		self.widgets.root:AddChild(line)
--
--		line.state.startPos = currentPos
--		line.state.endPos = line.state.startPos
--		self:SetLineSegmentPosition(line,line.state.startPos,line.state.endPos)
--
--		self.state.lastHeading.x = self.state.heading.x
--		self.state.lastHeading.y = self.state.heading.y
--	end
--
--	local playerGridCell = self:GetGridCellFromVec2(currentPos)
--	local playerIndex = self:ConvertCoordToIndex(playerGridCell.x,playerGridCell.y)
--
--	local pieceAtPlayer = self.widgets.grid[playerIndex]
--	if (pieceAtPlayer) then
--		if (self.state.goalCounter < #self.widgets.goals) then
--			local goalPiece = self.widgets.goals[self.state.goalCounter]
--			if (goalPiece.state.architype == pieceAtPlayer.state.architype) then
--				COutLine(kC_Debug,"Goal accomplished: x=%s",goalPiece.state.architype)
--				self.state.goalCounter = self.state.goalCounter + 1
--				-- TODO: -djr Question: How do they want to indicate that you activated a box, just
--				-- swap out the cell with another cell?
--			end
--		end
--
--		if (pieceAtPlayer.state.architype == "mark_end" and self.state.goalCounter >= #self.widgets.goals) then
--			COutLine(kC_Debug,"Game Over Detected")
--			self.state.gameOver = true
--			self.state.victory = true
--			return
--		end
--	end
--	if (self:CollideWithLine(currentPos.x,currentPos.y,true)) then
--		COutLine(kC_Debug,"Game Over - collided with own line")
--		self.state.gameOver = true
--		return
--	end
--
--	--COutLine(kC_Debug,"antivirusSpawnTimer=%i, dt=%f, rate=%i",self.state.antivirusSpawnTimer,dt,self.state.level.antivirusSpiderSpawnRate)
--	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
--	if (self.state.antivirusSpawnTimer < 0) then
--		self.state.antivirusSpawnTimer = self.state.level.antivirusSpiderSpawnRate
--		local x = math.random(self.INDEX_MAX_X)-1
--		local y = math.random(self.INDEX_MAX_Y)-1
--		local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.REFLEX_CELL_SIZE,self.REFLEX_CELL_SIZE}, material=self.gfx.antivirus_spider})
--		self.widgets.root:AddChild(spider)
--		table.insert(self.widgets.spiders,spider)
--		self:SetPositionByGrid(spider,x,y)
--		spider.state = self:CreateState("antivirus_spider")
--		spider.state.heading = self:Vec2Normal(math.random() * 2 - 1,math.random() * 2 - 1)
--		if (spider.state.heading.x == 0 and spider.state.heading.y == 0) then -- failsafe
--			spider.state.heading.x = 1
--		end
--		COutLine(kC_Debug,"spawnedSpider @ grid: x=%i, y=%i, heading = %.04f,%.04f",x,y,spider.state.heading.x,spider.state.heading.y)
--	end
--
--	--COutLine(kC_Debug,"Spider move")
--	for i,k in pairs(self.widgets.spiders) do
--		local pos = k:Rect()
--		COutLine(kC_Debug,"pos @ : x=%i, y=%i, dt=%.04f, heading = %.04f,%.04f, speed=%i",pos[1]+pos[3]/2,pos[2]+pos[4]/2,dt,k.state.heading.x,k.state.heading.y,self.state.level.antivirusSpiderSpeed)
--		local nextPos = self:LerpWidget(k,k.state.heading,dt,self.state.level.antivirusSpiderSpeed)
--		if (self:CollideWithBoard(nextPos.x,nextPos.y,false)) then
--			table.remove(self.widgets.spiders,i)
--			self.widgets.root:RemoveChild(k)
--			COutLine(kC_Debug,"remove spider @ : x=%i, y=%i",nextPos.x,nextPos.y)
--		else
--			UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
--			--COutLine(kC_Debug,"move spider to: x=%.02f, y=%.02f",nextPos.x,nextPos.y)
--			-- TODO: detect spider crossing a line segment
--			if (self:CollideWithLine(nextPos.x,nextPos.y,false)) then
--				COutLine(kC_Debug,"Game Over - spider crossed player line")
--				self.state.gameOver = true
--				return
--			end
--		end
--	end
end

memory_game = MemoryGame
