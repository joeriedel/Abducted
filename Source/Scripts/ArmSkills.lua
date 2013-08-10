-- ArmSkills.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

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
	local height = skillsTreeImageRect[4] / skillsTreeImageRect[3] * width
	
	skillsTreeImageRect[3] = width
	skillsTreeImageRect[4] = height
		
	self.skillsTreeRect = {
		0,
		0,
		skillsTreeImageRect[3],
		self.skillsRootWorkspaceSize[4]
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
	
	self.skillsTextRect = {
		self.skillsTreeRect[1] + self.skillsTreeRect[3] + gap,
		0,
		0,
		self.skillsRootWorkspaceSize[4]
	}
	
	self.skillsTextRect[3] = self.skillsRootWorkspaceSize[3] - self.skillsTextRect[1]
	
	self:SpawnSkillsTree(skillsTreeImageRect, scrollBar)
	
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
	
	self.widgets.skills.SkillsTree:SetClipRect(self.skillsTreeArea)
	self.widgets.skills.SkillsTree:SetEndStops({0, self.skillsTreeArea[4]*0.1})
	
	local w = UI:CreateWidget("MatWidget", {rect=imageRect, material=self.gfx.SkillsBackground})
	w:SetBlendWithParent(true)
	self.widgets.skills.SkillsTree:AddItem(w)
	w:Unmap()
	
	self.widgets.skills.SkillsTree:RecalcLayout()
	
end

function Arm.StartSkills(self)
	self.widgets.skills.Root:BlendTo({1,1,1,1}, 0.2)
end

function Arm.EndSkills(self)
	self.widgets.skills.Root:BlendTo({1,1,1,0}, 0.2)
end

function Arm.EndSkills(self)

end

function Arm.SkillsReset(self)
	self.widgets.skills.Root:BlendTo({1,1,1,0}, 0)
end