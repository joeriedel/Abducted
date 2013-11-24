-- GameNetwork.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Native:
-- Store.Create()
-- Store.RequestProductInfo()
-- Store.CreatePaymentRequest()
-- Store.RequestValidateApplication()
-- Store.RequestValidateProducts()
-- Store.RestoreProducts()

Store.kResponseCode_Success = 0
Store.kResponseCode_InvalidReceipt = 1
Store.kResponseCode_Unsupported = 2
Store.kResponseCode_ErrorUnavailable = 3
Store.kResponseCode_ErrorUnknown = 4

Store.kProductState_Hidden = 0
Store.kProductState_Available = 1
Store.kProductState_Purchasing = 2
Store.kProductState_Purchased = 3
Store.kProductState_Validating = 4
Store.kProductState_Failed = 5

Store.kTransactionState_Purchasing = 0
Store.kTransactionState_Purchased = 1
Store.kTransactionState_Failed = 2
Store.kTransactionState_Restored = 3

Store.Products = {
	{
		Id = "761997278",
		PublicId = "1",
		Title = "STORE_EP2_TITLE",
		Description = "STORE_EP2_DESCRIPTION",
		Thanks = "STORE_EP2_PURCHASE_THANKS",
		Icon = "UI/store_ep2_icon_M",
		Image = "UI/store_ep2_teaser_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 299,
		PurchaseAction = function()
			Store.PersistPurchase("761997278")
		end
	},
	{
		Id = "762002771",
		PublicId = "2",
		Title = "STORE_S1_TITLE",
		Description = "STORE_S1_DESCRIPTION",
		Thanks = "STORE_S1_PURCHASE_THANKS",
		Icon = "UI/store_s1_icon_M",
		Image = "UI/store_s1_teaser_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 1999,
		PurchaseAction = function()
			Store.PersistPurchase("762002771")
		end
	},
	{
		Id = "761996940",
		PublicId = "3",
		Title = "STORE_OMEGA_TITLE",
		Description = "STORE_OMEGA_DESCRIPTION",
		Thanks = "STORE_OMEGA_PURCHASE_THANKS",
		Icon = "UI/store_omega_icon_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 1499,
		Consumable = true,
		PurchaseAction = function()
			Store.PurchasedConsumable(0, 1)
		end
	},
	{
		Id = "764251225",
		PublicId = "4",
		Title = "STORE_SKP25_TITLE",
		Description = "STORE_SKP25_DESCRIPTION",
		Thanks = "STORE_SKP25_PURCHASE_THANKS",
		Icon = "UI/store_skp25_icon_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 999,
		Consumable = true,
		PurchaseAction = function()
			Store.PurchasedConsumable(25000, 0)
		end
	},
	{
		Id = "761842135",
		PublicId = "5",
		Title = "STORE_SKP10_TITLE",
		Description = "STORE_SKP10_DESCRIPTION",
		Thanks = "STORE_SKP10_PURCHASE_THANKS",
		Icon = "UI/store_skp10_icon_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 499,
		Consumable = true,
		PurchaseAction = function()
			Store.PurchasedConsumable(10000, 0)
		end
	},
	{
		Id = "761828172",
		PublicId = "6",
		Title = "STORE_SKP5_TITLE",
		Description = "STORE_SKP5_DESCRIPTION",
		Thanks = "STORE_SKP5_PURCHASE_THANKS",
		Icon = "UI/store_skp5_icon_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 299,
		Consumable = true,
		PurchaseAction = function()
			Store.PurchasedConsumable(5000, 0)
		end
	},
	{
		Id = "761820735",
		PublicId = "7",
		Title = "STORE_SKP1_TITLE",
		Description = "STORE_SKP1_DESCRIPTION",
		Thanks = "STORE_SKP1_PURCHASE_THANKS",
		Icon = "UI/store_skp1_icon_M",
		State = Store.kProductState_Hidden,
		DebugPrice = 99,
		Consumable = true,
		PurchaseAction = function()
			Store.PurchasedConsumable(1000, 0)
		end
	}
}

function Store.Initialize()

	Store.loading = true
	
	Store.Create()
	Store.enabled = Store.Enabled()
	Store.appGUID = Store.AppGUID()
	
	Store.ProductsById = {}
	
	for k,v in pairs(Store.Products) do
		Store.ProductsById[v.Id] = v
	end

	Store.Load()

end

function Store.Purchase(id, quantity)
	assert(Store.ProductsById[id])
	local payment = Store.CreatePaymentRequest(id, quantity)
	assert(payment)
	payment:Submit()
end

function Store.Load()

	Store.validApplication = true
	Store.validationPending = true
	Store.validationError = false
		
	Store.LoadSkillPoints()
	
	local guid = Persistence.ReadString(Globals, "guid")
	if (guid == nil) then
		guid = Store.appGUID
		Persistence.WriteString(Globals, "guid", guid)
		Globals:Save()
	end
	
	if (guid ~= Store.appGUID) then
	-- this app has been copied from one device to another
		Store.validApplication = false
		Session:Save()
		GameNetwork.LogEvent("GUIDCheckFailed", {guid=guid, appGuid=Store.appGuid})
		COutLine(kC_Debug, "GUIDCheckFailed")
	end

	local didValidate = Persistence.ReadBool(Session, "appValidated", false)
	if (didValidate) then
		Store.LoadProducts()
	else
		Persistence.WriteBool(Session, "appValidated", true)
		Session:Save()
		Store.DownloadSales()
		Store.RequestValidateApplication()
	end
end

function Store.DownloadSales()
	Store.salesHttpGet = System.NewHTTPGet()
	Store.salesHttpGet:SendRequest("www.sunsidegames.com", "/abducted_sales_ios.txt", "text/plain")
	
	local f = function()
		local r = Store.salesHttpGet:Status()
		if (r ~= kHTTP_OpStatus_Pending) then
			Store.salesRefreshTimer:Clean()
			Store.salesRefreshTimer = nil
			Store.ParseSales()
		end
	end
	
	Store.salesRefreshTimer = World.globalTimers:Add(f, 1, true)
end

function Store.ParseSales()
	local httpResponse = Store.salesHttpGet:Response()
	local data = httpResponse:Body()
	
	if (data) then
	
		local idPairs = Tokenize(data)
		
		for k,id in pairs(idPairs) do
		
			item = Tokenize(id)
			if (item and (#item == 2)) then
				for k,p in pairs(Store.Products) do
				
					if (p.PublicId == item[1]) then
						p.onSale = item[2]
						if (p.Idx) then
							Persistence.WriteString(Session, "store/productOnSale", item[2], p.Idx)
						end
						StoreUI:UpdateProductId(p.Id)
					end
				
				end
			end
			
		end
	
	end
	
	Session:Save()
end

function Store.LoadSkillPoints()
	Store.encumberedSkillPoints = 0
	Store.encumberedOmegaUpgrades = 0
	Store.skillPoints = Persistence.ReadNumber(Globals, "storeSkillPoints", 0)
	Store.omegaUpgrades = Persistence.ReadNumber(Globals, "omegaUpgrades", 0)
end

function Store.SaveSkillPoints()
	Persistence.WriteNumber(Globals, "storeSkillPoints", Store.skillPoints)
	Persistence.WriteNumber(Globals, "omegaUpgrades", Store.omegaUpgrades)
	Globals:Save()
end

function Store.PurchasedConsumable(numSkills, numOmegas)
	Store.skillPoints = Store.skillPoints + numSkills
	Store.omegaUpgrades = Store.omegaUpgrades + numOmegas
	Store.SaveSkillPoints()
end

function Store.EncumberSkillPoints(num)
	Store.encumberedSkillPoints = Store.encumberedSkillPoints + num
end

function Store.EncumberOmegaUpgrades(num)
	Store.encumberedOmegaUpgrades = Store.encumberedOmegaUpgrades + num
end

function Store.ApplyEncumberedBalances()

	Store.skillPoints = Store.skillPoints - Store.encumberedSkillPoints
	Store.omegaUpgrades = Store.omegaUpgrades - Store.encumberedOmegaUpgrades
	
	Store.encumberedSkillPoints = 0
	Store.encumberedOmegaUpgrades = 0
	
end

function Store.AvailableSkillPoints()
	return Store.skillPoints - Store.encumberedSkillPoints
end

function Store.AvailableOmegaUpgrades()
	return Store.omegaUpgrades - Store.encumberedOmegaUpgrades
end

function Store.LoadProducts()

	local numProducts = Persistence.ReadNumber(Session, "store/numProducts")
	if (numProducts == nil) then
		if (System.Platform() == kPlatPC) then
			Store.RequestTestProducts()
		else
			local ids = {}
			for k,v in pairs(Store.Products) do
				table.insert(ids, v.Id)
			end
			if (next(ids)) then
				Store.RequestProductInfo(ids)
			end
		end
		return
	end
	
	Store.validProducts = {}
	
	for i=1,numProducts do
	
		local id = Persistence.ReadString(Session, "store/productId", nil, i)
		local price = Persistence.ReadString(Session, "store/productPrice", nil, i)
		local onSale = Persistence.ReadBool(Session, "store/productOnSale", nil, i)
		
		local product = Store.ProductsById[id]
		if (product) then
			product.Price = price
			product.onSale = onSale
		end
	
		table.insert(Store.validProducts, id)
		
	end
	
	Store.LoadPurchases()

end

function Store.RequestTestProducts()

	local products = {}
	
	for k,v in pairs(Store.Products) do
	
		local product = {}
		
		product.id = v.Id
				
		local cents = v.DebugPrice
		local dollars = math.floor(cents / 100)
		cents = cents - dollars*100
		
		product.price = string.format("$%01d.%02d", dollars, cents)
		
		table.insert(products, product)
	end
	
	Store.OnProductInfoResponse(products)

end

function Store.LoadPurchases()

	local numPurchased = Persistence.ReadNumber(Session, "purchased/numProducts")
	if (numPurchased == nil) then
		Store.ValidateProducts()
	else
		Store.LoadPurchaseIds()

		for k,v in pairs(Store.purchases) do
			local product = Store.ProductsById[v]
			if (product) then
				product.State = Store.kProductState_Purchased
			end
		end
		
		StoreUI:ProductsListReady()
	end
end

function Store.ValidateProducts()

	local numPurchased = Persistence.ReadNumber(Globals, "purchased/numProducts", 0)
	
	local productsToValidate = {}
	
	for i=1,numPurchased do
		local id = Persistence.ReadString(Globals, "purchased/productId", nil, i)
		local product = Store.ProductsById[id]
		if (product and (product.State ~= Store.kProductState_Hidden)) then
			product.State = Store.kProductState_Validating
			table.insert(productsToValidate, product.Id)
		end
	end
	
	Store.purchases = {}
	Store.SavePurchaseIds()
	Store.numProductsWaitingForValidation = #productsToValidate
	
	if (next(productsToValidate)) then
		Store.RequestValidateProducts(productsToValidate)
	else
		Store.LoadPurchases()
	end
	
end

function Store.OnProductInfoResponse(products)

	COutLine(kC_Debug, "Store.OnProductInfoResponse")

	Store.validProducts = {}
	
	local productIdx = 0
	
	for k,v in pairs(products) do
	
		local product = Store.ProductsById[v.id]
		if (product) then
			product.Price = v.price
			product.State = Store.kProductState_Available
			
			productIdx = productIdx + 1
			product.Idx = productIdx
			Persistence.WriteString(Session, "store/productId", v.id, productIdx)
			Persistence.WriteString(Session, "store/productPrice", v.price, productIdx)
						
			table.insert(Store.validProducts, v.id)
		end
	
	end
	
	Persistence.WriteNumber(Session, "store/numProducts", productIdx)
	Session:Save()
	
	Store.LoadPurchases()
end

function Store.OnApplicationValidateResult(code)
	if (code == Store.kResponseCode_InvalidReceipt) then
		Store.validApplication = false
		Store.validationError = false
		GameNetwork.LogEvent("InvalidAppReceipt", {appGuid=Store.appGuid})
		COutLine(kC_Debug, "InvalidAppReceipt")
	elseif (code == Store.kResponseCode_ErrorUnavailable) then
		Store.validationError = true
		Store.validApplication = false
		GameNetwork.LogEvent("AppReceiptUnavailable", {appGuid=Store.appGuid})
		COutLine(kC_Debug, "AppReceiptUnavailable")
	end
	
	Store.validationPending = false
	Store.LoadProducts()
end

function Store.PersistPurchase(id)
	local numPurchased = Persistence.ReadNumber(Globals, "purchased/numProducts", 0)
	
	for i=1,numPurchased do
		local z = Persistence.ReadString(Globals, "purchased/productId", nil, i)
		if (z and (z == id)) then
			return
		end
	end
	
	numPurchased = numPurchased + 1
	
	Persistence.WriteNumber(Globals, "purchased/numProducts", numPurchased)
	Persistence.WriteString(Globals, "purchased/productId", id, numPurchased)
	Globals:Save()
end

function Store.AddPurchase(id)
	for k,v in pairs(Store.purchases) do
		if (v == id) then
			return
		end
	end
	
	table.insert(Store.purchases, id)
	Store.SavePurchaseIds()
end

function Store.RemovePurchase(id)

	for k,v in pairs(Store.purchases) do
	
		if (v == id) then
			Store.purchases[k] = nil
			Store.purchases = table.compact(Store.purchases)
			Store.SavePurchaseIds()
			return
		end
	
	end

end

function Store.SavePurchaseIds()

	Persistence.WriteNumber(Session, "purchased/numProducts", #Store.purchases)
	
	for k,v in pairs(Store.purchases) do
	
		Persistence.WriteString(Session, "purchased/productId", v, k)
	
	end
	
	Session:Save()

end

function Store.LoadPurchaseIds()

	Store.purchases = {}
	local numProducts = Persistence.ReadNumber(Session, "purchased/numProducts", 0)
	
	for i=1,numProducts do
	
		local id = Persistence.ReadString(Session, "purchased/productId", nul, i)
		assert(id)
		table.insert(Store.purchases, id)
	
	end

end

function Store.OnProductValidateResult(id, code)
	if (Store.validApplication == false) then
		code = Store.kResponseCode_InvalidReceipt
	end
	
	local product = Store.ProductsById[id]
		
	if (product) then
		if (code == Store.kResponseCode_InvalidReceipt) then
			product.State = Store.kProductState_Failed
			if (not product.Consumable) then
				Store.RemovePurchase(product.Id)
			end
		elseif (code == Store.kResponseCode_Success) then
			if (product.Consumable) then
				product.State = Store.kProductState_Available
			else
				product.State = Store.kProductState_Purchased
				Store.AddPurchase(product.Id)
			end
			
			if (Store.numRestored) then
				if (product.Transaction) then
					if (product.Transaction:State() == Store.kTransactionState_Restored) then
						Store.numRestored = Store.numRestored + 1
					end
				end
			end
			
			if (product.PurchaseAction) then
				product.PurchaseAction()
			end
		end
		
		if (product.Transaction) then
			product.Transaction:Finish()
			product.Transaction = nil
		end
		StoreUI:UpdateProductId(product.Id)
	end
	
	if (Store.numProductsWaitingForValidation) then
		Store.numProductsWaitingForValidation = Store.numProductsWaitingForValidation - 1
		if (Store.numProductsWaitingForValidation == 0) then
			Store.numProductsWaitingForValidation = nil
			Store.LoadPurchases()
		end
	end
end

function Store.OnUpdateTransaction(transaction)
	local state = transaction:State()
	local product = Store.ProductsById[transaction:ProductId()]
	
	if ((state == Store.kTransactionState_Purchased) or (state == Store.kTransactionState_Restored)) then
		if (product) then
			product.Transaction = transaction
			product.State = Store.kProductState_Validating
			Store.RequestValidateProducts({transaction:ProductId()})
		end
	elseif (state == Store.kTransactionState_Purchasing) then
		if (product) then
			product.State = Store.kProductState_Purchasing
		end
		StoreUI.UpdateProductId(product.Id)
	elseif (state == Store.kTransactionState_Failed) then
		if (product) then
			if (not product.Consumable) then
				Store.RemoveProduct(product.Id)
			end
			product.State = Store.kProductState_Failed
		end
		transaction:Finish()
		StoreUI.UpdateProductId(product.Id)
	end
end

function Store.StartRestoreProducts()
	Store.numRestored = 0
	Store.RestoreProducts()
end

function Store.OnRestoreProductsComplete(error, msg)
	if (error) then
		StoreUI:RestorePurchasesComplete(Store.numRestored, msg)
	else
		StoreUI:RestorePurchasesComplete(Store.numRestored)
	end
	Store.numRestored = nil
end

