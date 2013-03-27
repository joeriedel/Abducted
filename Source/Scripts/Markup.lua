-- Markup.lua
-- Copyright (c) 2012 Sunside Inc., All Rights Reserved
-- Author: Joe Riedel
-- See Abducted/LICENSE for licensing terms

Markup = Class:New()
Markup.AngleOpen = string.byte("<")
Markup.AngleClose = string.byte(">")
Markup.BackSlash = string.byte("\\")
Markup.ForwardSlash = string.byte("/")
Markup.Equals = string.byte("=")
Markup.Quote = string.byte("\"")
Markup.Space = string.byte(" ")
Markup.ArgDelim = {Markup.Space, Markup.Equals, Markup.AngleClose}
Markup.ValDelim = {Markup.Sapce, Markup.AngleClose}
Markup.TagDelim = {Markup.Space}

function Markup.ParseUI(script, options)

	local document = Markup.Parse(script)
	return Markup.CreateUI(document, options)

end

function Markup.CreateUI(document, options)
	
	local widgets = {}
	local strings = {
		typeface = nil,
		array = {},
		rect = {
			999999,
			999999,
			0,
			0
		}
	}
	
	local lines = Markup.FormatLines(document.tags, options)
	
	for k,line in pairs(lines) do
	
		local x = 0
		
		if (line.tag.center) then
			x = (options.maxWidth - line.w) / 2
		elseif (line.tag.ralign) then
			x = (options.maxWidth - line.w)
		end
		
		for k,elem in pairs(line.elements) do
		
			local w
			
			if (elem.type == "text") then
				strings = Mark.EmitText(widgets, strings, x, elem, options)
			elseif (elem.type == "image") then
				Mark.EmitImage(widgets, x, elem, options)
			end
		
		end
	
	end
	
	Mark.EmitStrings(widgets, strings)
	
end

function Markup.EmitText(widgets, strings, x, elem, options)

	local typeface = options.typefaces[elem.font]
	if (typeface == nil) then
		error(string.format("Markup typeface '%s' is not loaded", elem.font))
	end
	
	if (#strings.array >= options.maxLinesPerModel) then
		strings = Markup.EmitStrings(widgets, strings, typeface)
	end
	
	if (strings.typeface ~= typeface) then
		strings = Markup.EmitStrings(widgets, strings, typeface)
	end
	
	local string = {
		text = elem.text,
		rect = {elem.x+x, elem.y, elem.w, elem.h}
	}
	
	strings.rect[1] = Min(strings.rect[1], rect[1])
	strings.rect[2] = Min(strings.rect[2], rect[2])
	strings.rect[3] = Max(strings.rect[3], rect[3])
	strings.rect[4] = Max(strings.rect[4], rect[4])
	
	table.insert(strings.array, string)
	return strings

end

function Markup.EmitImage(widget, x, elem, options)

	local material = options.materials[elems.src]
	if (material == nil) then
		error(string.format("Markup image '%s' is not loaded", elems.src))
	end
	
	local w = UI:CreateWidget("MatWidget", {rect={elem.x+x, elem.y, elem.w, elem.h}, material=material})
	table.insert(widgets, w)

end

function Markup.EmitStrings(widgets, strings, typeface)

	if (next(strings.array) == nil) then
		strings.typeface = typeface
		return strings
	end
	
	local widgetStrings = {}
	
	for k,v in pairs(strings.array) do
	
		local string = {
			x = v.rect[1],
			y = v.rect[2],
			text = v.text,
			scaleX = UI.fontScale[1],
			scaleY = UI.fontScale[2]
		}
		
		table.insert(widgetStrings, string)
	
	end
	
	if (next(widgetStrings) ~= nil) then
		local w = UI:CreateWidget("TextModel", {rect=strings.rect, typeface=strings.typeface})
		w:SetText(widgetStrings)
		table.insert(widgets, w)
	end
	
	strings = {
		typeface = typeface,
		array = {},
		rect = {
			999999,
			999999,
			0,
			0
		}
	}
	
	return strings

end

function Markup.FormatLines(tags, options)

	local lines =  {}
	local curLine = {
		x = 0,
		y = 0,
		w = 0,
		h = 0,
		elements = {}
	}
	
	local prevTag = nil
	
	for k,tag in pairs(tags) do
		
		if (prevTag ~= nil) then
			curLine = Markup.FlushLineTag(lines, curLine, tag, prevTag)
		else
			curLine.tag = tag
		end
		
		prevTag = tag
		
		for k,v in pairs(tag.elems) do
			local w, h
			
			if (v.image) then
				curLine = Markup.AddImageElement(lines, curLine, tag, v, options)
			elseif (v.space) then
				curLine = Markup.AddSpaceElement(lines, curLine, tag, v, options)
			elseif (v.br) then
				curLine = Markup.AddLineBreak(lines, curLine, tag, options)
			elseif (v.text or v.string) then
				local first
				local second
				if (v.string) then
					first = StringTable.Get(v.string)
				else
					first = v.text
				end
				
				curLine = Markup.AddTextElement(lines, curLine, tag, v, text, options)
				
			else
				error("Invalid markup element")
			end
		end
	end
	
	Markup.FlushLine(lines, curLine)
	return lines
end

function Markup.AddImageElement(lines, curLine, tag, v, options)

	local element = {
		type = "image",
		material = options.materials[v.image],
		alwaysSpace = true,
		lineWrap = true
	}
	
	element.w, element.h = material:Dimensions()
	return Markup.AddLineElement(lines, curLine, element, options)
end

function Markup.AddSpaceElement(lines, curLine, tag, v, options)

	local element = {
		type = "space",
		neverSpace = true,
		lineWrap = false,
		w = v.size,
		h = 0
	}
	
	if (tag.pos) then
		local pos = Vec2ForString(tag.pos)
		element.x = pos[1]
		element.y = pos[2]
		element.explicit = true
		element.lineWrap = false
	end
	
	return Markup.AddLineElement(lines, curLine, element, options)

end

function Markup.AddTextElement(lines, curLine, tag, v, text, options)

	local element = {
		type = "text",
		lineWrap = true
	}
	
	local padd = Markup.GetElementPadd(curLine, element, options)
	local width = options.maxWidth - (curLine.x + padd)
	
	if (width < 1) then -- will wrap
		width = options.maxWidth
	end
	
	local font = Markup.SelectFont(tag, options)
	local a,d  = font:AscenderDescender()
	local advance = a+d
	
	local first, second = font:SplitStringAtSize(text, width)
	
	while (second) do
		if (first) then
		
			local w = UI:StringDimensions(font, first)
			
			element = {
				type = "text",
				text = first,
				lineWrap = true,
				font = font,
				w = w,
				h = advance,
			}
			
			curLine = Markup.AddLineElement(lines, curLine, element, options)
		end
		
		width = options.maxWidth
		
		first, second = font:SplitStringAtSize(text, width)
		
	end
	
	if (first) then
		
		local w = UI:StringDimensions(font, first)
		
		element = {
			type = "text",
			text = first,
			lineWrap = true,
			font = font,
			w = w,
			h = advance
		}
		
		curLine = Markup.AddLineElement(lines, curLine, element, options)
	end
	
	return curLine

end

function Markup.AddLineElement(lines, curLine, element, options)

	local padd = Markup.GetElementPadd(curLine, element, options)

	if (element.lineWrap and ((curLine.x + padd + element.w) > options.maxWidth)) then
		curLine = Markup.FlushLine(lines, curLine)
		padd = 0
	else
		curLine.x = curLine.x + padd
		curLine.w = curLine.x
	end
	
	table.insert(curLine.elements, element)
	
	if (not element.explicit) then
		element.x = curLine.x
		element.y = curLine.y
		curLine.x = curLine.x + element.w
		curLine.w = curLine.x
		curLine.h = Max(curLine.h, element.h + (options.lineSpace*UI.identityScale[2]))
		curLine.prevElement = element
	end
	
	return curLine

end

function Markup.AddLineBreak(lines, curLine, tag, options)

	if (next(curLine.elements) == nil) then
		curLine.y = curLine.y + options.lineBreak
		return curLine
	end
	
	return Markup.FlushLine(lines, curLine, tag)

end

function Markup.GetElementPadd(curLine, element, options)
	local padd = 0
	
	if ((not element.explicit) and (curLine.prevElement ~= nil)) then
		if ((not element.neverSpace) and ((curLine.prevElement.type ~= element.type) or element.alwaysSpace)) then
			padd = options.elemSpace
		end
	end
	
	return padd
end

function Markup.FlushLineTag(lines, curLine, curTag, oldTag)

	if (curTag.center ~= oldTag.center) then
		return Markup.FlushLine(lines, curLine, curTag)
	end
	
	if (curTag.ralign ~= oldTag.ralign) then
		return Markup.FlushLine(lines, curLine, curTag)
	end
	
	return curLine

end

function Markup.FlushLine(lines, curLine, tag)

	if (next(curLine.elements) == nil) then
		return curLine
	end
	
	if (tag == nil) then
		tag = curLine.tag
	end
	
	table.insert(lines, curLine)
	
	curLine = {
		x = 0,
		y = curLine.y + curLine.h + (options.lineSpace*UI.identityScale[2]),
		w = 0,
		h = 0,
		elements = {},
		tag = curLine.tag
	}

	return curLine
end

function Markup.SelectFont(tag, options)
	local font = options.font
	
	if (tag.bold and tag.italic) then
		font = options.boldItalic or font
	elseif(tag.bold) then
		font = options.bold or font
	elseif(tag.italic) then
		font = options.italic
	end
	
	return font
end

function Markup.Parse(script)

	local state = Markup.NewLexState(System.UTF8to32(script))
	
	while (state) do
		state, elem = Markup.ParseElement(state)
		if (elem) then
			Markup.AddElement(elem)
		end
	end
	
	if (next(elements) == nil) then
		elements = nil
	end
	
	local document = state.elems
	state = nil
	collectgarbage()
	return document

end

function Markup.NewLexState(utf32)
	local state = {
		pos = 1,
		buf = utf32,
		tags = LL_New(),
		elems = {}
	}
end

function Markup.ParseElement(state)
	local c = Fetch(state)
	if (c == nil) then
		return nil
	end
	
	if (c == Markup.AngleOpen) then
		while (c == Markup.AngleOpen) do
			Markup.Unfetch(state)
			state = Markup.ParseTag(state)
			
			if (state.img) then
				local img = state.img
				state.img = nil
				return {image=img}
			end
			
			if (state.string) then
				local string = state.string
				state.string = nil
				return {string=string}
			end
			
			if (state.br) then
				state.br = nil
				return {br=true}
			end
			
			if (state.space) then
				local space = state.space
				state.space = nil
				return {space=space}
			end
			
			c = Fetch(state)
		end
		
		if (c == nil) then
			error("Expected element")
		end
	end
		
	if (c == Markup.BackSlash) then -- escape sequence
		c = Markup.Fetch(state)
	end
	
	Markup.Unfetch(state)
	local text = ParseText(state)
	return {text=text}
end

function Markup.AddElement(state, elem)

	local tag = LL_Head(state.tags)
	if (tag == nil) then
		local t = { elems = { elem } }
		table.insert(state.elems, t)
	else
		table.insert(tag.elems, elem)
	end
	
end

function Markup.ParseTag(state)

	local c = Markup.RequireFetch(state, Markup.AngleOpen)
	if (c == Markup.ForwardSlash) then
		Markup.PopTag(state)
		Markup.RequireFetch(state, Markup.AngleClose)
	end

	-- fetch name
	local tag = {}
	tag.name = Markup.FetchToken(state, Markup.TagDelim)
	if (tag.name == nil) then
		error("Expected name")
	end
	
	tag.args = {}
	
	local arg = Markup.FetchToken(state, Markup.ArgDelim)
	while (arg) do
		local c = Markup.NextChar(state)
		local value = true
		if (c == Markup.Equals) then
			value = Markup.FetchToken(state, Markup.ValDelim)
			if (tag.value == nil) then
				error("Expected value after '='")
			end
		end
		
		tag.args[arg] = value
		
		if (c == Markup.AngleClose) then
			Markup.UngetChar(state)
			break
		end
		
		arg = Markup.FetchToken(state, Markup.ArgDelim)
	end
	
	Mark.RequireFetch(state, Markup.AngleClose)
	return tag
end

function Markup.ParseText(state)
	local token = {}
	local c = Mark.NextChar(state)
	while (c) do
		if (c == Markup.BackSlash) then
			c = Mark.SafeNextChar(state)
		elseif (c == Markup.AngleClose) then
			break
		end
		table.insert(token, c)
	end
	
	if (next(token) == nil) then
		token = nil
	else
		token = System.UTF32to8(token)
	end
	
	return token
end

function Markup.PopTag(state)
	local tag = LL_Pop(state.tags)
	if (tag == nil) then
		error("unmatched closing tag </>")
	end
	table.insert(state.elems, tag)
end

function Markup.FetchToken(state, delim)
	local token = {}
	local c = Markup.SafeFetch(state)
	
	if (Markup.IsDelimiter(c, delim)) then
		Markup.UngetChar(state)
		return nil
	end
		
	local quote = c == Markup.Quote
	if (quote) then
		c = Markup.SafeNextChar(state)
	end
	
	while (c ~= nil) do
		table.insert(token, c)
		local c = Markup.NextChar(state)
		if (quote) then
			if (c == Markup.Quote) then
				break
			end
		elseif (Markup.IsDelimiter(c, delim)) then
			Markup.UngetChar(state)
			break
		end
	end
	
	if (next(token) == nil) then
		token = nil
	end
	
	return token
end

function Markup.IsDelimiter(c, delim)
	if (delim == nil) then
		return false
	end
	
	for k,v in pairs(delim) do
		if (c == v) then
			return true
		end
	end
	
	return false
end

function Markup.Fetch(state, filter)
	if (filter == nil) then
		filter = Markup.FilterWhitespace
	end

	repeat
		local c = Markup.NextChar(state)
	until ((c == nil) or filter(c))
end

function Markup.FilterWhitespace(c)
	return (c > 32)
end

function Markup.SafeFetch(state, f)
	if (f == nil) then
		f = Markup.Fetch
	end
	
	local c = f(state)
	if (c == nil) then
		error("Expected character!")
	end
	return c
end

function Markup.RequireFetch(state, req, f)
	if (f == nil) then
		f = Markup.SafeFetch
	end
	local c = f(state)
	if (c ~= req) then
		error(string.format("Expected '%c', got '%c'", string.char(req), string.char(c)))
	end
	return c
end

function Markup.NextChar(state)
	local c = state.buf[state.pos]
	if (c ~= nil) then
		state.pos = state.pos + 1
	end
	return c
end

function Markup.SafeNextChar(state)
	local c = Markup.NextChar(state)
	if (c == nil) then
		error("Expected character!")
	end
	return c
end

function Markup.UngetChar(state)
	if (state.pos < 1) then
		error("Cannot unget")
	end
	state.pos = state.pos - 1
end

function Markup.Peek(state)
	return state.buf[state.pos]
end

function Markup.PeekAt(state, idx)
	return state.buf[state.pos+idx]
end