-- Lua stdlib
--
-- Copyright (c) 2000-2011 stdlib authors
--
-- See http://luaforge.net/projects/stdlib/ for more information.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.

local function require (f)
  package.loaded[f] = true
end
-- Debugging is off by default
_G._DEBUG = _G._DEBUG or false

--- Checks uses of undeclared global variables.
-- All global variables must be 'declared' through a regular
-- assignment (even assigning <code>nil</code> will do) in a top-level
-- chunk before being used anywhere or assigned to inside a function.
-- From Lua distribution (<code>etc/strict.lua</code>).
-- @class module
-- @name strict

local getinfo, error, rawset, rawget = debug.getinfo, error, rawset, rawget

local mt = getmetatable(_G)
if mt == nil then
  mt = {}
  setmetatable(_G, mt)
end

mt.__declared = {}

local function what ()
  local d = getinfo(3, "S")
  return d and d.what or "C"
end

mt.__newindex = function (t, n, v)
  if not mt.__declared[n] then
    local w = what()
    if w ~= "main" and w ~= "C" then
      error("assign to undeclared variable '"..n.."'", 2)
    end
    mt.__declared[n] = true
  end
  rawset(t, n, v)
end

mt.__index = function (t, n)
  if not mt.__declared[n] and what() ~= "C" then
    error("variable '"..n.."' is not declared", 2)
  end
  return rawget(t, n)
end

--- Adds to the existing global functions
module ("base", package.seeall)

--- Functional forms of infix operators.
-- Defined here so that other modules can write to it.
-- @class table
-- @name _G.op
_G.op = {}

require "table_ext"
require "list"
require "string_ext"
--require "io_ext" FIXME: allow loops


--- Return given metamethod, if any, or nil.
-- @param x object to get metamethod of
-- @param n name of metamethod to get
-- @return metamethod function or nil if no metamethod or not a
-- function
function _G.metamethod (x, n)
  local _, m = pcall (function (x)
                        return getmetatable (x)[n]
                      end,
                      x)
  if type (m) ~= "function" then
    m = nil
  end
  return m
end

--- Turn tables into strings with recursion detection.
-- N.B. Functions calling render should not recurse, or recursion
-- detection will not work.
-- @see render_OpenRenderer, render_CloseRenderer
-- @see render_ElementRenderer, render_PairRenderer
-- @see render_SeparatorRenderer
-- @param x object to convert to string
-- @param open open table renderer
-- @param close close table renderer
-- @param elem element renderer
-- @param pair pair renderer
-- @param sep separator renderer
-- @return string representation
function _G.render (x, open, close, elem, pair, sep, roots)
  local function stop_roots (x)
    return roots[x] or render (x, open, close, elem, pair, sep, table.clone (roots))
  end
  roots = roots or {}
  if type (x) ~= "table" or metamethod (x, "__tostring") then
    return elem (x)
  else
    local s = strbuf.new ()
    s = s .. open (x)
    roots[x] = elem (x)
    local i, v = nil, nil
    for j, w in pairs (x) do
      s = s .. sep (x, i, v, j, w) .. pair (x, j, w, stop_roots (j), stop_roots (w))
      i, v = j, w
    end
    s = s .. sep(x, i, v, nil, nil) .. close (x)
    return s:tostring ()
  end
end

---
-- @class function
-- @name render_OpenRenderer
-- @param t table
-- @return open table string

---
-- @class function
-- @name render_CloseRenderer
-- @param t table
-- @return close table string

---
-- @class function
-- @name render_ElementRenderer
-- @param e element
-- @return element string

---
-- @class function
-- @name render_PairRenderer
-- N.B. the function should not try to render i and v, or treat
-- them recursively.
-- @param t table
-- @param i index
-- @param v value
-- @param is index string
-- @param vs value string
-- @return element string

---
-- @class function
-- @name render_SeparatorRenderer
-- @param t table
-- @param i preceding index (nil on first call)
-- @param v preceding value (nil on first call)
-- @param j following index (nil on last call)
-- @param w following value (nil on last call)
-- @return separator string

--- Extend <code>tostring</code> to work better on tables.
-- @class function
-- @name _G.tostring
-- @param x object to convert to string
-- @return string representation
_G._tostring = tostring -- make original tostring available
local _tostring = tostring
function _G.tostring (x)
  return render (x,
                 function () return "{" end,
                 function () return "}" end,
                 _tostring,
                 function (t, _, _, i, v)
                   return i .. "=" .. v
                 end,
                 function (_, i, _, j)
                   if i and j then
                     return ","
                   end
                   return ""
                 end)
end

--- Pretty-print a table.
-- @param t table to print
-- @param indent indent between levels ["\t"]
-- @param spacing space before every line
-- @return pretty-printed string
function _G.prettytostring (t, indent, spacing)
  indent = indent or "\t"
  spacing = spacing or ""
  return render (t,
                 function ()
                   local s = spacing .. "{"
                   spacing = spacing .. indent
                   return s
                 end,
                 function ()
                   spacing = string.gsub (spacing, indent .. "$", "")
                   return spacing .. "}"
                 end,
                 function (x)
                   if type (x) == "string" then
                     return string.format ("%q", x)
                   else
                     return tostring (x)
                   end
                 end,
                 function (x, i, v, is, vs)
                   local s = spacing .. "["
                   if type (i) == "table" then
                     s = s .. "\n"
                   end
                   s = s .. is
                   if type (i) == "table" then
                     s = s .. "\n"
                   end
                   s = s .. "] ="
                   if type (v) == "table" then
                     s = s .. "\n"
                   else
                     s = s .. " "
                   end
                   s = s .. vs
                   return s
                 end,
                 function (_, i)
                   local s = "\n"
                   if i then
                     s = "," .. s
                   end
                   return s
                 end)
end

--- Turn an object into a table according to __totable metamethod.
-- @param x object to turn into a table
-- @return table or nil
function _G.totable (x)
  local m = metamethod (x, "__totable")
  if m then
    return m (x)
  elseif type (x) == "table" then
    return x
  else
    return nil
  end
end

--- Convert a value to a string.
-- The string can be passed to dostring to retrieve the value.
-- <br>TODO: Make it work for recursive tables.
-- @param x object to pickle
-- @return string such that eval (s) is the same value as x
function _G.pickle (x)
  if type (x) == "string" then
    return string.format ("%q", x)
  elseif type (x) == "number" or type (x) == "boolean" or
    type (x) == "nil" then
    return tostring (x)
  else
    x = totable (x) or x
    if type (x) == "table" then
      local s, sep = "{", ""
      for i, v in pairs (x) do
        s = s .. sep .. "[" .. pickle (i) .. "]=" .. pickle (v)
        sep = ","
      end
      s = s .. "}"
      return s
    else
      die ("cannot pickle " .. tostring (x))
    end
  end
end

--- Identity function.
-- @param ...
-- @return the arguments passed to the function
function _G.id (...)
  return ...
end

--- Turn a tuple into a list.
-- @param ... tuple
-- @return list
function _G.pack (...)
  return {...}
end

--- Partially apply a function.
-- @param f function to apply partially
-- @param ... arguments to bind
-- @return function with ai already bound
function _G.bind (f, ...)
  local fix = {...}
  return function (...)
           return f (unpack (list.concat (fix, {...})))
         end
end

--- Curry a function.
-- @param f function to curry
-- @param n number of arguments
-- @return curried version of f
function _G.curry (f, n)
  if n <= 1 then
    return f
  else
    return function (x)
             return curry (bind (f, x), n - 1)
           end
  end
end

--- Compose functions.
-- @param f1...fn functions to compose
-- @return composition of f1 ... fn
function _G.compose (...)
  local arg = {...}
  local fns, n = arg, #arg
  return function (...)
           local arg = {...}
           for i = n, 1, -1 do
             arg = {fns[i] (unpack (arg))}
           end
           return unpack (arg)
         end
end

--- Evaluate a string.
-- @param s string
-- @return value of string
function _G.eval (s)
  return loadstring ("return " .. s)()
end

--- An iterator like ipairs, but in reverse.
-- @param t table to iterate over
-- @return iterator function
-- @return the table, as above
-- @return #t + 1
function _G.ripairs (t)
  return function (t, n)
           n = n - 1
           if n > 0 then
             return n, t[n]
           end
         end,
  t, #t + 1
end

--- Tree iterator.
-- @see tree_Iterator
-- @param tr tree to iterate over
-- @return iterator function
function _G.nodes (tr)
  local function visit (n, p)
    if type (n) == "table" then
      coroutine.yield ("branch", p, n)
      for i, v in pairs (n) do
        table.insert (p, i)
        visit (v, p)
        table.remove (p)
      end
      coroutine.yield ("join", p, n)
    else
      coroutine.yield ("leaf", p, n)
    end
  end
  return coroutine.wrap (visit), tr, {}
end

---
-- @class function
-- @name tree_Iterator
-- @param n current node
-- @param p path to node within the tree
-- @return type ("leaf", "branch" (pre-order) or "join" (post-order))
-- @return path to node ({i1...ik})
-- @return node

--- Collect the results of an iterator.
-- @param i iterator
-- @return results of running the iterator on its arguments
function _G.collect (i, ...)
  local t = {}
  for e in i (...) do
    table.insert (t, e)
  end
  return t
end

--- Map a function over an iterator.
-- @param f function
-- @param i iterator
-- @return result table
function _G.map (f, i, ...)
  local t = {}
  for e in i (...) do
    local r = f (e)
    if r then
      table.insert (t, r)
    end
  end
  return t
end

--- Filter an iterator with a predicate.
-- @param p predicate
-- @param i iterator
-- @return result table containing elements e for which p (e)
function _G.filter (p, i, ...)
  local t = {}
  for e in i (...) do
    if p (e) then
      table.insert (t, e)
    end
  end
  return t
end

--- Fold a binary function into an iterator.
-- @param f function
-- @param d initial first argument
-- @param i iterator
-- @return result
function _G.fold (f, d, i, ...)
  local r = d
  for e in i (...) do
    r = f (r, e)
  end
  return r
end

--- Extend to allow formatted arguments.
-- @param v value to assert
-- @param f format
-- @param ... arguments to format
-- @return value
function _G.assert (v, f, ...)
  if not v then
    if f == nil then
      f = ""
    end
    error (string.format (f, ...))
  end
  return v
end

--- Give warning with the name of program and file (if any).
-- @param ... arguments for format
function _G.warn (...)
  if prog.name then
    io.stderr:write (prog.name .. ":")
  end
  if prog.file then
    io.stderr:write (prog.file .. ":")
  end
  if prog.line then
    io.stderr:write (tostring (prog.line) .. ":")
  end
  if prog.name or prog.file or prog.line then
    io.stderr:write (" ")
  end
  io.writeline (io.stderr, string.format (...))
end

--- Die with error.
-- @param ... arguments for format
function _G.die (...)
  warn (unpack (arg))
  error ()
end

-- Function forms of operators.
-- FIXME: Make these visible in LuaDoc (also list.concat in list)
_G.op["[]"] =
  function (t, s)
    return t[s]
  end

_G.op["+"] =
  function (a, b)
    return a + b
  end
_G.op["-"] =
  function (a, b)
    return a - b
  end
_G.op["*"] =
  function (a, b)
    return a * b
  end
_G.op["/"] =
  function (a, b)
    return a / b
  end
_G.op["and"] =
  function (a, b)
    return a and b
  end
_G.op["or"] =
  function (a, b)
    return a or b
  end
_G.op["not"] =
  function (a)
    return not a
  end
_G.op["=="] =
  function (a, b)
    return a == b
  end
_G.op["~="] =
  function (a, b)
    return a ~= b
  end

-- Additions to the package module.
module ("package", package.seeall)


--- Make named constants for <code>package.config</code> (undocumented
-- in 5.1; see luaconf.h for C equivalents).
-- @class table
-- @name package
-- @field dirsep directory separator
-- @field pathsep path separator
-- @field path_mark string that marks substitution points in a path template
-- @field execdir (Windows only) replaced by the executable's directory in a path
-- @field igmark Mark to ignore all before it when building <code>luaopen_</code> function name.
dirsep, pathsep, path_mark, execdir, igmark =
  string.match (package.config, "^([^\n]+)\n([^\n]+)\n([^\n]+)\n([^\n]+)\n([^\n]+)")

--- Additions to the debug module
module ("debug", package.seeall)

require "debug_init"
require "io_ext"
require "string_ext"

--- To activate debugging set _DEBUG either to any true value
-- (equivalent to {level = 1}), or as documented below.
-- @class table
-- @name _DEBUG
-- @field level debugging level
-- @field call do call trace debugging
-- @field std do standard library debugging (run examples & test code)


--- Print a debugging message
-- @param n debugging level, defaults to 1
-- @param ... objects to print (as for print)
function say (n, ...)
  local level = 1
  local arg = {n, ...}
  if type (arg[1]) == "number" then
    level = arg[1]
    table.remove (arg, 1)
  end
  if _DEBUG and
    ((type (_DEBUG) == "table" and type (_DEBUG.level) == "number" and
      _DEBUG.level >= level)
       or level <= 1) then
    io.writeline (io.stderr, table.concat (list.map (tostring, arg), "\t"))
  end
end

---
-- debug.say is also available as the global function <code>debug</code>
-- @class function
-- @name debug
-- @see say
getmetatable (_M).__call =
   function (self, ...)
     say (...)
   end

--- Trace function calls
-- Use as debug.sethook (trace, "cr"), which is done automatically
-- when _DEBUG.call is set.
-- Based on test/trace-calls.lua from the Lua distribution.
-- @class function
-- @name trace
-- @param event event causing the call
local level = 0
function trace (event)
  local t = getinfo (3)
  local s = " >>> " .. string.rep (" ", level)
  if t ~= nil and t.currentline >= 0 then
    s = s .. t.short_src .. ":" .. t.currentline .. " "
  end
  t = getinfo (2)
  if event == "call" then
    level = level + 1
  else
    level = math.max (level - 1, 0)
  end
  if t.what == "main" then
    if event == "call" then
      s = s .. "begin " .. t.short_src
    else
      s = s .. "end " .. t.short_src
    end
  elseif t.what == "Lua" then
    s = s .. event .. " " .. (t.name or "(Lua)") .. " <" ..
      t.linedefined .. ":" .. t.short_src .. ">"
  else
    s = s .. event .. " " .. (t.name or "(C)") .. " [" .. t.what .. "]"
  end
  io.writeline (io.stderr, s)
end

-- Set hooks according to _DEBUG
if type (_DEBUG) == "table" and _DEBUG.call then
  sethook (trace, "cr")
end

-- Extensions to the table module
module ("table", package.seeall)

--require "list" FIXME: allow require loops


local _sort = sort
--- Make table.sort return its result.
-- @param t table
-- @param c comparator function
-- @return sorted table
function sort (t, c)
  _sort (t, c)
  return t
end

--- Return whether table is empty.
-- @param t table
-- @return <code>true</code> if empty or <code>false</code> otherwise
function empty (t)
  return not next (t)
end

--- Find the number of elements in a table.
-- @param t table
-- @return number of elements in t
function size (t)
  local n = 0
  for _ in pairs (t) do
    n = n + 1
  end
  return n
end

--- Make the list of indices of a table.
-- @param t table
-- @return list of indices
function indices (t)
  local u = {}
  for i, v in pairs (t) do
    insert (u, i)
  end
  return u
end

--- Make the list of values of a table.
-- @param t table
-- @return list of values
function values (t)
  local u = {}
  for i, v in pairs (t) do
    insert (u, v)
  end
  return u
end

--- Invert a table.
-- @param t table <code>{i=v, ...}</code>
-- @return inverted table <code>{v=i, ...}</code>
function invert (t)
  local u = {}
  for i, v in pairs (t) do
    u[v] = i
  end
  return u
end

--- Rearrange some indices of a table.
-- @param m table <code>{old_index=new_index, ...}</code>
-- @param t table to rearrange
-- @return rearranged table
function rearrange (m, t)
  local r = clone (t)
  for i, v in pairs (m) do
    r[v] = t[i]
    r[i] = nil
  end
  return r
end

--- Make a shallow copy of a table, including any metatable (for a
-- deep copy, use tree.clone).
-- @param t table
-- @param nometa if non-nil don't copy metatable
-- @return copy of table
function clone (t, nometa)
  local u = {}
  if not nometa then
    setmetatable (u, getmetatable (t))
  end
  for i, v in pairs (t) do
    u[i] = v
  end
  return u
end

--- Merge two tables.
-- If there are duplicate fields, u's will be used. The metatable of
-- the returned table is that of t.
-- @param t first table
-- @param u second table
-- @return merged table
function merge (t, u)
  local r = clone (t)
  for i, v in pairs (u) do
    r[i] = v
  end
  return r
end

--- Make a table with a default value for unset keys.
-- @param x default entry value (default: <code>nil</code>)
-- @param t initial table (default: <code>{}</code>)
-- @return table whose unset elements are x
function new (x, t)
  return setmetatable (t or {},
                       {__index = function (t, i)
                                    return x
                                  end})
end

--- Tables as lists.
module ("list", package.seeall)

require "base"
require "table_ext"


--- An iterator over the elements of a list.
-- @param l list to iterate over
-- @return iterator function which returns successive elements of the list
-- @return the list <code>l</code> as above
-- @return <code>true</code>
function elems (l)
  local n = 0
  return function (l)
           n = n + 1
           if n <= #l then
             return l[n]
           end
         end,
  l, true
end

--- An iterator over the elements of a list, in reverse.
-- @param l list to iterate over
-- @return iterator function which returns precessive elements of the list
-- @return the list <code>l</code> as above
-- @return <code>true</code>
function relems (l)
  local n = #l + 1
  return function (l)
           n = n - 1
           if n > 0 then
             return l[n]
           end
         end,
  l, true
end

--- Map a function over a list.
-- @param f function
-- @param l list
-- @return result list <code>{f (l[1]), ..., f (l[#l])}</code>
function map (f, l)
  return _G.map (f, elems, l)
end

--- Map a function over a list of lists.
-- @param f function
-- @param ls list of lists
-- @return result list <code>{f (unpack (ls[1]))), ..., f (unpack (ls[#ls]))}</code>
function mapWith (f, l)
  return _G.map (compose (f, unpack), elems, l)
end

--- Filter a list according to a predicate.
-- @param p predicate (function of one argument returning a boolean)
-- @param l list of lists
-- @return result list containing elements <code>e</code> of
--   <code>l</code> for which <code>p (e)</code> is true
function filter (p, l)
  return _G.filter (p, elems, l)
end

--- Return a slice of a list.
-- (Negative list indices count from the end of the list.)
-- @param l list
-- @param from start of slice (default: 1)
-- @param to end of slice (default: <code>#l</code>)
-- @return <code>{l[from], ..., l[to]}</code>
function slice (l, from, to)
  local m = {}
  local len = #l
  from = from or 1
  to = to or len
  if from < 0 then
    from = from + len + 1
  end
  if to < 0 then
    to = to + len + 1
  end
  for i = from, to do
    table.insert (m, l[i])
  end
  return m
end

--- Return a list with its first element removed.
-- @param l list
-- @return <code>{l[2], ..., l[#l]}</code>
function tail (l)
  return slice (l, 2)
end

--- Fold a binary function through a list left associatively.
-- @param f function
-- @param e element to place in left-most position
-- @param l list
-- @return result
function foldl (f, e, l)
  return _G.fold (f, e, elems, l)
end

--- Fold a binary function through a list right associatively.
-- @param f function
-- @param e element to place in right-most position
-- @param l list
-- @return result
function foldr (f, e, l)
  return _G.fold (function (x, y) return f (y, x) end,
                  e, relems, l)
end

--- Prepend an item to a list.
-- @param l list
-- @param x item
-- @return <code>{x, unpack (l)}</code>
function cons (l, x)
  return {x, unpack (l)}
end

--- Append an item to a list.
-- @param l list
-- @param x item
-- @return <code>{l[1], ..., l[#l], x}</code>
function append (l, x)
  local r = {unpack (l)}
  table.insert (r, x)
  return r
end

--- Concatenate lists.
-- @param ... lists
-- @return <code>{l<sub>1</sub>[1], ...,
-- l<sub>1</sub>[#l<sub>1</sub>], ..., l<sub>n</sub>[1], ...,
-- l<sub>n</sub>[#l<sub>n</sub>]}</code>
function concat (...)
  local r = {}
  for _, l in ipairs ({...}) do
    for _, v in ipairs (l) do
      table.insert (r, v)
    end
  end
  return r
end

--- Repeat a list.
-- @param l list
-- @param n number of times to repeat
-- @return <code>n</code> copies of <code>l</code> appended together
function rep (l, n)
  local r = {}
  for i = 1, n do
    r = list.concat (r, l)
  end
  return r
end

--- Reverse a list.
-- @param l list
-- @return list <code>{l[#l], ..., l[1]}</code>
function reverse (l)
  local m = {}
  for i = #l, 1, -1 do
    table.insert (m, l[i])
  end
  return m
end

--- Transpose a list of lists.
-- This function in Lua is equivalent to zip and unzip in more
-- strongly typed languages.
-- @param ls <code>{{l<sub>1,1</sub>, ..., l<sub>1,c</sub>}, ...,
-- {l<sub>r,1<sub>, ..., l<sub>r,c</sub>}}</code>
-- @return <code>{{l<sub>1,1</sub>, ..., l<sub>r,1</sub>}, ...,
-- {l<sub>1,c</sub>, ..., l<sub>r,c</sub>}}</code>
function transpose (ls)
  local ms, len = {}, #ls
  for i = 1, math.max (unpack (map (function (l) return #l end, ls))) do
    ms[i] = {}
    for j = 1, len do
      ms[i][j] = ls[j][i]
    end
  end
  return ms
end

--- Zip lists together with a function.
-- @param f function
-- @param ls list of lists
-- @return <code>{f (ls[1][1], ..., ls[#ls][1]), ..., f (ls[1][N], ..., ls[#ls][N])</code>
-- where <code>N = max {map (function (l) return #l end, ls)}</code>
function zipWith (f, ls)
  return mapWith (f, transpose (ls))
end

--- Project a list of fields from a list of tables.
-- @param f field to project
-- @param l list of tables
-- @return list of <code>f</code> fields
function project (f, l)
  return map (function (t) return t[f] end, l)
end

--- Turn a table into a list of pairs.
-- <br>FIXME: Find a better name.
-- @param t table <code>{i<sub>1</sub>=v<sub>1</sub>, ...,
-- i<sub>n</sub>=v<sub>n</sub>}</code>
-- @return list <code>{{i<sub>1</sub>, v<sub>1</sub>}, ...,
-- {i<sub>n</sub>, v<sub>n</sub>}}</code>
function enpair (t)
  local ls = {}
  for i, v in pairs (t) do
    table.insert (ls, {i, v})
  end
  return ls
end

--- Turn a list of pairs into a table.
-- <br>FIXME: Find a better name.
-- @param ls list <code>{{i<sub>1</sub>, v<sub>1</sub>}, ...,
-- {i<sub>n</sub>, v<sub>n</sub>}}</code>
-- @return table <code>{i<sub>1</sub>=v<sub>1</sub>, ...,
-- i<sub>n</sub>=v<sub>n</sub>}</code>
function depair (ls)
  local t = {}
  for _, v in ipairs (ls) do
    t[v[1]] = v[2]
  end
  return t
end

--- Flatten a list.
-- @param l list to flatten
-- @return flattened list
function flatten (l)
  local m = {}
  for _, v in ipairs (l) do
    if type (v) == "table" then
      m = concat (m, flatten (v))
    else
      table.insert (m, v)
    end
  end
  return m
end

--- Shape a list according to a list of dimensions.
--
-- Dimensions are given outermost first and items from the original
-- list are distributed breadth first; there may be one 0 indicating
-- an indefinite number. Hence, <code>{0}</code> is a flat list,
-- <code>{1}</code> is a singleton, <code>{2, 0}</code> is a list of
-- two lists, and <code>{0, 2}</code> is a list of pairs.
-- <br>
-- Algorithm: turn shape into all positive numbers, calculating
-- the zero if necessary and making sure there is at most one;
-- recursively walk the shape, adding empty tables until the bottom
-- level is reached at which point add table items instead, using a
-- counter to walk the flattened original list.
-- <br>
-- @param s <code>{d<sub>1</sub>, ..., d<sub>n</sub>}</code>
-- @param l list to reshape
-- @return reshaped list
function shape (s, l)
  l = flatten (l)
  -- Check the shape and calculate the size of the zero, if any
  local size = 1
  local zero
  for i, v in ipairs (s) do
    if v == 0 then
      if zero then -- bad shape: two zeros
        return nil
      else
        zero = i
      end
    else
      size = size * v
    end
  end
  if zero then
    s[zero] = math.ceil (#l / size)
  end
  local function fill (i, d)
    if d > #s then
      return l[i], i + 1
    else
      local t = {}
      for j = 1, s[d] do
        local e
        e, i = fill (i, d + 1)
        table.insert (t, e)
      end
      return t, i
    end
  end
  return (fill (1, 1))
end

--- Make an index of a list of tables on a given field
-- @param f field
-- @param l list of tables <code>{t<sub>1</sub>, ...,
-- t<sub>n</sub>}</code>
-- @return index <code>{t<sub>1</sub>[f]=1, ...,
-- t<sub>n</sub>[f]=n}</code>
function indexKey (f, l)
  local m = {}
  for i, v in ipairs (l) do
    local k = v[f]
    if k then
      m[k] = i
    end
  end
  return m
end

--- Copy a list of tables, indexed on a given field
-- @param f field whose value should be used as index
-- @param l list of tables <code>{i<sub>1</sub>=t<sub>1</sub>, ...,
-- i<sub>n</sub>=t<sub>n</sub>}</code>
-- @return index <code>{t<sub>1</sub>[f]=t<sub>1</sub>, ...,
-- t<sub>n</sub>[f]=t<sub>n</sub>}</code>
function indexValue (f, l)
  local m = {}
  for i, v in ipairs (l) do
    local k = v[f]
    if k then
      m[k] = v
    end
  end
  return m
end
permuteOn = indexValue

-- Metamethods for lists
metatable = {
  -- list .. table = list.concat
  __concat = list.concat,
  __append = list.append,
}

--- List constructor.
-- Needed in order to use metamethods.
-- @param t list (as a table)
-- @return list (with list metamethods)
function new (l)
  return setmetatable (l, metatable)
end

-- Function forms of operators
_G.op[".."] = list.concat

--- Tables as trees.
module ("tree", package.seeall)

require "list"


local metatable = {}
--- Make a table into a tree
-- @param t table
-- @return tree
function new (t)
  return setmetatable (t or {}, metatable)
end

--- Tree <code>__index</code> metamethod.
-- @param tr tree
-- @param i non-table, or list of indices <code>{i<sub>1</sub> ...
-- i<sub>n</sub>}</code>
-- @return <code>tr[i]...[i<sub>n</sub>]</code> if i is a table, or
-- <code>tr[i]</code> otherwise
function metatable.__index (tr, i)
  if type (i) == "table" then
    return list.foldl (op["[]"], tr, i)
  else
    return rawget (tr, i)
  end
end

--- Tree <code>__newindex</code> metamethod.
-- Sets <code>tr[i<sub>1</sub>]...[i<sub>n</sub>] = v</code> if i is a
-- table, or <code>tr[i] = v</code> otherwise
-- @param tr tree
-- @param i non-table, or list of indices <code>{i<sub>1</sub> ...
-- i<sub>n</sub>}</code>
-- @param v value
function metatable.__newindex (tr, i, v)
  if type (i) == "table" then
    for n = 1, #i - 1 do
      if type (tr[i[n]]) ~= "table" then
        tr[i[n]] = tree.new ()
      end
      tr = tr[i[n]]
    end
    rawset (tr, i[#i], v)
  else
    rawset (tr, i, v)
  end
end

--- Make a deep copy of a tree, including any metatables
-- @param t table
-- @param nometa if non-nil don't copy metatables
-- @return copy of table
function clone (t, nometa)
  local r = {}
  if not nometa then
    setmetatable (r, getmetatable (t))
  end
  local d = {[t] = r}
  local function copy (o, x)
    for i, v in pairs (x) do
      if type (v) == "table" then
        if not d[v] then
          d[v] = {}
          if not nometa then
            setmetatable (d[v], getmetatable (v))
          end
          o[i] = copy (d[v], v)
        else
          o[i] = d[v]
        end
      else
        o[i] = v
      end
    end
    return o
  end
  return copy (r, t)
end

--- Additions to the string module
-- TODO: Pretty printing (use in getopt); see source for details.
module ("string", package.seeall)


-- Write pretty-printing based on:
--
--   John Hughes's and Simon Peyton Jones's Pretty Printer Combinators
--
--   Based on The Design of a Pretty-printing Library in Advanced
--   Functional Programming, Johan Jeuring and Erik Meijer (eds), LNCS 925
--   http://www.cs.chalmers.se/~rjmh/Papers/pretty.ps
--   Heavily modified by Simon Peyton Jones, Dec 96
--
--   Haskell types:
--   data Doc     list of lines
--   quote :: Char -> Char -> Doc -> Doc    Wrap document in ...
--   (<>) :: Doc -> Doc -> Doc              Beside
--   (<+>) :: Doc -> Doc -> Doc             Beside, separated by space
--   ($$) :: Doc -> Doc -> Doc              Above; if there is no overlap it "dovetails" the two
--   nest :: Int -> Doc -> Doc              Nested
--   punctuate :: Doc -> [Doc] -> [Doc]     punctuate p [d1, ... dn] = [d1 <> p, d2 <> p, ... dn-1 <> p, dn]
--   render      :: Int                     Line length
--               -> Float                   Ribbons per line
--               -> (TextDetails -> a -> a) What to do with text
--               -> a                       What to do at the end
--               -> Doc                     The document
--               -> a                       Result


--- Give strings a subscription operator.
-- @param s string
-- @param i index
-- @return <code>string.sub (s, i, i)</code> if i is a number, or
-- falls back to any previous metamethod (by default, string methods)
local old__index = getmetatable ("").__index
getmetatable ("").__index =
  function (s, i)
    if type (i) == "number" then
      return sub (s, i, i)
    -- Fall back to old metamethods
    elseif type (old__index) == "function" then
      return old__index (s, i)
    else
      return old__index[i]
    end
  end

--- Give strings an append metamethod.
-- @param s string
-- @param c character (1-character string)
-- @return <code>s .. c</code>
getmetatable ("").__append =
  function (s, c)
    return s .. c
  end

--- Capitalise each word in a string.
-- @param s string
-- @return capitalised string
function caps (s)
  return (gsub (s, "(%w)([%w]*)",
                function (l, ls)
                  return upper (l) .. ls
                end))
end

--- Remove any final newline from a string.
-- @param s string to process
-- @return processed string
function chomp (s)
  return (gsub (s, "\n$", ""))
end

--- Escape a string to be used as a pattern
-- @param s string to process
-- @return
--   @param s_: processed string
function escapePattern (s)
  return (gsub (s, "(%W)", "%%%1"))
end

-- Escape a string to be used as a shell token.
-- Quotes spaces, parentheses, brackets, quotes, apostrophes and
-- whitespace.
-- @param s string to process
-- @return processed string
function escapeShell (s)
  return (gsub (s, "([ %(%)%\\%[%]\"'])", "\\%1"))
end

--- Return the English suffix for an ordinal.
-- @param n number of the day
-- @return suffix
function ordinalSuffix (n)
  n = math.mod (n, 100)
  local d = math.mod (n, 10)
  if d == 1 and n ~= 11 then
    return "st"
  elseif d == 2 and n ~= 12 then
    return "nd"
  elseif d == 3 and n ~= 13 then
    return "rd"
  else
    return "th"
  end
end

--- Extend to work better with one argument.
-- If only one argument is passed, no formatting is attempted.
-- @param f format
-- @param ... arguments to format
-- @return formatted string
local _format = format
function format (f, arg1, ...)
  if arg1 == nil then
    return f
  else
    return _format (f, arg1, ...)
  end
end

--- Justify a string.
-- When the string is longer than w, it is truncated (left or right
-- according to the sign of w).
-- @param s string to justify
-- @param w width to justify to (-ve means right-justify; +ve means
-- left-justify)
-- @param p string to pad with (default: <code>" "</code>)
-- @return justified string
function pad (s, w, p)
  p = rep (p or " ", math.abs (w))
  if w < 0 then
    return sub (p .. s, w)
  end
  return sub (s .. p, 1, w)
end

--- Wrap a string into a paragraph.
-- @param s string to wrap
-- @param w width to wrap to (default: 78)
-- @param ind indent (default: 0)
-- @param ind1 indent of first line (default: ind)
-- @return wrapped paragraph
function wrap (s, w, ind, ind1)
  w = w or 78
  ind = ind or 0
  ind1 = ind1 or ind
  assert (ind1 < w and ind < w,
          "the indents must be less than the line width")
  s = rep (" ", ind1) .. s
  local lstart, len = 1, len (s)
  while len - lstart > w - ind do
    local i = lstart + w - ind
    while i > lstart and sub (s, i, i) ~= " " do
      i = i - 1
    end
    local j = i
    while j > lstart and sub (s, j, j) == " " do
      j = j - 1
    end
    s = sub (s, 1, j) .. "\n" .. rep (" ", ind) ..
      sub (s, i + 1, -1)
    local change = ind + 1 - (i - j)
    lstart = j + change
    len = len + change
  end
  return s
end

--- Write a number using SI suffixes.
-- The number is always written to 3 s.f.
-- @param n number
-- @return string
function numbertosi (n)
  local SIprefix = {
    [-8] = "y", [-7] = "z", [-6] = "a", [-5] = "f",
    [-4] = "p", [-3] = "n", [-2] = "mu", [-1] = "m",
    [0] = "", [1] = "k", [2] = "M", [3] = "G",
    [4] = "T", [5] = "P", [6] = "E", [7] = "Z",
    [8] = "Y"
  }
  local t = format("% #.2e", n)
  local _, _, m, e = t:find(".(.%...)e(.+)")
  local man, exp = tonumber (m), tonumber (e)
  local siexp = math.floor (exp / 3)
  local shift = exp - siexp * 3
  local s = SIprefix[siexp] or "e" .. tostring (siexp)
  man = man * (10 ^ shift)
  return tostring (man) .. s
end

--- Do find, returning captures as a list.
-- @param s target string
-- @param p pattern
-- @param init start position (default: 1)
-- @param plain inhibit magic characters (default: nil)
-- @return start of match
-- @return end of match
-- @return table of captures
function findl (s, p, init, plain)
  local function pack (from, to, ...)
    return from, to, {...}
  end
  return pack (p.find (s, p, init, plain))
end

--- Do multiple <code>find</code>s on a string.
-- @param s target string
-- @param p pattern
-- @param init start position (default: 1)
-- @param plain inhibit magic characters (default: nil)
-- @return list of <code>{from, to; capt = {captures}}</code>
function finds (s, p, init, plain)
  init = init or 1
  local l = {}
  local from, to, r
  repeat
    from, to, r = findl (s, p, init, plain)
    if from ~= nil then
      table.insert (l, {from, to, capt = r})
      init = to + 1
    end
  until not from
  return l
end

--- Perform multiple calls to gsub.
-- @param s string to call gsub on
-- @param sub <code>{pattern1=replacement1 ...}</code>
-- @param n upper limit on replacements (default: infinite)
-- @return result string
-- @return number of replacements made
function gsubs (s, sub, n)
  local r = 0
  for i, v in pairs (sub) do
    local rep
    if n ~= nil then
      s, rep = gsub (s, i, v, n)
      r = r + rep
      n = n - rep
      if n == 0 then
        break
      end
    else
      s, rep = i.gsub (s, i, v)
      r = r + rep
    end
  end
  return s, r
end

--- Split a string at a given separator.
-- FIXME: Consider Perl and Python versions.
-- @param s string to split
-- @param sep separator regex
-- @return list of strings
function split (s, sep)
  -- finds gets a list of {from, to, capt = {}} lists; we then
  -- flatten the result, discarding the captures, and prepend 0 (1
  -- before the first character) and append 0 (1 after the last
  -- character), and then read off the result in pairs.
  local pairs = list.concat ({0}, list.flatten (finds (s, sep)), {0})
  local l = {}
  for i = 1, #pairs, 2 do
    table.insert (l, sub (s, pairs[i] + 1, pairs[i + 1] - 1))
  end
  return l
end

--- Remove leading matter from a string.
-- @param s string
-- @param r leading regex (default: <code>"%s+"</code>)
-- @return string without leading r
function ltrim (s, r)
  r = r or "%s+"
  return (gsub (s, "^" .. r, ""))
end

--- Remove trailing matter from a string.
-- @param s string
-- @param r trailing regex (default: <code>"%s+"</code>)
-- @return string without trailing r
function rtrim (s, r)
  r = r or "%s+"
  return (gsub (s, r .. "$", ""))
end

--- Remove leading and trailing matter from a string.
-- @param s string
-- @param r leading/trailing regex (default: <code>"%s+"</code>)
-- @return string without leading/trailing r
function trim (s, r)
  return rtrim (ltrim (s, r), r)
end

--- Additions to the io module
module ("io", package.seeall)

require "base"
require "package_ext"


-- Get file handle metatable
local file_metatable = getmetatable (io.stdin)


--- Read a file into a list of lines and close it.
-- @param h file handle or name (default: <code>io.input ()</code>)
-- @return list of lines
function readlines (h)
  if h == nil then
    h = input ()
  elseif _G.type (h) == "string" then
    h = io.open (h)
  end
  local l = {}
  for line in h:lines () do
    table.insert (l, line)
  end
  h:close ()
  return l
end
file_metatable.readlines = readlines

--- Write values adding a newline after each.
-- @param h file handle (default: <code>io.output ()</code>
-- @param ... values to write (as for write)
function writeline (h, ...)
  if io.type (h) ~= "file" then
    io.write (h, "\n")
    h = io.output ()
  end
  for _, v in ipairs ({...}) do
    h:write (v, "\n")
  end
end
file_metatable.writeline = writeline

--- Split a directory path into components.
-- Empty components are retained: the root directory becomes <code>{"", ""}</code>.
-- @param path path
-- @return list of path components
function splitdir (path)
  return string.split (path, package.dirsep)
end

--- Concatenate one or more directories and a filename into a path.
-- @param ... path components
-- @return path
function catfile (...)
  return table.concat ({...}, package.dirsep)
end

--- Concatenate two or more directories into a path, removing the trailing slash.
-- @param ... path components
-- @return path
function catdir (...)
  return (string.gsub (catfile (...), "^$", package.dirsep))
end

--- Perform a shell command and return its output.
-- @param c command
-- @return output, or nil if error
function shell (c)
  local h = io.popen (c)
  local o
  if h then
    o = h:read ("*a")
    h:close ()
  end
  return o
end

--- Process files specified on the command-line.
-- If no files given, process <code>io.stdin</code>; in list of files,
-- <code>-</code> means <code>io.stdin</code>.
-- <br>FIXME: Make the file list an argument to the function.
-- @param f function to process files with, which is passed
-- <code>(name, arg_no)</code>
function processFiles (f)
  -- N.B. "arg" below refers to the global array of command-line args
  if #arg == 0 then
    table.insert (arg, "-")
  end
  for i, v in ipairs (arg) do
    if v == "-" then
      io.input (io.stdin)
    else
      io.input (v)
    end
    prog.file = v
    f (v, i)
  end
end

-- Set datatype.
module ("set", package.seeall)


-- Primitive methods (know about representation)
-- The representation is a table whose tags are the elements, and
-- whose values are true.

--- Say whether an element is in a set
-- @param s set
-- @param e element
-- @return <code>true</code> if e is in set, <code>false</code>
-- otherwise
function member (s, e)
  return rawget (s, e) == true
end

--- Insert an element into a set
-- @param s set
-- @param e element
function insert (s, e)
  rawset (s, e, true)
end

--- Delete an element from a set
-- @param s set
-- @param e element
function delete (s, e)
  rawset (s, e, nil)
end

--- Make a list into a set
-- @param l list
-- @return set
local metatable = {}
function new (l)
  local s = setmetatable ({}, metatable)
  for _, e in ipairs (l) do
    insert (s, e)
  end
  return s
end

--- Iterator for sets
-- TODO: Make the iterator return only the key
elements = pairs


-- High level methods (representation-independent)

--- Find the difference of two sets
-- @param s set
-- @param t set
-- @return s with elements of t removed
function difference (s, t)
  local r = new {}
  for e in elements (s) do
    if not member (t, e) then
      insert (r, e)
    end
  end
  return r
end

--- Find the symmetric difference of two sets
-- @param s set
-- @param t set
-- @return elements of s and t that are in s or t but not both
function symmetric_difference (s, t)
  return difference (union (s, t), intersection (t, s))
end

--- Find the intersection of two sets
-- @param s set
-- @param t set
-- @return set intersection of s and t
function intersection (s, t)
  local r = new {}
  for e in elements (s) do
    if member (t, e) then
      insert (r, e)
    end
  end
  return r
end

--- Find the union of two sets
-- @param s set
-- @param t set
-- @return set union of s and t
function union (s, t)
  local r = new {}
  for e in elements (s) do
    insert (r, e)
  end
  for e in elements (t) do
    insert (r, e)
  end
  return r
end

--- Find whether one set is a subset of another
-- @param s set
-- @param t set
-- @return <code>true</code> if s is a subset of t, <code>false</code>
-- otherwise
function subset (s, t)
  for e in elements (s) do
    if not member (t, e) then
      return false
    end
  end
  return true
end

--- Find whether one set is a proper subset of another
-- @param s set
-- @param t set
-- @return <code>true</code> if s is a proper subset of t, false otherwise
function propersubset (s, t)
  return subset (s, t) and not subset (t, s)
end

--- Find whether two sets are equal
-- @param s set
-- @param t set
-- @return <code>true</code> if sets are equal, <code>false</code>
-- otherwise
function equal (s, t)
  return subset (s, t) and subset (t, s)
end

--- Metamethods for sets
-- set:method ()
metatable.__index = _M
-- set + table = union
metatable.__add = union
-- set - table = set difference
metatable.__sub = difference
-- set * table = intersection
metatable.__mul = intersection
-- set / table = symmetric difference
metatable.__div = symmetric_difference
-- set <= table = subset
metatable.__le = subset
-- set < table = proper subset
metatable.__lt = propersubset
