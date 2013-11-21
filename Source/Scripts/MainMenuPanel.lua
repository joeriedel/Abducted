-- MainMenuPanel.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenuPanel = Class:New()

function MainMenuPanel.Create(self, options, parent)

	options = options or {}
	
	if (options.rect == nil) then
		options.rect = {0,0,8,8}
	end
	
	if (options.background == nil) then
		options.background = MainMenu.gfx.MMPanel
	end
	
	self.widgets = {}
	self.widgets.panel = UI:CreateWidget("MatWidget", {material=options.background, rect=options.rect})
	
	if (parent) then
		parent:AddChild(self.widgets.panel)
	end
	
	return options
	
end

function MainMenuPanel.Show(self, show)
	self.widgets.panel:SetVisible(show)
end

function MainMenuPanel.Layout(self)

end

function MainMenuPanel.AddBlendedBorders(self)

end

function MainMenuPanel.TransitionIn(self, scale, time, onComplete)

	MainMenu.eatInput = true
	
	self:PrepareContents()
	self.widgets.panel:ScaleTo(scale, {0,0})
	self.widgets.panel:ScaleTo({1,1}, {time,time})
	self.widgets.panel:SetVisible(true)
	
	local f = function()
		local f = function()
			MainMenu.eatInput = false
			if (onComplete) then
				onComplete()
			end
		end
		self:AnimateContents(f)
	end
	
	World.globalTimers:Add(f, time)

end

function MainMenuPanel.TransitionOut(self, scale, time, fade, onComplete)

	MainMenu.eatInput = true
	
	self:FadeOutContents(fade)
	
	local f = function()
		self.widgets.panel:ScaleTo(scale, {time,time})
		if (onComplete) then
			local f = function()
				MainMenu.eatInput = false
				onComplete()
			end
			World.globalTimers:Add(f, time)
		else
			MainMenu.eatInput = false
		end
	end
	
	World.globalTimers:Add(f, fade)
end

function MainMenuPanel.PrepareContents(self)

end

function MainMenuPanel.AnimateContents(self, onComplete)
	if (onComplete) then
		onComplete()
	end
end
