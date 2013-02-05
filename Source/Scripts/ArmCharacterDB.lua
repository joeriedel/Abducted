-- ArmCharacterDB.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.CharDBInset = { 32, 16 }
Arm.CharDBTextSpace = {12, 8}
Arm.CharDBSectionHeight = 250

function Arm.SpawnCharacterDB(self)

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
		Arm.CharDBSectionHeight * UI.identityScale[2],
		self.charDBWorkspaceSize[3],
		1
	}
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material = self.gfx.LineBorder4})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
	
	rect = UI:MaterialSize(self.gfx.CharPortrait)
	local h = ((Arm.CharDBSectionHeight - 4) * UI.identityScale[2])
	local scale =  h / rect[4]
	rect[4] = h
	rect[3] = rect[3] * scale
	
	rect[1] = self.charDBWorkspaceSize[3] - rect[3] - (24 * UI.identityScale[1])
	rect[2] = 0
	
	w = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.CharPortrait})
	self.widgets.db.CharRoot:AddChild(w)
	w:SetBlendWithParent(true)
end
