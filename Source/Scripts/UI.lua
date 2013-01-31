-- UI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

UI = Class:New()
UI.kLayer_Mask = 0
UI.kLayer_UI = 1
UI.kLayer_MainMenu = 2
UI.kLayer_HUD = 2 -- not used at same time as main menu
UI.kLayer_Arm = 3
UI.kLayer_TerminalPuzzles = 4
UI.kLayer_LB = 5
UI.kLayer_Popups = 6
UI.kLayer_Notifications = 7
UI.kLayer_Feedback = 8
UI.kLayer_FX = 9
UI.kLayer_Debug = 10

function UI.Spawn(self)
	UI.entity = self
	
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
	
	UI.invIdentityScale = {
		1 / UI.identityScale[1],
		1 / UI.identityScale[2]
	}
	
	UI.gestureMode = false
	
	UI.gfx = {}
	UI.widgets = {}
	
	UI:LoadSharedMaterials()
	UI:CreateFXLayer()
	UI:CreateFeedbackLayer()
		
--	self.think = UI.Think
--	self:SetNextThink(1/30)
	
	UI:BlendTo({0, 0, 0, 1}, 0) -- start black
	
end

function UI.OnLevelStart(self)
	UI:FadeIn(1)
end

function UI.Think(self)
end

function UI.LoadSharedMaterials(self)
	self.gfx.shared = {}
	self.gfx.shared.Solid = World.Load("UI/Solid_M")
end

function UI.CreateFeedbackLayer(self)
	
	self.gfx.feedback = {}
	self.widgets.feedback = {}
	
	self.widgets.feedback.Root = UI:CreateRoot(UI.kLayer_Feedback)
		
	self.gfx.feedback.Finger = World.Load("UI/finger_shadow_M")
	self.widgets.feedback.Finger = self:CreateWidget("MatWidget", {rect={0, 0, 128, 128}, material=self.gfx.feedback.Finger}) 
	self.widgets.feedback.Root:AddChild(self.widgets.feedback.Finger)
	self.widgets.feedback.Finger:BlendTo({0, 0, 0, 0}, 0)
	
end

function UI.CreateFXLayer(self)
	self.widgets.fx = {}
	self.widgets.fx.Screen = self:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=self.gfx.shared.Solid})
	World.SetRootWidget(UI.kLayer_FX, self.widgets.fx.Screen)
	self.widgets.fx.Screen.color = { 0, 0, 0, 0 }
	self.widgets.fx.Screen:BlendTo({0, 0, 0, 0}, 0) -- disable
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

function UI.FadeInLetterBox(self, color, time)

end

function UI.FadeOutLetterBox(self, color, time)

end

function UI.CreateRoot(self, layer)
	local root = UI:CreateWidget("Widget", {rect=UI.fullscreenRect})
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
	Widget Positioning Utils
-----------------------------------------------------------------------------]]

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
	
	r[1] = rect[1] + ((rect[3]-d[1]) / 2)
	r[3] = w
	r[4] = h
	
	label:SetRect(r)
	return r
end

function UI.VCenterLabel(self, label, xf, yp)

	local d = label:Dimensions()
	
	local r = label:Rect()
	
	r[2] = rect[2] + ((rect[4]-d[2]) / 2)
	r[3] = w
	r[4] = h
	
	label:SetRect(r)
	return r
end

function UI.CenterLabel(self, label, xp, yp)

	local d = label:Dimensions()
	
	local r = {
		rect[1] + ((rect[3]-d[1]) / 2),
		rect[2] + ((rect[4]-d[2]) / 2),
		w,
		h
	}
	
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
	
	local r = {x-w, y, d[1], d[2]}
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
	
	local r = {x, y-h, d[1], d[2]}
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
	
	local r = {x-w, y-h, d[1], d[2]}
	label:SetRect(r)
	return r

end

function UI.SizeLabelToContents(self, label, x, y)

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
	
	local r = {x, y, d[1], d[2]}
	label:SetRect(r)
	return r

end

function UI.EatInput(self, e)
	return true
end

function UI.SizeLabel(self, label)

	local d = label:Dimensions()
	
	local r = label:Rect()
	r[3] = d[1]
	r[4] = d[1]
	label:SetRect(r)
	return r
	
end

function UI.MaterialSize(self, material, rect)
	rect = rect or {}
	local d = material:Dimensions()
	rect[3] = d[1]
	rect[4] = d[2]
	
	return rect
end

function UI.LineWrapCenterText(self, label, lineSpace, lines)

	if (type(lines) == "string") then
		lines = string.split(lines, "\n")
	end
	
	lineSpace = lineSpace * UI.identityScale[2]
	
	local r = label:Rect()
	local font = label:Typeface()
	local advance = UI:FontAdvanceSize(font)
	
	local labelStrings = {}
	local size = 0
	
	for k,line  in pairs(lines) do
	
		local strings = UI:WordWrap(font, line, r[3])
		for k,v in pairs(strings) do
		
				if (next(labelStrings) ~= nil) then
					size = size + lineSpace
				end
		
				local w, h = UI:StringDimensions(font, v)
				local x = (r[3] - w) / 2
							
				local string = {
					x = x,
					y = size,
					text = v,
					scaleX = UI.identityScale[1],
					scaleY = UI.identityScale[2]
				}
				
				label:SetText({string})
				local ds = label:Dimensions()
				local fx, fy = UI:StringDimensions(font, v)
				
				size = size + h
				table.insert(labelStrings, string)
		end
	end
	
	local y = (r[4] - (size + advance)) / 2
	
	for k,v in pairs(labelStrings) do
		v.y = v.y + y
	end
	
	label:SetText(labelStrings)

end

--[[---------------------------------------------------------------------------
	Fonts are unaware of our UI scaling
-----------------------------------------------------------------------------]]

function UI.StringDimensions(self, font, text)

	local w,h = font:StringDimensions(text)
	w = w * UI.identityScale[1]
	h = h * UI.identityScale[2]
	return w,h
end

function UI.FontAdvanceSize(self, font)
	local a,d = font:AscenderDescender()
	return (a+d)*UI.identityScale[2]
end

function UI.SplitStringAtSize(self, font, text, width)

	-- width is in UI coordinates, font operates in absolute
--	width = width * UI.invTextScale[1]
	return font:SplitStringAtSize(text, width)

end

function UI.WordWrap(self, font, text, width)
--	width = width * UI.invTextScale[1]
	return font:WordWrap(text, width)
end

ui_code = UI
