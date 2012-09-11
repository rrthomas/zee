-- Key bindings and extended commands
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of Zee.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

-- Key binding.

function self_insert_command ()
  local key = term_keytobyte (lastkey ())
  deactivate_mark ()
  if not key then
    ding ()
    return false
  end

  if string.char (key):match ("%s") and cur_bp.autofill and get_goalc () > get_variable_number ("fill_column") then
    fill_break_line ()
  end

  insert_char (string.char (key))
  return true
end

Defun ("edit-insert-character",
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

function call_command (f, ...)
  thisflag = {defining_macro = lastflag.defining_macro}

  -- Execute the command.
  _this_command = f
  _interactive = true
  local ok = execute_function (f, ...)
  _interactive = false
  _last_command = _this_command

  -- Only add keystrokes if we were already in macro defining mode
  -- before the function call, to cope with start-kbd-macro.
  if lastflag.defining_macro and thisflag.defining_macro then
    add_cmd_to_macro ()
  end

  if cur_bp and _last_command ~= "edit-undo" then
    cur_bp.next_undop = cur_bp.last_undop
  end

  lastflag = thisflag

  return ok
end

function get_and_run_command ()
  local keys = get_key_sequence ()
  local name = get_function_by_keys (keys)

  minibuf_clear ()

  if function_exists (name) then
    call_command (name, lastflag.set_uniarg and (prefix_arg or 1))
  else
    minibuf_error (tostring (keys) .. " is undefined")
  end
end

root_bindings = tree.new ()

function key_bind (key,  cmd)
  execute_function ("global-set-key", key, cmd)
end

function init_default_bindings ()
  -- Bind all printing keys to self_insert_command
  for i = 0, 0xff do
    if posix.isprint (string.char (i)) then
      root_bindings[{keycode (string.char (i))}] = "edit-insert-character"
    end
  end

  -- Bind special key names to edit-insert-character
  list.map (function (e)
              root_bindings[{keycode (e)}] = "edit-insert-character"
            end,
            {"\\SPC", "\\TAB", "\\RET", "\\\\"})

  key_bind ("\\M-m", "back-to-indentation")
  key_bind ("\\C-b", "move-previous-character")
  key_bind ("\\LEFT", "move-previous-character")
  key_bind ("\\BACKSPACE", "backward-edit-delete-next-character")
  key_bind ("\\C-?", "backward-edit-delete-next-character")
  key_bind ("\\M-\\BACKSPACE", "edit-kill-word-backward")
  key_bind ("\\C-\\M-?", "edit-kill-word-backward")
  key_bind ("\\M-{", "move-previous-paragraph")
  key_bind ("\\M-b", "move-previous-word")
  key_bind ("\\M-\\LEFT", "move-previous-word")
  key_bind ("\\M-<", "move-start-file")
  key_bind ("\\C-a", "move-start-line")
  key_bind ("\\HOME", "move-start-line")
  key_bind ("\\C-xe", "call-last-kbd-macro")
  key_bind ("\\M-w", "edit-copy")
  key_bind ("\\C-d", "edit-delete-next-character")
  key_bind ("\\DELETE", "edit-delete-next-character")
  key_bind ("\\M-\\\\", "delete-horizontal-space")
  key_bind ("\\C-hb", "describe-bindings")
  key_bind ("\\F1b", "describe-bindings")
  key_bind ("\\C-hf", "describe-function")
  key_bind ("\\F1f", "describe-function")
  key_bind ("\\C-hk", "help-key")
  key_bind ("\\F1k", "help-key")
  key_bind ("\\C-hv", "describe-variable")
  key_bind ("\\F1v", "describe-variable")
  key_bind ("\\C-x)", "macro-stop")
  key_bind ("\\M->", "move-end-file")
  key_bind ("\\C-e", "move-end-line")
  key_bind ("\\END", "move-end-line")
  key_bind ("\\C-x\\C-x", "edit-select-other-end")
  key_bind ("\\M-x", "execute-command")
  key_bind ("\\M-q", "edit-wrap-paragraph")
  key_bind ("\\RIGHT", "move-next-character")
  key_bind ("\\C-f", "move-next-character")
  key_bind ("\\M-}", "move-next-paragraph")
  key_bind ("\\M-f", "move-next-word")
  key_bind ("\\M-\\RIGHT", "move-next-word")
  key_bind ("\\M-gg", "edit-goto-line")
  key_bind ("\\M-g\\M-g", "edit-goto-line")
  key_bind ("\\TAB", "indent-for-tab-command")
  key_bind ("\\C-xi", "insert-file")
  key_bind ("\\C-r", "edit-find-backward")
  key_bind ("\\C-s", "edit-find")
  key_bind ("\\C-g", "keyboard-quit")
  key_bind ("\\C-k", "edit-kill-line")
  key_bind ("\\C-w", "edit-kill-selection")
  key_bind ("\\M-d", "edit-kill-word")
  key_bind ("\\RET", "edit-insert-newline")
  key_bind ("\\C-j", "edit-insert-newline-and-indent")
  key_bind ("\\C-n", "move-next-line")
  key_bind ("\\DOWN", "move-next-line")
  key_bind ("\\C-o", "open-line")
  key_bind ("\\C-xo", "other-window")
  key_bind ("\\C-p", "move-previous-line")
  key_bind ("\\UP", "move-previous-line")
  key_bind ("\\M-%", "edit-replace")
  key_bind ("\\C-q", "edit-insert-quoted")
  key_bind ("\\C-l", "move-redraw")
  key_bind ("\\C-x\\C-s", "file-save")
  key_bind ("\\C-x\\C-c", "file-quit")
  key_bind ("\\M-v", "move-previous-page")
  key_bind ("\\PRIOR", "move-previous-page")
  key_bind ("\\C-v", "move-next-page")
  key_bind ("\\NEXT", "move-next-page")
  key_bind ("\\C-xf", "set-fill-column")
  key_bind ("\\C-@", "set-mark-command")
  key_bind ("\\M-|", "edit-shell-command")
  key_bind ("\\C-x(", "macro-record")
  key_bind ("\\C-x\\C-z", "file-suspend")
  key_bind ("\\C-z", "file-suspend")
  key_bind ("\\M-i", "edit-insert-tab")
  key_bind ("\\C-x\\C-q", "preferences-toggle-read-only")
  key_bind ("\\C-xu", "edit-undo")
  key_bind ("\\C-_", "edit-undo")
  key_bind ("\\C-u", "edit-repeat")
  key_bind ("\\C-hw", "where-is")
  key_bind ("\\F1w", "where-is")
  key_bind ("\\C-x\\C-w", "write-file")
  key_bind ("\\C-y", "edit-paste")
end

function do_binding_completion (as)
  local bs = ""

  if lastflag.set_uniarg then
    local arg = math.abs (prefix_arg or 1)
    repeat
      bs = string.char (arg % 10 + string.byte ('0')) .. " " .. bs
      arg = math.floor (arg / 10)
    until arg == 0
  end

  if prefix_arg and prefix_arg < 0 then
    bs = "- " .. bs
  end

  minibuf_write (((lastflag.set_uniarg or lastflag.uniarg_empty) and "C-u " or "") ..
                 bs .. as .. "-")
  local key = getkey (GETKEY_DEFAULT)
  minibuf_clear ()

  return key
end

local function walk_bindings (tree, process, st)
  local function walk_bindings_tree (tree, keys, process, st)
    for key, node in pairs (tree) do
      table.insert (keys, tostring (key))
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
  local keys = keystrtovec ""

  local key
  repeat
    key = getkey (GETKEY_DEFAULT)
  until key ~= nil
  table.insert (keys, key)

  local func
  while true do
    func = root_bindings[keys]
    if type (func) ~= "table" then
      break
    end
    local s = tostring (keys)
    table.insert (keys, do_binding_completion (s))
  end

  return keys
end

function get_function_by_keys (keys)
  -- Detect Meta-digit
  if #keys == 1 then
    local key = keys[1]
    if key.META and key.key < 255 and string.match (string.char (key.key), "[%d%-]") then
      return "edit-repeat"
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
Argument is a command name.
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
          minibuf_write ("%s is on %s", name, g.bindings)
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
      keystr = tostring (keys)
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
