-- Kill ring facility functions
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
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

local kill_ring_text

local function maybe_free_kill_ring ()
  if _last_command ~= "kill-region" then
    kill_ring_text = nil
  end
end

local function kill_ring_push (es)
  kill_ring_text = estr_cat (kill_ring_text or estr_new (""), es)
end

local function copy_or_kill_region (kill, rp)
  kill_ring_push (get_buffer_region (cur_bp, rp))

  if kill then
    if cur_bp.readonly then
      minibuf_error ("Read only text copied to kill ring")
    else
      assert (delete_region (rp))
    end
  end

  _this_command = "kill-region"
  deactivate_mark ()

  return true
end

local function copy_or_kill_the_region (kill)
  local rp = calculate_the_region ()

  if rp then
    maybe_free_kill_ring ()
    copy_or_kill_region (kill, rp)
    return true
  end

  return false
end

local function kill_text (uniarg, mark_func)
  maybe_free_kill_ring ()

  if warn_if_readonly_buffer () then
    return false
  end

  push_mark ()
  execute_function (mark_func, uniarg)
  execute_function ("kill-region")
  pop_mark ()

  _this_command = "kill-region"
  minibuf_write ("") -- Erase "Set mark" message.
  return true
end

Defun ("kill-word",
       {"number"},
[[
Kill characters forward until encountering the end of a word.
With argument @i{arg}, do this that many times.
]],
  true,
  function (arg)
    return kill_text (arg, "mark-word")
  end
)

Defun ("backward-kill-word",
       {"number"},
[[
Kill characters backward until encountering the end of a word.
With argument @i{arg}, do this that many times.
]],
  true,
  function (arg)
    return kill_text (-(arg or 1), "mark-word")
  end
)

Defun ("kill-sexp",
       {"number"},
[[
Kill the sexp (balanced expression) following the cursor.
With @i{arg}, kill that many sexps after the cursor.
Negative arg -N means kill N sexps before the cursor.
]],
  true,
  function (arg)
    return kill_text (arg, "mark-sexp")
  end
)

Defun ("yank",
       {},
[[
Reinsert the last stretch of killed text.
More precisely, reinsert the stretch of killed text most recently
killed @i{or} yanked.  Put point at end, and set mark at beginning.
]],
  true,
  function ()
    if not kill_ring_text then
      minibuf_error ("Kill ring is empty")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    execute_function ("set-mark-command")
    insert_estr (kill_ring_text)
    deactivate_mark ()
  end
)

Defun ("kill-region",
       {},
[[
Kill between point and mark.
The text is deleted but saved in the kill ring.
The command @kbd{C-y} (yank) can retrieve it from there.
If the buffer is read-only, Zile will beep and refrain from deleting
the text, but put the text in the kill ring anyway.  This means that
you can use the killing commands to copy text from a read-only buffer.
If the previous command was also a kill command,
the text killed this time appends to the text killed last time
to make one entry in the kill ring.
]],
  true,
  function ()
    return copy_or_kill_the_region (true)
  end
)

Defun ("copy-region-as-kill",
       {},
[[
Save the region as if killed, but don't kill it.
]],
  true,
  function ()
    return copy_or_kill_the_region (false)
  end
)

local function kill_to_bol ()
  return bolp () or
    copy_or_kill_region (true, region_new (get_buffer_line_o (cur_bp), get_buffer_o (cur_bp)))
end

local function kill_line (whole_line)
  local ok = true
  local only_blanks_to_end_of_line = true

  if not whole_line then
    for i = get_buffer_o (cur_bp) - get_buffer_line_o (cur_bp), buffer_line_len (cur_bp) - 1 do
      local c = get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i)
      if not (c == ' ' or c == '\t') then
        only_blanks_to_end_of_line = false
        break
      end
    end
  end

  if eobp () then
    minibuf_error ("End of buffer")
    return false
  end

  undo_start_sequence ()

  if not eolp () then
    ok = copy_or_kill_region (true, region_new (get_buffer_o (cur_bp), get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp)))
  end

  if ok and (whole_line or only_blanks_to_end_of_line) and not eobp () then
    if not execute_function ("delete-char") then
      return false
    end

    kill_ring_push ({s = "\n", eol = coding_eol_lf})
    _this_command = "kill-region"
  end

  undo_end_sequence ()

  return ok
end

local function kill_whole_line ()
  return kill_line (true)
end

local function kill_line_backward ()
  return previous_line () and kill_whole_line ()
end

Defun ("kill-line",
       {"number"},
[[
Kill the rest of the current line; if no nonblanks there, kill thru newline.
With prefix argument @i{arg}, kill that many lines from point.
Negative arguments kill lines backward.
With zero argument, kills the text before point on the current line.

If @samp{kill-whole-line} is non-nil, then this command kills the whole line
including its terminating newline, when used at the beginning of a line
with no argument.
]],
  true,
  function (arg)
    local ok = true

    maybe_free_kill_ring ()

    if not arg then
      ok = kill_line (bolp () and get_variable_bool ("kill-whole-line"))
    else
      undo_start_sequence ()
      if arg <= 0 then
        kill_to_bol ()
      end
      if arg ~= 0 and ok then
        ok = execute_with_uniarg (true, arg, kill_whole_line, kill_line_backward)
      end
      undo_end_sequence ()
    end

    deactivate_mark ()
    return ok
  end
)
