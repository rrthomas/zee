-- Key bindings
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

Defun ("edit-insert-character",
[[
Insert the character you type.
Whichever character you type to run this command is inserted.
]],
  function ()
    local key = term_keytobyte (lastkey ())
    deactivate_mark ()
    if not key then
      ding ()
      return false
    end

    if string.char (key):match ("%s") and cur_bp.autofill and get_goalc () > tonumber (get_variable ("fill-column")) then
      fill_break_line ()
    end

    insert_char (string.char (key))
    return true
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
  local ok = execute_function (f, ...) -- FIXME: Most of this (except _interactive) should be inside execute_function
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
  local key = get_key_chord ()
  local name = get_function_by_key (key)

  popup_clear ()
  minibuf_clear ()

  if function_exists (name) then
    call_command (name)
  else
    minibuf_error (tostring (key) .. " is undefined")
  end
end

root_bindings = {}

local function key_canon (keystr)
  local key = tostring (keycode (keystr)) -- canonicalize the string
  if not key then
    minibuf_error (string.format ("Key sequence %s is invalid", keystr))
    return
  end
  return key
end

function key_unbind (keystr)
  local key = key_canon (keystr)
  if key then
    root_bindings[key] = nil
  end
end

function key_bind (keystr, cmd)
  local key = key_canon (keystr)
  if key then
    if not function_exists (cmd) then -- Possible if called non-interactively
      minibuf_error (string.format ("No such function `%s'", cmd))
      return
    end

    root_bindings[key] = cmd
  end
end

function init_default_bindings ()
  -- Bind all printing keys to edit-insert-character
  for i = 0, 0xff do
    if posix.isprint (string.char (i)) then
      root_bindings[string.char (i)] = "edit-insert-character"
    end
  end

  -- Bind special key names to edit-insert-character
  list.map (function (e)
              root_bindings[tostring (keycode (e))] = "edit-insert-character"
            end,
            {"SPC", "TAB", "RET"})

  key_bind ("C-b", "move-previous-character")
  key_bind ("LEFT", "move-previous-character")
  key_bind ("BACKSPACE", "edit-delete-previous-character")
  key_bind ("C-?", "edit-delete-previous-character")
  key_bind ("M-BACKSPACE", "edit-kill-word-backward")
  key_bind ("C-M-?", "edit-kill-word-backward")
  key_bind ("M-{", "move-previous-paragraph")
  key_bind ("M-b", "move-previous-word")
  key_bind ("M-LEFT", "move-previous-word")
  key_bind ("M-<", "move-start-file")
  key_bind ("C-a", "move-start-line")
  key_bind ("HOME", "move-start-line")
  key_bind ("M-m", "call-last-kbd-macro")
  key_bind ("M-w", "edit-copy")
  key_bind ("C-d", "edit-delete-next-character")
  key_bind ("DELETE", "edit-delete-next-character")
  key_bind ("M-\\", "delete-horizontal-space")
  key_bind ("M->", "move-end-file")
  key_bind ("C-e", "move-end-line")
  key_bind ("END", "move-end-line")
  key_bind ("M-x", "execute-command")
  key_bind ("M-q", "edit-wrap-paragraph")
  key_bind ("RIGHT", "move-next-character")
  key_bind ("C-f", "move-next-character")
  key_bind ("M-}", "move-next-paragraph")
  key_bind ("M-f", "move-next-word")
  key_bind ("M-RIGHT", "move-next-word")
  key_bind ("M-g", "edit-goto-line")
  key_bind ("TAB", "indent-relative")
  key_bind ("C-r", "edit-find-backward")
  key_bind ("C-s", "edit-find")
  key_bind ("C-g", "keyboard-quit")
  key_bind ("C-w", "edit-kill-selection")
  key_bind ("M-d", "edit-kill-word")
  key_bind ("RET", "edit-insert-newline")
  key_bind ("C-j", "edit-insert-newline-and-indent")
  key_bind ("C-n", "move-next-line")
  key_bind ("DOWN", "move-next-line")
  key_bind ("C-p", "move-previous-line")
  key_bind ("UP", "move-previous-line")
  key_bind ("M-%", "edit-replace")
  key_bind ("C-q", "edit-insert-quoted")
  key_bind ("C-l", "move-redraw")
  key_bind ("M-s", "file-save")
  key_bind ("C-M-q", "file-quit")
  key_bind ("M-v", "move-previous-page")
  key_bind ("PAGEDOWN", "move-previous-page")
  key_bind ("C-v", "move-next-page")
  key_bind ("PAGEUP", "move-next-page")
  key_bind ("C-@", "edit-select-on")
  key_bind ("M-|", "edit-shell-command")
  key_bind ("C-z", "file-suspend")
  key_bind ("M-i", "edit-insert-tab")
  key_bind ("M-r", "preferences-toggle-read-only")
  key_bind ("C-_", "edit-undo")
  key_bind ("C-y", "edit-paste")
end

function get_function_by_key (key)
  return root_bindings[tostring (key)]
end

Defun ("where-is",
[[
Print message listing key sequences that invoke the command DEFINITION.
Argument is a command name.
]],
  function ()
    local name = minibuf_read_function_name ("Where is command: ")

    if name and function_exists (name) then
      local keys = {}
      for k, n in pairs (root_bindings) do
        if n == name then
          table.insert (keys, k)
        end
      end

      if #keys == 0 then
        minibuf_write (name .. " is not on any key")
      else
        minibuf_write ("%s is on %s", name, table.concat (keys, ", "))
      end
      return true
    end
  end
)
