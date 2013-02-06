-- ArmCharacterDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.CharDBInset = { 32, 16 }
Arm.CharDBTextSpace = {12, 8}
Arm.CharDBSectionHeightPct = 0.42
Arm.CharDBPulsePctSize = 0.95
Arm.CharDBPulseInset = 8

function Arm.SpawnCharacterDB(self)

	if (UI.systemScreen.aspect == "4x3") then
		Arm.CharDBInset[2] = 0
		Arm.CharDBSectionHeightPct = 0.41
	end

	self.charDBWorkspace = {
		self.dbWorkspace[1] + (Arm.CharDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[2] + (Arm.CharDBInset[2] * UI.identityScale[2]),
		self.dbWorkspace[3] - (Arm.CharDBInset[1] * UI.identityScale[1]),
		self.dbWorkspace[4] - (Arm.CharDBInset[2] * UI.identityScale[2])
	}
	
	self.charDBWorkspaceSize = {
		0,
		0,
		self.charDBWorkspace[3],
		self.charDBWorkspace[4]
	}

	self.widgets.db.CharRoot = UI:CreateWidget("Widget", {rect=self.charDBWorkspace})
	self.widgets.db.CharRoot:SetBlendWithParent(true)
	self.widgets.db.Root:AddChild(self.widgets.db.CharRoot)
	
	local charDBSectionHeight = Arm.CharDBSectionHeightPct * self.charDBWorkspaceSize[4]
	
	--[[---------------------------------------------------------------------------
		Name & Portrait
	-----------------------------------------------------------------------------]]

	local y = 0
	local advance = UI:FontAdvanceSize(UI.typefaces.StandardButtonDark)

	local w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_NAME"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, GameDB.playerName)
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_SPECIES"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_HUMAN"))
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_GENDER"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_FEMALE"))
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_AGE"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_UNKNOWN"))
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_BIRTHPLACE"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_UNKNOWN"))
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	local rect = {
		0,
		charDBSectionHeight,
		self.charDBWorkspaceSize[3],
		7
	}
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material = self.gfx.LineBorder4})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	rect = UI:MaterialSize(self.gfx.CharPortrait)
	local h = charDBSectionHeight - (4 * UI.identityScale[2])
	local scale =  h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	
	rect[1] = self.charDBWorkspaceSize[3] - rect[3] - (24 * UI.identityScale[1])
	rect[2] = 0
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.CharPortrait})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	--[[---------------------------------------------------------------------------
		Health Status
	-----------------------------------------------------------------------------]]
	
	local sectionY = charDBSectionHeight + (Arm.CharDBTextSpace[2]) * UI.identityScale[2]
	y = sectionY
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_HEALTH_STATUS"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_GOOD"))
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	rect = UI:MaterialSize(self.gfx.HeartBeat1)
	h = (charDBSectionHeight*2) - (4 * UI.identityScale[2]) - y
	scale = h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	rect[1] = 0
	rect[2] = y
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material = self.gfx.HeartBeat1})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	self.charDBHeartBeatRect = {
		0,
		0,
		rect[3],
		rect[4]
	}
	
	rect = UI:MaterialSize(self.gfx.HeartBeat2)
	h = self.charDBHeartBeatRect[4] * Arm.CharDBPulsePctSize
	scale = h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	rect[2] = 4 * UI.identityScale[2]
	
	self.charDBPulseRect = rect
	
	self.widgets.db.CharPulse = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.HeartBeat2})
	w:AddChild(self.widgets.db.CharPulse)
	self.widgets.db.CharPulse:SetBlendWithParent(true)
	
	UI:VCenterWidget(self.widgets.db.CharPulse, self.charDBHeartBeatRect)
	
	rect = UI:MaterialSize(self.gfx.HumanHeart)
	h = charDBSectionHeight - (8 * UI.identityScale[2])
	scale = h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	rect[1] = self.charDBWorkspaceSize[3] - rect[3] - (self.charDBWorkspaceSize[3] * 0.15)
	rect[2] = sectionY
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.HumanHeart})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	rect = {
		0,
		charDBSectionHeight * 2,
		self.charDBWorkspaceSize[3],
		7
	}
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material = self.gfx.LineBorder4})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	--[[---------------------------------------------------------------------------
		Time played
	-----------------------------------------------------------------------------]]
	
	sectionY = (charDBSectionHeight*2) + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	y = sectionY
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_TIME_PLAYED"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	self.widgets.db.TimePlayed = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(self.widgets.db.TimePlayed)
	self.widgets.db.TimePlayed:SetBlendWithParent(true)
	UI:SetLabelText(self.widgets.db.TimePlayed, "000-00-00-00")
	
	y = y + advance + (Arm.CharDBTextSpace[2] * UI.identityScale[2])
	
	w = UI:CreateWidget("TextLabel", {rect = {0, y, 8, 8}, typeface = UI.typefaces.StandardButtonDark})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, StringTable.Get("ARM_CHARDB_DISCOVERIES"))
	r = UI:SizeLabelToContents(w)
	
	r[1] = r[1] + r[3] + (Arm.CharDBTextSpace[1] * UI.identityScale[1])
	
	w = UI:CreateWidget("TextLabel", {rect = r, typeface = UI.typefaces.StandardButton})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	UI:SetLabelText(w, tostring(GameDB.numDiscoveries))
	
end

function Arm.EnterCharDB(self, enter, callback, time)
	if (time == nil) then
		time = 0
	end
	
	if (enter) then
		Arm:DBAnimateCharHeartbeat(true)
		Arm:DBAnimateTimePlayed(true)
		self.widgets.db.CharRoot:BlendTo({1,1,1,1}, time)
	else
		self.widgets.db.CharRoot:BlendTo({0,0,0,0}, time)
	end
	
	local f = function()
		if (not enter) then
			Arm:DBAnimateCharHeartbeat(false)
			Arm:DBAnimateTimePlayed(false)
		end
		if (callback) then
			callback()
		end
	end
	
	if (time > 0) then
		self.dbTimer = World.globalTimers:Add(f, time, true)
	else
		f()
	end
end

function Arm.DBAnimateCharHeartbeat(self, animate)
	if (not animate) then
		self.sfx.HeartBeat:FadeOutAndStop(0.5)
		if (self.pulseTimer) then
			self.pulseTimer:Clean()
			self.pulseTimer = nil
		end
		return
	end
	
	
	self.sfx.HeartBeat:FadeVolume(1, 0)
	self.sfx.HeartBeat:Rewind()
	self.sfx.HeartBeat:Play(kSoundChannel_UI, 0)
	self:DBPulseStart()	

end

function Arm.DBAnimateTimePlayed(self, animate)

	if (not animate) then
		if (self.dbTimePlayedTimer) then
			self.dbTimePlayedTimer:Clean()
			self.dbTimePlayedTimer = nil
		end
		return
	end
	
	local f = function()
		local s = GameDB:TimePlayedString()
		UI:SetLabelText(self.widgets.db.TimePlayed, s)
	end

	self.dbTimePlayedTimer = World.globalTimers:Add(f, 0)
end

Arm.DBPulseTimes = {0.75, 0.2, 0.4, 0.65}

function Arm.DBPulseStart(self)
	local x = self.charDBHeartBeatRect[3] - ((Arm.CharDBPulseInset+64) * UI.identityScale[1])
	
	self.widgets.db.CharPulse:ScaleTo({1,0}, {0,0})
	self.widgets.db.CharPulse:MoveTo({x, self.charDBPulseRect[2]}, {0,0})
	
	x = (Arm.CharDBPulseInset - 64) * UI.identityScale[1]
	self.widgets.db.CharPulse:MoveTo({x, self.charDBPulseRect[2]}, {Arm.DBPulseTimes[1], 0})
	
	local f = function()
		Arm:DBPulseBeat()
	end
	
	self.pulseTimer = World.globalTimers:Add(f, Arm.DBPulseTimes[2], true)
end

function Arm.DBPulseBeat(self)
	self.widgets.db.CharPulse:ScaleTo({1,1}, {0, 0.1})
	local f = function()
		Arm:DBPulseBeat2()
	end
	self.pulseTimer = World.globalTimers:Add(f, Arm.DBPulseTimes[3], true)
end

function Arm.DBPulseBeat2(self)
	self.widgets.db.CharPulse:ScaleTo({1,0}, {0, 0.1})
	local f = function()
		Arm:DBPulseStart()
	end
	self.pulseTimer = World.globalTimers:Add(f, Arm.DBPulseTimes[4], true)
end
