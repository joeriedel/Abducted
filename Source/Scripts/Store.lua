-- GameNetwork.lua
-- Copyright (c) 2013 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

-- Native:
-- Store.Create()
-- Store.RequestProducts()
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
		Title = "STORE_EP2_TITLE",
		Description = "STORE_EP2_DESCRIPTION",
		Icon = "UI/store_ep2_icon_M",
		Image = "UI/store_ep2_teaser_M",
		State = Store.kProductState_Hidden,
		BasePrice = 499
	},
	{
		Id = "762002771",
		Title = "STORE_S1_TITLE",
		Description = "STORE_S1_DESCRIPTION",
		Icon = "UI/store_s1_icon_M",
		Image = "UI/store_s1_teaser_M",
		State = Store.kProductState_Hidden,
		BasePrice = 2499
	},
	{
		Id = "761996940",
		Title = "STORE_OMEGA_TITLE",
		Description = "STORE_OMEGA_DESCRIPTION",
		Icon = "UI/store_omega_icon_M",
		State = Store.kProductState_Hidden,
		BasePrice = 1999
	},
	{
		Id = "761842135",
		Title = "STORE_SKP10_TITLE",
		Description = "STORE_SKP10_DESCRIPTION",
		Icon = "UI/store_skp10_icon_M",
		State = Store.kProductState_Hidden,
		BasePrice = 799
	},
	{
		Id = "761828172",
		Title = "STORE_SKP5_TITLE",
		Description = "STORE_SKP5_DESCRIPTION",
		Icon = "UI/store_skp5_icon_M",
		State = Store.kProductState_Hidden,
		BasePrice = 399
	},
	{
		Id = "761820735",
		Title = "STORE_SKP1_TITLE",
		Description = "STORE_SKP1_DESCRIPTION",
		Icon = "UI/store_skp1_icon_M",
		State = Store.kProductState_Hidden,
		BasePrice = 99
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
		Store.RequestValidateApplication()
	end
end

function Store.LoadSkillPoints()
	Store.skillPoints = Persistence.ReadNumber(Globals, "storeSkillPoints", 0)
	Store.omegaUpgrades = Persistence.ReadNumber(Globals, "omegaUpgrades", 0)
end

function Store.SaveSkillPoints()
	Persistence.WriteNumber(Globals, "storeSkillPoints", Store.skillPoints)
	Persistence.WriteNumber(Globals, "omegaUpgrades", Store.omegaUpgrades)
	Globals:Save()
end

function Store.LoadProducts()

	local numProducts = Persistence.ReadNumber(Session, "store/numProducts")
	if (numProducts == nil) then
		if (System.Platform() == kPlatPC) then
			Store.RequestTestProducts()
		else
			Store.RequestProducts()
		end
		return
	end
	
	Store.validProducts = {}
	
	for i=1,numProducts do
	
		local id = Persistence.ReadString(Session, "store/productId", nil, i)
		local price = Persistence.ReadString(Session, "store/productPrice", nil, i)
		local usPrice = Persistence.ReadNumber(Session, "store/productusPrice", nil, i)
		
		local product = Store.ProductsById[id]
		if (product) then
			product.Price = price
			product.usPrice = usPrice
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
		product.usPrice = v.BasePrice
		
		local cents = v.BasePrice
		local dollars = math.floor(cents / 100)
		cents = cents - dollars*100
		
		product.price = string.format("$%01d.%02d", dollars, cents)
		
		table.insert(products, product)
	end
	
	Store.OnProductsResponse(products)

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

function Store.OnProductsResponse(products)

	COutLine(kC_Debug, "Store.OnProductsResponse")

	Store.validProducts = {}
	
	local productIdx = 0
	
	for k,v in pairs(products) do
	
		local product = Store.ProductsById[v.id]
		if (product) then
			product.Price = v.price
			product.usPrice = v.usPrice
			product.State = Store.kProductState_Available
			
			productIdx = productIdx + 1
			Persistence.WriteString(Session, "store/productId", v.Id, productIdx)
			Persistence.WriteString(Session, "store/productPrice", v.price, productIdx)
			Persistence.WriteNumber(Session, "store/productusPrice", v.usPrice, productIdx)
			
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
			Store.RemovePurchase(product.Id)
		elseif (code == Store.kResponseCode_Purchased) then
			product.State = Store.kProductState_Purchased
			Store.AddPurchase(product.Id)
		end
		
		if (product.Callback) then
			local callback = product.Callback
			product.Callback = nil
			callback()
		end
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
		product.Transaction = transaction
		Store.RequestValidateProducts({transaction:ProductId()})
	elseif (state == Store.kTransactionState_Purchasing) then
		if (product) then
			product.State = Store.kProductState_Purchasing
		end
	elseif (state == Store.kTransactionState_Failed) then
		if (product) then
			Store.RemoveProduct(product.Id)
			product.State = Store.kProductState_Failed
		end
		transaction:Finish()
		if (product and product.Callback) then
			local callback = product.Callback
			product.Callback = nil
			callback()
		end
	end
end
