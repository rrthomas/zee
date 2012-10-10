-- Miscellaneous editing commands
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

Define ("file-suspend",
[[
Stop editor and return to superior process.
]],
  function ()
    posix.raise (posix.SIGTSTP)
  end
)

Define ("preferences-toggle-read-only",
[[
Change whether this file can be modified.
]],
  function ()
    buf.readonly = not buf.readonly
  end
)

Define ("preferences-toggle-wrap-mode",
[[
Toggle wrap mode.
In wrap mode, inserting a space or newline at a column beyond
`wrap-column' automatically breaks the line at a previous space.
Paragraphs can also be wrapped using `edit-wrap-paragraph'.
]],
  function ()
    buf.wrap = not buf.wrap
  end
)

Define ("edit-select-other-end",
[[
When selecting text, move the cursor to the other end of the selection.
]],
  function ()
    if not buf.mark then
      return minibuf_error ("No mark set in this buffer")
    end

    local tmp = get_buffer_pt (buf)
    goto_offset (buf.mark.o)
    buf.mark.o = tmp
    activate_mark ()
    thisflag.need_resync = true
  end
)

function select_on ()
  set_mark ()
  activate_mark ()
end

Define ("edit-select-on",
[[
Start selecting text.
]],
  function ()
    select_on ()
    minibuf_write ("Mark set")
  end
)

Define ("edit-select-off",
[[
Stop selecting text.
]],
  function ()
    deactivate_mark ()
  end
)

Define ("edit-select-toggle",
[[
Toggle selection mode.
]],
  function ()
    if buf.mark then
      execute_command ("edit-select-off")
    else
      execute_command ("edit-select-on")
    end
  end
)

Define ("edit-insert-quoted",
[[
Read next input character and insert it.
This is useful for inserting control characters.
]],
  function ()
    minibuf_write ("C-q-")
    insert_char (string.char (bit32.band (getkey_unfiltered (GETKEY_DEFAULT), 0xff)))
    minibuf_clear ()
  end
)

Define ("edit-wrap-paragraph",
[[
Wrap the paragraph at or after the cursor. The wrap column
is given by the variable `wrap-column'.
]],
  function ()
    local m = point_marker ()

    undo_start_sequence ()

    execute_command ("move-next-paragraph")
    if is_empty_line () then
      move_line (-1)
    end
    local m_end = point_marker ()

    execute_command ("move-previous-paragraph")
    if is_empty_line () then -- Move to next line if between two paragraphs.
      move_line (1)
    end

    while buffer_end_of_line (buf, get_buffer_pt (buf)) < m_end.o do
      execute_command ("move-end-line")
      delete_char ()
      execute_command ("edit-delete-horizontal-space")
      insert_char (' ')
    end
    unchain_marker (m_end)

    execute_command ("move-end-line")
    while get_goalc () > tonumber (get_variable ("wrap-column")) + 1 and wrap_break_line () do end

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
    if buf.mark_active then
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
    return ding ()
  end
  if ms == "" then
    return
  end

  return ms
end

Define ("edit-shell-command",
[[
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command; passes the selection as input to the
shell command.
If the shell command produces any output, it is inserted into the
file, replacing the selection if any.
]],
  function (cmd)
    local ok = true

    if not cmd then
      cmd = minibuf_read_shell_command ()
    end

    if cmd then
      local rp = calculate_the_region ()
      local h, tempfile
      if rp then
        tempfile = os.tmpname ()

        h = io.open (tempfile, "w")
        if not h then
          ok = minibuf_error ("Cannot open temporary file")
        else
          local fd = posix.fileno (h)
          activate_mark ()
          local s = get_buffer_region (buf, calculate_the_region ())
          local written, err = alien.default.write (fd, s.buf.buffer:topointer (), #s)
          if not written then
            ok = minibuf_error ("Error writing to temporary file: " .. err)
          end
        end
      end

      if ok then
        ok = pipe_command (cmd, tempfile)
      end
      if h then
        h:close ()
      end
      if rp then
        os.remove (tempfile)
      end
    end
    return ok
  end
)

local function move_paragraph (dir, line_extremum)
  repeat until not is_empty_line () or not move_line (dir)
  repeat until is_empty_line () or not move_line (dir)

  if is_empty_line () then
    execute_command ("move-start-line")
  else
    execute_command (line_extremum)
  end
end

Define ("move-previous-paragraph",
[[
Move the cursor backward to the start of the paragraph.
]],
  function ()
    return move_paragraph (-1, "move-start-line")
  end
)

Define ("move-next-paragraph",
[[
Move the cursor forward to the end of the paragraph.
]],
  function ()
    return move_paragraph (1, "move-end-line")
  end
)

Define ("move-start-line-text",
[[
Move the cursor to the first non-whitespace character on this line.
]],
  function ()
    goto_offset (get_buffer_line_o (buf))
    while not eolp () and following_char ():match ("%s") do
      move_char (1)
    end
  end
)


-- Move through words
local function move_word (dir)
  local gotword = false
  repeat
    while not (dir > 0 and eolp or bolp) () do
      if get_buffer_char (buf, get_buffer_pt (buf) - (dir < 0 and 1 or 0)):match ("%w") then
        gotword = true
      elseif gotword then
        break
      end
      move_char (dir)
    end
  until gotword or not move_char (dir)
  return gotword
end

Define ("move-next-word",
[[
Move the cursor forward one word.
]],
  function ()
    return move_word (1)
  end
)

Define ("move-previous-word",
[[
Move the cursor backwards one word.
]],
  function ()
    return move_word (-1)
  end
)
