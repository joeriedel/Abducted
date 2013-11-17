-- ReflexGame.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Dave Reese
-- See Abducted/LICENSE for licensing terms

ReflexGame = Class:New()
ReflexGame.active = false

function ReflexGame.DebugStart(self)
	Abducted.entity.eatInput = true
	UI:BlendTo({1,1,1,1}, 0.3)
	local f = function()
		local f = function()
			UI:BlendTo({1,1,1,1}, 0.3)
			local f = function()
				UI:BlendTo({0,0,0,0}, 0.3)
				ReflexGame:ShowBoard(false)
				ReflexGame:ResetGame()
				collectgarbage()
				Abducted.entity.eatInput = false
			end
			World.globalTimers:Add(f, 0.3)
		end
		ReflexGame:InitGame(3)
		UI:BlendTo({0,0,0,0}, 0.3)
		ReflexGame:ShowBoard(true)
		ReflexGame:StartGame("discover Bugs;award 6;message ARM_REWARD_MSG_POWER_RESTORED;unlock_topic LockedTest ARM_REWARD_LOCKEDTEST", f)
	end
	World.globalTimers:Add(f, 0.3)
end

function ReflexGame.Spawn(self)
	ReflexGame.entity = self
	
end

function ReflexGame.PostSpawn(self)

	if (Game.entity.type == "Map") then
		self:LoadMaterials()
		self:InitUI()
		PuzzleScoreScreen:Init(self.screen)
		-- Note: InitUI sets widgets.root to be invisible (hides the whole board)
	end
	
end

function ReflexGame.ShowBoard(self, show)
	local self = ReflexGame.entity
	-- NOTE: show board is called *after* InitGame
	-- InitGame should get the board ready to be seen
	self.widgets.root:SetVisible(show)
	self.widgets.root2:SetVisible(show)
	self.widgets.root3:SetVisible(show)
	self.widgets.root4:SetVisible(show)
	World.SetDrawUIOnly(show) -- < disable/enable 3D rendering
end

function ReflexGame.InitGame(self, terminalSkill)
	local self = ReflexGame.entity
	self.think = nil
	self.gameReady = false
	self.tickTimer = false
	self.animatingRetry = false
	self.exiting = false
	
	self.skill = Clamp(terminalSkill, 1, #self.db.levels)
	
	local levelBank = self.db.levels[self.skill]
	self.level = levelBank[IntRand(1, #levelBank)]
	
	-- InitGame: prep the board to be shown with ShowBoard
	-- but we should not start until StartGame is called.
	self:CreateBoard()
	self.state.timeLeft = self.level.time
	self.widgets.timeLeftLabel:SetTypeface(self.typefaces.TimerText)
	self:UpdateHud()
	
	if (self.timerTask == nil) then
		self.timerTask = World.globalTimers:Add(function (dt) self:UpdateTimer(dt) end, 0, true)
	end
end

function ReflexGame.StartGame(self, actions, gameCompleteCallback)
	local self = ReflexGame.entity
	
	ReflexGame.active = true
	self.gameCompleteCallback = gameCompleteCallback
	self.actions = actions
	
	World.FlushInput(true)
	
	self:AnimateIntro()
end

function ReflexGame.EndGame(self, result)
	self.think = nil
	self.exiting = true
	
	if (self.timerTask) then
		self.timerTask:Clean()
		self.timerTask = nil
	end
	
	World.FlushInput(true)
	
	if (result == "f") then
		EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", "HACK_TERMINAL_FAIL")
	end
	
	self.sfx.Enter:Play(kSoundChannel_UI, 0)
	
	if (self.gameCompleteCallback) then
		self.gameCompleteCallback(result)
	end

end

function ReflexGame.ResetGame(self)
	self = ReflexGame.entity
	PuzzleScoreScreen:Unlink()
	self:DeInitUI()
	-- clean up game-board, get ready for another ShowBoard/StartGame call sometime in the future.
	-- NOTE: the terminal puzzle UI is hidden right now
	ReflexGame.active = false
end

function ReflexGame.LoadLevels(self)
	self.db = { }
	self.db.levels = { }    
	
	self.db.levels = {  
		{ self:CreateLevel1x1(), self:CreateLevel1x2(), self:CreateLevel1x3(), self:CreateLevel1x4() },
		{ self:CreateLevel2x1(), self:CreateLevel2x2(), self:CreateLevel2x3(), self:CreateLevel2x4() },
		{ self:CreateLevel3x1(), self:CreateLevel3x2(), self:CreateLevel3x3(), self:CreateLevel3x4() },
		{ self:CreateLevel4x1(), self:CreateLevel4x2(), self:CreateLevel4x3(), self:CreateLevel4x4() }
		}
end

function ReflexGame.OnInputEvent(self,e)
	self = ReflexGame.entity
	
	if ((self.gameReady) and (e.type == kI_KeyDown)) then
		if (e.data[1] == kKeyCode_Up) then
			self.state.heading.x = 0
			self.state.heading.y = -1
		elseif (e.data[1] == kKeyCode_Down) then
			self.state.heading.x = 0
			self.state.heading.y = 1
		elseif (e.data[1] == kKeyCode_Left) then
			self.state.heading.x = -1
			self.state.heading.y = 0
		elseif (e.data[1] == kKeyCode_Right) then
			self.state.heading.x = 1
			self.state.heading.y = 0
		end
		return true
	end
	
	return false
end

function ReflexGame.DPadInput(widget, e)
	local self = ReflexGame.entity

	if (Input.IsTouchBegin(e)) then
		if (self.dpadTouch == nil) then
			self.widgets.dpadRoot:SetCapture(true)
			self.dpadTouch = e.touch
			self.dpadInput = {0, 0}
			self.dpadDir = {0, 0}
			self.dpadOrder = 0
		else
			return true
		end
	elseif (self.dpadTouch and Input.IsTouchEnd(e, self.dpadTouch)) then
		self.dpadTouch = nil
		self.widgets.dpadRoot:SetCapture(false)
		self.widgets.dpad.left:SetMaterial(self.gfx.RightArrow)
		self.widgets.dpad.right:SetMaterial(self.gfx.RightArrow)
		self.widgets.dpad.up:SetMaterial(self.gfx.RightArrow)
		self.widgets.dpad.down:SetMaterial(self.gfx.RightArrow)
		return true
	elseif (self.dpadTouch == nil) then
		return true
	end

	local size = self.widgets.dpadRoot.size / 2
	local dx = e.data[1] - size
	local dy = e.data[2] - size
	
	-- deadband
	if (math.abs(dx) < (size/3)) then
		dx = 0
	end
	
	if (math.abs(dy) < (size/3)) then
		dy = 0
	end
	
	if (dx < 0) then
		dx = -1
	elseif (dx > 0) then
		dx = 1
	else
		dx = 0
	end
	
	if (dy < 0) then
		dy = -1
	elseif (dy > 0) then
		dy = 1
	else
		dy = 0
	end
	
	-- impulse changes?
	if ((dx == 0) and (self.dpadInput[1] ~= 0)) then -- xinput centered, apply Y
		self.dpadInput[2] = 0 -- clear y input to force impulse
	end
	
	if ((dy == 0) and (self.dpadInput[2] ~= 0)) then -- yinput cleared, apply X
		self.dpadInput[1] = 0 -- clear x input to force impulse
	end
	
	local ddx = 0
	local ddy = 0
	
	if (dx ~= self.dpadInput[1]) then
		ddx = dx
		self.dpadInput[1] = dx
	end
	
	if (dy ~= self.dpadInput[2]) then
		ddy = dy
		self.dpadInput[2] = dy
	end
	
	if (ddx == self.dpadDir[1]) then
		ddx = 0
	end
	
	if (ddy == self.dpadDir[2]) then
		ddy = 0
	end
		
	if (ddx ~= 0) then -- x input changed
		self.dpadDir[1] = ddx
		self.dpadDir[2] = 0
	
		self.state.heading.x = ddx
		self.state.heading.y = 0
		
		self.sfx.DPad:Play(kSoundChannel_UI, 0)
		
		self.widgets.dpad.up:SetMaterial(self.gfx.RightArrow)
		self.widgets.dpad.down:SetMaterial(self.gfx.RightArrow)
		
		if (ddx < 0) then
			self.widgets.dpad.left:SetMaterial(self.gfx.RightArrowPressed)
			self.widgets.dpad.right:SetMaterial(self.gfx.RightArrow)
		else
			self.widgets.dpad.left:SetMaterial(self.gfx.RightArrow)
			self.widgets.dpad.right:SetMaterial(self.gfx.RightArrowPressed)
		end
	elseif (ddy ~= 0) then
		self.dpadDir[1] = 0
		self.dpadDir[2] = ddy
		
		self.state.heading.x = 0
		self.state.heading.y = ddy
		
		self.sfx.DPad:Play(kSoundChannel_UI, 0)
		
		self.widgets.dpad.left:SetMaterial(self.gfx.RightArrow)
		self.widgets.dpad.right:SetMaterial(self.gfx.RightArrow)
		
		if (ddy < 0) then
			self.widgets.dpad.up:SetMaterial(self.gfx.RightArrowPressed)
			self.widgets.dpad.down:SetMaterial(self.gfx.RightArrow)
		else
			self.widgets.dpad.up:SetMaterial(self.gfx.RightArrow)
			self.widgets.dpad.down:SetMaterial(self.gfx.RightArrowPressed)
		end
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

    self.gfx.blocker_green = {
		World.Load("Puzzles/reflex-block1_M"),
		World.Load("Puzzles/reflex-block2_M"),
		World.Load("Puzzles/reflex-block3_M"),
		World.Load("Puzzles/reflex-block4_M")
	}

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
	self.gfx.SpiderParticles = World.Load("Puzzles/spiderparticles_M")
	self.gfx.GoalOpen = World.Load("Puzzles/reflex-goal_open_M")
	self.gfx.SwipeBar = World.Load("UI/lineborder3_M")
	self.gfx.Outline = World.Load("UI/lineborder1_M")
	self.gfx.ItemBackground = World.Load("UI/MMItemBackground_M")
	
	self.typefaces = {}
    self.typefaces.TimerText = World.Load("UI/TerminalPuzzlesTimeFont_TF")
    self.typefaces.TimerTextFlash = World.Load("UI/TerminalPuzzlesTimeFontFlash_TF")
    self.typefaces.TimerTextRed = World.Load("UI/TerminalPuzzlesTimeFontRed_TF")
    self.typefaces.SwipeToMoveText = World.Load("UI/TerminalPuzzlesFont_TF")

	self.sfx = {}
	self.sfx.Enter = World.LoadSound("Puzzles/Enter")
	self.sfx.Success = World.LoadSound("Puzzles/Success")
	self.sfx.Fail = World.LoadSound("Puzzles/Fail")
	self.sfx.Complete = World.LoadSound("Puzzles/Complete")
    self.sfx.DPad = World.LoadSound("Audio/dpad")
    self.sfx.SpikeBombEnter = World.LoadSound("Puzzles/SpikeBombEnter", 4)
	self.sfx.Reveal = World.LoadSound("Puzzles/Reveal")
	self.sfx.Acquired = World.LoadSound("Puzzles/GlyphAcquired", 4)
	self.sfx.Explode = World.LoadSound("Puzzles/Explode", 4)
	self.sfx.Suck = World.LoadSound("Puzzles/Suck", 4)
	
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
    self.LINE_SPAWN_TIME = 0.5
	self.PLAYER_SPEED = 140
	self.COORD_MIN_X = self.REFLEX_BOARD_OFFSET[1]
	self.COORD_MIN_Y = self.REFLEX_BOARD_OFFSET[2]
	self.COORD_MAX_X = self.REFLEX_BOARD_OFFSET[1] + self.REFLEX_CELL_SIZE[1] + self.INDEX_MAX_X * self.REFLEX_CELL_SIZE[1]
	self.COORD_MAX_Y = self.REFLEX_BOARD_OFFSET[2] + self.REFLEX_CELL_SIZE[2] + self.INDEX_MAX_Y * self.REFLEX_CELL_SIZE[2]	

	self:LoadLevels()	
		
	-- define structure self.widgets
	self.widgets = {}
	
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=ReflexGame.OnInputEvent})
	World.SetRootWidget(UI.kLayer_HackGame, self.widgets.root)
	self.widgets.root:SetOpaqueLayerInput(true) -- no input goes past this
	
	self.widgets.root:SetVisible(false)
	
	self.widgets.root2 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
	World.SetRootWidget(UI.kLayer_HackGame2, self.widgets.root2)
	
	self.widgets.root2:SetVisible(false)
	
	self.widgets.root3 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
	World.SetRootWidget(UI.kLayer_HackGame3, self.widgets.root3)
	
	self.widgets.root3:SetVisible(false)
	
	self.widgets.root4 = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
	World.SetRootWidget(UI.kLayer_HackGame4, self.widgets.root4)
	
	self.widgets.root4:SetVisible(false)
	
	self.widgets.black = UI:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=UI.gfx.Solid})
	self.widgets.root4:AddChild(self.widgets.black)
	
	if (UI.mode == kGameUIMode_Mobile) then
		self.widgets.dpad = {}
		
		local dpadButtonSize = 64*UI.identityScale[1]*UI.fontScale[1]
		local dpadSize = dpadButtonSize*3
		local dpadPad = 16*UI.identityScale[1]
		local dpadLeft = UI.screenWidth-dpadSize-(48*UI.identityScale[1])
		local dpadTop = UI.screenHeight-dpadSize-(72*UI.identityScale[1])
		
		self.widgets.dpadRoot = UI:CreateWidget("Widget",
			{rect={dpadLeft, dpadTop, dpadSize, dpadSize}, OnInputEvent=ReflexGame.DPadInput}
		)
		self.widgets.dpadRoot.size = dpadSize
		self.widgets.root3:AddChild(self.widgets.dpadRoot)
		
		self.widgets.dpad.left = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft-dpadPad, dpadTop+dpadButtonSize, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow}
		)
		
		self.widgets.dpad.left:RotateTo({dpadButtonSize/2, dpadButtonSize/2, 180}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.left)
		
		self.widgets.dpad.right = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadSize-dpadButtonSize+dpadPad, dpadTop+dpadButtonSize, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow}
		)
				
		self.widgets.root3:AddChild(self.widgets.dpad.right)
		
		self.widgets.dpad.up = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadButtonSize, dpadTop-dpadPad, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow}
		)
		
		self.widgets.dpad.up:RotateTo({dpadButtonSize/2, dpadButtonSize/2, -90}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.up)
		
		self.widgets.dpad.down = UI:CreateWidget(
			"MatWidget", 
			{rect={dpadLeft+dpadButtonSize, dpadTop+dpadSize-dpadButtonSize+dpadPad, dpadButtonSize, dpadButtonSize}, 
			 material=self.gfx.RightArrow}
		)
		
		self.widgets.dpad.down:RotateTo({dpadButtonSize/2, dpadButtonSize/2, 90}, {0,0,0})
		self.widgets.root3:AddChild(self.widgets.dpad.down)
		
	end

	self.widgets.border = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.border})
	self.widgets.board = UI:CreateWidget("MatWidget", {rect=self.magicBoardRect, material=self.gfx.board})
	self.widgets.root:AddChild(self.widgets.border)
	self.widgets.root:AddChild(self.widgets.board)
	
	self.widgets.outline = UI:CreateWidget("MatWidget", {rect=self.screen, material=self.gfx.Outline})
	self.widgets.root:AddChild(self.widgets.outline)
	self.widgets.outline:SetHAlign(kHorizontalAlign_Left)
	self.widgets.outline:SetVAlign(kVerticalAlign_Top)
	self.widgets.outline:SetVisible(false)
	
	self.widgets.timeLeftLabel = UI:CreateWidget("TextLabel", {rect={self.screen[1], self.screen[2], 8,8}, typeface=self.typefaces.TimerText})
    self.widgets.root4:AddChild(self.widgets.timeLeftLabel)

    self.widgets.swipeToMoveLabel = UI:CreateWidget("TextLabel", {rect={ 0, 0, 8, 8}, typeface=self.typefaces.SwipeToMoveText})
    
    if (UI.mode == kGameUIMode_Mobile) then
		UI:SetLabelText(self.widgets.swipeToMoveLabel, StringTable.Get("TAP_DPAD_TO_MOVE"))
	else
		UI:SetLabelText(self.widgets.swipeToMoveLabel, StringTable.Get("PRESS_ARROW_KEYS_TO_MOVE"))
	end
	
    UI:SizeLabelToContents(self.widgets.swipeToMoveLabel)
    local swipeLabelRect = UI:CenterLabel(self.widgets.swipeToMoveLabel, UI.fullscreenRect)
    self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, 0)
	
	self.widgets.swipeToMoveLabelBkg = UI:CreateWidget("MatWidget", {rect=ExpandRect(swipeLabelRect, 32*UI.identityScale[1], 32*UI.identityScale[2]),material=self.gfx.ItemBackground})
	self.widgets.swipeToMoveLabelBkg:BlendTo({1,1,1,0}, 0)
	
	self.widgets.root3:AddChild(self.widgets.swipeToMoveLabelBkg)
    self.widgets.root3:AddChild(self.widgets.swipeToMoveLabel)
	
end

function ReflexGame.DeInitUI(self)
	if (self.widgets) then
		if (self.widgets.goals) then
			for k,v in pairs(self.widgets.goals) do
				self.widgets.root2:RemoveChild(v.particles)
				v.particles:Unmap()
				v.particles = nil
				v:Unmap()
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
		if (self.widgets.trash) then
			for k,v in pairs(self.widgets.trash) do
				self.widgets.root:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.trash = nil
		end
		if (self.widgets.lines) then
			for k,v in pairs(self.widgets.lines) do
				self.widgets.root:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.lines = nil
		end
		self.widgets.current = nil
		if (self.widgets.blackholes) then
			for k,v in pairs(self.widgets.blackholes) do
				self.widgets.root2:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.blackholes = nil
		end
		if (self.widgets.spiders) then
			for k,v in pairs(self.widgets.spiders) do
				self.widgets.root2:RemoveChild(v.particles)
				v.particles:Unmap()
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
		
		if (self.widgets.swipeBar) then
			self.widgets.root2:RemoveChild(self.widgets.swipeBar)
			self.widgets.swipeBar:Unmap()
			self.widgets.swipeBar = nil
		end
		
		self.widgets.portal = nil
		self.widgets.cells = nil
		
		collectgarbage()
	end
end

function ReflexGame.CreateBoard(self)
	self:DeInitUI()
	
	local oldState = self.state
		
	local level = self.level
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
	self.state.victory = false
	self.state.level = level
	self.state.antivirusSpawnTimer = FloatRand(level.antivirusSpiderSpawnRate[1], level.antivirusSpiderSpawnRate[2])
    self.state.lineTimerEnabledTimer = level.blockChaseTime
    self.state.lineTimer = 0
	self.state.goalCounter = 0
    self.state.lineIndex = 1
    self.state.swipeToMoveTimer = 1
    
    if (oldState) then
		self.state.timeLeft = oldState.timeLeft
	end
    
	self.widgets.goals = { }
	self.widgets.lines = { }
	self.widgets.spiders = { }
    self.widgets.blackholes = { }
	self.widgets.grid = {}
	self.widgets.cells = {}
	self.widgets.portal = nil
	self.widgets.current = nil
	self.widgets.trash = {}
	
	local goalimages = {}
	for i=1,27 do
		goalimages[i] = i
	end
	
	goalimages = ShuffleArray(goalimages)
	local goalnum = 1
	
	COutLine(kC_Debug, "reflex.level.name=" .. self.state.level.name)

	COutLine(kC_Debug, "Creating Board")	
	-- board step: board grid is x,y structure
	for i,v in ipairs(self.state.level.board) do

        local objectTable = self.widgets.cells
        local objectSize = { self.REFLEX_CELL_SIZE[1], self.REFLEX_CELL_SIZE[2] }
		local layer = self.widgets.root
		local archetype = v.img
		local gfx = nil
		
		if (v.img == "blocker_green") then
			gfx = self.gfx.blocker_green[IntRand(1, #self.gfx.blocker_green)]
		else
			if (self.gfx[v.img] == self.gfx.blackhole) then
				objectTable = self.widgets.blackholes
				objectSize = { self.BLACKHOLE_WIDGET_SIZE[1], self.BLACKHOLE_WIDGET_SIZE[2] }
				layer = self.widgets.root2
			elseif (v.img == "mark_start") then
				layer = nil
				archetype = "player"
			end
			
			gfx = self.gfx[v.img]
		end
		
		local b = UI:CreateWidget("MatWidget", {rect={0,0,objectSize[1],objectSize[2]}, material=gfx})
		local index = self:ConvertCoordToIndex(v.x,v.y)
		b.state = self:CreateState(archetype,v)
        if (layer) then
			layer:AddChild(b)
		end
        if ((objectTable == self.widgets.cells) and (v.img ~= "mark_start")) then
		    self.widgets.grid[index] = b
        end
        table.insert(objectTable,b)
		self:SetPositionByGrid(b,v.x,v.y,objectTable==self.widgets.blackholes)
		if (v.img == "mark_start") then
            local player = b
            player:SetMaterial(self.gfx.mark_current)
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
        if (string.find(b.state.archetype,"cell_") ~= nil) then
			local img = self.gfx[string.format("cell_%02i", goalimages[goalnum])]
			goalnum = goalnum + 1
			b:SetMaterial(img)
            table.insert(self.widgets.goals,b)
		end
    end
    
    if (self.widgets.player) then
		-- on top of blackholes
		self.widgets.root2:AddChild(self.widgets.player)
    end
    
    self.widgets.playerParticles = UI:CreateWidget("MatWidget", {rect={0,0,310*UI.identityScale[1],310*UI.identityScale[1]}, material=self.gfx.PlayerParticles})
	self.widgets.playerParticles:SetVisible(false)
	
	self.widgets.swipeBar = UI:CreateWidget("MatWidget", {rect={self.screen[1],self.screen[2],232,self.screen[4]},material=self.gfx.SwipeBar})
	self.widgets.root2:AddChild(self.widgets.swipeBar)
	self.widgets.swipeBar:SetVisible(false)
	
	for k,v in pairs(self.widgets.goals) do
		local w = UI:CreateWidget("MatWidget", {rect={0,0,310*UI.identityScale[1],310*UI.identityScale[1]}, material=self.gfx.GoalParticles})
		w:SetVisible(false)
		v.particles = w
	end

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


function ReflexGame.CreateState(self,archetype,ref)
	local state = { }
	state.archetype = archetype
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
        if (string.find(piece.state.archetype,"cell_") ~= nil) then
            -- -djr do effect
            self:ExplodeGoal(piece)
            self.widgets.grid[index] = nil
            self.widgets.root:RemoveChild(piece)
            self.state.goalCounter = self.state.goalCounter + 1
            self.sfx.Acquired:Play(kSoundChannel_UI, 0)
            if (self.state.goalCounter == #self.widgets.goals) then
				self.widgets.portal:SetMaterial(self.gfx.GoalOpen)
				self.sfx.Complete:Play(kSoundChannel_UI, 0)
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
		if (piece.state.archetype == "mark_end" or piece.state.archetype == "mark_start") then
			return false
		end

		if (string.find(piece.state.archetype,"cell_") ~= nil) then
			return false
		end
				
		COutLine(kC_Debug,"CollidePlayerWithBoard found Piece @ x=%i, y=%i, type=%s",x,y,piece.state.archetype)			
		return true
	end
	
	return false
end

function ReflexGame.SuckupPieces(self,x,y,w,h)
	local min,max = self:RectCellBounds(x,y,w,h)
	
	local kdd = w*w
	
	local didSuck = false
	
	for gx=min.x,max.x do
		for gy=min.y,max.y do
			local vec = self:GetPositionByGrid(gx,gy)
			local index = self:ConvertCoordToIndex(gx, gy)
			local piece = self.widgets.grid[index]
			if (piece and (piece.state.archetype == "blocker_green")) then
				local pos = self:GetPosition(piece)
				local dx = pos.x-vec.x
				local dy = pos.y-vec.y
				local dd = dx*dx+dy*dy
				if (dd < kdd) then
					self:SuckupWidget(piece,x,y)
					self.widgets.grid[index] = nil
					table.insert(self.widgets.trash, piece)
					didSuck = true
				end
			end
		end
	end
	
	for k,v in pairs(self.widgets.spiders) do
	
		local pos = self:GetPosition(v)
		
		local dx = pos.x-x
		local dy = pos.y-y
		local dd = dx*dx+dy*dy
		if (dd < kdd*1.5) then
			self:SuckupSpider(k,v,x,y)
			didSuck = true
		end
	
	end
	
	return didSuck
end

function ReflexGame.SuckupPlayer(self,x,y)
	self:SuckupWidget(self.widgets.player,x,y)
end

function ReflexGame.SuckupSpider(self,index,spider,x,y)
	table.remove(self.widgets.spiders,index)
	table.insert(self.widgets.trash, spider)
	spider.particles:Unmap()
	self:SuckupWidget(spider,x,y)
	if (next(self.widgets.spiders) == nil) then
		MemoryGame.entity.sfx.Stage2:Stop()
	end
end

function ReflexGame.SuckupWidget(self,w,x,y)
	local tr = w:Rect()
	tr[1] = tr[1] + x - (tr[1]+(tr[3]/2))
	tr[2] = tr[2] + y - (tr[2]+(tr[4]/2))
	w:MoveTo(tr, {0.7,0.7})
	w:ScaleTo({0,0}, {0.7,0.7})
	w:BlendTo({1,1,1,0}, 0.7)
end

function ReflexGame.UpdateHud(self)
    local minutes = math.floor(math.ceil(self.state.timeLeft) / 60)
    local seconds = math.ceil(self.state.timeLeft) - (minutes * 60)
    local text = string.format("%01i:%02i",minutes,seconds)
    UI:SetLabelText(self.widgets.timeLeftLabel, text, {1,1})
    UI:SizeLabelToContents(self.widgets.timeLeftLabel)
    UI:VCenterLabel(self.widgets.timeLeftLabel, {self.screen[1], self.screen[2], self.screen[3], self.REFLEX_CELL_SIZE[1]})
end

function ReflexGame.UpdateTimer(self, dt)
	if (self.tickTimer) then
		local oldTime = self.state.timeLeft
		self.state.timeLeft = self.state.timeLeft - dt
		if (self.state.timeLeft <= 0) then
			self.state.timeLeft = 0
			self.timerTask:Clean()
			self.timerTask = nil
		end
		
		if ((oldTime >= 30) and (self.state.timeLeft < 30)) then
			self.widgets.timeLeftLabel:SetTypeface(self.typefaces.TimerTextFlash)
		elseif (self.state.timeLeft == 0) then
			self.widgets.timeLeftLabel:SetTypeface(self.typefaces.TimerTextRed)
			if (not self.animatingRetry) then
				self.state.gameOver = true
				self.gameReady = false
				self.think = nil
				self:HandleTimeUp()
			end
		end
		
		self:UpdateHud()
	end
end

function ReflexGame.HandleTimeUp(self)
	Abducted.entity.eatInput = true
	local f = function()
		self:EndGame("f")
	end
	World.globalTimers:Add(f, 2.5)
end

function ReflexGame.PrepBoardIntro(self)

	for k,v in pairs(self.widgets.goals) do
		v:BlendTo({1,1,1,0}, 0)
	end
	
	for k,v in pairs(self.widgets.blackholes) do
		v:BlendTo({1,1,1,0}, 0)
	end
	
	for k,v in pairs(self.widgets.grid) do
		if (v.state.archetype == "blocker_green") then
			v:BlendTo({1,1,1,0}, 0)
		end
	end

	self.widgets.player:BlendTo({1,1,1,0}, 0)
	self.widgets.current:BlendTo({1,1,1,0}, 0)
	self.widgets.portal:BlendTo({1,1,1,0}, 0)
	
	self.revealColumn = 0
	
end

function ReflexGame.ReadyGameStart(self)
	Abducted.entity.eatInput = false
	self.gameReady = true
	self.animatingRetry = false
	self.widgets.swipeToMoveLabel:BlendTo({1,1,1,1}, .2)
	self.widgets.swipeToMoveLabelBkg:BlendTo({1,1,1,1}, .2)
	if (self.widgets.dpad) then
		self.dpadTouch = nil
		self.widgets.dpadRoot:SetCapture(false)
		for k,v in pairs(self.widgets.dpad) do
			v:SetMaterial(self.gfx.RightArrow)
			v:BlendTo({1,1,1,1}, 0.2)
		end
	end
	
	self.think = ReflexGame.Think
	self:SetNextThink(0)
end

function ReflexGame.RevealBoard(self)
	self.widgets.swipeBar:SetVisible(true)
	self.widgets.swipeBar:MoveTo({self.screen[1]+self.screen[3], self.screen[2]}, {1.5,0})
	self.sfx.Reveal:Play(kSoundChannel_UI, 0)
	
	local f = function()
	
		if (self.state.timeLeft <= 0) then
			self.widgets.swipeBar:SetVisible(false)
			if (self.revealTimer) then
				self.revealTimer:Clean()
				self.revealTimer = nil
			end
			self.animatingRetry = false
			self:HandleTimeUp()
			return
		end
	
		for k,v in pairs(self.widgets.goals) do
		
			local grid = self:GetGridCellFromVec2(self:GetPosition(v))
			if (grid.x == self.revealColumn) then
				v:BlendTo({1,1,1,1}, 0.1)
			end
			
		end
		
		local grid = self:GetGridCellFromVec2(self:GetPosition(self.widgets.player))
		if (grid.x == self.revealColumn) then
			self.widgets.player:BlendTo({1,1,1,1}, 0.1)
			self.widgets.current:BlendTo({1,1,1,1}, 0.1)
		end
		
		grid = self:GetGridCellFromVec2(self:GetPosition(self.widgets.portal))
		if (grid.x == self.revealColumn) then
			self.widgets.portal:BlendTo({1,1,1,1}, 0.1)
		end
		
		if (self.revealColumn >= self.INDEX_MAX_X) then
			local f = function()
				for k,v in pairs(self.widgets.blackholes) do
					v:BlendTo({1,1,1,1}, 0.7)
				end
				self:ReadyGameStart()
			end
			self.revealTimer:Clean()
			self.revealTimer = nil
			self.widgets.swipeBar:SetVisible(false)
			World.globalTimers:Add(f, 0.5)
		else
			self.revealColumn = self.revealColumn + 1
		end
	end
		
	f()
	if (self.state.timeLeft > 0) then
		local dt = 1.5/(self.INDEX_MAX_X+1)
		self.revealTimer = World.globalTimers:Add(f, dt, true)
	end
	
end

function ReflexGame.AnimateIntro(self)
	self.tickTimer = false
	self:PrepBoardIntro()
	
	self.sfx.Enter:Play(kSoundChannel_UI, 0)
	self.widgets.board:BlendTo({1,1,1,0}, 0)
	self.widgets.black:BlendTo({0,0,0,0}, 0)
		
	if (self.widgets.dpad) then
		for k,v in pairs(self.widgets.dpad) do
			v:BlendTo({1,1,1,0}, 0)
		end
	end
	
	local f = function()
		self.widgets.outline:SetVisible(true)
		self.widgets.outline:BlendTo({1,1,1,1}, 0)
		self.widgets.outline:ScaleTo({0,0}, {0,0})
		self.widgets.outline:ScaleTo({1,1}, {0.2, 0.2})
		
		local f = function()
			self.widgets.outline:BlendTo({0,0,0,0}, 0.1)
			self.widgets.board:BlendTo({1,1,1,1}, 0.3)
			
			for k,v in pairs(self.widgets.grid) do
				if (v.state.archetype == "blocker_green") then
					v:BlendTo({1,1,1,1}, 0.3)
				end
			end
			
			local f = function()
				self:RevealBoard()
			end
			
			World.globalTimers:Add(f, 0.3)
		end
		
		World.globalTimers:Add(f, 0.2)
	end
	
	World.globalTimers:Add(f, 0.4)
end

function ReflexGame.DoRetry(self)
	if (self.exiting) then
		return
	end
	
	self.animatingRetry = true
	self.widgets.black:BlendTo({0,0,0,1}, 0.3)
	MemoryGame.entity.sfx.Stage2:FadeOutAndStop(1)
	
	local f = function()
		PuzzleScoreScreen:Unlink()
	
		if (self.widgets.dpad) then
			for k,v in pairs(self.widgets.dpad) do
				v:BlendTo({1,1,1,0}, 0)
			end
		end
	
		self:CreateBoard()
				
		local f = function()
			
			self:PrepBoardIntro()
			self.widgets.black:BlendTo({0,0,0,0}, 0.3)
			
			local f = function()
			
				for k,v in pairs(self.widgets.grid) do
					if (v.state.archetype == "blocker_green") then
						v:BlendTo({1,1,1,1}, 0.3)
					end
				end
				
				local f = function()
					self:RevealBoard()
				end
				
				World.globalTimers:Add(f, 0.3)
			
			end
			
			World.globalTimers:Add(f, 0.3)
		end
		
		World.globalTimers:Add(f, 0)
	end
	World.globalTimers:Add(f, 0.3)
end

function ReflexGame.DoQuit(self)
	if (not self.exiting) then
		self:EndGame("f")
	end
end

function ReflexGame.Think(self,dt)
	if (self.state.gameOver) then
		-- game over never advances past here
		self.think = nil
		self.gameReady = false
		
		if (self.widgets.dpad) then
			self.widgets.dpadRoot:SetCapture(false)
			self.widgets.dpad.left:SetMaterial(self.gfx.RightArrow)
			self.widgets.dpad.right:SetMaterial(self.gfx.RightArrow)
			self.widgets.dpad.up:SetMaterial(self.gfx.RightArrow)
			self.widgets.dpad.down:SetMaterial(self.gfx.RightArrow)
		end
		
		if (self.state.victory) then
			
			self.tickTimer = false
			self:ExplodeSpiders()
			MemoryGame.entity.sfx.Stage2:FadeOutAndStop(1)
			
			local f = function()
				if (self.widgets.dpad) then
					for k,v in pairs(self.widgets.dpad) do
						v:BlendTo({1,1,1,0}, 0.2)
					end
				end
				
				EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", "HACK_TERMINAL_SUCCESS")
	
				PuzzleScoreScreen:DoSuccessScreen(
					self.widgets.root3,
					self.actions,
					self.skill,
					function ()
						self:EndGame("w")
					end
				)
			end
			World.globalTimers:Add(f, 1.5)
		else
			self.sfx.Fail:Play(kSoundChannel_UI, 0)
			PuzzleScoreScreen:DoRetryQuitScreen(
				self.widgets.root3,
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

	if (self.state.heading.x == 0 and self.state.heading.y == 0) then
		-- NO HEADING: Game hasn't started		
		return
    end
    
    self.tickTimer = true

    if (not self.state.labelFadeOut) then
		self.widgets.swipeToMoveLabel:BlendTo({1,1,1,0}, .5)
		self.widgets.swipeToMoveLabelBkg:BlendTo({1,1,1,0}, .5)
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
            local b = UI:CreateWidget("MatWidget", {rect={0,0,self.REFLEX_CELL_SIZE[1],self.REFLEX_CELL_SIZE[2]}, material=self.gfx.blocker_green[IntRand(1, #self.gfx.blocker_green)]})
            b.state = self:CreateState("blocker_green")
            self.widgets.root:AddChild(b)
            assert(self.widgets.grid[index] == nil)
            self.widgets.grid[index] = b
            self:SetPositionByGrid(b,v.x,v.y)
            b:ScaleTo({0,0}, {0,0})
            b:ScaleTo({1,1}, {self.state.level.blockGrowTime,self.state.level.blockGrowTime})
            self.state.lineIndex = self.state.lineIndex + 1
            COutLine(kC_Debug, "Spawned block")
        end
    end

	local playerGridCell = self:GetGridCellFromVec2(currentPos)
	local playerIndex = self:ConvertCoordToIndex(playerGridCell.x,playerGridCell.y)	 

	local pieceAtPlayer = self.widgets.grid[playerIndex]	
	if (pieceAtPlayer) then
		if (pieceAtPlayer.state.archetype == "mark_end" and self.state.goalCounter >= #self.widgets.goals) then
			COutLine(kC_Debug,"Game Over Detected")
			self.state.gameOver = true
            self.state.victory = true
            local pos = self:GetPosition(pieceAtPlayer)
            self:SuckupPlayer(pos.x, pos.y)
			return
		end
    end
    
    for i,k in pairs(self.widgets.spiders) do
		self:SpiderThink(i, k, dt)
	end	
	
	self.state.antivirusSpawnTimer =  self.state.antivirusSpawnTimer - dt
	if (self.state.antivirusSpawnTimer < 0) then
		self.state.antivirusSpawnTimer = FloatRand(self.state.level.antivirusSpiderSpawnRate[1], self.state.level.antivirusSpiderSpawnRate[2])		
		local spawned = false
		for i=1,5 do
			local x = math.random(self.INDEX_MAX_X)-1
			local y = math.random(self.INDEX_MAX_Y)-1
			local playerGrid = self:GetGridCellFromVec2({x=self.widgets.current.state.endPos.x, y=self.widgets.current.state.endPos.y})
			local dx = playerGrid.x - x
			local dy = playerGrid.y - y
			local dd = math.sqrt(dx*dx+dy*dy)
			if (dd >= 6) then
				local spider = UI:CreateWidget("MatWidget", {rect={200,200,self.SPIDER_WIDGET_SIZE[1],self.SPIDER_WIDGET_SIZE[2]}, material=self.gfx.antivirus_spider})
				spider.particles = UI:CreateWidget("MatWidget", {rect={0,0,310*UI.identityScale[1],310*UI.identityScale[1]},material=self.gfx.SpiderParticles})
				self.widgets.root2:AddChild(spider)	
				local firstSpider = next(self.widgets.spiders) == nil
				table.insert(self.widgets.spiders,spider)
				self:SetPositionByGrid(spider,x,y)		
				spider.state = self:CreateState("antivirus_spider")
				spider.state.lifetime = FloatRand(self.state.level.antivirusSpiderLifetime[1], self.state.level.antivirusSpiderLifetime[2])
				self:SpiderPickHeading(spider)
				COutLine(kC_Debug,"spawnedSpider @ grid: x=%i, y=%i, heading = %.04f,%.04f, distFromPlayer = %f",x,y,spider.state.heading.x,spider.state.heading.y, dd)
				spawned = true
				self.sfx.SpikeBombEnter:Play(kSoundChannel_UI, 0)
				if (firstSpider) then
					MemoryGame.entity.sfx.Stage2:Rewind()
					MemoryGame.entity.sfx.Stage2:FadeVolume(0, 0)
					MemoryGame.entity.sfx.Stage2:Play(kSoundChannel_UI, 0)
					MemoryGame.entity.sfx.Stage2:FadeVolume(1, 1)
				end
				break
			end
		end
		
		if (not spawned) then
			COutLine(kC_Debug, "FAILED TO SPAWN SPIDER!")
		end
    end

	local didSuck = false
	
    for i,k in pairs(self.widgets.blackholes) do
        if (k.state.heading == null) then
            k.state.heading = {}
            k.state.heading.x = k.state.ref.heading[1]
            k.state.heading.y = k.state.ref.heading[2]
            k.state.headingTime = FloatRand(self.state.level.blackholeMoveTime[1], self.state.level.blackholeMoveTime[2])
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
            k.state.headingTime = FloatRand(self.state.level.blackholeMoveTime[1], self.state.level.blackholeMoveTime[2])
            nextPos = self:LerpWidget(k,k.state.heading,2*dt,k.state.speed,false)
            COutLine(kC_Debug,"blackhole bounce @ : x=%i, y=%i",nextPos.x,nextPos.y)
        end
        UI:MoveWidgetByCenter(k,nextPos.x,nextPos.y)
        didSuck = self:SuckupPieces(nextPos.x,nextPos.y,self.BLACKHOLE_SIZE[1],self.BLACKHOLE_SIZE[2]) or didSuck
        if (not self.state.gameOver) then
			if (self:CheckTouchPlayer(nextPos.x,nextPos.y,self.BLACKHOLE_SIZE[1],self.BLACKHOLE_SIZE[2])) then
				didSuck = true
				self:SuckupPlayer(nextPos.x, nextPos.y)
				self.state.gameOver = true
				COutLine(kC_Debug, "Player eaten by blackhole.")
			end
		end
    end
    
    if (didSuck) then
		self.sfx.Suck:Play(kSoundChannel_UI, 0)
    end
end

function ReflexGame.SpiderThink(self, index, spider, dt)
	if (spider.state.lifetime < dt) then
		table.remove(self.widgets.spiders,index)
		spider.particles:Unmap()
		self.widgets.root2:RemoveChild(spider)
		spider:Unmap()
		if (next(self.widgets.spiders) == nil) then
			MemoryGame.entity.sfx.Stage2:Stop()
		end
		return
	else
		spider.state.lifetime = spider.state.lifetime - dt
	end
	
	local pos = self:GetPosition(spider)
	local gridRange = self.REFLEX_CELL_SIZE[1] * self.state.level.antivirusSpiderSeekPlayerRange
	self.seekPlayer = self:CheckTouchPlayer(pos.x, pos.y, gridRange, gridRange)
	
	local speed = self.state.level.antivirusSpiderSpeed[1]
	
	if (self.seekPlayer) then
	
		--COutLine(kC_Debug, "Spider seeking player")
		-- seek the player
		spider.state.headingTime = 0
		spider.state.heading = self:GetHeadingTowardsPlayer(pos.x,pos.y)
		speed = self.state.level.antivirusSpiderSpeed[2]
	else
	
		if (spider.state.headingTime < dt) then
			self:SpiderPickHeading(spider)
		else
			spider.state.headingTime = spider.state.headingTime - dt
		end
	
	end
	
	local pos = spider:Rect()
	local nextPos = self:LerpWidget(spider,spider.state.heading,dt,speed,false)
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
	self.widgets.playerParticles:ScaleTo({1,1}, {1,1})
	
	local f = function()
		self.widgets.playerParticles:BlendTo({1,1,1,0}, 0.6)
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
	widget.particles:ScaleTo({1,1}, {1,1})
	
	local f = function()
		widget.particles:BlendTo({1,1,1,0}, 0.6)
	end
	
	World.globalTimers:Add(f, 0.4)
end

function ReflexGame.ExplodeSpider(self, spider)

	local pos = self:GetPosition(spider)
	spider:SetVisible(false)
	
	local particles = spider.particles
	
	UI:MoveWidgetByCenter(particles, pos.x, pos.y)
	self.widgets.root2:AddChild(particles)
	
	particles:SetVisible(true)
	particles:ScaleTo({0,0}, {0,0})
	particles:ScaleTo({1,1}, {1,0.7})
	
	local f = function()
		particles:BlendTo({1,1,1,0}, 0.3)
	end
	
	World.globalTimers:Add(f, 0.4)
	
	self.sfx.Explode:Play(kSoundChannel_UI, 0)
end

function ReflexGame.ExplodeSpiders(self)

	if (#self.widgets.spiders < 1) then
		return
	end

	local timeOfs = 0
	local timeStep = 0.2
	local totalTime = timeStep * (#self.widgets.spiders - 1)
	
	if (totalTime > 3) then
		timeStep = 3 / (#self.widgets.spiders-1)
	end
	
	
	for k,v in pairs(self.widgets.spiders) do
	
		local f = function()
			self:ExplodeSpider(v)
		end
		
		World.globalTimers:Add(f, timeOfs)
		timeOfs = timeOfs + timeStep
	
	end

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

-------------------------------------------------------------------------------
-- Score screen common stuff

PuzzleScoreScreen = Class:New()

function PuzzleScoreScreen.Init(self, screenRect)

	self.gameBoard = screenRect

	self.typefaces = {}
	self.typefaces.Score1 = World.Load("UI/TerminalPuzzlesScoreFont_TF")
	self.typefaces.Score2 = World.Load("UI/TerminalPuzzlesScoreFont2_TF")
	
	self.gfx = {}
	self.gfx.BackgroundSuccess = World.Load("UI/puzzle_success_bkg_M")
	self.gfx.BackgroundFail = World.Load("UI/MMItemBackground2_M")
	
	self.widgets = {}
	self.widgets.root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect,OnInputEvent=UI.EatInput})
		
	self:CreateRetryQuit()
	self:CreateSuccess()
end

function PuzzleScoreScreen.CreateSuccess(self)

	local screen = {0, 0, 0, 0}
	screen[3] = self.gameBoard[3] - 64*4*UI.identityScale[1]
	screen[4] = self.gameBoard[4]--UI.fullscreenRect - 64*2*UI.identityScale[2]
	screen = CenterRectInRect(screen, self.gameBoard)
	
	self.widgets.SuccessRoot = UI:CreateWidget("MatWidget", {rect={screen[1], self.gameBoard[2], screen[3], self.gameBoard[4]}, material=self.gfx.BackgroundSuccess})
	self.widgets.root:AddChild(self.widgets.SuccessRoot)
	
	screen = {0, 0, screen[3], screen[4]}
	
	self.widgets.SuccessLabel = UI:CreateWidget("TextLabel", {rect=screen, typeface=self.typefaces.Score1})
	UI:SetLabelText(self.widgets.SuccessLabel, StringTable.Get("SUCCESS"))
	UI:SizeLabelToContents(self.widgets.SuccessLabel)
	UI:VAlignLabelTop(self.widgets.SuccessLabel, nil, 16*UI.identityScale[2])
	self.successLabelRect = UI:HCenterLabel(self.widgets.SuccessLabel, screen)
	self.widgets.SuccessLabel:SetBlendWithParent(true)
	self.widgets.SuccessRoot:AddChild(self.widgets.SuccessLabel)

	self.successScreenRect = screen
end

function PuzzleScoreScreen.CreateRetryQuit(self)

	--local inflate8 = {8*UI.identityScale[1], 8*UI.identityScale[2]}
	local inflate32 = {32*UI.identityScale[1], 32*UI.identityScale[2]}
		
	local retryText = StringTable.Get("RETRY")
	local quitText = StringTable.Get("QUIT")
	local rw,rh = UI:StringDimensions(self.typefaces.Score1, retryText)
	local qw,qh = UI:StringDimensions(self.typefaces.Score1, quitText)
	
	rh = math.max(rh, qh)
	
	local retryRect = {0, 0, rw+inflate32[1], rh+inflate32[2]}
	self.widgets.Retry = UI:CreateStylePushButton(
		retryRect,
		function ()
			self:Retry()
		end,
		{
			typeface=self.typefaces.Score1,
			highlight={on={0,0,0,0}},
			solidBackground = true
		}
	)
	
	self.widgets.Retry:SetBlendWithParent(true)

	local quitRect = {0, 0, qw+inflate32[1], rh+inflate32[2]}

	self.widgets.Quit = UI:CreateStylePushButton(
		quitRect,
		function ()
			self:Quit()
		end,
		{
			typeface=self.typefaces.Score1,
			highlight={on={0,0,0,0}},
			solidBackground = true
		}
	)
	
	self.widgets.Quit:SetBlendWithParent(true)
	
	UI:SetLabelText(self.widgets.Retry.label, retryText)
	UI:CenterLabel(self.widgets.Retry.label, self.widgets.Retry.label:Rect())
	UI:SetLabelText(self.widgets.Quit.label, quitText)
	UI:CenterLabel(self.widgets.Quit.label, self.widgets.Quit.label:Rect())
		
	local gap = self.gameBoard[3] * 0.25
	local totalWidth = retryRect[3] + quitRect[3] + gap
	local leftEdge = self.gameBoard[1] + (self.gameBoard[3] - totalWidth) / 2
	local topEdge = self.gameBoard[2] + (self.gameBoard[4] - rh - inflate32[2]) / 2
	
	retryRect[1] = leftEdge
	retryRect[2] = topEdge
	self.widgets.Retry:SetRect(retryRect)
	
	quitRect[1] = leftEdge + retryRect[3] + gap
	quitRect[2] = topEdge
	self.widgets.Quit:SetRect(quitRect)

	self.widgets.RetryQuitRoot = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
	self.widgets.root:AddChild(self.widgets.RetryQuitRoot)
	
	local rootRect = {
		leftEdge - inflate32[1],
		topEdge - inflate32[2],
		totalWidth + inflate32[2]*2,
		rh + inflate32[2]*3
	}
	
	self.widgets.RetryQuitBackground = UI:CreateWidget("MatWidget", {rect=rootRect, material=self.gfx.BackgroundFail})
	self.widgets.RetryQuitBackground:SetBlendWithParent(true)
	self.widgets.RetryQuitRoot:AddChild(self.widgets.RetryQuitBackground)
	self.widgets.RetryQuitRoot:AddChild(self.widgets.Retry)
	self.widgets.RetryQuitRoot:AddChild(self.widgets.Quit)
end

function PuzzleScoreScreen.RevealNextItem(self, callback)
	local w = self.widgets.items[self.nextItem]
	if (w) then
		self.nextItem = self.nextItem + 1
		w:BlendTo({1,1,1,1}, 0.5)
	else
		self.revealTimer:Clean()
		self.revealTimer = nil
		World.globalTimers:Add(callback, #self.widgets.items * 0.5 + 1)
	end
end

function PuzzleScoreScreen.CreateRewardWidget(self)
	local w = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.Score2})
	self.widgets.SuccessRoot:AddChild(w)
	w:SetBlendWithParent(true)
	w:BlendTo({1,1,1,0}, 0)
	return w
end
	
function PuzzleScoreScreen.DoSuccessScreen(self, layer, actions, skillLevel, callback, logevent)

	ReflexGame.entity.sfx.Success:Play(kSoundChannel_UI, 0)
	
	self.successHook = callback
	
	layer:AddChild(self.widgets.root)
	
	self.widgets.RetryQuitRoot:SetVisible(false)
	self.widgets.SuccessRoot:SetVisible(true)
	self.widgets.SuccessRoot:BlendTo({1,1,1,0}, 0)
	self.widgets.SuccessRoot:BlendTo({1,1,1,1}, 0.3)
	
	self.rewardTopic = nil
	self.rewardMessage = nil
	self.rewardSkill = nil
	self.rewardSkillPoints = nil
	self.rewardDiscover = nil
	
	self:ProcessActions(actions, skillLevel)
	
	local kSpace = 24*UI.identityScale[2]
	local totalHeight = 0--self.successLabelRect[2] + self.successLabelRect[4] + kSpace
	
	self.widgets.items = {}
	
	if (self.rewardSkillPoints) then
		local w = self:CreateRewardWidget()
		
		local msg = "+"..tostring(self.rewardSkillPoints)
		if (self.rewardSkillPoints > 1) then
			msg = msg.." "..StringTable.Get("ARM_REWARD_SKILLPOINTS")
		else
			msg = msg.." "..StringTable.Get("ARM_REWARD_SKILLPOINT")
		end
		
		w:SetText(msg) -- no zoom on fonts here, gets too crowded
		UI:SizeLabelToContents(w)
		local r = UI:HCenterLabel(w, self.successScreenRect)
		table.insert(self.widgets.items, w)
		totalHeight = totalHeight + r[4]
	end
	
	if (self.rewardTopic) then
		for k,v in pairs(self.rewardTopic) do
			local w = self:CreateRewardWidget()
			w:SetText(StringTable.Get("ARM_REWARD_TOPIC").." "..Arm:FindChatString(v))
			UI:SizeLabelToContents(w)
			local r = UI:HCenterLabel(w, self.successScreenRect)
			table.insert(self.widgets.items, w)
			totalHeight = totalHeight + r[4]
		end
	end
	
	if (self.rewardDiscover) then
		for k,v in pairs(self.rewardDiscover) do
			local dbItem = Arm.Discoveries[v]
			if (dbItem) then
				local w = self:CreateRewardWidget()
				w:SetText(StringTable.Get("ARM_REWARD_DISCOVERY")..": "..StringTable.Get(dbItem.title))
				UI:SizeLabelToContents(w)
				local r = UI:HCenterLabel(w, self.successScreenRect)
				table.insert(self.widgets.items, w)
				totalHeight = totalHeight + r[4]
			end
		end
	end
	
	if (self.rewardMessage) then
		for k,v in pairs(self.rewardMessage) do
			local w = self:CreateRewardWidget()
			w:SetText(StringTable.Get(v))
			UI:SizeLabelToContents(w)
			local r = UI:HCenterLabel(w, self.successScreenRect)
			table.insert(self.widgets.items, w)
			totalHeight = totalHeight + r[4]
		end
	end
	
	-- center widgets for better presentation
	totalHeight = totalHeight + (#self.widgets.items - 1) * kSpace
	
	local topBorder = self.successLabelRect[2] + self.successLabelRect[4] + kSpace
	local verticalArea = self.successScreenRect[4]-- - topBorder
	local y = (verticalArea - totalHeight) / 2
	y = math.max(y, topBorder)

	for k,v in pairs(self.widgets.items) do
	
		local r = v:Rect()
		r[2] = y
		y = y + r[4] + kSpace
		v:SetRect(r)
		
	end
	
	if (next(self.widgets.items) == nil) then
		World.globalTimers:Add(callback, 3)
	else
		self.nextItem = 1
		local f = function()
			self:RevealNextItem(callback)
		end
		self.revealTimer = World.globalTimers:Add(f, 0.9, true)
	end
	
	self.unlink = function()
		if (self.revealTimer) then
			self.revealTimer:Clean()
			self.revealTimer = nil
		end
		if (self.widgets.items) then
			for k,v in pairs(self.widgets.items) do
				self.widgets.SuccessRoot:RemoveChild(v)
				v:Unmap()
			end
			self.widgets.items = nil
		end
		layer:RemoveChild(self.widgets.root)
	end

end

function PuzzleScoreScreen.ProcessActions(self, actions, skillLevel)
	if (actions == nil) then
		return
	end
	
	actions = string.split(actions, ";")
	
	for k,v in pairs(actions) do
		local tokens = Tokenize(v)
		if (#tokens > 0) then
			self:ProcessActionTokens(tokens)
		end
	end
	
	self.rewardSkillPoints = skillLevel * PlayerSkills.kTerminalReward
	PlayerSkills:AwardSkillPoints(self.rewardSkillPoints)
end

function PuzzleScoreScreen.ProcessActionTokens(self, tokens)

	if (tokens[1] == "trigger") then
		Arm:ProcessTriggerTokens(tokens)
	elseif (tokens[1] == "unlock_topic") then
		if (Arm:UnlockTopic(tokens[2])) then
			self.rewardTopic = self.rewardTopic or {}
			table.insert(self.rewardTopic, tokens[2])
		end
	elseif (tokens[1] == "message") then
		self.rewardMessage = self.rewardMessage or {}
		table.insert(self.rewardMessage, tokens[2])
		EventLog:AddEvent(GameDB:ArmDateString(), "!EVENT", tokens[2])
	elseif (tokens[1] == "unlock_skill") then
		self.rewardSkill = tokens[2]
	elseif (tokens[1] == "discover") then
		if (GameDB:Discover(tokens[2], "terminal", true)) then
			self.rewardDiscover = self.rewardDiscover or {}
			table.insert(self.rewardDiscover, tokens[2])
		end
	end
end

function PuzzleScoreScreen.DoRetryQuitScreen(self, layer, retry, quit)
	self.retryHook = retry
	self.quitHook = quit
	self.successHook = nil
	
	layer:AddChild(self.widgets.root)
	
	self.widgets.SuccessRoot:SetVisible(false)
	self.widgets.RetryQuitRoot:SetVisible(true)
	self.widgets.RetryQuitRoot:BlendTo({1,1,1,0}, 0)
	self.widgets.RetryQuitRoot:BlendTo({1,1,1,1}, 0.3)
	
	self.unlink = function()
		layer:RemoveChild(self.widgets.root)
	end
	
end

function PuzzleScoreScreen.DismissRetryQuit(self, callback)
	Abducted.entity.eatInput = true
	callback()
end

function PuzzleScoreScreen.Unlink(self)
	if (self.unlink) then
		self.unlink()
		self.unlink = nil
	end
end

function PuzzleScoreScreen.Retry(self)
	self:DismissRetryQuit(self.retryHook)
end

function PuzzleScoreScreen.Quit(self)
	self:DismissRetryQuit(self.quitHook)
end