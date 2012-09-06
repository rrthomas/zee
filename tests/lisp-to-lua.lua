#!/usr/bin/env lua5.1
-- Convert Zile's Lisp test scripts to Lua

-- Usage: lisp-to-lua LISP-FILE


require "std"

function read_char (s, pos)
  if pos <= #s then
    return s[pos], pos + 1
  end
  return -1, pos
end

function read_token (s, pos)
  local c
  local doublequotes = false
  local tok = ""

  -- Chew space to next token
  repeat
    c, pos = read_char (s, pos)

    -- Munch comments
    if c == ";" then
      repeat
        c, pos = read_char (s, pos)
      until c == -1 or c == "\n"
    end
  until c ~= " " and c ~= "\t"

  -- Snag token
  if c == "(" or c == ")" or c == "'" or c == "\n" or c == -1 then
    return tok, c, pos
  end

  -- It looks like a string. Snag to the next whitespace.
  if c == "\"" then
    doublequotes = true
    c, pos = read_char (s, pos)
  end

  repeat
    tok = tok .. c
    if not doublequotes then
      if c == ")" or c == "(" or c == ";" or c == " " or c == "\n"
        or c == "\r" or c == -1 then
        pos = pos - 1
        tok = string.sub (tok, 1, -2)
        return tok, "word", pos
      end
    else
      if c == "\n" or c == "\r" or c == -1 then
        pos = pos - 1
      end
      if c == "\"" then
        tok = string.sub (tok, 1, -2)
        return tok, "word", pos
      end
    end
    c, pos = read_char (s, pos)
  until false
end

function lisp_read (s)
  local pos = 1
  local function read ()
    local l = {}
    repeat
      local tok, tokenid
      tok, tokenid, pos = read_token (s, pos)
      if tokenid ~= "'" then
        if tokenid == "(" then
          table.insert (l, read ())
        elseif tokenid == "word" then
          table.insert (l, tok)
        end
      end
    until tokenid == ")" or tokenid == -1
    return l
  end

  return read ()
end

for _, lisp_file in ipairs (arg) do
  local h = io.open (lisp_file:gsub ("%.el$", ".lua"), "w")
  for l in io.lines (lisp_file) do
    if l:match ("^;") then -- the line is a comment
      l = l:gsub ("^(;+)", "--")
    elseif l:match ("^%s*$") then -- blank line
    else
      local lisp = list.flatten (lisp_read (l))
      local lua = {}
      for _, v in ipairs (lisp) do
        if not v:match ("^\"") then
          v = "\"" .. v .. "\""
        end
        table.insert (lua, (v:gsub ("\\", "\\\\")))
      end
      l = "call_command (" .. table.concat (lua, ", ") .. ")"
    end
    h:write (l .. "\n")
  end
  h:close ()
end
