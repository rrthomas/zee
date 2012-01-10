-- Miscellaneous Emacs functions
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

Defun ("keyboard-quit",
       {},
[[
Cancel current command.
]],
  true,
  function ()
    deactivate_mark ()
    minibuf_error ("Quit")
    return false
  end
)

Defun ("suspend-emacs",
       {},
[[
Stop Zile and return to superior process.
]],
  true,
  function ()
    posix.raise (posix.SIGTSTP)
  end
)

Defun ("toggle-read-only",
       {},
[[
Change whether this buffer is visiting its file read-only.
]],
  true,
  function ()
    cur_bp.readonly = not cur_bp.readonly
  end
)

Defun ("auto-fill-mode",
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

Defun ("exchange-point-and-mark",
       {},
[[
Put the mark where point is now, and point where the mark is now.
]],
  true,
  function ()
    if not cur_bp.mark then
      minibuf_error ("No mark set in this buffer")
      return false
    end

    local tmp = cur_bp.o
    goto_offset (cur_bp.mark.o)
    cur_bp.mark.o = tmp
    activate_mark ()
    thisflag.need_resync = true
  end
)

function write_temp_buffer (name, show, func, ...)
  local old_wp = cur_wp
  local old_bp = cur_bp

  -- Popup a window with the buffer "name".
  local wp = find_window (name)
  if show and wp then
    set_current_window (wp)
  else
    local bp = find_buffer (name)
    if show then
      set_current_window (popup_window ())
    end
    if bp == nil then
      bp = buffer_new ()
      bp.name = name
    end
    switch_to_buffer (bp)
  end

  -- Remove the contents of that buffer.
  local new_bp = buffer_new ()
  new_bp.name = cur_bp.name
  kill_buffer (cur_bp)
  cur_bp = new_bp
  cur_wp.bp = cur_bp

  -- Make the buffer a temporary one.
  cur_bp.needname = true
  cur_bp.noundo = true
  cur_bp.nosave = true
  set_temporary_buffer (cur_bp)

  -- Use the "callback" routine.
  func (...)

  execute_function ("beginning-of-buffer")
  cur_bp.readonly = true
  cur_bp.modified = false

  -- Restore old current window.
  set_current_window (old_wp)

  -- If we're not showing the new buffer, switch back to the old one.
  if not show then
    switch_to_buffer (old_bp)
  end
end

Defun ("universal-argument",
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
      if key == KBD_CANCEL then
        ok = execute_function ("keyboard-quit")
        break
      -- Digit pressed.
      elseif string.char (bit.band (key, 0xff)):match ("%d") then
        local digit = bit.band (key, 0xff) - string.byte ('0')
        thisflag.uniarg_empty = false

        if bit.band (key, KBD_META) ~= 0 then
          as = "ESC "
        end

        as = as .. string.format ("%d", digit)

        if i == 0 then
          arg = digit
        else
          arg = arg * 10 + digit
        end

        i = i + 1
      elseif key == bit.bor (KBD_CTRL, string.byte ('u')) then
        as = as .. "C-u"
        if i == 0 then
          arg = arg * 4
        else
          break
        end
      elseif key == bit.bor (KBD_META, string.byte ('-')) and i == 0 then
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

local function write_buffers_list (old_wp)
  -- FIXME: Underline next line properly.
  insert_string ("CRM Buffer                Size  Mode             File\n")
  insert_string ("--- ------                ----  ----             ----\n")

  -- Rotate buffer list to get current buffer at head.
  local bufs = table.clone (buffers)
  for i = #buffers, 1, -1 do
    if buffers[i] == old_wp.bp then
      break
    end
    table.insert (bufs, 1, table.remove (bufs))
  end

  -- Print buffers.
  for _, bp in ripairs (bufs) do
    -- Print all buffers whose names don't start with space except
    -- this one (the *Buffer List*).
    if cur_bp ~= bp and bp.name[1] ~= ' ' then
      insert_string (string.format ("%s%s%s %-19s %6u  %-17s",
                                    old_wp.bp == bp and '.' or ' ',
                                    bp.readonly and '%' or ' ',
                                    bp.modified and '*' or ' ',
                                    bp.name, get_buffer_size (bp), "Fundamental"))
      if bp.filename then
        insert_string (compact_path (bp.filename))
      end
      insert_newline ()
    end
  end
end

Defun ("list-buffers",
       {},
[[
Display a list of names of existing buffers.
The list is displayed in a buffer named `*Buffer List*'.

The C column has a `.' for the buffer from which you came.
The R column has a `%' if the buffer is read-only.
The M column has a `*' if it is modified.
After this come the buffer name, its size in characters,
its major mode, and the visited file name (if any).
]],
  true,
  function ()
    write_temp_buffer ("*Buffer List*", true, write_buffers_list, cur_wp)
  end
)

Defun ("set-mark",
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
    execute_function ("set-mark")
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
      local o = get_buffer_o (cur_bp) - get_buffer_line_o (cur_bp)
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
      minibuf_error ("set-fill-column requires an explicit argument")
      return false
    end

    minibuf_write (string.format ("Fill column set to %d (was %d)", n, get_variable_number ("fill-column")))
    set_variable ("fill-column", tostring (n))
    return true
  end
)

Defun ("quoted-insert",
       {},
[[
Read next input character and insert it.
This is useful for inserting control characters.
]],
  true,
  function ()
    minibuf_write ("C-q-")
    local c = getkey_unfiltered (GETKEY_DEFAULT)
    insert_char (string.char (c))
    minibuf_clear ()
  end
)

Defun ("fill-paragraph",
       {},
[[
Fill paragraph at or after point.
]],
  true,
  function ()
    local m = point_marker ()

    undo_start_sequence ()

    execute_function ("forward-paragraph")
    if is_empty_line () then
      previous_line ()
    end
    local m_end = point_marker ()

    execute_function ("backward-paragraph")
    if is_empty_line () then -- Move to next line if between two paragraphs.
      next_line ()
    end

    while buffer_end_of_line (cur_bp, get_buffer_o (cur_bp)) < m_end.o do
      execute_function ("end-of-line")
      delete_char ()
      execute_function ("just-one-space")
    end
    unchain_marker (m_end)

    execute_function ("end-of-line")
    while get_goalc () > get_variable_number ("fill-column") + 1 and fill_break_line () do end

    goto_offset (m.o)
    unchain_marker (m)

    undo_end_sequence ()
  end
)

local function pipe_command (cmd, tempfile, insert, do_replace)
  local cmdline = string.format ("%s 2>&1 <%s", cmd, tempfile)
  local pipe = io.popen (cmdline, "r")
  if not pipe then
    minibuf_error ("Cannot open pipe to process")
    return false
  end

  local out = pipe:read ("*a")
  pipe:close ()
  local eol = string.find (out, "\n")

  if #out == 0 then
    minibuf_write ("(Shell command succeeded with no output)")
  else
    if insert then
      local del = 0
      if do_replace and not warn_if_no_mark () then
        local r = calculate_the_region ()
        goto_offset (r.start)
        del = get_region_size (r)
      end
      replace (del, out)
    else
      local more_than_one_line = eol and eol ~= #out
      write_temp_buffer ("*Shell Command Output*", more_than_one_line, insert_string, out)
      if not more_than_one_line then
        minibuf_write (out)
      end
    end
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

Defun ("shell-command",
       {"string", "boolean"},
[[
Execute string @i{command} in inferior shell; display output, if any.
With prefix argument, insert the command's output at point.

Command is executed synchronously.  The output appears in the buffer
`*Shell Command Output*'.  If the output is short enough to display
in the echo area, it is shown there, but it is nonetheless available
in buffer `*Shell Command Output*' even though that buffer is not
automatically displayed.

The optional second argument @i{output-buffer}, if non-nil,
says to insert the output in the current buffer.
]],
  true,
  function (cmd, insert)
    if not cmd then
      cmd = minibuf_read_shell_command ()
    end
    if not insert then
      insert = lastflag.set_uniarg
    end

    if cmd then
      return pipe_command (cmd, "/dev/null", insert, false)
    end
    return true
  end
)

-- The `start' and `end' arguments are fake, hence their string type,
-- so they can be ignored.
Defun ("shell-command-on-region",
       {"string", "string", "string", "boolean"},
[[
Execute string command in inferior shell with region as input.
Normally display output (if any) in temp buffer `*Shell Command Output*'
Prefix arg means replace the region with it.  Return the exit code of
command.

If the command generates output, the output may be displayed
in the echo area or in a buffer.
If the output is short enough to display in the echo area, it is shown
there.  Otherwise it is displayed in the buffer `*Shell Command Output*'.
The output is available in that buffer in both cases.
]],
  true,
  function (start, finish, cmd, insert)
    local ok = true

    if not cmd then
      cmd = minibuf_read_shell_command ()
    end
    if not insert then
      insert = lastflag.set_uniarg
    end

    if cmd then
      local rp = calculate_the_region ()

      if not rp then
        ok = false
      else
        local tempfile = os.tmpname ()
        local fd = io.open (tempfile, "w")

        if not fd then
          minibuf_error ("Cannot open temporary file")
          ok = false
        else
          local written, err = fd:write (get_buffer_region (cur_bp, rp).s)

          if not written then
            minibuf_error ("Error writing to temporary file: " .. err)
            ok = false
          else
            ok = pipe_command (cmd, tempfile, insert, true)
          end

          fd:close ()
          os.remove (tempfile)
        end
      end
    end
    return ok
  end
)

Defun ("delete-region",
       {},
[[
Delete the text between point and mark.
]],
  true,
  function ()
    local rp = calculate_the_region ()

    if not rp or not delete_region (rp) then
      return false
    end
    deactivate_mark ()
    return true
  end
)

Defun ("delete-blank-lines",
       {},
[[
On blank line, delete all surrounding blank lines, leaving just one.
On isolated blank line, delete that one.
On nonblank line, delete any immediately following blank lines.
]],
  true,
  function ()
    local m = point_marker ()
    local r = region_new (get_buffer_line_o (cur_bp), get_buffer_line_o (cur_bp))

    undo_start_sequence ()

    -- Find following blank lines.
    if execute_function ("forward-line") and is_blank_line () then
      r.start = get_buffer_o (cur_bp)
      repeat
        r.finish = buffer_next_line (cur_bp, get_buffer_o (cur_bp))
      until not execute_function ("forward-line") or not is_blank_line ()
    end
    goto_offset (m.o)

    -- If this line is blank, find any preceding blank lines.
    local singleblank = true
    if is_blank_line () then
      r.finish = math.max (r.finish, buffer_next_line (cur_bp, get_buffer_o (cur_bp) or math.huge))
      repeat
        r.start = get_buffer_line_o (cur_bp)
      until not execute_function ("forward-line", -1) or not is_blank_line ()

      goto_offset (m.o)
      if r.start ~= get_buffer_line_o (cur_bp) or r.finish > buffer_next_line (cur_bp, get_buffer_o (cur_bp)) then
        singleblank = false
      end
      r.finish = math.min (r.finish, get_buffer_size (cur_bp))
    end

    -- If we are deleting to EOB, need to fudge extra line.
    local at_eob = r.finish == get_buffer_size (cur_bp) and r.start > 0
    if at_eob then
      r.start = r.start - #get_buffer_eol (cur_bp)
    end

    -- Delete any blank lines found.
    if r.start < r.finish then
      buffer_replace (cur_bp, r.start, get_region_size (r), "")
    end

    -- If we found more than one blank line, leave one.
    if not singleblank then
      if not at_eob then
        intercalate_newline ()
      else
        insert_newline ()
      end
    end

    undo_end_sequence ()

    unchain_marker (m)
    deactivate_mark ()
  end
)

Defun ("forward-line",
       {"number"},
[[
Move N lines forward (backward if N is negative).
Precisely, if point is on line I, move to the start of line I + N.
]],
  true,
  function (n)
    n = n or current_prefix_arg or 1
    if n ~= 0 then
      execute_function ("beginning-of-line")
      return move_line (n)
    end
    return false
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
    execute_function ("beginning-of-line")
  else
    execute_function (line_extremum)
  end
  return true
end

Defun ("backward-paragraph",
       {"number"},
[[
Move backward to start of paragraph.  With argument N, do it N times.
]],
  true,
  function (n)
    return move_paragraph (n or 1, previous_line, next_line, "beginning-of-line")
  end
)

Defun ("forward-paragraph",
       {"number"},
[[
Move forward to end of paragraph.  With argument N, do it N times.
]],
  true,
  function (n)
    return move_paragraph (n or 1, next_line, previous_line, "end-of-line")
  end
)


-- Move through balanced expressions (sexps)
local function move_sexp (dir)
  local gotsexp, level = false, 0
  local gotsexp, single_quote, double_quote = false, dir < 0, dir < 0

  local function isopenbracketchar (c)
    return (c == '(') or (c == '[') or ( c== '{') or ((c == '\"') and not double_quote) or ((c == '\'') and not single_quote)
  end

  local function isclosebracketchar (c)
    return (c == ')') or (c == ']') or (c == '}') or ((c == '\"') and double_quote) or ((c == '\'') and single_quote)
  end

  while true do
    while not (dir > 0 and eolp or bolp) () do
      local o = get_buffer_o (cur_bp) - (dir < 0 and 1 or 0)
      local c = get_buffer_char (cur_bp, o)

      -- Skip escaped quotes.
      if (c == '"' or c == '\'') and o > get_buffer_line_o (cur_bp) and get_buffer_char (cur_bp, o - 1) == '\\' then
        move_char (dir)
        c = 'a' -- Treat ' and " like word chars.
      end

      if (dir > 0 and isopenbracketchar or isclosebracketchar) (c) then
        if level == 0 and gotsexp then
          return true
        end

        level = level + 1
        gotsexp = true
        if c == '\"' then
          double_quote = not double_quote
        end
        if c == '\'' then
          single_quote = not single_quote
        end
      elseif (dir > 0 and isclosebracketchar or isopenbracketchar) (c) then
        if level == 0 and gotsexp then
          return true
        end

        level = level - 1
        gotsexp = true
        if c == '\"' then
          double_quote = not double_quote
        end
        if c == '\'' then
          single_quote = not single_quote
        end
        if level < 0 then
          minibuf_error ("Scan error: \"Containing expression ends prematurely\"")
          return false
        end
      end

      move_char (dir)

      if not c:match ("[%a_$]") then
        if gotsexp and level == 0 then
          if not (isopenbracketchar (c) or isclosebracketchar (c)) then
            move_char (-dir)
          end
          return true
        end
      else
        gotsexp = true
      end
    end
    if gotsexp and level == 0 then
      return true
    end
    if not (dir > 0 and next_line or previous_line) () then
      if level ~= 0 then
        minibuf_error ("Scan error: \"Unbalanced parentheses\"")
      end
      return false
    end
    if dir > 0 then
      execute_function ("beginning-of-line")
    else
      execute_function ("end-of-line")
    end
  end
  return false
end

Defun ("forward-sexp",
       {"number"},
[[
Move forward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move backward across N balanced expressions.
]],
  true,
  function (n)
    return move_with_uniarg (n or 1, move_sexp)
  end
)

Defun ("backward-sexp",
       {"number"},
[[
Move backward across one balanced expression (sexp).
With argument, do it that many times.  Negative arg -N means
move forward across N balanced expressions.
]],
  true,
  function (n)
    return move_with_uniarg (-(n or 1), move_sexp)
  end
)

local function mark (uniarg, func)
  execute_function ("set-mark")
  local ret = execute_function (func, uniarg)
  if ret then
    execute_function ("exchange-point-and-mark")
  end
  return ret
end

Defun ("mark-word",
       {"number"},
[[
Set mark argument words away from point.
]],
  true,
  function (n)
    return mark (n, "forward-word")
  end
)

Defun ("mark-sexp",
       {"number"},
[[
Set mark @i{arg} sexps from point.
The place mark goes is the same place @kbd{C-M-f} would
move to with the same argument.
]],
  true,
  function (n)
    return mark (n, "forward-sexp")
  end
)

Defun ("mark-paragraph",
       {},
[[
Put point at beginning of this paragraph, mark at end.
The paragraph marked is the one that contains point or follows point.
]],
  true,
  function ()
    if _last_command == "mark-paragraph" then
      execute_function ("exchange-point-and-mark")
      execute_function ("forward-paragraph")
      execute_function ("exchange-point-and-mark")
    else
      execute_function ("forward-paragraph")
      execute_function ("set-mark")
      execute_function ("backward-paragraph")
    end
  end
)

Defun ("mark-whole-buffer",
       {},
[[
Put point at beginning and mark at end of buffer.
]],
  true,
  function ()
    execute_function ("end-of-buffer")
    execute_function ("set-mark-command")
    execute_function ("beginning-of-buffer")
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
      if iswordchar (get_buffer_char (cur_bp, get_buffer_o (cur_bp) - (dir < 0 and 1 or 0))) then
        gotword = true
      elseif gotword then
        break
      end
      move_char (dir)
    end
  until gotword or not move_char (dir)
  return gotword
end

Defun ("forward-word",
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

Defun ("backward-word",
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

local function setcase_word (rcase)
  if not iswordchar (following_char ()) then
    if not move_word (1) or not move_word (-1) then
      return false
    end
  end

  local as = ""
  for i = get_buffer_o (cur_bp) - get_buffer_line_o (cur_bp), buffer_line_len (cur_bp) do
    local c = get_buffer_char (cur_bp, get_buffer_line_o (cur_bp) + i)
    if iswordchar (c) then
      as = as .. c
    else
      break
    end
  end

  if #as > 0 then
    replace (#as, recase (as, rcase))
  end

  cur_bp.modified = true

  return true
end

Defun ("downcase-word",
       {"number"},
[[
Convert following word (or @i{arg} words) to lower case, moving over.
]],
  true,
  function (arg)
    return execute_with_uniarg (true, arg, function () return setcase_word ("lower") end)
  end
)

Defun ("upcase-word",
       {"number"},
[[
Convert following word (or @i{arg} words) to upper case, moving over.
]],
  true,
  function (arg)
    return execute_with_uniarg (true, arg, function () return setcase_word ("upper") end)
  end
)

Defun ("capitalize-word",
       {"number"},
[[
Capitalize the following word (or @i{arg} words), moving over.
This gives the word(s) a first character in upper case
and the rest lower case.
]],
  true,
  function (arg)
    return execute_with_uniarg (true, arg, function () return setcase_word ("capitalized") end)
  end
)

-- Set the region case.
local function setcase_region (func)
  local rp = calculate_the_region ()

  if warn_if_readonly_buffer () or not rp then
    return false
  end

  undo_start_sequence ()

  local m = point_marker ()
  goto_offset (rp.start)
  for _ = get_region_size (rp), 1, -1 do
    local c = func (following_char ())
    delete_char ()
    insert_char (c)
  end
  goto_offset (m.o)
  unchain_marker (m)

  cur_bp.modified = true
  undo_end_sequence ()

  return true
end

Defun ("upcase-region",
       {},
[[
Convert the region to upper case.
]],
  true,
  function ()
    return setcase_region (string.upper)
  end
)

Defun ("downcase-region",
       {},
[[
Convert the region to lower case.
]],
  true,
  function ()
    return setcase_region (string.lower)
  end
)


-- Transpose functions
local function region_to_string ()
  activate_mark ()
  return get_buffer_region (cur_bp, calculate_the_region ()).s
end

local function transpose_subr (move_func)
  -- For transpose-chars.
  if move_func == move_char and eolp () then
    move_func (-1)
  end
  -- For transpose-lines.
  if move_func == move_line and get_buffer_line_o (cur_bp) == 0 then
    move_func (1)
  end

  -- Backward.
  if not move_func (-1) then
    minibuf_error ("Beginning of buffer")
    return false
  end

  -- Mark the beginning of first string.
  push_mark ()
  local m1 = point_marker ()

  -- Check to make sure we can go forwards twice.
  if not move_func (1) or not move_func (1) then
    if move_func == move_line then
      -- Add an empty line.
      execute_function ("end-of-line")
      execute_function ("newline")
    else
      pop_mark ()
      goto_offset (m1.o)
      minibuf_error ("End of buffer")

      unchain_marker (m1)
      return false
    end
  end

  goto_offset (m1.o)

  -- Forward.
  move_func (1)

  -- Save and delete 1st marked region.
  local as1 = region_to_string ()

  execute_function ("delete-region")

  -- Forward.
  move_func (1)

  -- For transpose-lines.
  local m2, as2
  if move_func == move_line then
    m2 = point_marker ()
  else
    -- Mark the end of second string.
    set_mark ()

    -- Backward.
    move_func (-1)
    m2 = point_marker ()

    -- Save and delete 2nd marked region.
    as2 = region_to_string ()
    execute_function ("delete-region")
  end

  -- Insert the first string.
  goto_offset (m2.o)
  unchain_marker (m2)
  insert_string (as1)

  -- Insert the second string.
  if as2 then
    goto_offset (m1.o)
    insert_string (as2)
  end
  unchain_marker (m1)

  -- Restore mark.
  pop_mark ()
  deactivate_mark ()

  -- Move forward if necessary.
  if move_func ~= move_line then
    move_func (1)
  end

  return true
end

local function transpose (uniarg, move)
  if warn_if_readonly_buffer () then
    return false
  end

  local ret = true
  undo_start_sequence ()
  for uni = 1, uniarg do
    ret = transpose_subr (move)
    if not ret then
      break
    end
  end
  undo_end_sequence ()

  return ret
end

Defun ("transpose-chars",
       {"number"},
[[
Interchange characters around point, moving forward one character.
With prefix arg ARG, effect is to take character before point
and drag it forward past ARG other characters (backward if ARG negative).
If no argument and at end of line, the previous two chars are exchanged.
]],
  true,
  function (n)
    return transpose (n or 1, move_char)
  end
)

Defun ("transpose-words",
       {"number"},
[[
Interchange words around point, leaving point at end of them.
With prefix arg ARG, effect is to take word before or around point
and drag it forward past ARG other words (backward if ARG negative).
If ARG is zero, the words around or after point and around or after mark
are interchanged.
]],
  true,
  function (n)
    return transpose (n or 1, move_word)
  end
)

Defun ("transpose-sexps",
       {"number"},
[[
Like @kbd{M-x transpose-words} but applies to sexps.
]],
  true,
  function (n)
    return transpose (n or 1, move_sexp)
  end
)

Defun ("transpose-lines",
       {"number"},
[[
Exchange current line and previous line, leaving point after both.
With argument ARG, takes previous line and moves it past ARG lines.
With argument 0, interchanges line point is in with line mark is in.
]],
  true,
  function (n)
    return transpose (n or 1, move_line)
  end
)
