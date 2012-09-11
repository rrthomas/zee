-- Miscellaneous Emacs functions
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

local function get_region ()
  activate_mark ()
  return get_buffer_region (cur_bp, calculate_the_region ())
end

Defun ("keyboard-quit",
       {},
[[
Cancel current command.
]],
  true,
  function ()
    deactivate_mark ()
    return minibuf_error ("Quit")
  end
)

Defun ("file-suspend",
       {},
[[
Stop editor and return to superior process.
]],
  true,
  function ()
    posix.raise (posix.SIGTSTP)
  end
)

Defun ("preferences-toggle-read-only",
       {},
[[
Change whether this buffer is visiting its file read-only.
]],
  true,
  function ()
    cur_bp.readonly = not cur_bp.readonly
  end
)

Defun ("preferences-toggle-wrap-mode",
       {},
[[
Toggle Auto Fill mode.
In Auto Fill mode, inserting a space at a column beyond `fill-column'
automatically breaks the line at a previous space.
]],
  true,
  function ()
    cur_bp.autofill = not cur_bp.autofill
  end
)

Defun ("edit-select-other-end",
       {},
[[
Put the mark where point is now, and point where the mark is now.
]],
  true,
  function ()
    if not cur_bp.mark then
      return minibuf_error ("No mark set in this buffer")
    end

    local tmp = get_buffer_pt (cur_bp)
    goto_offset (cur_bp.mark.o)
    cur_bp.mark.o = tmp
    activate_mark ()
    thisflag.need_resync = true
  end
)

Defun ("edit-repeat",
       {},
[[
Begin a numeric argument for the following command.
Digits or minus sign following @kbd{C-u} make up the numeric argument.
@kbd{C-u} following the digits or minus sign ends the argument.
@kbd{C-u} without digits or minus sign provides 4 as argument.
Repeating @kbd{C-u} without digits or minus sign multiplies the argument
by 4 each time.
]],
  true,
  function ()
    local ok = true

    -- Need to process key used to invoke universal-argument.
    pushkey (lastkey ())

    thisflag.uniarg_empty = true

    local i = 0
    local arg = 1
    local sgn = 1
    local keys = {}
    while true do
      local as = ""
      local key = do_binding_completion (table.concat (keys, " "))

      -- Cancelled.
      if key == keycode "\\C-g" then
        ok = execute_function ("keyboard-quit")
        break
      -- Digit pressed.
      elseif string.match (string.char (key.key), "%d") then
        local digit = key.key - string.byte ('0')
        thisflag.uniarg_empty = false

        if key.META then
          as = "ESC "
        end

        as = as .. string.format ("%d", digit)

        if i == 0 then
          arg = digit
        else
          arg = arg * 10 + digit
        end

        i = i + 1
      elseif key == keycode "\\C-u" then
        as = as .. "C-u"
        if i == 0 then
          arg = arg * 4
        else
          break
        end
      elseif key == keycode "\\M--" and i == 0 then
        if sgn > 0 then
          sgn = -sgn
          as = as .. "-"
          -- The default negative arg is -1, not -4.
          arg = 1
          thisflag.uniarg_empty = false
        end
      else
        ungetkey (key)
        break
      end

      table.insert (keys, as)
    end

    if ok then
      prefix_arg = arg * sgn
      thisflag.set_uniarg = true
      minibuf_clear ()
    end

    return ok
  end
)

Defun ("edit-select-on",
       {},
[[
Set this buffer's mark to point.
]],
  false,
  function ()
    set_mark ()
    activate_mark ()
  end
)

Defun ("set-mark-command",
       {},
[[
Set the mark where point is.
]],
  true,
  function ()
    execute_function ("edit-select-on")
    minibuf_write ("Mark set")
  end
)

Defun ("set-fill-column",
       {"number"},
[[
Set `fill-column' to specified argument.
Use C-u followed by a number to specify a column.
Just C-u as argument means to use the current column.
]],
  true,
  function (n)
    if not n and _interactive then
      local o = get_buffer_pt (cur_bp) - get_buffer_line_o (cur_bp)
      if lastflag.set_uniarg then
        n = current_prefix_arg
      else
        n = minibuf_read_number (string.format ("Set fill-column to (default %d): ", o))
        if not n then -- cancelled
          return false
        elseif n == "" then
          n = o
        end
      end
    end

    if not n then
      return minibuf_error ("set-fill-column requires an explicit argument")
    end

    minibuf_write (string.format ("Fill column set to %d (was %d)", n, get_variable_number ("fill-column")))
    set_variable ("fill-column", tostring (n))
    return true
  end
)

Defun ("edit-insert-quoted",
       {},
[[
Read next input character and insert it.
This is useful for inserting control characters.
]],
  true,
  function ()
    minibuf_write ("C-q-")
    insert_char (string.char (bit.band (getkey_unfiltered (GETKEY_DEFAULT), 0xff)))
    minibuf_clear ()
  end
)

Defun ("edit-wrap-paragraph",
       {},
[[
Fill paragraph at or after point.
]],
  true,
  function ()
    local m = point_marker ()

    undo_start_sequence ()

    execute_function ("move-next-paragraph")
    if is_empty_line () then
      previous_line ()
    end
    local m_end = point_marker ()

    execute_function ("move-previous-paragraph")
    if is_empty_line () then -- Move to next line if between two paragraphs.
      next_line ()
    end

    while buffer_end_of_line (cur_bp, get_buffer_pt (cur_bp)) < m_end.o do
      execute_function ("move-end-line")
      delete_char ()
      execute_function ("delete-horizontal-space")
      insert_char (' ')
    end
    unchain_marker (m_end)

    execute_function ("move-end-line")
    while get_goalc () > get_variable_number ("fill-column") + 1 and fill_break_line () do end

    goto_offset (m.o)
    unchain_marker (m)

    undo_end_sequence ()
  end
)

local function pipe_command (cmd, tempfile)
  local cmdline = string.format ("%s 2>&1%s", cmd, tempfile and " <" .. tempfile or "")
  local pipe = io.popen (cmdline, "r")
  if not pipe then
    return minibuf_error ("Cannot open pipe to process")
  end

  local out = pipe:read ("*a")
  pipe:close ()

  if #out == 0 then
    minibuf_write ("(Shell command succeeded with no output)")
  elseif not warn_if_readonly_buffer () then
    local del = 0
    if cur_bp.mark_active then
      local r = calculate_the_region ()
      goto_offset (r.start)
      del = get_region_size (r)
    end
    replace_astr (del, AStr (out))
  end

  return true
end

local function minibuf_read_shell_command ()
  local ms = minibuf_read ("Shell command: ", "")

  if not ms then
    execute_function ("keyboard-quit")
    return
  end
  if ms == "" then
    return
  end

  return ms
end

-- The `start' and `end' arguments are fake, hence their string type,
-- so they can be ignored.
Defun ("edit-shell-command",
       {"string", "string", "string"},
[[
Execute string command in inferior shell with region as input.
The output is inserted in the buffer, replacing the region if any.
Return the exit code of command.
]],
  true,
  function (start, finish, cmd)
    local ok = true

    if not cmd then
      cmd = minibuf_read_shell_command ()
    end

    if cmd then
      local rp = calculate_the_region ()
      local fd

      local tempfile
      if rp then
        tempfile = os.tmpname ()

        fd = io.open (tempfile, "w")
        if not fd then
          ok = minibuf_error ("Cannot open temporary file")
        else
          local written, err = fd:write (tostring (get_region ()))
          if not written then
            ok = minibuf_error ("Error writing to temporary file: " .. err)
          end
        end
      end

      if ok then
        ok = pipe_command (cmd, tempfile)
      end
      if fd then
        fd:close ()
      end
      if rp then
        os.remove (tempfile)
      end
    end
    return ok
  end
)

local function move_paragraph (uniarg, forward, backward, line_extremum)
  if uniarg < 0 then
    uniarg = -uniarg
    forward = backward
  end

  for i = uniarg, 1, -1 do
    repeat until not is_empty_line () or not forward ()
    repeat until is_empty_line () or not forward ()
  end

  if is_empty_line () then
    execute_function ("move-start-line")
  else
    execute_function (line_extremum)
  end
  return true
end

Defun ("move-previous-paragraph",
       {"number"},
[[
Move backward to start of paragraph.  With argument N, do it N times.
]],
  true,
  function (n)
    return move_paragraph (n or 1, previous_line, next_line, "move-start-line")
  end
)

Defun ("move-next-paragraph",
       {"number"},
[[
Move forward to end of paragraph.  With argument N, do it N times.
]],
  true,
  function (n)
    return move_paragraph (n or 1, next_line, previous_line, "move-end-line")
  end
)

Defun ("back-to-indentation",
       {},
[[
Move point to the first non-whitespace character on this line.
]],
  true,
  function ()
    goto_offset (get_buffer_line_o (cur_bp))
    while not eolp () and following_char ():match ("%s") do
      move_char (1)
    end
  end
)


-- Move through words
local function iswordchar (c)
  return c and (c:match ("[%w$]"))
end

local function move_word (dir)
  local gotword = false
  repeat
    while not (dir > 0 and eolp or bolp) () do
      if iswordchar (get_buffer_char (cur_bp, get_buffer_pt (cur_bp) - (dir < 0 and 1 or 0))) then
        gotword = true
      elseif gotword then
        break
      end
      move_char (dir)
    end
  until gotword or not move_char (dir)
  return gotword
end

Defun ("move-next-word",
       {"number"},
[[
Move point forward one word (backward if the argument is negative).
With argument, do this that many times.
]],
  true,
  function (n)
    return move_with_uniarg (n or 1, move_word)
  end
)

Defun ("move-previous-word",
       {"number"},
[[
Move backward until encountering the end of a word (forward if the
argument is negative).
With argument, do this that many times.
]],
  true,
  function (n)
    return move_with_uniarg (-(n or 1), move_word)
  end
)
