-- UI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

UI = Class:New()
UI.kLayer_Mask = 0
UI.kLayer_UI = 1
UI.kLayer_MainMenu = 2
UI.kLayer_HUD = 2 -- not used at same time as main menu
UI.kLayer_Interactive = 3
UI.kLayer_Arm = 4
UI.kLayer_TerminalPuzzles = 5
UI.kLayer_LB = 6
UI.kLayer_AlertPanel = 7
UI.kLayer_Notifications = 8
UI.kLayer_Feedback = 9
UI.kLayer_FX = 10
UI.kLayer_Debug = 11

function UI.Spawn(self)
	UI.entity = self
	
	UI.mode = System.UIMode()
	
	StringTable.Global = World.Load("UI/Globals")
	
	UI.systemScreen = System.ScreenSize()
	COutLine(kC_Debug, "UI.Spawn: Viewport = %dx%d", UI.systemScreen.width, UI.systemScreen.height)
		
	UI.systemRect = {0, 0, UI.systemScreen.width, UI.systemScreen.height}
	UI.systemScreen.diagonalSq = UI.systemScreen.width*UI.systemScreen.width + UI.systemScreen.height*UI.systemScreen.height
	UI.systemScreen.diagonal = math.sqrt(UI.systemScreen.diagonalSq)
	
	local uiScreen
	
	if ((UI.systemScreen.aspect == "16x9") or (UI.systemScreen.aspect == "16x10")) then
		UI.systemScreen.aspect = "16x9"
		uiScreen = {width=1280, height=720}
		UI.wideScreen = true
	elseif (UI.systemScreen.aspect == "4x3") then
		uiScreen = {width=1024, height=768}
	else
		assert(UI.systemScreen.aspect == "3x2")
		uiScreen = {width=960, height=640}
	end
	
	World.SetUIViewport(0, 0, uiScreen.width, uiScreen.height)
	UI.fullscreenRect = {0, 0, uiScreen.width, uiScreen.height}
	
	UI.xAspect = uiScreen.width / uiScreen.height
	UI.yAspect = uiScreen.height / uiScreen.width
	UI.screenWidth = uiScreen.width
	UI.screenHeight = uiScreen.height
	UI.screenDiagonalSq = UI.screenWidth*UI.screenWidth + UI.screenHeight*UI.screenHeight
	UI.screenDiagonal = math.sqrt(UI.screenDiagonalSq)
	
	UI.screenUIScale = {
		uiScreen.width / UI.systemScreen.width,
		uiScreen.height / UI.systemScreen.height
	}
	
	UI.invScreenUIScale = {
		UI.systemScreen.width / uiScreen.width,
		UI.systemScreen.height / uiScreen.height
	}
	
	-- text was authored at 720p
	
	UI.identityScale = {
		uiScreen.width / 1280,
		uiScreen.height / 720
	}
	
	if (UI.systemScreen.aspect == "4x3") then
	-- don't stretch weird on 4x3
		UI.identityScale[2] = UI.identityScale[1]
	end
	
	UI.invIdentityScale = {
		1 / UI.identityScale[1],
		1 / UI.identityScale[2]
	}
	
	local upscaleFonts = false
	if (System.Platform() == kPlatIPhone) then
		upscaleFonts = true
	elseif (DebugUI.Enabled and (System.Platform() == kPlatPC)) then
	-- iPhone emulation
		if (((UI.systemScreen.width == 960) and (UI.systemScreen.height == 640)) or
		    ((UI.systemScreen.width == 1136) and (UI.systemScreen.height == 640))) then
			upscaleFonts = true
		end
	end
	
	if (upscaleFonts) then
		-- make the text larger on the phones
		UI.fontScale = {1.25, 1.25}
		UI.invFontScale = {
			1 / UI.fontScale[1], 
			1 / UI.fontScale[2]
		}
	elseif ((UI.screenWidth == 1024) and (UI.screenHeight == 768)) then
		UI.fontScale = {
			1,
			1
		}
		
		UI.invFontScale = {
			1,
			1
		}
	else
		UI.fontScale = {
			UI.identityScale[1],
			UI.identityScale[2]
		}
		
		UI.invFontScale = {
			UI.invIdentityScale[1],
			UI.invIdentityScale[2]
		}
	end
	
	UI.gestureMode = false
	
	UI.gfx = {}
	UI.sfx = {}
	UI.typefaces = {}
	
	UI.widgets = {}

	UI:LoadShared()
	UI:CreateMaskLayer()
	UI:CreateFXLayer()
	UI:CreateFeedbackLayer()
	AlertPanel:Create()
		
	UI:BlendTo({0, 0, 0, 1}, 0) -- start black
	
end

function UI.OnLevelStart(self)
	UI:FadeIn(1)
end

function UI.Think(self)
end

function UI.LoadShared(self)
	self.gfx.Solid = World.Load("UI/Solid_M")
	self.gfx.Button = World.Load("UI/arm_buttons_M")
	self.gfx.ButtonOverbright = World.Load("UI/arm_buttons_overbright_M")
	
	self.typefaces.StandardButton = World.Load("UI/StandardButton_TF")
	self.typefaces.StandardButtonDark = World.Load("UI/StandardButtonDark_TF")
	self.typefaces.StandardButtonSmall = World.Load("UI/StandardButtonSmall_TF")
	
	self.sfx.Command = World.LoadSound("Audio/ui_command", 2)
end

function UI.InitMap(self)
	UI:CreateInteractiveLayer()
	self.gfx.Glyphs = {}
	
	for i=1,27 do
		local s = string.format("Puzzles/glyph%02d_M", i)
		self.gfx.Glyphs[i] = World.Load(s)
	end
	
	self.gfx.AnimatedGlpyh = World.Load("Puzzles/+0glyph_M")
	self.gfx.AnimatedGlpyhPressed = World.Load("Puzzles/+0glyphPressed_M")
end

function UI.CreateInteractiveLayer(self)
	self.widgets.interactive = {}
	self.widgets.interactive.Root = UI:CreateRoot(UI.kLayer_Interactive)
end

function UI.CreateFeedbackLayer(self)
	
	self.gfx.feedback = {}
	self.widgets.feedback = {}
	
	self.widgets.feedback.Root = UI:CreateRoot(UI.kLayer_Feedback)
		
	self.gfx.feedback.Finger = World.Load("UI/finger_shadow_M")
	self.widgets.feedback.Finger = self:CreateWidget("MatWidget", {rect={0, 0, 128*UI.identityScale[1], 128*UI.identityScale[2]}, material=self.gfx.feedback.Finger}) 
	self.widgets.feedback.Root:AddChild(self.widgets.feedback.Finger)
	self.widgets.feedback.Finger:BlendTo({0, 0, 0, 0}, 0)
	
end

function UI.CreateFXLayer(self)
	self.widgets.fx = {}
	self.widgets.fx.Screen = self:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=self.gfx.Solid})
	World.SetRootWidget(UI.kLayer_FX, self.widgets.fx.Screen)
	self.widgets.fx.Screen.color = { 0, 0, 0, 0 }
	self.widgets.fx.Screen:BlendTo({0, 0, 0, 0}, 0) -- disable
end

function UI.CreateMaskLayer(self)
	self.widgets.mask = {}
	self.widgets.mask.Screen = self:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=self.gfx.Solid})
	World.SetRootWidget(UI.kLayer_Mask, self.widgets.mask.Screen)
	self.widgets.mask.Screen.color = { 0, 0, 0, 0 }
	self.widgets.mask.Screen:BlendTo({0, 0, 0, 0}, 0) -- disable
end

--[[---------------------------------------------------------------------------
	UI Library
-----------------------------------------------------------------------------]]

function UI.AckFinger(self, pos)
	UI:ShowFinger(true, 0)
	UI:PlaceFinger(pos[1], pos[2])
	UI:ShowFinger(false, 1)
end

function UI.ShowFinger(self, show, time)

	if show then
		self.widgets.feedback.Finger:BlendTo({1, 1, 1, 1}, time)
	else
		self.widgets.feedback.Finger:BlendTo({0, 0, 0, 0}, time)
	end

end

function UI.PlaceFinger(self, x, y)

	local r = self.widgets.feedback.Finger:Rect()
	local z = { x-r[3]/2, y-r[4]/2, r[3], r[4] }
	self.widgets.feedback.Finger:SetRect(z)

end

function UI.FadeIn(self, time)

	local c = self.widgets.fx.Screen.color
	self.widgets.fx.Screen:BlendTo({c[1], c[2], c[3], 0}, time)

end

function UI.BlendTo(self, color, time)
	self.widgets.fx.Screen.color = color
	self.widgets.fx.Screen:BlendTo(color, time)
end


function UI.MaskFadeIn(self, time)

	local c = self.widgets.mask.Screen.color
	self.widgets.mask.Screen:BlendTo({c[1], c[2], c[3], 0}, time)

end

function UI.MaskBlendTo(self, color, time)
	self.widgets.mask.Screen.color = color
	self.widgets.mask.Screen:BlendTo(color, time)
end

function UI.FadeInLetterBox(self, color, time)

end

function UI.FadeOutLetterBox(self, color, time)

end

function UI.CreateRoot(self, layer, input)
	local root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect, OnInputEvent=input})
	World.SetRootWidget(layer, root)
	return root
end

function UI.CreateWidget(self, type, parms)

	local w = World.CreateWidget(type, parms)
	
	if (w) then
		if (parms.OnInputEvent) then
			w.OnInputEvent = parms.OnInputEvent
		end
		
		if (parms.OnInputGesture) then
			w.OnInputGesture = parms.OnInputGesture
		end
	end
	
	return w
end

--[[---------------------------------------------------------------------------
	Common Stylized Widgets
-----------------------------------------------------------------------------]]

function UI.CreateStylePushButton(self, rect, OnPressed, options, parent)

	options = options or {}

	local enabled = nil
	
	if ((options.background == nil) or (options.background)) then
		options.background = true
		enabled = self.gfx.Button
	end
	
	local typeface
	if (options.typeface) then
		typeface = options.typeface
	elseif ((options.fontSize ~= nil) and (options.fontSize == "small")) then
		typeface = self.typefaces.StandardButtonSmall
	else
		typeface = self.typefaces.StandardButton
	end
	
	local label = nil
	if (not options.nolabel) then
		label = {typeface = typeface}
	end
	
	local sound = options.pressed or self.sfx.Command
	
	local highlight = nil

	if (not options.nohighlight) then
		highlight = options.highlight or {}
	
		if (highlight.on == nil) then
			if (options.background) then
				highlight.on = {0.5, 0.5, 0.5, 1}
			else
				highlight.on = {0, 0, 0, 0}
			end
		end
		
		if (highlight.off == nil) then
			highlight.off = {0,0,0,0}
		end
		
		if (highlight.overbright == nil) then
			highlight.overbright = {1,1,1,1}
		end
		
		if (highlight.time == nil) then
			highlight.time = 0.1
		end
		
		if (highlight.overbrightTime == nil) then
			highlight.overbrightTime = 0.1
		end
	end
	
	local w = UIPushButton:Create(
		rect,
		{
			enabled = enabled,
			highlight = self.gfx.ButtonOverbright
		},
		{
			pressed = self.sfx.Command
		},
		{
			pressed = OnPressed
		},
		{
			highlight = highlight,
			label = label
		},
		parent
	)
	
	return w

end

--[[---------------------------------------------------------------------------
	Widget Positioning Utils
-----------------------------------------------------------------------------]]

function UI.MapToUI(self, p)
	local x = {
		p[1] * UI.screenUIScale[1],
		p[2] * UI.screenUIScale[2]
	}
	return x
end

function UI.MapInputEvent(self, e)

	if (e.type == kI_KeyDown) or (e.type == kI_KeyUp) then
		return e
	end
	
	local x = { 
		original = e,
		type = e.type,
		data = { 
			e.data[1] * UI.screenUIScale[1], 
			e.data[2] * UI.screenUIScale[2], 
			e.data[3] 
		}
	}
	
	return x

end

function UI.MapInputGesture(self, g)

	local x = {
		original = g,
		mins = {
			g.mins[1] * UI.screenUIScale[1],
			g.mins[2] * UI.screenUIScale[2]
		},
		maxs = {
			g.maxs[1] * UI.screenUIScale[1],
			g.maxs[2] * UI.screenUIScale[2]
		},
		origin = {
			g.origin[1] * UI.screenUIScale[1],
			g.origin[2] * UI.screenUIScale[2]
		}
	}

	return x

end

function UI.MoveWidgetByCenter(self, widget, x, y)
	local r = widget:Rect()
	
	r[1] = x - r[3]/2
	r[2] = y - r[4]/2
	
	widget:SetRect(r)
end


function UI.MoveWidget(self, widget, x, y)
	local r = widget:Rect()
	if (x) then
		r[1] = x
	end
	
	if (y) then
		r[2] = y
	end
	
	widget:SetRect(r)
end

function UI.HCenterWidget(self, widget, rect)

	local r = widget:Rect()
	
	r[1] = rect[1] + ((rect[3]-r[3]) / 2)
	
	widget:SetRect(r)
	
	return r
end

function UI.VCenterWidget(self, widget, rect)

	local r = widget:Rect()
		
	r[2] = rect[2] + ((rect[4]-r[4]) / 2)
	
	widget:SetRect(r)
	return r
end

function UI.CenterWidget(self, widget, rect)

	local r = widget:Rect()
	
	r[1] = rect[1] + ((rect[3]-r[3]) / 2)
	r[2] = rect[2] + ((rect[4]-r[4]) / 2)
	
	widget:SetRect(r)
	return r
end

function UI.RAlignWidget(self, widget, x, y)

	local r = widget:Rect()
	
	if (x == nil) then
		x = r[1]
	end
	
	if (y == nil) then
		y = r[2]
	end
	
	r[1] = x - r[3]
	r[2] = y
	
	widget:SetRect(r)
	return r

end

function UI.VAlignWidget(self, widget, x, y)

	local r = widget:Rect()
	
	if (x == nil) then
		x = r[1]
	end
	
	if (y == nil) then
		y = r[2]
	end
	
	r[1] = x
	r[2] = y - r[4]
	
	widget:SetRect(r)
	return r

end

function UI.RVAlignWidget(self, widget, x, y)

	local r = widget:Rect()
	
	if (x == nil) then
		x = r[1]
	end
	
	if (y == nil) then
		y = r[2]
	end
	
	r[1] = x - r[3]
	r[2] = y - r[4]
	
	widget:SetRect(r)
	return r

end

function UI.HCenterLabel(self, label, rect)

	local d = label:Dimensions()
	local r = label:Rect()
	
	r[1] = rect[1] + ((rect[3]-(d[3]-d[1])) / 2) - d[1]
	r[3] = d[3]
	r[4] = d[4]
	
	label:SetRect(r)
	return r
end

function UI.VCenterLabel(self, label, rect)

	local d = label:Dimensions()
	local r = label:Rect()
	
	r[2] = rect[2] + ((rect[4]-(d[4]-d[2])) / 2) - d[2]
	r[3] = d[3]
	r[4] = d[4]
	
	label:SetRect(r)
	return r
end

function UI.CenterLabel(self, label, rect)

	local d = label:Dimensions()
	
	local r = {
		rect[1] + ((rect[3]-(d[3]-d[1])) / 2) - d[1],
		rect[2] + ((rect[4]-(d[4]-d[2])) / 2) - d[2],
		d[3],
		d[4]
	}
	
	label:SetRect(r)
	return r
end

function UI.MoveLabelNoPadd(self, label, x, y)
	local d = label:Dimensions()
	local r = label:Rect()
	
	if (x) then
		r[1] = x - d[1]
	end
	
	if (y) then
		r[2] = y - d[2]
	end
	
	label:SetRect(r)
	return r
end

function UI.RAlignLabel(self, label, x, y)

	local d = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x-d[3], y, d[3], d[4]}
	label:SetRect(r)
	return r

end


function UI.VAlignLabel(self, label, x, y)

	local d = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x, y-d[4], d[3], d[4]}
	label:SetRect(r)
	return r

end

function UI.RVAlignLabel(self, label, x, y)

	local d = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x-d[3], y-d[4], d[3], d[4]}
	label:SetRect(r)
	return r

end

function UI.EatInput(self, e)
	return true
end

function UI.MaterialSize(self, material, rect)
	rect = rect or {0, 0, 0, 0}
	local d = material:Dimensions()
	rect[3] = d[1]
	rect[4] = d[2]
	
	return rect
end

function UI.LineWrapCenterText(self, label, maxWidth, sizeToFit, lineSpace, lines, fontScale, invFontScale)

	if (type(lines) == "string") then
		lines = string.split(lines, "\n")
	end
	
	if (fontScale == nil) then
		fontScale = UI.fontScale
	end
	
	if (invFontScale == nil) then
		invFontScale = UI.invFontScale
	end
	
	if (lineSpace < 0) then
		lineSpace = -lineSpace -- absolute
	else
		lineSpace = lineSpace * fontScale[2]
	end
	
	local r = label:Rect()
	local font = label:Typeface()
	
	if (maxWidth == nil) then
		maxWidth = r[3]
	end
	
	lineSpace = lineSpace + UI:FontAdvanceSize(font, fontScale)
	
	local labelStrings = {}
	local size = 0
	local widestLine = 0
	
	for k,line  in pairs(lines) do
	
		local strings = UI:WordWrap(font, line, maxWidth, invFontScale)
		for k,v in pairs(strings) do
		
				local w, h = UI:StringDimensions(font, v, fontScale)
											
				local string = {
					x = 0,
					y = size,
					text = v,
					scaleX = fontScale[1],
					scaleY = fontScale[2],
					w = w
				}
				
				size = size + lineSpace
				widestLine = Max(widestLine, w)
				table.insert(labelStrings, string)
		end
	end
	
	if (sizeToFit) then
		r[3] = maxWidth
		r[4] = size
		label:SetRect(r)
	end
	
	local y = (r[4] - size) / 2
	
	for k,v in pairs(labelStrings) do
		v.y = v.y + y
		v.x = (r[3] - v.w) / 2
	end
	
	label:SetText(labelStrings)
	
	if (not sizeToFit) then
	-- return the size we would have been had we auto-sized
		r[3] = maxWidth
		r[4] = size
	end
	
	return r
end

function UI.LineWrapLJustifyText(self, label, maxWidth, sizeToFit, lineSpace, lines, fontScale, invFontScale)

	if (type(lines) == "string") then
		lines = string.split(lines, "\n")
	end
	
	if (fontScale == nil) then
		fontScale = UI.fontScale
	end
	
	if (invFontScale == nil) then
		invFontScale = UI.invFontScale
	end
	
	if (lineSpace < 0) then
		lineSpace = -lineSpace -- absolute
	else
		lineSpace = lineSpace * fontScale[2]
	end
	
	local r = label:Rect()
	local font = label:Typeface()
	
	if (maxWidth == nil) then
		maxWidth = r[3]
	end
	
	lineSpace = lineSpace + UI:FontAdvanceSize(font, fontScale)
	
	local labelStrings = {}
	local size = 0
	local widestLine = 0
		
	for k,line  in pairs(lines) do
	
		local strings = UI:WordWrap(font, line, maxWidth, invFontScale)
		for k,v in pairs(strings) do
				
				local w, h = UI:StringDimensions(font, v, fontScale)
								
				local string = {
					x = 0,
					y = size,
					text = v,
					scaleX = fontScale[1],
					scaleY = fontScale[2]
				}
				
				size = size + lineSpace
				widestLine = Max(widestLine, w)
				table.insert(labelStrings, string)
		end
	end
	
	if (sizeToFit) then
		r[3] = widestLine
		r[4] = size
		label:SetRect(r)
	end
	
	local y = (r[4] - size) / 2
		
	for k,v in pairs(labelStrings) do
		v.y = v.y + y
	end
	
	label:SetText(labelStrings)
	
	if (not sizeToFit) then
	-- return the size we would have been had we auto-sized
		r[3] = widestLine
		r[4] = size
	end
	
	return r
end

--[[---------------------------------------------------------------------------
	Fonts are unaware of our UI scaling
-----------------------------------------------------------------------------]]


function UI.SizeLabelToContents(self, label, x, y)

	local d = label:Dimensions()
	-- NOTE: dimensions are real-size scale here.
	
	local r = label:Rect()
	
	if ((x == nil) or (y == nil)) then
		
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	r[1] = x
	r[2] = y
	r[3] = (d[3] + d[1])
	r[4] = (d[4] + d[2])
	
	label:SetRect(r)
	return r, {d[1], d[2], d[3], d[4]}
end

function UI.SetLabelText(self, label, text, fontScale)
	if (fontScale == nil) then
		fontScale = UI.fontScale
	end
	
	local string = {
		x = 0,
		y = 0,
		text = text,
		scaleX = fontScale[1],
		scaleY = fontScale[2]
	}
	
	label:SetText({string})

end

function UI.StringDimensions(self, font, text, fontScale)
	if (fontScale == nil) then
		fontScale = UI.fontScale
	end
	
	local w,h = font:StringDimensions(text)
	w = w * fontScale[1]
	h = h * fontScale[2]
	return w,h
end

function UI.FontAdvanceSize(self, font, fontScale)
	if (fontScale == nil) then
		fontScale = UI.fontScale
	end
	local a,d = font:AscenderDescender()
	return a*fontScale[2]
end

function UI.SplitStringAtSize(self, font, text, width, invFontScale)
	if (invFontScale == nil) then
		invFontScale = UI.invFontScale
	end
	width = width * invFontScale[1]
	return font:SplitStringAtSize(text, width)
end

function UI.WordWrap(self, font, text, width, invFontScale)
	if (invFontScale == nil) then
		invFontScale = UI.invFontScale
	end
	width = width * invFontScale[1]
	return font:WordWrap(text, width)
end

ui_code = UI
