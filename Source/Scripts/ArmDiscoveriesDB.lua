-- ArmDiscoveriesDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.Discoveries = {
	Bugs = {
		picture = "UI/discovery_bugs_M",
		title = "ARM_DISCOVERY_BUGS_TITLE",
		text = "ARM_DISCOVERY_BUGS",
		index = 1,
		chat = "Genesis1"
	},
	Pod = {
		picture = "UI/discovery_pod_M",
		title = "ARM_DISCOVERY_POD_TITLE",
		text = "ARM_DISCOVERY_POD",
		index = 2,
		chat = "Genesis2"
	},
	Tentacles = {
		picture = "UI/discovery_tentacles_M",
		title = "ARM_DISCOVERY_TENTACLES_TITLE",
		text = "ARM_DISCOVERY_TENTACLES",
		index = 3,
		chat = "Genesis3"
	},
	Terminals = {
		picture = "UI/discovery_terminals_M",
		title = "ARM_DISCOVERY_TERMINALS_TITLE",
		text = "ARM_DISCOVERY_TERMINALS",
		index = 4,
		chat = "Genesis4"
	}
}

Arm.DiscoveriesDBInset = { 16, 16 }

function Arm.SpawnDiscoveriesDB(self)

	self.discoveriesDBWorkspace = {
		self.dbWorkspace[1] + (Arm.DiscoveriesDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[2] + (Arm.DiscoveriesDBInset[2] * UI.identityScale[2]),
		self.dbWorkspace[3] - (Arm.DiscoveriesDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[4] - (Arm.DiscoveriesDBInset[2] * UI.identityScale[2])
	}
	
	self.discoveriesDBWorkspaceSize = {
		0,
		0,
		self.discoveriesDBWorkspace[3],
		self.discoveriesDBWorkspace[4]
	}
	
	-- big scrolling list of text
	self.widgets.db.DiscoveriesRoot = UI:CreateWidget("Widget", {rect=self.discoveriesDBWorkspace})
	self.widgets.db.Root:AddChild(self.widgets.db.DiscoveriesRoot)
	self.widgets.db.DiscoveriesRoot:SetBlendWithParent(true)
	
	local rect = UI:MaterialSize(self.gfx.DiscoveriesScrollBar)
	local h = self.discoveriesDBWorkspaceSize[4]
	local scale = h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	rect[1] = self.discoveriesDBWorkspaceSize[3] - rect[3]
	rect[2] = 0
	
	self.widgets.db.DiscoveriesScrollBar = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.DiscoveriesScrollBar})
	self.widgets.db.DiscoveriesRoot:AddChild(self.widgets.db.DiscoveriesScrollBar)
	self.widgets.db.DiscoveriesScrollBar:SetBlendWithParent(true)
	
	self.discoveriesDBArea = {
		0,
		0,
		rect[1] - (4 * UI.identityScale[1]),
		self.discoveriesDBWorkspaceSize[4]
	}
	
	self.widgets.db.Discoveries = UI:CreateWidget("VListWidget", {rect=self.discoveriesDBArea})
	self.widgets.db.DiscoveriesRoot:AddChild(self.widgets.db.Discoveries)
	self.widgets.db.Discoveries:SetBlendWithParent(true)
	self.widgets.db.Discoveries:SetClipRect(self.discoveriesDBArea)
	self.widgets.db.Discoveries:SetEndStops({0, self.discoveriesDBArea[4]*0.1})
	
	self.discoveryTime = -1

	for k,v in pairs(Arm.Discoveries) do
	
		v.picture = World.Load(v.picture)
		v.pictureSize = UI:MaterialSize(v.picture)
		v.pictureSize[3] = v.pictureSize[3] * UI.identityScale[1]
		v.pictureSize[4] = v.pictureSize[4] * UI.identityScale[2]
	
	end
end

function Arm.EnterDiscoveriesDB(self, enter, callback, time)
	if (time == nil) then	
		time = 0
	end
	
	if (enter) then
		Arm:LoadDiscoveries()
		self.widgets.db.DiscoveriesRoot:BlendTo({1,1,1,1}, time)
	else
		self.widgets.db.DiscoveriesRoot:BlendTo({0,0,0,0}, time)
	end
	
	if (callback and (time > 0)) then
		self.dbTimer = World.globalTimers:Add(callback, time, true)
	elseif (callback) then
		callback()
	end
end

function Arm.ClearDiscoveries(self)
	self.discoveryTime = -1
	if (self.discoveryList) then
		for k,v in pairs(self.discoveryList) do
			if (v.button) then
				v.button:Unmap() -- mark gc
			end
		end
		self.discoveryList = nil
	end
	self.widgets.db.Discoveries:Clear()
end

function Arm.DiscoveryPressed(self, discovery)
	self.requestedTopic = discovery.chat
	Arm:TalkPressed()
end

function Arm.LayoutDiscovery(self, discovery, section, state)

	-- discoveries have a title, a picture, and a description
	local y = 0
	
	local f = function()
		Arm:DiscoveryPressed(discovery)
	end
	
	local rect = {
		0,
		y+state.y,
		self.discoveriesDBArea[3],
		state.titleAdvance
	}

	local w
	local title = StringTable.Get(discovery.title)
	local underlineY
	
	if (discovery.chat) then
		w = UI:CreateStylePushButton(
			rect,
			f,
			{
				background = false,
				highlight = { 
					on = {0.75, 0.75, 0.75, 1}, 
					time = 0.2 
				}, 
				typeface = state.titleTypeface,
				pressed = state.sound
			}
		)
		
		self.widgets.db.Discoveries:AddItem(w)
		
		local sw,sh = UI:StringDimensions(state.titleTypeface, title)
		local lastY
		
		if (sw > self.discoveriesDBArea[3]) then
			rect, lastY = UI:LineWrapLJustifyText(w.label, self.discoveriesDBArea[3], true, state.advance, title)
		else
			UI:SetLabelText(w.label, title)
			rect = UI:SizeLabelToContents(w.label)
			lastY = 0
		end
		
		-- expand the button around the label
		local buttonRect = {
			0,
			y+state.y,
			rect[3] + 16*UI.identityScale[1],
			rect[4] + 16*UI.identityScale[2]
		}
			
		w:SetRect(buttonRect)
		w:SetBlendWithParent(true)
		w.highlight:SetRect({0, 0, buttonRect[3], buttonRect[4]})
		rect = CenterChildRectInRect(buttonRect, rect)
		w.label:SetRect(rect)
		
		-- put an underline under the title
		underlineY = y + state.y + lastY + rect[2] + state.titleUnderline
		
		section.button = w
	else
		-- just make a plain text label
		local lastY
		w = UI:CreateWidget("TextLabel", {rect=rect, typeface=state.titleTypeface})
		rect, lastY = UI:LineWrapLJustifyText(w, self.discoveriesDBArea[3], true, state.advance, title)
		self.widgets.db.Discoveries:AddItem(w)
		w:SetBlendWithParent(true)
		w:Unmap() -- mark gc
			
		-- put an underline under the title
		underlineY = y + state.y + lastY + state.titleUnderline
		
	end
	
	section.y = y+state.y

	local underlineWidth = self.discoveriesDBArea[3] - discovery.pictureSize[3] - (12*UI.identityScale[1])
	
	-- put in the picture
	
	y = y + state.titleAdvance + state.titleSpace
	
	rect = {
		self.discoveriesDBArea[3] - (4*UI.identityScale[1]) - discovery.pictureSize[3],
		y + state.y,
		discovery.pictureSize[3],
		discovery.pictureSize[4]
	}
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material=discovery.picture})
	w:SetBlendWithParent(true)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
	-- margins
	local textX = 8 * UI.identityScale[1]
	local textMargin = self.discoveriesDBArea[3] - (2*textX)
	
	local pictureMaxY = rect[4] + (16 * UI.identityScale[2])
	local pictureMargin = rect[1] - (12*UI.identityScale[1]) - textX
	
	-- insert text
	
	local text = StringTable.Get(discovery.text)
	text = string.split(text, "\n")

	local modelStrings = {}
	local textY = 0
	
	for k,v in pairs(text) do

		if (v and (v ~= "")) then
			local a = nil
			local b = v
		
			-- do we have to insert text lines that fit inside this picture area?
			while (textY <= pictureMaxY) do

				a,b = UI:SplitStringAtSize(state.typeface, b, pictureMargin)
					
				local string = {
					x = 0,
					y = textY,
					text = a,
					scaleX = UI.fontScale[1],
					scaleY = UI.fontScale[2]
				}
				
				table.insert(modelStrings, string)
				textY = textY + state.advance
					
				if (b == nil) then
					break
				end
			
			end
			
			if (b ~= nil) then
				local lines = UI:WordWrap(state.typeface, b, textMargin)
				
				for k,v in pairs(lines) do
				
					local string = {
						x = 0,
						y = textY,
						text = v,
						scaleX = UI.fontScale[1],
						scaleY = UI.fontScale[2]
					}
					
					table.insert(modelStrings, string)
					textY = textY + state.advance
				
				end
				
			end
		end
		
		textY = textY + state.advance
	
	end
	
	textY = Max(textY, pictureMaxY)
	
	w = UI:CreateWidget("TextLabel", {rect={textX, y+state.y, 8, 8}, typeface=state.typeface})
	w:SetBlendWithParent(true)
	w:SetText(modelStrings)
	UI:SizeLabelToContents(w)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
	state.y = state.y + y + textY + state.discoverySpace
	
	-- underline last
	
	w = UI:CreateWidget("MatWidget", {rect={0,underlineY, underlineWidth, 7}, material=self.gfx.LineBorder4})
	w:SetBlendWithParent(true)
	self.widgets.db.Discoveries:AddItem(w)
	w:Unmap() -- mark gc
	
end

function Arm.LoadDiscoveries(self)

	if (self.discoveryTime >= GameDB.discoveryTime) then
	
		for k,v in pairs(self.discoveryList) do
			if (v.button) then
				v.button.class:ResetHighlight(v.button, true)
			end
		end
	
		return
	end
	
	Arm:ClearDiscoveries()
	
	self.discoveryTime = GameDB.discoveryTime
	self.discoveryList = {}
	
	for k,v in pairs(Arm.Discoveries) do
		if (GameDB:CheckDiscovery(k)) then
			self.discoveryList[v.index] = v
		end
	end
	
	local state = {
		y = 0,
		titleSpace = 8*UI.identityScale[2],
		advance = UI:FontAdvanceSize(self.typefaces.Chat),
		titleAdvance = UI:FontAdvanceSize(self.typefaces.LogArmAsk) + (12 * UI.identityScale[2]),
		titleTypeface = self.typefaces.LogArmAsk,
		titleUnderline = UI:FontAdvanceSize(self.typefaces.LogArmAsk) + (6 * UI.identityScale[2]),
		typeface = self.typefaces.Chat,
		sound = self.sfx.Button,
		discoverySpace = 32 * UI.identityScale[2]
	}
	
	for k,v in pairs(self.discoveryList) do
		self:LayoutDiscovery(v, v, state)
	end
	
	self.widgets.db.Discoveries:RecalcLayout()

end
