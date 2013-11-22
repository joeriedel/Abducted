-- StoreUI.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

StoreUI = Class:New()
StoreUI.kItemsWidth = 0.6
StoreUI.kDescriptionWidth = 1 - StoreUI.kItemsWidth
StoreUI.kButtonBarHeight = 130
StoreUI.kCloseButtonHeight = 75
StoreUI.kProductHeight = 130
StoreUI.kProductIconSize = 100

function StoreUI.InitForGame(self)

	local screenRect = {0, 0, 0.7*UI.fullscreenRect[3], 0.6*UI.fullscreenRect[4]*UI.fontScale[2]}
	screenRect = CenterRectInRect(screenRect, UI.fullscreenRect)

	self.widgets = {}
	self.widgets.root = UI:CreateRoot(UI.kLayer_Store, AlertPanel.EatInput)
	self.widgets.panel = UI:CreateWidget("Widget", {rect=screenRect})
	self.widgets.root:AddChild(self.widgets.panel)
	self.widgets.root:SetVisible(false)
	
	StoreUI:InitUI(true)
	
	self.widgets.panel:SetRect(self.widgets.border:Rect())
	UI:CenterWidget(self.widgets.panel, UI.fullscreenRect)
end

function StoreUI.InitForMainMenu(self, mmPanel)

end

function StoreUI.InitUI(self, closeButton)

	for k,v in pairs(Store.Products) do
		v.Icon = World.Load(v.Icon)
		if (v.Image) then
			v.Image = World.Load(v.Image)
		end
	end

	self.gfx = {}
	self.gfx.Border = World.Load("UI/alertpanelbackground_M")
	self.gfx.LineBorder = World.Load("UI/vertlines2_M")
	self.gfx.HorzLine = World.Load("UI/horizline1_M")
	self.gfx.LineBorder4 = World.Load("UI/lineborder4_M")
	self.gfx.MMItemBackground = World.Load("UI/MMItemBackground_M")
	self.gfx.MMItemBackground2 = World.Load("UI/MMItemBackground3_M")
	self.gfx.Spinner = World.Load("UI/Spinner_M")
	
	self.typefaces = {}
	self.typefaces.Gold = World.Load("UI/MMGold_TF")
	self.typefaces.ItemTitle = World.Load("UI/StoreItemTitle_TF")
	self.typefaces.DescTitle = World.Load("UI/StoreItemDescTitle_TF")
	self.typefaces.DescText = World.Load("UI/SkillDescription_TF")
	self.typefaces.PriceSmall = World.Load("UI/PriceSmall_TF")
	self.typefaces.PriceLarge = World.Load("UI/PriceLarge_TF")
	self.typefaces.StatusRed = World.Load("UI/StoreItemStatusRed_TF")
	self.typefaces.StatusGreen = World.Load("UI/StoreItemStatusGreen_TF")
	self.typefaces.StatusOrange = World.Load("UI/StoreItemStatusOrange_TF")
	
	self.contentWidgets = {}
	
	local screenRect = self.widgets.panel:Rect()
	local contentRect = { 0, 0, screenRect[3], screenRect[4] }
	local panelBorder = 8--*UI.identityScale[1]
	
	local closeButtonHeight = 0
	if (closeButton) then
		closeButtonHeight = StoreUI.kCloseButtonHeight * UI.identityScale[2]
	end
	
	local borderRect = {contentRect[1], contentRect[2], contentRect[3], contentRect[4]+closeButtonHeight}
	self.widgets.border = UI:CreateWidget("MatWidget", {rect=borderRect, material=self.gfx.Border})
	self.widgets.panel:AddChild(self.widgets.border)
	
	local outerContents = {0, 0, contentRect[3], contentRect[4]}
	self.widgets.contents = UI:CreateWidget("Widget", {rect=contentRect})
	self.widgets.border:AddChild(self.widgets.contents)
	
	-- title
	self.widgets.title = UI:CreateWidget("TextLabel", {rect={0,8,8,8}, typeface=self.typefaces.Gold})
	UI:SetLabelText(self.widgets.title, StringTable.Get("STORE_TITLE"))
	UI:SizeLabelToContents(self.widgets.title)
	self.widgets.contents:AddChild(self.widgets.title)
	self.widgets.title:SetBlendWithParent(true)
	
	local titleRect = UI:HCenterLabel(self.widgets.title, contentRect)
	contentRect[2] = titleRect[2] + titleRect[4] + 4
	contentRect[4] = contentRect[4] - contentRect[2]
	
	local titleLineRect = {0, contentRect[2], contentRect[3], panelBorder}
	self.widgets.titleLine = UI:CreateWidget("MatWidget", {rect=titleLineRect, material=self.gfx.HorzLine})
	self.widgets.titleLine:SetBlendWithParent(true)
	self.widgets.contents:AddChild(self.widgets.titleLine)
	
	contentRect[2] = contentRect[2] + panelBorder
	contentRect[4] = contentRect[4] - panelBorder
	
	local buttonBarHeight = StoreUI.kButtonBarHeight * UI.identityScale[2]
	local controlsRect = {0, contentRect[2], contentRect[3], contentRect[4] - buttonBarHeight}
	local buttonBarRect = {0, contentRect[2]+contentRect[4] - buttonBarHeight, contentRect[3], buttonBarHeight}
	local listRect = {8, controlsRect[2], controlsRect[3]*StoreUI.kItemsWidth-8, controlsRect[4]}
	local descRect = {listRect[3]+panelBorder, controlsRect[2], controlsRect[3]*StoreUI.kDescriptionWidth-panelBorder, controlsRect[4]}
	
	local lineRect = {listRect[3], controlsRect[2], panelBorder, controlsRect[4]}
	
	self.widgets.line = UI:CreateWidget("MatWidget", {rect=lineRect, material=self.gfx.LineBorder})
	self.widgets.line:SetBlendWithParent(true)
	self.widgets.contents:AddChild(self.widgets.line)
	table.insert(self.contentWidgets, self.widgets.line)
	
	local lineRect2 = {0, controlsRect[2]+controlsRect[4], contentRect[3], panelBorder}
	self.widgets.line2 = UI:CreateWidget("MatWidget", {rect=lineRect2, material=self.gfx.HorzLine})
	self.widgets.line2:SetBlendWithParent(true)
	self.widgets.contents:AddChild(self.widgets.line2)
	table.insert(self.contentWidgets, self.widgets.line2)
	
	if (closeButton) then
		local lineRect3 = {0, buttonBarRect[2]+buttonBarRect[4]+4, borderRect[3], panelBorder}
		self.widgets.line3 = UI:CreateWidget("MatWidget", {rect=lineRect3, material=self.gfx.HorzLine})
		self.widgets.line3:SetBlendWithParent(true)
		self.widgets.contents:AddChild(self.widgets.line3)
	
		self.widgets.closeButton = UI:CreateStylePushButton(
			{0,0,8,8},
			function() StoreUI:ClosePressed() end,
			{highlight={on={0,0,0,0}}, solidBackground = true}, 
			self.widgets.contents
		)
		
		local rect = UI:LineWrapCenterText(
			self.widgets.closeButton.label,
			contentRect[3]*0.2,
			true,
			0,
			StringTable.Get("STORE_CLOSE_BTN")
		)
		
		rect = ExpandRect(rect, 64*UI.identityScale[1], 16*UI.identityScale[2])
		rect[1] = (borderRect[3]-rect[3])/2
		rect[2] = borderRect[4]-rect[4]-8*UI.identityScale[2]+4
		
		self.widgets.closeButton:SetRect(rect)
		self.widgets.closeButton.highlight:SetRect({0,0,rect[3],rect[4]})
		UI:CenterLabel(self.widgets.closeButton.label, {0,0,rect[3],rect[4]})
		
		self.widgets.closeButton:SetBlendWithParent(true)
	end
	
	self.widgets.restoreButton = UI:CreateStylePushButton(
		{0,0,8,8},
		function() StoreUI:RestoreItemsPressed() end,
		{highlight={on={0,0,0,0}}, solidBackground = true}, 
		self.widgets.contents
	)
	
	local rect = UI:LineWrapCenterText(
		self.widgets.restoreButton.label,
		contentRect[3] * 0.4,
		true,
		0,
		StringTable.Get("STORE_RESTORE_BTN")
	)
	
	rect = ExpandRect(rect, 32*UI.identityScale[1], 32*UI.identityScale[2])
	rect[1] = buttonBarRect[1] + 10*UI.identityScale[1]
	rect[2] = buttonBarRect[2] + 10*UI.identityScale[1]
	self.widgets.restoreButton:SetRect(rect)
	self.widgets.restoreButton.highlight:SetRect({0,0,rect[3],rect[4]})
	UI:CenterLabel(self.widgets.restoreButton.label, {0,0,rect[3],rect[4]})
	self.widgets.restoreButton:SetBlendWithParent(true)
	table.insert(self.contentWidgets, self.widgets.restoreButton)
	
	self.widgets.purchaseButton = UI:CreateStylePushButton(
		{0,0,8,8},
		function() StoreUI:PurchasePressed() end,
		{highlight={on={0,0,0,0}}, solidBackground = true}, 
		self.widgets.contents
	)
	
	rect = UI:LineWrapCenterText(
		self.widgets.purchaseButton.label,
		contentRect[3] * 0.4,
		true,
		0,
		StringTable.Get("STORE_PURCHASE_BTN")
	)
	
	rect = ExpandRect(rect, 32*UI.identityScale[1], 32*UI.identityScale[2])
	rect[1] = descRect[1] + (descRect[3] - rect[3])/2
	rect[2] = buttonBarRect[2] + buttonBarRect[4] - rect[4]
	self.widgets.purchaseButton:SetRect(rect)
	self.widgets.purchaseButton.highlight:SetRect({0,0,rect[3],rect[4]})
	UI:CenterLabel(self.widgets.purchaseButton.label, {0,0,rect[3],rect[4]})
	self.widgets.purchaseButton:SetBlendWithParent(true)
	self.widgets.purchaseButton:SetVisible(false)
	table.insert(self.contentWidgets, self.widgets.purchaseButton)
	
	self.widgets.priceLabel = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.PriceLarge})
	self.widgets.priceLabel:SetBlendWithParent(true)
	self.widgets.contents:AddChild(self.widgets.priceLabel)
	self.priceRect = {descRect[1], buttonBarRect[2]+4, descRect[3], 32*UI.identityScale[2]}
	table.insert(self.contentWidgets, self.widgets.priceLabel)
	
	local descTitleAdv = UI:FontAdvanceSize(self.typefaces.DescTitle)*2
	
	self.widgets.descTitleLine = UI:CreateWidget("MatWidget", {rect={descRect[1],descRect[2]+descTitleAdv,descRect[3],panelBorder}, material=self.gfx.HorzLine})
	self.widgets.contents:AddChild(self.widgets.descTitleLine)
	self.widgets.descTitleLine:SetBlendWithParent(true)
	table.insert(self.contentWidgets, self.widgets.descTitleLine)
	
	self.descTitleRect = {descRect[1],descRect[2],descRect[3],descTitleAdv}
	self.widgets.descTitle = UI:CreateWidget("TextLabel", {rect=self.descTitleRect, typeface=self.typefaces.DescTitle})
	self.widgets.contents:AddChild(self.widgets.descTitle)
	self.widgets.descTitle:SetBlendWithParent(true)
	table.insert(self.contentWidgets, self.widgets.descTitle)
	
	descRect[4] = descRect[4] - self.descTitleRect[4] - 4
	descRect[2] = self.descTitleRect[2] + self.descTitleRect[4] + 4
	
	self.widgets.descList = UI:CreateWidget("VListWidget", {rect=descRect})
	self.widgets.contents:AddChild(self.widgets.descList)
	self.widgets.descList:SetBlendWithParent(true)
	table.insert(self.contentWidgets, self.widgets.descList)
	
	if (UI.mode == kGameUIMode_PC) then
        UI:CreateVListWidgetScrollBar(
            self.widgets.descList,
	        24,
	        24,
	        8
		)
	    descRect[3] = descRect[3] - 24
    end
    
	self.widgets.descList:SetClipRect(descRect)
	self.widgets.descList:SetEndStops({0, descRect[4]*0.1})
	
	self.widgets.descText = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.DescText})
	self.widgets.descText:SetBlendWithParent(true)
	self.widgets.descList:AddItem(self.widgets.descText)
	
	self.descRect = descRect
	
	self.widgets.descImage = UI:CreateWidget("MatWidget", {rect={0,0,8,8}})
	self.widgets.descImage:SetBlendWithParent(true)
	self.widgets.descList:AddItem(self.widgets.descImage)
	
	self.widgets.productList = UI:CreateWidget("VListWidget", {rect=listRect})
	self.widgets.contents:AddChild(self.widgets.productList)
	self.widgets.productList:SetBlendWithParent(true)
	table.insert(self.contentWidgets, self.widgets.productList)
	
	if (UI.mode == kGameUIMode_PC) then
        UI:CreateVListWidgetScrollBar(
            self.widgets.productList,
	        24,
	        24,
	        8
		)
	    listRect[3] = listRect[3] - 24
    end
	
	self.widgets.productList:SetClipRect(listRect)
	self.widgets.productList:SetEndStops({0, listRect[4]*0.1})
	
	self.productsWidth = listRect[3]-panelBorder
	
	self.widgets.connectingText = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.Gold})
	UI:SetLabelText(self.widgets.connectingText, StringTable.Get("STORE_CONNECTING"))
	self.widgets.contents:AddChild(self.widgets.connectingText)
	self.widgets.connectingText:SetBlendWithParent(true)
	UI:SizeLabelToContents(self.widgets.connectingText)
	local connectingRect = UI:CenterLabel(self.widgets.connectingText, outerContents)
	
	local spinnerSize = 64*UI.identityScale[1]
	local spinnerRect = {connectingRect[1]-spinnerSize-8, connectingRect[2]+(connectingRect[4]-spinnerSize)/2, spinnerSize, spinnerSize}
	self.widgets.connectingSpinner = UI:CreateWidget("MatWidget", {rect=spinnerRect, material=self.gfx.Spinner})
	self.widgets.connectingSpinner:SetBlendWithParent(true)
	self.widgets.contents:AddChild(self.widgets.connectingSpinner)
	
	for k,v in pairs(self.contentWidgets) do
		v:BlendTo({1,1,1,0}, 0)
	end
end

function StoreUI.ProductsListReady(self)

	self:CreateProductWidgets(self.productsWidth)
	
	if (self.productWidgets) then
		self:SelectProduct(self.productWidgets[next(self.productWidgets)])
	end
	
	for k,v in pairs(self.contentWidgets) do
		v:BlendTo({1,1,1,1}, 0.3)
	end
	
	self.widgets.connectingText:BlendTo({1,1,1,0}, 0.3)
	self.widgets.connectingSpinner:BlendTo({1,1,1,0}, 0.3)

end

function StoreUI.SelectProduct(self, panel)

	if (self.selected) then
		self.selected:SetMaterial(self.gfx.MMItemBackground)
	end
	
	self.selected = panel
	panel:SetMaterial(self.gfx.MMItemBackground2)
	self:UpdateProductState(panel)
	
end

function StoreUI.CreateProductWidgets(self, width)

	local yOfs = 0
	
	self.productWidgets = {}
	
	if (Store.validProducts) then
		local first = true
		
		for k,v in pairs(Store.validProducts) do
			if (not first) then
				local w = self:CreateLineBorder(yOfs, width)
				yOfs = yOfs + 7
				self.widgets.productList:AddItem(w)
			end
			
			local w,h = self:CreateProductWidget(Store.ProductsById[v], width, yOfs)
			self.widgets.productList:AddItem(w)
			self.productWidgets[v] = w
			yOfs = yOfs + h
			first = false
		end
	end

	self.widgets.productList:RecalcLayout()
end

function StoreUI.OnProductInputEvent(self, panel, e)
	if (Input.IsTouchBegin(e)) then
		panel:SetCapture(true)
		panel:SetMaterial(self.gfx.MMItemBackground2)
	elseif (Input.IsTouchEnd(e)) then
		panel:SetCapture(false)
		if (e.type ~= kI_TouchCancelled) then
			UI.sfx.Command:Play(kSoundChannel_UI, 0)
			self:SelectProduct(panel)
		elseif (self.selected ~= panel) then
			panel:SetMaterial(self.gfx.MMItemBackground)
		end
	end

	return true
end

function StoreUI.CreateProductWidget(self, product, width, yOfs)

	local padd = 6
	local height = StoreUI.kProductHeight * UI.identityScale[2]
	local rect = {0,yOfs,width,height}
	
	local panel = UI:CreateWidget("MatWidget", {rect=rect, material=self.gfx.MMItemBackground})
	panel:SetBlendWithParent(true)
	panel.width = width
	panel.height = height
	panel.product = product
	
	local OnInputEvent = function(widget, e)
		return self:OnProductInputEvent(panel, e)
	end
	
	panel.OnInputEvent = OnInputEvent
	
	local iconSize = StoreUI.kProductIconSize*UI.identityScale[1]
	local iconRect = {padd, (height-iconSize)/2, iconSize, iconSize}
	
	local icon = UI:CreateWidget("MatWidget", {rect=iconRect, material=product.Icon})
	icon:SetBlendWithParent(true)
	panel:AddChild(icon)
	
	local titleRect = {iconRect[1]+iconRect[3]+padd*2, 0, width-iconRect[1]-padd, height}
	local title = UI:CreateWidget("TextLabel", {rect=titleRect, typeface=self.typefaces.ItemTitle})
	title:SetBlendWithParent(true)
	
	local text = StringTable.Get(product.Title)
	local maxWidth = width - iconRect[1] - iconRect[3]
	if (UI:StringDimensions(self.typefaces.ItemTitle, text) > maxWidth-16) then
		maxWidth = maxWidth / 2
		UI:LineWrapCenterLJustifyText(
			title,
			maxWidth,
			true,
			0,
			text
		)
	else
		UI:SetLabelText(title, text)
		UI:SizeLabelToContents(title)
	end
	
	UI:VCenterWidget(title, titleRect)
	
	panel:AddChild(title)
	
	local price = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.PriceSmall})
	price:SetBlendWithParent(true)
	UI:SetLabelText(price, product.Price)
	UI:SizeLabelToContents(price)
	UI:RAlignLabel(price, width-8, 8)
	panel:AddChild(price)
	
	local status = UI:CreateWidget("TextLabel", {rect={0,0,8,8}, typeface=self.typefaces.StatusGreen})
	status:SetBlendWithParent(true)
	panel.status = status
	panel:AddChild(status)
	
	panel.spinnerSize = 32*UI.identityScale[1]
	local spinner = UI:CreateWidget("MatWidget", {rect={0,0,panel.spinnerSize,panel.spinnerSize}, material=self.gfx.Spinner})
	spinner:SetBlendWithParent(true)
	spinner:BlendTo({1,1,1,0}, 0)
	panel.spinner = spinner
	panel:AddChild(spinner)
	
	self:UpdateProductState(panel)
	
	return panel,height
end

function StoreUI.UpdateProductId(self, id)

	local w = self.productWidgets[id]
	if (w) then
		self:UpdateProductStatus(w)
	end
	
end

function StoreUI.UpdateProductState(self, panel)

	local typeface, text, spinner, buttonText
	
	if ((panel.product.State == Store.kProductState_Purchasing) or
		(panel.product.State == Store.kProductState_Validating)) then
		typeface = self.typefaces.StatusOrange
		text = StringTable.Get("STORE_PROCESSING")
		spinner = true
	elseif (panel.product.State == Store.kProductState_Purchased) then
		typeface = self.typefaces.StatusGreen
		text = StringTable.Get("STORE_PURCHASED")
	elseif (panel.product.State == Store.kProductState_Failed) then
		typeface = self.typefaces.StatusRed
		text = StringTable.Get("STORE_INVALID")
	else
		typeface = self.typefaces.StatusGreen
		text = StringTable.Get("STORE_AVAILABLE")
		buttonText = StringTable.Get("STORE_PURCHASE_BTN")
	end
	
	panel.status:SetTypeface(typeface)
	UI:SetLabelText(panel.status, text)
	UI:SizeLabelToContents(panel.status)
	local rect = UI:RVAlignLabelBottom(panel.status, panel.width-8, panel.height-8)
	
	rect[1] = rect[1] - panel.spinnerSize - 4
	rect[2] = rect[2] + (rect[4]-panel.spinnerSize)/2
	rect[3] = panel.spinnerSize
	rect[4] = panel.spinnerSize
	
	panel.spinner:SetRect(rect)
	
	if (spinner) then
		panel.spinner:BlendTo({1,1,1,1}, 0.3)
		panel.busy = true
	else
		panel.spinner:BlendTo({1,1,1,0}, 0.3)
		panel.busy = false
	end
	
	if (self.selected == panel) then
		local busy = panel.busy or (panel.product.State == Store.kProductState_Purchased)
		if (busy) then
			self.widgets.purchaseButton.class:SetEnabled(self.widgets.purchaseButton, false)
		else
			self.widgets.purchaseButton.class:SetEnabled(self.widgets.purchaseButton, true)
		end
		self.widgets.purchaseButton:SetVisible(true)
		buttonText = buttonText or text
		local rect = self.widgets.purchaseButton:Rect()
		rect[1] = 0
		rect[2] = 0
		UI:LineWrapCenterText(
			self.widgets.purchaseButton.label,
			rect[3],
			true,
			0,
			buttonText
		)
		
		UI:SetLabelText(self.widgets.priceLabel, panel.product.Price)
		UI:CenterLabel(self.widgets.priceLabel, self.priceRect)
		
		UI:LineWrapCenterText(
			self.widgets.descTitle,
			self.descTitleRect[3],
			true,
			0,
			StringTable.Get(panel.product.Title)
		)
		
		UI:CenterWidget(self.widgets.descTitle, self.descTitleRect)
		
		local imageY = 0
		
		if (panel.product.Image) then
			self.widgets.descImage:SetMaterial(panel.product.Image)
			local size = panel.product.Image:Dimensions()
			
			local imageRect = {0,0,0,0}
			imageRect[3] = self.descRect[3] * 0.5
			imageRect[4] = imageRect[3] * (size[2]/size[1])
			imageRect[1] = (self.descRect[3]-imageRect[3])/2
			imageRect[2] = 16*UI.identityScale[2]
			
			self.widgets.descImage:SetRect(imageRect)
			imageY = imageRect[2] + imageRect[4]
			
			self.widgets.descImage:SetVisible(true)
		else
			self.widgets.descImage:SetVisible(false)
		end
		
		local r = UI:LineWrapCenterLJustifyText(
			self.widgets.descText,
			self.descRect[3] - 8*UI.identityScale[1],
			true,
			0,
			StringTable.Get(panel.product.Description)
		)
		
		r[1] = 4*UI.identityScale[1]
		r[2] = 4*UI.identityScale[2]+imageY+16*UI.identityScale[2]
		self.widgets.descText:SetRect(r)
		
		self.widgets.descList:RecalcLayout()
		self.widgets.descList:ScrollTo({0,0}, 0)
	end
end

function StoreUI.CreateLineBorder(self, yOfs, width)
	local w = UI:CreateWidget("MatWidget", {rect={0,yOfs,width,7}, material=self.gfx.LineBorder4})
	w:SetBlendWithParent(true)
	return w
end

function StoreUI.ClosePressed(self)
	Game.entity.eatInput = true
	local f = function()
		local f = function()
			if (self.widgets.root) then
				self.widgets.root:SetVisible(false)
			end
			Game.entity.eatInput = false
			if (self.callback) then
				local callback = self.callback
				self.callback = nil
				callback()
			end
		end
		self.widgets.border:ScaleTo({0,0}, {0.3, 0.3})
		World.globalTimers:Add(f, 0.3)
	end
	
	self.widgets.contents:BlendTo({1,1,1,0}, 0.3)
	World.globalTimers:Add(f, 0.3)
end

function StoreUI.RestoreItemsPressed(self)
end

function StoreUI.PurchasePressed(self)

end

function StoreUI.Do(self, callback)

	self.callback = callback
	
	Game.entity.eatInput = true
	
	if (self.widgets.root) then
		self.widgets.root:SetVisible(true)
	end
	
	self.widgets.contents:BlendTo({1,1,1,0}, 0)
	self.widgets.border:ScaleTo({0,0}, {0,0})
	self.widgets.border:ScaleTo({1,1}, {0.3, 0.3})
	
	local f = function()
		local f = function()
			Game.entity.eatInput = false
		end
		self.widgets.contents:BlendTo({1,1,1,1}, 0.3)
		World.globalTimers:Add(f, 0.3)
	end
	
	World.globalTimers:Add(f, 0.3)
	
end