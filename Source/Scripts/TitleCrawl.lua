-- TitleCrawl.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TitleCrawl = Class:New()
TitleCrawl.Active = true

function TitleCrawl.Spawn(self)

	local typeface = World.Load("UI/Title_TF")
	
	self.titlePrinter = TextPrinter:New(typeface, nil, {0,0,8,8}, 1, 40, nil, UI.widgets.titlecrawl.Root, {UI.identityScale[1], UI.identityScale[1]})
	
	local typeface = World.Load("UI/Subtitle_TF")
	self.subtitlePrinter = TextPrinter:New(typeface, nil, {0,0,8,8}, 1, 40, nil, UI.widgets.titlecrawl.Root, {UI.identityScale[1], UI.identityScale[1]})

	local totalSize = self.titlePrinter.lineAdvance*1.5 + self.subtitlePrinter.lineAdvance
	local centerY = (UI.screenHeight - totalSize) / 2
	
	self.titlePrinter:SetRect({0,centerY,UI.screenWidth,self.titlePrinter.lineAdvance})
	self.subtitlePrinter:SetRect({0,centerY+self.titlePrinter.lineAdvance*1.5,UI.screenWidth,self.subtitlePrinter.lineAdvance})
	
end

function TitleCrawl.Print(self, title, subtitle, callback)
	
	local f = function()
		
		local f = function()
			World.globalTimers:Add(callback, 2)
		end
		
		self.subtitlePrinter:PrintHCentered(
			nil,
			subtitle,
			0,
			2,
			-1,
			0,
			f
		)
	
	end
	
	self.titlePrinter:PrintHCentered(
		nil,
		title,
		1,
		2,
		-1,
		0,
		f
	)

end

function TitleCrawl.Clear(self)

	UI.widgets.titlecrawl.Root:RemoveChild(self.titlePrinter.panel)
	UI.widgets.titlecrawl.Root:RemoveChild(self.subtitlePrinter.panel)
	UI.widgets.titlecrawl.Root:SetVisible(false)
	TitleCrawl.Active = false
	
	self.titlePrinter = nil
	self.subtitlePrinter = nil
	
	collectgarbage() -- clean up these big fonts
end