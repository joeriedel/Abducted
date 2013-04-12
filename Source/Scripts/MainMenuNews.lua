-- MainMenuNews.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

MainMenu.News = MainMenuPanel:New()

function MainMenu.InitNews(self)

	self.newsPanel = MainMenu.News:New()
	self.newsPanel:Create({rect=self.contentRect}, self.widgets.root)
	self.newsPanel:DownloadNews()
	self.newsPanel.widgets.panel:SetHAlign(kHorizontalAlign_Left)
	self.newsPanel.widgets.panel:SetVAlign(kVerticalAlign_Bottom)
	self.newsPanel.widgets.panel:SetVisible(false)
	
end

function MainMenu.News.Create(self, options, parent)

	options = MainMenuPanel.Create(self, options, parent)
	
	self.widgets.vlist = UI:CreateWidget("VListWidget", {rect={0,0,8,8}})
	self.widgets.vlist:SetBlendWithParent(true)
	self.widgets.panel:AddChild(self.widgets.vlist)

end

function MainMenu.News.DownloadNews(self)
	-- download news
	self.httpGet = System.NewHTTPGet()
	self.httpGet:SendRequest("www.sunsidegames.com", "/abducted_news_en.txt", "text/plain")
	self.newsPending = true
end

function MainMenu.News.LayoutNews(self)

	if (self.newsPending) then
	
		local r = self.httpGet:Status()
		if (r == kHTTP_OpStatus_Pending) then
			self.news = { {text=StringTable.Get("MM_LOADING_NEWS")} }
		else
		
			if (r < 0) then
				self.news = { {text=StringTable.Get("MM_NEWS_SERVER_UNAVAILABLE")} }
			else
				local httpResponse = self.httpGet:Response()
				local data = httpResponse:Body()
				if (data) then
					self.news = MainMenu.News.Parse(data)
				end
			end
			
			self.httpGet = nil
			self.newsPending = false
		
		end
		self:Layout(self.news)
	end

end

function MainMenu.News.Layout(self, news)

	MainMenuPanel.Layout()
	
	if (news) then
		local panelRect = self.widgets.panel:Rect()
		self.widgets.vlist:Clear()
		self.widgets.vlist:SetRect({0,0,panelRect[3], panelRect[4]})
		self.widgets.vlist:SetClipRect({0,0,panelRect[3], panelRect[4]})
		self.widgets.vlist:SetEndStops({0, panelRect[4]*0.1})
				
		local xInset = 16 * UI.identityScale[1]
		local titleSpace = 8 * UI.fontScale[2]
		local lineSpace = 8
		local itemSpace = 16 * UI.identityScale[2]
		local yOffset = 16 * UI.identityScale[2]
		local maxWidth = panelRect[3] - (xInset*2)
		
		for k,v in pairs(news) do
		
			-- layout header stuff.
			if (v.title) then
				local w = UI:CreateWidget("TextLabel", {rect={xInset,yOffset,8,8}, typeface=MainMenu.typefaces.Gold})
				local r = UI:LineWrapLJustifyText(w, maxWidth, true, lineSpace, v.title)
				
				w:SetBlendWithParent(true)
				self.widgets.vlist:AddItem(w)
				w:Unmap()
				
				yOffset = yOffset + r[4] + titleSpace
			end
			
			if (v.text) then
				local w = UI:CreateWidget("TextLabel", {rect={xInset,yOffset,8,8}, typeface=MainMenu.typefaces.Normal})
				local r = UI:LineWrapLJustifyText(w, maxWidth, true, lineSpace, v.text)
				
				w:SetBlendWithParent(true)
				self.widgets.vlist:AddItem(w)
				w:Unmap()
				
				yOffset = yOffset + r[4] + itemSpace
			end
		
		end
		
		self.widgets.vlist:RecalcLayout()
		self.widgets.vlist:ScrollTo({0,0}, 0)
		collectgarbage()
	end
	
end

function MainMenu.News.PrepareContents(self)
	MainMenuPanel.PrepareContents(self)
	self.widgets.vlist:BlendTo({1,1,1,0}, 0)
end

function MainMenu.News.AnimateContents(self, onComplete)
	self.widgets.vlist:BlendTo({1,1,1,1}, 0.2)
	if (onComplete) then
		local f = function()
			onComplete()
		end
		World.globalTimers:Add(f, 0.2, true)
	end
end

function MainMenu.News.FadeOutContents(self, time)
	self.widgets.vlist:BlendTo({1,1,1,0}, time)
end

function MainMenu.News.Parse(text)

	-- parse into news items that can be layed out
	local news = {}
	
	local t
	local i = 1
	while (i <= #text) do
	
		-- parse tokens
		local item = {}
		
		while (true) do
			t, i = GetToken(text, i)
			if (t == nil) then
				break
			end
		
			if (t == "{") then
				break
			end
			
			if (item.title == nil) then
				item.title = t
			end
		end
		
		if (t ~= "{") then
			break
		end
		
		i = SkipWhitespace(text, i)
		if (i == nil) then
			break
		end
		
		item.text = ""
		while (i <= #text) do
		
			local c = text:sub(i, i)
			i = i + 1
			
			if (c == "}") then
				break
			elseif (c ~= "\r") then
				item.text = item.text..c
			end
		end
		
		table.insert(news, item)
	
	end
	
	return news

end