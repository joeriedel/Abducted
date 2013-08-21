-- Animation.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Animation = Class:New()

function MakeAnimatable(entity)
	entity.PlayAnim = function (self, state, model, callback)
		return Animation.Play(self, state, model, callback)
	end
	entity.CancelAnimCallbacks = function(self)
		Animation.Cancel(entity)
	end
end

function LoadModel(name)
	local m = World.Load(name)
	if (m.SetRootController) then
		m:SetRootController("BlendToController")
	end
	return m
end

function LoadSkModel(name)
	local m = World.Load(name)
	m:SetRootController("BlendToController") -- no check: must be SkModel or VtModel
	return m
end

function Animation.Play(entity, state, model, restart)

	Animation.Cancel(entity)
	
	entity.animationStates = {
		chain = LL_New()
	}
	
	if (restart == nil) then
		restart = true
	end

	local blend = {}
	blend.OnEndFrame = function (self)
		local next = LL_Pop(self.animationStates.chain)
		if (next) then
			assert(next.seq)
			local head = self.animationStates.chain
			next.seq(entity)
						
			next = LL_Head(head)
			while (next and next._and) do
				next = LL_Pop(head)
				assert(next._and)
				next._and(entity)
				next = LL_Head(self.animationStates.chain)
			end
		end
	end
	
	blend.Seq = function (state, restart)
		if (type(state) == "string") then
			if (restart == nil) then
				restart = true
			end
			local x = state
			state = function (entity)
				model:BlendToState(x, nil, restart, entity, blend)
			end
		end
		
		LL_Append(entity.animationStates.chain, {seq=state})
		
		return blend
	end
	
	blend.And = function (state, restart)
		if (type(state) == "string") then
			local x = state
			if (restart == nil) then
				restart = true
			end
			state = function (entity)
				model:BlendToState(x, nil, restart, entity, blend)
			end
		end
		LL_Append(entity.animationStates.chain, {_and=state})
		return blend
	end	
	
	entity.animationStates.notify = model:BlendToState(state, nil, restart, entity, blend)
	
	if (entity.animationStates.notify) then
		return blend
	end
	
	return nil
end

function Animation.Cancel(entity)
	if (entity.animationStates) then
		if (entity.animationStates.notify) then
			entity.animationStates.notify:Release()
		end
	end
	entity.animationStates = nil
end