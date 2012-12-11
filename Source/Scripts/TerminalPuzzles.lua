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

end

function TerminalPuzzles.Think(self)

end

function TerminalPuzzles.InitUI(self)

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
	self.gfx.Green = World.Load("UI/TerminalPuzzleGreen_M")
	self.gfx.Blue = World.Load("UI/TerminalPuzzleBlue_M")
	self.gfx.Border = World.Load("UI/TerminalPuzzleBorder_M")
	
	self.typefaces = {}
	self.typefaces.BigText = World.Load("UI/TerminalPuzzlesBigFont_TF")
	
end

terminal_puzzles = TerminalPuzzles
