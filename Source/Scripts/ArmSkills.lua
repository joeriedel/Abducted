-- ArmSkills.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Arm.SkillIconSize = {128, 128}

function Arm.SpawnSkills(self)

	local rect = {
		Arm.DBBorder[1] * UI.identityScale[1],
		Arm.DBBorder[2] * UI.identityScale[2],
		0,
		0
	}
	
	rect[3] = self.workspaceLeftSize[3] - rect[1] + (Arm.DBBorder[3] * UI.identityScale[1])
	rect[4] = self.workspaceLeftSize[4] - rect[2] + (Arm.DBBorder[4] * UI.identityScale[2])
	
	self.skillsRootWorkspaceSize = {
		0,
		0,
		rect[3],
		rect[4]
	}

	self.widgets.skills = {}
	self.widgets.skills.Root = UI:CreateWidget("Widget", {rect=rect})
	self.widgets.skills.Root:BlendTo({1,1,1,0}, 0)
	self.widgets.WorkspaceLeft:AddChild(self.widgets.skills.Root)
	
	local skillsTreeImageRect = UI:MaterialSize(self.gfx.SkillsBackground)
	local scale = skillsTreeImageRect[3] / 2048
	local width = scale * UI.screenWidth
	
	scale = width / skillsTreeImageRect[3]
	
	local height = skillsTreeImageRect[4] * scale
	
	self.SkillsTreeScale = scale
	
	skillsTreeImageRect[3] = width
	skillsTreeImageRect[4] = height
	
	self.skillsFontScale = {1,1}
	
	if (UI.mode == kGameUIMode_Mobile) then
		if (UI.screenWidth == 960) then
--			self.skillsFontScale = {0.75, 0.75}
		end
	end
	
	local skillPointsFontH = UI:FontAdvanceSize(self.typefaces.SkillPoints, self.skillsFontScale)
		
	self.skillsTreeRect = {
		0,
		0,
		skillsTreeImageRect[3],
		self.skillsRootWorkspaceSize[4] - self.menuButtonSize[2] - (8*UI.identityScale[1])
	}
	
	local gap = 24 * UI.identityScale[1]
	
	local scrollBar = UI.mode == kGameUIMode_PC
	local scrollGap = 0
	if (scrollBar) then
		scrollGap = 24
	end
	
	self.skillsTreeRect[3] = self.skillsTreeRect[3] + scrollGap
	
	self.skillsTreeArea = {
		0,
		0,
		self.skillsTreeRect[3],
		self.skillsTreeRect[4]
	}
	
	local lineSpace = 28 * UI.identityScale[1]
	
	local w = UI:CreateWidget("MatWidget", {
		rect={self.skillsTreeRect[1] + self.skillsTreeRect[3], 0, lineSpace, self.skillsTreeRect[4]},
		material=self.gfx.SkillsThinVertLines2
	})
	
	self.skillsTreeFooter = {
		self.skillsTreeRect[1],
		self.skillsTreeRect[2] + self.skillsTreeRect[4] + (10*UI.identityScale[1]),
		self.skillsRootWorkspaceSize[3],
		0
	}
	
	self.skillsTreeFooter[4] = self.skillsRootWorkspaceSize[4] - self.skillsTreeFooter[2]
	
	w:SetBlendWithParent(true)
	self.widgets.skills.Root:AddChild(w)
	
	local titleSize = self.skillsRootWorkspaceSize[4] * 0.2
	
	self.skillsTextRect = {
		self.skillsTreeRect[1] + self.skillsTreeRect[3] + lineSpace,
		0,
		0,
		self.skillsRootWorkspaceSize[4]
	}
	
	self.skillsTextRect[3] = self.skillsRootWorkspaceSize[3] - self.skillsTextRect[1]
	
	self.skillsTitleRect = {
		self.skillsTextRect[1],
		self.skillsTextRect[2],
		self.skillsTextRect[3],
		titleSize
	}
	
	self.skillsDescriptionRect = {
		self.skillsTextRect[1],
		self.skillsTitleRect[2] + self.skillsTitleRect[4] + (10*UI.identityScale[2]),
		self.skillsTextRect[3],
		0
	}
	
	self.skillsDescriptionRect[4] = self.skillsRootWorkspaceSize[4] - self.skillsDescriptionRect[2] - self.skillsTreeFooter[4] - (10*UI.identityScale[2])
	
	self.skillsDescriptionArea = {
		0,
		0,
		self.skillsDescriptionRect[3] - scrollGap,
		self.skillsDescriptionRect[4]
	}
	
	w = UI:CreateWidget("MatWidget",
		{rect={self.skillsTitleRect[1], self.skillsTitleRect[2]+self.skillsTitleRect[4], self.skillsTextRect[3], 10*UI.identityScale[2]}, material=self.gfx.SkillsHorzLine1}
	)
	w:SetBlendWithParent(true)
	self.widgets.skills.Root:AddChild(w)
	
	w = UI:CreateWidget("MatWidget",
		{rect={self.skillsTreeRect[1], self.skillsTreeRect[2]+self.skillsTreeRect[4], self.skillsRootWorkspaceSize[3], 10*UI.identityScale[2]}, material=self.gfx.SkillsHorzLine1}
	)
	w:SetBlendWithParent(true)
	self.widgets.skills.Root:AddChild(w)
	
	self.widgets.skills.Title = UI:CreateWidget("TextLabel", {rect=self.skillsTitleRect,typeface=self.typefaces.SkillTitle})
	self.widgets.skills.Title:SetBlendWithParent(true)
	self.widgets.skills.Root:AddChild(self.widgets.skills.Title)
	
	self.widgets.skills.DescriptionList = UI:CreateWidget("VListWidget", {rect=self.skillsDescriptionRect})
	
	if (scrollBar) then
		UI:CreateVListWidgetScrollBar(
			self.widgets.skills.DescriptionList,
			24,
			24,
			8
		)
	end
	
	self.widgets.skills.DescriptionList:SetClipRect({self.skillsDescriptionRect[1],self.skillsDescriptionRect[2],self.skillsDescriptionArea[3],self.skillsDescriptionArea[4]})
	self.widgets.skills.DescriptionList:SetEndStops({0, self.skillsDescriptionRect[4]*0.1})
	self.widgets.skills.Root:AddChild(self.widgets.skills.DescriptionList)
	
	self.widgets.skills.ShortDescriptionText = UI:CreateWidget("TextLabel", {rect=self.skillsDescriptionArea,typeface=self.typefaces.SkillDescription})
	self.widgets.skills.ShortDescriptionText:SetBlendWithParent(true)
	self.widgets.skills.DescriptionList:AddItem(self.widgets.skills.ShortDescriptionText)
	
	self.widgets.skills.DescriptionDivider = UI:CreateWidget("MatWidget",
		{rect={0, 0, self.skillsDescriptionArea[3], 10*UI.identityScale[2]}, material=self.gfx.SkillsHorzLine1}
	)
	self.widgets.skills.DescriptionDivider:SetBlendWithParent(true)
	self.widgets.skills.DescriptionDivider:SetVisible(false)
	self.widgets.skills.DescriptionList:AddItem(self.widgets.skills.DescriptionDivider)
	
	self.widgets.skills.StatsText = UI:CreateWidget("TextLabel", {rect=self.skillsDescriptionArea,typeface=self.typefaces.SkillStats})
	self.widgets.skills.StatsText:SetBlendWithParent(true)
	self.widgets.skills.DescriptionList:AddItem(self.widgets.skills.StatsText)
	
	self.widgets.skills.StatsDivider = UI:CreateWidget("MatWidget",
		{rect={0, 0, self.skillsDescriptionArea[3], 10*UI.identityScale[2]}, material=self.gfx.SkillsHorzLine1}
	)
	self.widgets.skills.StatsDivider:SetBlendWithParent(true)
	self.widgets.skills.StatsDivider:SetVisible(false)
	self.widgets.skills.DescriptionList:AddItem(self.widgets.skills.StatsDivider)
	
	self.widgets.skills.LongDescriptionText = UI:CreateWidget("TextLabel", {rect=self.skillsDescriptionArea,typeface=self.typefaces.SkillDetails})
	self.widgets.skills.LongDescriptionText:SetBlendWithParent(true)
	self.widgets.skills.DescriptionList:AddItem(self.widgets.skills.LongDescriptionText)
	
	local y = self.skillsTreeFooter[2] + self.skillsTreeFooter[4] - self.menuButtonSize[2]
	local buttonSpace = 16 * UI.identityScale[1]
	local buttonWidth = (self.skillsTreeFooter[3]-(buttonSpace*2)) / 3
	local controlSize = buttonWidth*3
	local buttonWidth2 = buttonWidth * 1.15
	buttonWidth = (controlSize-buttonWidth2) / 2
	controlSize = controlSize + (buttonSpace*2)
	local x = self.skillsTreeFooter[1] + (self.skillsTreeFooter[3] - controlSize) / 2
			
	self.widgets.skills.Store = UI:CreateStylePushButton(
		{ x, y, buttonWidth, self.menuButtonSize[2] },
		function () self:OnStoreButtonPressed() end,
		{ pressed = self.sfx.Button, highlight={on={0,0,0,0}} },
		self.widgets.skills.Root
	)
	self.widgets.skills.Store:SetBlendWithParent(true)
	
	local text = StringTable.Get("SKILL_STORE")
	UI:LineWrapCenterText(
		self.widgets.skills.Store.label,
		nil,
		nil,
		0,
		text,
		UI.identityScale,
		UI.invIdentityScale
	)
	
	x = x + buttonWidth + buttonSpace
	
	self.widgets.skills.Train = UI:CreateStylePushButton(
		{ x, y, buttonWidth, self.menuButtonSize[2] },
		function () self:OnTrainButtonPressed() end,
		{ pressed = self.sfx.Button, highlight={on={0,0,0,0}} },
		self.widgets.skills.Root
	)
	self.widgets.skills.Train:SetBlendWithParent(true)
	
	local text = StringTable.Get("SKILL_TRAIN")
	UI:LineWrapCenterText(
		self.widgets.skills.Train.label,
		nil,
		nil,
		0,
		text,
		UI.identityScale,
		UI.invIdentityScale
	)
	
	self.widgets.skills.Train:SetVisible(false)
	
	x = x + buttonWidth + buttonSpace
	
	self.widgets.skills.Purchase = UI:CreateStylePushButton(
		{ x, y, buttonWidth2, self.menuButtonSize[2]/1.5 },
		function () self:OnPurchaseButtonPressed() end,
		{ pressed = self.sfx.Button, highlight={on={0,0,0,0}} },
		self.widgets.skills.Root
	)
	self.widgets.skills.Purchase:SetBlendWithParent(true)
	
	local text = StringTable.Get("SKILL_PURCHASE")
	UI:LineWrapCenterText(
		self.widgets.skills.Purchase.label,
		nil,
		nil,
		0,
		text,
		UI.identityScale,
		UI.invIdentityScale
	)
	
	self.widgets.skills.Purchase:SetVisible(false)
	
	self.skillPointsLabelRect = {x, y+(self.menuButtonSize[2]/1.5), buttonWidth2, 0}
	self.skillPointsLabelRect[4] = self.skillsRootWorkspaceSize[4] - self.skillPointsLabelRect[2]
	self.widgets.skills.SkillPointsLabel = UI:CreateWidget("TextLabel", {
		rect=self.skillPointsLabelRect,
		typeface=self.typefaces.SkillPoints
	})
	
	self.widgets.skills.SkillPointsLabel:SetBlendWithParent(true)
	self.widgets.skills.Root:AddChild(self.widgets.skills.SkillPointsLabel)
	
	self:SpawnSkillsTree(skillsTreeImageRect, scrollBar)
	
	self.selectedSkill = nil
	
	Arm:UpdateSkillsUI()
end

function Arm.CreateSkillInterace(self, skill)

	local iconRect = {skill.Graphics.Icon.Pos[1], skill.Graphics.Icon.Pos[2], Arm.SkillIconSize[1], Arm.SkillIconSize[2]}
	iconRect[1] = iconRect[1] * self.SkillsTreeScale
	iconRect[2] = iconRect[2] * self.SkillsTreeScale
	iconRect[3] = iconRect[3] * self.SkillsTreeScale
	iconRect[4] = iconRect[4] * self.SkillsTreeScale
	
	local m = World.Load(skill.Graphics.Icon.Material)
	local w = UI:CreateWidget("MatWidget", {rect=iconRect,material=m})
	w:SetBlendWithParent(true)
	w:SetVisible(false)
	self.widgets.skills.SkillsTree:AddItem(w)
	
	skill.Graphics.Icon.Widget = w
	
	local d = Arm.gfx.SkillLevel[1]:Dimensions()
	
	local h = d[2] * self.SkillsTreeScale
	local skillRect = {0,0,iconRect[3],0}
	
	skillRect[1] = iconRect[4] - h - (16 * UI.identityScale[2])
	skillRect[4] = h
		
	local skillIcon = UI:CreateWidget("MatWidget", {rect=skillRect})
	skillIcon:SetBlendWithParent(true)
	skillIcon:SetVisible(false)
	w:AddChild(skillIcon)
	
	skill.Graphics.Icon.SkillIcon = skillIcon
	skill.Graphics.Icon.SkillRect = skillRect
	
	if (skill.Graphics.Lines) then
	
		for k,v in pairs(skill.Graphics.Lines) do
		
			m = Arm.gfx[v.Material]
			assert(m)
			d = m:Dimensions()
			
			local r = {v.Pos[1]*self.SkillsTreeScale,v.Pos[2]*self.SkillsTreeScale,d[1]*self.SkillsTreeScale,d[2]*self.SkillsTreeScale}
			w = UI:CreateWidget("MatWidget", {rect=r,material=m})
			w:SetVisible(false)
			
			if (v.Rotation) then
				w:RotateTo({r[3]/2, r[4]/2, v.Rotation}, {0,0,0})
			end
			
			self.widgets.skills.SkillsTree:AddItem(w)
			v.Widget = w
		end
	
	end
	
	local f = function(widget, e)
		return self:OnSkillWidgetInputEvent(skill, e)
	end
	
	w = UI:CreateWidget("Widget", {rect=iconRect,OnInputEvent=f})
	self.widgets.skills.SkillsTree:AddItem(w)
	skill.UI = w
	
end

function Arm.SpawnSkillsTree(self, imageRect, scrollBar)
	self.widgets.skills.SkillsTree = UI:CreateWidget("VListWidget", {rect=self.skillsTreeRect})
	self.widgets.skills.Root:AddChild(self.widgets.skills.SkillsTree)
	self.widgets.skills.SkillsTree:SetBlendWithParent(true)
	
	if (scrollBar) then
		UI:CreateVListWidgetScrollBar(
			self.widgets.skills.SkillsTree,
			24,
			24,
			8
		)
	end
	
	self.widgets.skills.SkillsTree:SetClipRect(self.skillsTreeRect)
	self.widgets.skills.SkillsTree:SetEndStops({0, self.skillsTreeRect[4]*0.1})
	
	local w = UI:CreateWidget("MatWidget", {rect=imageRect, material=self.gfx.SkillsBackground})
	w:SetBlendWithParent(true)
	self.widgets.skills.SkillsTree:AddItem(w)
	w:Unmap()
	
	self:CreateSkillInterace(PlayerSkills.Data.FastHands)
	
	self.widgets.skills.Highlight = UI:CreateWidget("MatWidget", {rect={0,0,Arm.SkillIconSize[1]*self.SkillsTreeScale, Arm.SkillIconSize[2]*self.SkillsTreeScale},material=self.gfx.SkillsHighlight})
	self.widgets.skills.Highlight:SetBlendWithParent(true)
	self.widgets.skills.Highlight:SetVisible(false)
	self.widgets.skills.SkillsTree:AddItem(self.widgets.skills.Highlight)
	
	self.widgets.skills.SkillsTree:RecalcLayout()
	
end

function Arm.UpdateSkillsUI(self)
	self:UpdateSkillPointsLabel()
end

function Arm.UpdateSkillUI(self, skill)
end

function Arm.UpdateSkillPointsLabel(self)

	local text = StringTable.Get("SKILL_POINTS").." "..tostring(PlayerSkills.SkillPoints)
	UI:SetLabelText(self.widgets.skills.SkillPointsLabel, text, self.skillsFontScale)
	UI:CenterLabel(self.widgets.skills.SkillPointsLabel, self.skillPointsLabelRect)
	
end

function Arm.OnSkillWidgetInputEvent(self, skill, e)

	if (Input.IsTouchBegin(e)) then
		self:HighlightSkill(skill)
		skill.busy = e.touch
		skill.UI:SetCapture(true)
		return true
	elseif (skill.busy and Input.IsTouchMove(e)) then
		return true
	elseif (skill.busy and Input.IsTouchEnd(e, skill.busy)) then
		skill.busy = nil
		skill.UI:SetCapture(false)
		
		if (e.type == kI_TouchCancelled) then
			self:HighlightSkill() -- reset
		else
			self:SelectSkill(skill)
		end
		return true
	end

	return false
end

function Arm.OnStoreButtonPressed(self)

end

function Arm.OnTrainButtonPressed(self)

end

function Arm.OnPurchaseButtonPressed(self)

end

function Arm.SelectSkill(self, skill)
	self.selectedSkill = skill
	Arm:HighlightSkill()
	
	UI:LineWrapAlignTopLJustifyText(
		self.widgets.skills.Title, 
		self.skillsTitleRect[3],
		false,
		0,
		StringTable.Get(skill.Title),
		self.skillsFontScale
	)
	
	local r = UI:LineWrapAlignTopLJustifyText(
		self.widgets.skills.ShortDescriptionText, 
		self.skillsDescriptionArea[3],
		true,
		0,
		StringTable.Get(skill.ShortDescription),
		self.skillsFontScale
	)
	
	self.widgets.skills.DescriptionDivider:MoveTo({0, r[2]+r[4]}, {0,0})
	self.widgets.skills.DescriptionDivider:SetVisible(true)
	r[4] = r[4] + (10*UI.identityScale[2])
	
	local level = skill:CurrentLevel()
	local text = ""
	
	if (skill.Stats) then
		text = skill:Stats(level)
	end
	
	if (skill[level+1]) then
		text = text.."\n\n"..StringTable.Get("SKILL_NEXT_LEVEL").."\n"
		
		if (skill.Stats) then
			text = text.."\n"..skill:Stats(level+1)
		end
		
		text = text.."\n"..StringTable.Get("SKILL_COST").." "..tostring(skill[level+1].Cost).." "..StringTable.Get("ARM_REWARD_SKILLPOINTS")
		
		self.widgets.skills.Purchase:SetVisible(true)
	else
		self.widgets.skills.Purchase:SetVisible(false)
	end
	
	self.widgets.skills.Train:SetVisible(true)
	
	self.widgets.skills.StatsText:SetRect({0,r[2]+r[4], self.skillsDescriptionRect[3], 8})
	
	r = UI:LineWrapAlignTopLJustifyText(
		self.widgets.skills.StatsText, 
		self.skillsDescriptionArea[3],
		true,
		0,
		text,
		self.skillsFontScale
	)
	
	if (skill.LongDescription) then
		self.widgets.skills.StatsDivider:SetVisible(true)
		self.widgets.skills.LongDescriptionText:SetVisible(true)
		
		self.widgets.skills.StatsDivider:MoveTo({0,r[2]+r[4]}, {0,0})
		r[4] = r[4] + (10*UI.identityScale[1])
		self.widgets.skills.LongDescriptionText:SetRect({0,r[2]+r[4],self.skillsDescriptionRect[3],8})
		
		UI:LineWrapAlignTopLJustifyText(
			self.widgets.skills.LongDescriptionText, 
			self.skillsDescriptionArea[3],
			true,
			0,
			StringTable.Get(skill.LongDescription),
			self.skillsFontScale
		)
	else
		self.widgets.skills.StatsDivider:SetVisible(false)
		self.widgets.skills.LongDescriptionText:SetVisible(false)
	end
	
	self.widgets.skills.DescriptionList:RecalcLayout()
end

function Arm.HighlightSkill(self, skill)

	if (skill == nil) then
		skill = self.selectedSkill
	end
	
	if (skill) then
		self.widgets.skills.Highlight:SetVisible(true)
		local r = skill.Graphics.Icon.Widget:Rect()
		local hr = self.widgets.skills.Highlight:Rect()
		hr[1] = r[1]
		hr[2] = r[2]
		self.widgets.skills.Highlight:SetRect(hr)
	else
		self.widgets.skills.Highlight:SetVisible(false)
	end

end

function Arm.StartSkills(self)
	self.widgets.skills.Root:BlendTo({1,1,1,1}, 0.2)
end

function Arm.EndSkills(self, callback)
	self.widgets.skills.Root:BlendTo({1,1,1,0}, 0.2)
	World.globalTimers:Add(callback, 0.2)
end

function Arm.SkillsReset(self)
	self.widgets.skills.Root:BlendTo({1,1,1,0}, 0)
end