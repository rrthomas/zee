-- Key bindings and extended commands
--
-- Copyright (c) 2010-2011 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

-- Key binding.

function self_insert_command ()
  local ret = true
  -- Mask out ~KBD_CTRL to allow control sequences to be themselves.
  local key = lastkey ()
  key = bit.band (key, bit.bnot (KBD_CTRL))
  deactivate_mark ()
  if key <= 0xff then
    if string.match (string.char (key), "%s") and cur_bp.autofill and get_goalc () > get_variable_number ("fill-column") then
      fill_break_line ()
    end
    insert_char (string.char (key))
  else
    ding ()
    ret = false
  end

  return ret
end

Defun ("self-insert-command",
       {},
[[
Insert the character you type.
Whichever character you type to run this command is inserted.
]],
  true,
  function ()
    return execute_with_uniarg (true, current_prefix_arg, self_insert_command)
  end
)

_last_command = nil
_this_command = nil
_interactive = false

function process_command ()
  local keys = get_key_sequence ()
  local name = get_function_by_keys (keys)

  thisflag = {defining_macro = lastflag.defining_macro}
  minibuf_clear ()

  if function_exists (name) then
    _this_command = name
    _interactive = true
    execute_function (name, lastflag.set_uniarg and (prefix_arg or 1))
    _interactive = false
    _last_command = _this_command
  else
    minibuf_error (keyvectostr (keys) .. " is undefined")
  end

  -- Only add keystrokes if we were already in macro defining mode
  -- before the function call, to cope with start-kbd-macro.
  if lastflag.defining_macro and thisflag.defining_macro then
    add_cmd_to_macro ()
  end

  if _last_command ~= "undo" then
    cur_bp.next_undop = cur_bp.last_undop
  end

  lastflag = thisflag
end

root_bindings = tree.new ()

function init_default_bindings ()
  -- Bind all printing keys to self_insert_command
  for i = 0, 0xff do
    if posix.isprint (string.char (i)) then
      root_bindings[{i}] = "self-insert-command"
    end
  end

  lisp_loadfile (PATH_DATA .. "/default-bindings.el")
end

function do_binding_completion (as)
  local key
  local bs = ""

  if lastflag.set_uniarg then
    local arg = prefix_arg

    if arg < 0 then
      bs = bs .. "- "
      arg = -arg
    end

    repeat
      bs = " " .. bs
      bs = string.char (arg % 10 + string.byte ('0')) .. bs
      arg = math.floor (arg / 10)
    until arg == 0
  end

  minibuf_write (((lastflag.set_uniarg or lastflag.uniarg_empty) and "C-u " or "") ..
                 bs .. as .. "-")
  key = getkey (GETKEY_DEFAULT)
  minibuf_clear ()

  return key
end

local function walk_bindings (tree, process, st)
  local function walk_bindings_tree (tree, keys, process, st)
    for key, node in pairs (tree) do
      table.insert (keys, chordtostr (key))
      if type (node) == "string" then
        process (table.concat (keys, " "), node, st)
      else
        walk_bindings_tree (node, keys, process, st)
      end
      table.remove (keys)
    end
  end

  walk_bindings_tree (tree, {}, process, st)
end

-- Get a key sequence from the keyboard; the sequence returned
-- has at most the last stroke unbound.
function get_key_sequence ()
  local keys = {}

  local key
  repeat
    key = getkey (GETKEY_DEFAULT)
  until key ~= KBD_NOKEY
  table.insert (keys, key)

  local func
  while true do
    func = root_bindings[keys]
    if type (func) ~= "table" then
      break
    end
    local s = keyvectostr (keys)
    table.insert (keys, do_binding_completion (s))
  end

  return keys
end

function get_function_by_keys (keys)
  -- Detect Meta-digit
  if #keys == 1 then
    local key = keys[1]
    if bit.band (key, KBD_META) ~= 0 and (string.match (string.char (bit.band (key, 0xff)), "%d") or bit.band (key, 0xff) == string.byte ('-')) then
      return "universal-argument"
    end
  end

  local func = root_bindings[keys]
  return type (func) == "string" and func or nil
end

-- gather_bindings_state:
-- {
--   f: name of function
--   bindings: bindings
-- }

function gather_bindings (key, p, g)
  if p == g.f then
    if #g.bindings > 0 then
      g.bindings = g.bindings .. ", "
    end
    g.bindings = g.bindings .. key
  end
end

Defun ("where-is",
       {},
[[
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command name.  If the prefix arg is non-nil, insert the
message in the buffer.
]],
  true,
  function ()
    local name = minibuf_read_function_name ("Where is command: ")
    local g = {}

    if name then
      g.f = name
      if function_exists (g.f) then
        g.bindings = ""
        walk_bindings (root_bindings, gather_bindings, g)

        if #g.bindings == 0 then
          minibuf_write (name .. " is not on any key")
        else
          local s = string.format ("%s is on %s", name, g.bindings)
          if lastflag.set_uniarg then
            insert_string (s)
          else
            minibuf_write (s)
          end
        end
        return true
      end
    end
  end
)

local function print_binding (key, func)
  insert_string (string.format ("%-15s %s\n", key, func))
end

local function write_bindings_list (key, binding)
  insert_string ("Key translations:\n")
  insert_string (string.format ("%-15s %s\n", "key", "binding"))
  insert_string (string.format ("%-15s %s\n", "---", "-------"))

  walk_bindings (root_bindings, print_binding)
end

Defun ("describe-bindings",
       {},
[[
Show a list of all defined keys, and their definitions.
]],
  true,
  function ()
    write_temp_buffer ("*Help*", true, write_bindings_list)
    return true
  end
)


Defun ("global-set-key",
       {"string", "string"},
[[
Bind a command to a key sequence.
Read key sequence and function name, and bind the function to the key
sequence.
]],
  true,
  function (keystr, name)
    local keys

    if keystr then
      keys = keystrtovec (keystr)
      if not keys then
        minibuf_error (string.format ("Key sequence %s is invalid", keystr))
        return
      end
    else
      minibuf_write ("Set key globally: ")
      keys = get_key_sequence ()
      keystr = keyvectostr (keys)
    end

    if not name then
      name = minibuf_read_function_name (string.format ("Set key %s to command: ", keystr))
      if not name then
        return
      end
    end

    if not function_exists (name) then -- Possible if called non-interactively
      minibuf_error (string.format ("No such function `%s'", name))
      return
    end

    root_bindings[keys] = name

    return true
  end
)
