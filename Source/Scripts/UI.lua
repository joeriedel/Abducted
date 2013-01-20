-- UI.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

UI = Class:New()
UI.kLayer_Mask = 0
UI.kLayer_UI = 1
UI.kLayer_MainMenu = 2
UI.kLayer_TerminalPuzzles = 3
UI.kLayer_Popups = 4
UI.kLayer_LB = 5
UI.kLayer_Notifications = 6
UI.kLayer_Feedback = 7
UI.kLayer_FX = 8
UI.kLayer_Debug = 9

function UI.Spawn(self)
	UI.entity = self
	
	local screenSize = System.ScreenSize()
	COutLine(kC_Debug, "UI.Spawn: Viewport = %dx%d", screenSize.width, screenSize.height)
	World.SetUIViewport(0, 0, screenSize.width, screenSize.height)
	
	UI.fullscreenRect = {0, 0, screenSize.width, screenSize.height}
	UI.aspect = screenSize.aspect
	UI.xAspect = screenSize.width / screenSize.height
	UI.yAspect = screenSize.height / screenSize.width
	UI.screenWidth = screenSize.width
	UI.screenHeight = screenSize.height
	
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
	
	self.widgets.feedback.Root = self:CreateWidget("Widget", {rect=UI.fullscreenRect})
	World.SetRootWidget(UI.kLayer_Feedback, self.widgets.feedback.Root)
	
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

ui_code = UI
