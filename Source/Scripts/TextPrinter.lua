-- TextPrinter.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

TextPrinter = Class:New()

function TextPrinter.New(outer, typeface, background, rect, maxMessages, charsPerSecond, lineSpace, root, fontScale, timeBase)

	local self = Class.New(TextPrinter)
	self.typeface = typeface
	
	if (timeBase == nil) then
		timeBase = World.globalTimers
	end
	
	if (fontScale == nil) then
		self.fontScale = UI.fontScale
		self.invFontScale = UI.invFontScale
	else
		self.fontScale = fontScale
		self.invFontScale = {1/fontScale[1], 1/fontScale[2]}
	end
	
	self.timers = timeBase
	self.cps = charsPerSecond
	self.background = background
	
	self.panel = UI:CreateWidget("Widget", {rect=rect})
	self.panelRect = {0, 0, rect[3], rect[4]}
	if (root) then
		root:AddChild(self.panel)
	end
	
	local ascenderSize = typeface:AscenderDescender() * self.fontScale[2]
	self.caretSize = {
		ascenderSize * 0.75,
		ascenderSize
	}
	
	self.caretSpace = 8*UI.identityScale[1]
	self.lineAdvance = UI:FontAdvanceSize(typeface, self.fontScale)

	if (lineSpace) then
		self.lineAdvance = self.lineAdvance + (lineSpace*UI.identityScale[1])
	end
	
	self.caret = UI:CreateWidget("MatWidget", {rect={0,0,self.caretSize[1],self.caretSize[2]}, material=UI.gfx.Solid})
	self.caret:SetVisible(false)
	self.panel:AddChild(self.caret)
	
	-- preallocate text blocks
	self.freeBlocks = LL_New()
	self.usedBlocks = LL_New()
	
	for i=1,maxMessages do
	
		local w = UI:CreateWidget("Widget", {rect={0,0,8,8}})
		w:SetVisible(false)
		self.panel:AddChild(w)
		
		if (background) then
			local bkg = UI:CreateWidget("MatWidget", {rect={0,0,8,8}, material=background})
			w:AddChild(bkg)
			w.bkg = bkg
		end
		
		LL_Append(self.freeBlocks, w)
	end
	
	self.freeText = LL_New()
	
	self.usedIcons = LL_New()
	self.freeIcons = LL_New()
	
	return self
end

function TextPrinter.SetRect(self, rect)

	self.panel:SetRect(rect)
	self.panelRect = {0, 0, rect[3], rect[4]}

end

function TextPrinter.Clear(self)

	while (not LL_Empty(self.usedBlocks)) do
	
		local block = LL_Pop(self.usedBlocks)
		self:FreeTextBlock(block)
	
	end

end

function TextPrinter.GetTextBlock(self)

	local block = LL_Pop(self.freeBlocks)
	if (block == nil) then
		block = LL_PopTail(self.usedBlocks)
		self:FreeTextBlock(block)
		block = LL_Pop(self.freeBlocks)
	end
	
	assert(block)
	return block

end

function TextPrinter.FreeTextBlock(self, block)

	if (LL_List(block)) then
		LL_Remove(LL_List(block), block)
	end
	
	if (block.timer) then
		block.timer:Clean()
		block.timer = nil
	end

	block.lines = nil
	if (block.widgets) then
		for k,v in pairs(block.widgets) do
			v:SetVisible(false)
			block:RemoveChild(v)
			LL_Append(self.freeText, v)
		end
		block.widgets = nil
	end
	
	if (block.icon) then
		block:RemoveChild(block.icon)
		self:FreeIcon(block.icon)
		block.icon = nil
	end
	
	block:SetVisible(false)
	LL_Append(self.freeBlocks, block)

end

function TextPrinter.GetTextWidget(self)

	local w = LL_Pop(self.freeText)
	if (w) then
		w:Clear()
	else
		w = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typeface})
		w:SetBlendWithParent(true)
	end
	
	return w
	
end

function TextPrinter.GetIcon(self, parms)
	if (parms == nil) then
		return nil
	end
	
	local icon = LL_Pop(self.freeIcons)
	if (icon) then
		icon:SetMaterial(parms.material)
		icon:SetRect({0,0,parms.size[1],parms.size[2]})
	else
		icon = UI:CreateWidget("MatWidget", {rect={0,0,parms.size[1],parms.size[2]}, material=parms.material})
		icon:SetBlendWithParent(true)
	end
	return icon
end

function TextPrinter.FreeIcon(self, icon)
	LL_Append(self.freeIcons, icon)
end

function TextPrinter.PrintRightAligned(self, icon, text, caretStartDelay, caretEndDelay, timeToLive, fadeOutTime, callback, useStringTable)

	local block, blockRect = 
		self:InternalPrint(
			icon,
			text,
			caretStartDelay,
			caretEndDelay,
			timeToLive,
			fadeOutTime,
			callback,
			useStringTable
		)
		
	blockRect[1] = self.panelRect[1] + self.panelRect[3] - blockRect[3]
	blockRect[2] = self.panelRect[2]
	
	self:FinishPrint(block, blockRect)
end

function TextPrinter.PrintLeftAligned(self, icon, text, caretStartDelay, caretEndDelay, timeToLive, fadeOutTime, callback, useStringTable)

	local block, blockRect = 
		self:InternalPrint(
			icon,
			text,
			caretStartDelay,
			caretEndDelay,
			timeToLive,
			fadeOutTime,
			callback,
			useStringTable
		)
		
	blockRect[1] = self.panelRect[1]
	blockRect[2] = self.panelRect[2]
	
	self:FinishPrint(block, blockRect)
end

function TextPrinter.PrintHCentered(self, icon, text, caretStartDelay, caretEndDelay, timeToLive, fadeOutTime, callback, useStringTable)

	local block, blockRect = 
		self:InternalPrint(
			icon,
			text,
			caretStartDelay,
			caretEndDelay,
			timeToLive,
			fadeOutTime,
			callback,
			useStringTable
		)
		
	blockRect[1] = self.panelRect[1] + ((self.panelRect[3]-blockRect[3])/2)
	blockRect[2] = self.panelRect[2]
	
	self:FinishPrint(block, blockRect)
end

function TextPrinter.PrintCustom(self, icon, text, caretStartDelay, caretEndDelay, timeToLive, fadeOutTime, callback, useStringTable)

	return self:InternalPrint(
		icon,
		text,
		caretStartDelay,
		caretEndDelay,
		timeToLive,
		fadeOutTime,
		callback,
		useStringTable
	)
	
end

function TextPrinter.InternalPrint(self, icon, text, caretStartDelay, caretEndDelay, timeToLive, fadeOutTime, callback, useStringTable)

	if (useStringTable == nil) then
		useStringTable = true
	end
	
	if (useStringTable) then
		text = StringTable.Get(text)
	end

	local marginPad = self.caretSize[1] + self.caretSpace
	
	local block = self:GetTextBlock()
	block.caretStartDelay = caretStartDelay
	block.caretEndDelay = caretEndDelay
	block.timeToLive = timeToLive
	block.fadeOutTime = fadeOutTime
	block.callback = callback
	
	block.icon = self:GetIcon(icon)
	
	local iconRect = nil
	if (block.icon) then
		block:AddChild(block.icon)
		block.icon:SetVisible(true)
		block.icon:BlendTo({1,1,1,0}, 0)
		block.icon:BlendTo({1,1,1,1}, 0.1)
		iconRect = block.icon:Rect()
		iconRect[3] = iconRect[3] + 8*UI.identityScale[1]
	else
		iconRect = {0,0,0,0}
	end
	
	local textArea = self.panelRect[3]-marginPad-iconRect[3]
	local lines = text:split("\n")
	
	local maxWidth = 0
	local height = 0
	
	block.widgets = {}
	block.lines = {}
	
	local numLines = 0
	
	for k,line in pairs(lines) do
		if (line:len() > 0) then
			local strings = UI:WordWrap(self.typeface, line, textArea, self.invFontScale)
			
			for k,v in pairs(strings) do
				local w = self:GetTextWidget()
				block:AddChild(w)
				w:AllocateText(v)
				w:SetVisible(true)
				w.done = false
				w.y = height
						
				local lineWidth = UI:StringDimensions(self.typeface, v, self.fontScale)
				maxWidth = math.max(lineWidth, maxWidth)
				
				-- convert text to UTF32 so we can reveal character by character in extended character sets
				table.insert(block.lines, System.UTF8To32(v))
				block.widgets[#block.lines] = w
				
				height = height + self.lineAdvance
				numLines = numLines + 1
			end
		else
			height = height + self.lineAdvance
			numLines = numLines + 1
		end
	end
	
	height = math.max(height, iconRect[4])
	
	for k,v in pairs(block.widgets) do
	
		v:SetRect({iconRect[3], v.y, maxWidth, self.lineAdvance})
	
	end
	
	self:ScrollItems(height)
	
	block:SetVisible(true)
	block:BlendTo({1,1,1,1}, 0)
	block.x = iconRect[3]
	block.y = 0
		
	LL_Insert(self.usedBlocks, block)
	
	--[[if (numLines == 1) then
		maxWidth = maxWidth + self.caretSize[1] + self.caretSpace + 16*UI.identityScale[1]
	end]]
		
	local blockRect = {iconRect[3], 0, maxWidth+iconRect[3]+4*UI.identityScale[1], height}
	return block, blockRect
end

function TextPrinter.FinishPrint(self, block, blockRect)
	
	block:SetRect(blockRect)
	block.x = blockRect[1]
	block.y = blockRect[2]
	
	if (block.bkg) then
		block.bkg:SetRect({0,0,blockRect[3],blockRect[4]})
	end
	
	self:AnimateBlock(block)

end

function TextPrinter.TickBlock(self, block, dt)

	block.charTime = block.charTime + dt*self.cps
	if (block.charTime > 1) then
		
		block.charTime = block.charTime - 1
		
		-- print next character
		local utf32 = block.lines[block.busy]
		local w = block.widgets[block.busy]
		
		local chars = utf32:sub(1, block.char)
		local utf8 = System.UTF32To8(chars)
				
		UI:SetLabelText(w, utf8, self.fontScale)
		
		local lineWidth = UI:StringDimensions(self.typeface, utf8, self.fontScale)
		
		-- move caret after character
		self.caret:MoveTo({block.x+lineWidth+self.caretSpace, block.y+w.y}, {0,0})
		
		block.char = block.char + 4
		if (block.char > utf32:len()) then
			block.char = 4
			block.busy = block.busy + 1
			w.done = true
			
			if (block.busy > #block.lines) then
				-- no more lines
				block.timer:Clean()
				
				if (block.caretEndDelay > 0) then
					local f = function()
						self:FinishBlock(block)
					end
					self.caret:SetMaterial(UI.gfx.CaretBlink)
					block.timer = self.timers:Add(f, block.caretEndDelay)
				else
					self:FinishBlock(block)
				end
				
			end
		end
	end

end

function TextPrinter.AnimateBlock(self, block)

	block.charTime = 0
	block.busy = 1
	block.char = 4
	
	if (block.bkg) then
		block.bkg:BlendTo({1,1,1,0}, 0)
		block.bkg:BlendTo({1,1,1,1}, 0.2)
	end
	
	self.caret:SetVisible(true)
	self.caret:MoveTo({block.x, block.y}, {0,0})
		
	local tick = function(dt)
		self:TickBlock(block, dt)
	end
		
	if (block.caretStartDelay > 0) then
		self.caret:SetMaterial(UI.gfx.CaretBlink)
		local f = function()
			self.caret:SetMaterial(UI.gfx.Solid)
			block.timer = self.timers:Add(tick, 0, true)
		end
		block.timer = self.timers:Add(f, block.caretStartDelay)
	else
		self.caret:SetMaterial(UI.gfx.Solid)
		block.timer = self.timers:Add(tick, 0, true)
	end

end

function TextPrinter.ScrollItems(self, amount)

	local block = LL_Head(self.usedBlocks)
	
	while (block) do
	
		block.y = block.y + amount
		
		local r = block:Rect()
		block:MoveTo({r[1], block.y}, {0, 0.1})
		
		if (block.busy) then
			self:FinishBlock(block)
		end
	
		block = LL_Next(block)
	end

end

function TextPrinter.FinishBlock(self, block)
	self.caret:SetVisible(false)
	
	if (block.timer) then
		block.timer:Clean()
		block.timer = nil
	end
	
	for k,v in pairs(block.lines) do
		local w = block.widgets[k]
		if (not w.done) then
			UI:SetLabelText(w, System.UTF32To8(v), self.fontScale)
			w.done = true
		end
	end
	
	if (block.callback) then
		block.callback()
		block.callback = nil
	end
	
	block.busy = nil
	self:QueueBlockFade(block)
end

function TextPrinter.FadeBlock(self, block)

	block:BlendTo({1,1,1,0}, block.fadeOutTime)
	
	if (block.bkg) then
		block.bkg:BlendTo({1,1,1,0}, block.fadeOutTime)
	end
	
	local f = function()
		self:FreeTextBlock(block)
	end
	
	block.timer = self.timers:Add(f, block.fadeOutTime)

end

function TextPrinter.QueueBlockFade(self, block)

	if (block.timeToLive > 0) then
		local f = function()
			self:FadeBlock(block)
		end
		block.timer = self.timers:Add(f, block.timeToLive)
	elseif (block.timeToLive == 0) then
		self:FadeBlock(block)
	end

end