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
	
	local uiScreen
	
	if ((UI.systemScreen.aspect == "16x9") or (UI.systemScreen.aspect == "16x10")) then
		uiScreen = {width=1280, height=720}
		UI.wideScreen = true
	else
		uiScreen = {width=1280, height=720}
	end
	
	World.SetUIViewport(0, 0, uiScreen.width, uiScreen.height)
	UI.fullscreenRect = {0, 0, uiScreen.width, uiScreen.height}
	
	UI.xAspect = uiScreen.width / uiScreen.height
	UI.yAspect = uiScreen.height / uiScreen.width
	UI.screenWidth = uiScreen.width
	UI.screenHeight = uiScreen.height
	UI.screenUIScale = {
		uiScreen.width / UI.systemScreen.width,
		uiScreen.height / UI.systemScreen.height
	}
	
	UI.gestureMode = false
	
	UI.gfx = {}
	UI.widgets = {}
	
	UI:LoadSharedMaterials()
	UI:CreateFXLayer()
	UI:CreateFeedbackLayer()
	
	DebugUI:Spawn()
	
	self.think = UI.Think
	self:SetNextThink(33)
	
	UI:FadeToColor({0, 0, 0, 1}, 0) -- start black
	
end

function UI.OnLevelStart(self)
	UI:FadeIn(1)
end

function UI.Think(self)
	DebugUI:Think()
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
	self.widgets.feedback.Finger:FadeTo({0, 0, 0, 0}, 0)
	
end

function UI.CreateFXLayer(self)
	self.widgets.fx = {}
	self.widgets.fx.Screen = self:CreateWidget("MatWidget", {rect=UI.fullscreenRect, material=self.gfx.shared.Solid})
	World.SetRootWidget(UI.kLayer_FX, self.widgets.fx.Screen)
	self.widgets.fx.Screen.color = { 0, 0, 0, 0 }
	self.widgets.fx.Screen:FadeTo({0, 0, 0, 0}, 0) -- disable
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
		self.widgets.feedback.Finger:FadeTo({1, 1, 1, 1}, time)
	else
		self.widgets.feedback.Finger:FadeTo({0, 0, 0, 0}, time)
	end

end

function UI.PlaceFinger(self, x, y)

	local r = self.widgets.feedback.Finger:Rect()
	local z = { x-r[3]/2, y-r[4]/2, r[3], r[4] }
	self.widgets.feedback.Finger:SetRect(z)

end

function UI.FadeIn(self, time)

	local c = self.widgets.fx.Screen.color
	self.widgets.fx.Screen:FadeTo({c[1], c[2], c[3], 0}, time)

end

function UI.FadeToColor(self, color, time)
	self.widgets.fx.Screen.color = color
	self.widgets.fx.Screen:FadeTo(color, time)
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
	r[1] = x
	r[2] = y
	widget:SetRect(r)
end

function UI.HCenterWidget(self, widget, xp, yf)

	local r = widget:Rect()
	
	if (xp == nil) then
		xp = UI.fullscreenRect[3] / 2
	end
	
	xp = xp * 2
	
	if (type(yf) == "function") then
		yf = yf(r[4])
	end
	
	local x = (xp-r[3])/2
	
	r[1] = x
	r[2] = yf
	
	widget:SetRect(r)
	
	return r
end

function UI.VCenterWidget(self, widget, xf, yp)

	local r = widget:Rect()
		
	if (yp == nil) then
		yp = UI.fullscreenRect[4] / 2
	end
	
	yp = yp * 2
	
	if (type(xf) == "function") then
		xf = xf(r[3])
	end
	
	local y = (yp-r[4])/2
	
	r[1] = xf
	r[2] = y
	widget:SetRect(r)
	return r
end

function UI.CenterWidget(self, widget, xp, yp)

	local r = widget:Rect()
	
	if (xp == nil) then
		xp = UI.fullscreenRect[3] / 2
	end
	if (yp == nil) then
		yp = UI.fullscreenRect[4] / 2
	end
	
	xp = xp * 2
	yp = yp * 2
	
	local x = (xp-r[3])/2
	local y = (yp-r[4])/2
	
	r[1] = x
	r[2] = y
	
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

function UI.HCenterLabel(self, label, xp, yf)

	local w, h = label:Dimensions()
	
	if (xp == nil) then
		xp = UI.fullscreenRect[3] / 2
	end
	
	xp = xp * 2
	
	if (type(yf) == "function") then
		yf = yf(h)
	end
	
	local x = (xp-w)/2
	
	local r = {x,yf,w,h}
	label:SetRect(r)
	return r
end

function UI.VCenterLabel(self, label, xf, yp)

	local w, h = label:Dimensions()
	
	if (yp == nil) then
		yp = UI.fullscreenRect[4] / 2
	end
	
	yp = yp * 2
	
	if (type(xf) == "function") then
		xf = xf(w)
	end
	
	local y = (yp-h)/2
	
	local r = {xf,y,w,h}
	label:SetRect(r)
	return r
end

function UI.CenterLabel(self, label, xp, yp)

	local w, h = label:Dimensions()
	
	if (xp == nil) then
		xp = UI.fullscreenRect[3] / 2
	end
	if (yp == nil) then
		yp = UI.fullscreenRect[4] / 2
	end
	
	xp = xp * 2
	yp = yp * 2
	
	local x = (xp-w)/2
	local y = (yp-h)/2
	
	local r = {x,y,w,h}
	label:SetRect(r)
	return r
end

function UI.RAlignLabel(self, label, x, y)

	local w, h = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x-w, y, w, h}
	label:SetRect(r)
	return r

end


function UI.VAlignLabel(self, label, x, y)

	local w, h = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x, y-h, w, h}
	label:SetRect(r)
	return r

end

function UI.RVAlignLabel(self, label, x, y)

	local w, h = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x-w, y-h, w, h}
	label:SetRect(r)
	return r

end

function UI.SizeLabelToContents(self, label, x, y)

	local w, h = label:Dimensions()
	
	if ((x == nil) or (y == nil)) then
		local r = label:Rect()
		if (x == nil) then
			x = r[1]
		end
		
		if (y == nil) then
			y = r[2]
		end
	end
	
	local r = {x, y, w, h}
	label:SetRect(r)
	return r

end

function UI.EatInput(self, e)
	return true
end

function UI.SizeLabel(self, label)

	local w, h = label:Dimensions()
	
	local r = label:Rect()
	r[3] = w
	r[4] = h
	label:SetRect(r)
	return r
	
end

function UI.MaterialSize(self, material, rect)
	rect = rect or {}
	local w, h = material:Dimensions()
	rect[3] = w
	rect[4] = h
	
	return rect
end

ui_code = UI
