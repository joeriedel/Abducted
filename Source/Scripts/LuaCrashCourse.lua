-- Quick lua crash course.

--[[

    * It'll be useful to read up on lua as needed, that's what I did at least. Lua 5.1 manual: http://www.lua.org/manual/5.1/
    
    Lua is a duck typed language, variables are typed by assignment and can be assigned values of different types. By default variables
    that are declared contain "nil", the lua keyword meaning null.
    
    Lua is not object oriented, its only mechanism is extremely fast implementation of tables and functions both are built-in language
    features. Functions are first class citizens of the language and can be declared and created anywhere. Feel free to take advantage
    of lua coolness like closures as it makes sense.
    
]]--

-- PAY NO ATTENTION TO ME PREFIXING THINGS WITH UIexample this is just to avoid collisions in the global namespace.

-- Everything in lua uses a table. For example this variable declaration:

UIexample_x = nil -- this variable is actually an entry in the lua "GLOBALS" table hashed with the string "UIexample_x" and assigned nil as a value.
UIexample_x = 10 -- now it's a number, NOTE: numbers are all double precision floats in LUA
UIexample_x = "String" -- now it's a string

UIexample_concatStringsA = "A + "
UIexample_concatStringsB = UIexample_concatStringsA.."AnotherString" -- string do not use + in lua, use .. instead

-- This declares an empty table

UIexample_emptyTable = {}

-- This table has a string in it

UIexample_tableWithAString = {
	aStringEntry = "String Value In Quotes"
}

UIexample_tableWithATableOfStrings = {
	anotherTable = {
		aStringEntry = "Another String",
		anotherStringEntry = "Another String OMFG",
		anInteger = 5,
		butIntegersAreActuallyFloatsThereAreNoInts = 7.8832
	}
}

-- Which can be accessed like this. Tables function like structs in C++.

UIexample_variable = UIexample_tableWithAString.aStringEntry
UIexample_variable = UIexample_tableWithAString["aStringEntry"] -- Same Thing!
UIexample_variable = UIexample_tableWithATableOfStrings.anotherTable.anotherStringEntry

-- Tables can also contain functions like this:

UIexample_tableWithAFunction = {
	aFunction = function (arg1, arg2)
	end,
	aString = "BLAH",
	anotherFunction = function ()
	end
}

-- There is another more "programmer" like way to declare a function inside a struct
function UIexample_tableWithAFunction.aFunction (arg1, arg2)
	-- Same thing as above.
	
	-- ALL VARIABLES ARE GLOBAL BY DEFAULT!
	thisIsGlobalEvenThoughItsDeclaredHere = 5
	local useLocalToForceVariablesToTheirDeclaredScope = 5
	local localIsVeryImportant
end


--[[
	Lua has some support for doing object oriented programming. The first to note is the hidden "self" parameter when declaring
	a function inside a table, or when calling a function inside a table.
	
	The lua ':' symbole instead of a '.' when indexing a table or calling a function inserts a hidden parameter called "self".
]]

-- NOTE: declare the table first!
UIexample_functionTable = {}

function UIexample_functionTable:FunctionWithHiddenSelf(arg1)
	self.argument = arg1 -- this line actually inserts "argument" into self, which is a hidden parameter.
	-- What self is depends on how the function is called
end

UIexample_functionTable:FunctionWithHiddenSelf("astring") -- <-- inserts "argument" field into UIexample_functionTable table.
UIexample_functionTable.FunctionWithHiddenSelf(UIexample_functionTable, "astring") -- <-- identical to above!

--[[
	So with the code above lua lets us make things that act/behave like object oriented code, there are even ways of doing inheritence. 
]]

ExampleClass = Class:New() -- I'll explain this in a second

function ExampleClass.MethodAdd(self, a, b)
	return a + b
end

function ExampleClass.MethodSub(self, a, b)
	return a - b
end

instanceOfExampleClass = ExampleClass:New()
result = instanceOfExampleClass:MethodAdd(5, 3)

--[[
	Class:New does some lua stuff to enable support for inheritence behaviour. I can now make a derrived class from ExampleClass like so:
]]

DerrivedClass = ExampleClass:New()

function ExampleClass.MethodAdd(self, a, b) -- overrides ExampleClass
	return a * b
end

instanceOfDerrivedClass = DerrivedClass:New()
result = instanceOfDerrivedClass:MethodAdd(5, 3)
result = instanceOfDerrivedClass:MethodSub(8, 5)