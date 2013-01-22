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
	m:SetRootController("BlendToController") -- no check: must be SkModel
	return m
end

function Animation.Play(entity, state, model)

	Animation.Cancel(entity)
	
	entity.animationStates = {
		chain = LL_New()
	}

	local blend = {}
	blend.OnEndFrame = function (self)
		local next = LL_Pop(self.animationStates.chain)
		if (next) then
			next.seq(entity)
		end
	end
	
	blend.Seq = function (state)
		if (type(seq) == "string") then
			local x = state
			state = function (entity)
				model:BlendToState(x, nil, true, entity, blend)
			end
		end
		LL_Append(entity.animationStates.chain, {seq=state})
	end
	
	if (model:BlendToState(state, nil, true, entity, blend)) then
		return blend
	end
	
	return nil
end

function Animation.Cancel(entity)
	if (entity.animationStates) then
		if (entity.animationStates.notify) then
			entity.animationStates.notify:SetMasked()
		end
	end
	entity.animationStates = nil
end