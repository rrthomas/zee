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

Define ("edit-select-other-end",
[[
When selecting text, move the cursor to the other end of the selection.
]],
  function ()
    if not buf.mark then
      return minibuf_error ("No selection")
    end

    local tmp = get_buffer_pt (buf)
    goto_offset (buf.mark.o)
    buf.mark.o = tmp
    thisflag.need_resync = true
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
  until gotword or move_char (dir)
  return gotword
end

Define ("move-next-word",
[[
Move the cursor forward one word.
]],
  function ()
    return not move_word (1)
  end
)

Define ("move-previous-word",
[[
Move the cursor backwards one word.
]],
  function ()
    return not move_word (-1)
  end
)

local function delete_text (move_func)
  local pt = get_buffer_pt (buf)
  undo_start_sequence ()
  execute_command (move_func)
  delete_region (region_new (pt, get_buffer_pt (buf)))
  undo_end_sequence ()
  goto_offset (pt)
end

Define ("edit-delete-word",
[[
Delete forward up to the end of a word.
]],
  function ()
    return delete_text ("move-next-word")
  end
)

Define ("edit-delete-word-backward",
[[
Delete backward up to the end of a word.
]],
  function ()
    return delete_text ("move-previous-word")
  end
)

local function move_paragraph (dir, line_extremum)
  repeat until not is_empty_line () or move_line (dir)
  repeat until is_empty_line () or move_line (dir)

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
    move_paragraph (-1, "move-start-line")
  end
)

Define ("move-next-paragraph",
[[
Move the cursor forward to the end of the paragraph.
]],
  function ()
    move_paragraph (1, "move-end-line")
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

Define ("edit-shell-command",
[[
Reads a line of text using the minibuffer and creates an inferior shell
to execute the line as a command; passes the selection as input to the
shell command.
If the shell command produces any output, it is inserted into the
file, replacing the selection if any.
]],
  function (cmd)
    if not cmd and interactive () then
      cmd = minibuf_read ("Shell command: ", "")
    end
    if not cmd then
      return ding ()
    end
    if cmd == "" then
      return
    end

    local ok = true

    local h, tempfile
    if buf.mark then
      tempfile = os.tmpname ()

      h = io.open (tempfile, "w")
      if not h then
        ok = minibuf_error ("Cannot open temporary file")
      else
        local fd = posix.fileno (h)
        local s = get_buffer_region (buf, calculate_the_selection ())
        local written, err = alien.default.write (fd, s.buf.buffer:topointer (), #s)
        if not written then
          ok = minibuf_error ("Error writing to temporary file: " .. err)
        end
      end
    end

    if ok then
      local cmdline = string.format ("%s 2>&1%s", cmd, tempfile and " <" .. tempfile or "")
      local pipe = io.popen (cmdline, "r")
      if not pipe then
        ok = minibuf_error ("Cannot open pipe to process")
      else
        local out = pipe:read ("*a") -- FIXME: make efficient
        pipe:close ()

        if #out == 0 then
          minibuf_write ("(Shell command succeeded with no output)")
        else
          local del = 0
          if buf.mark then
            local r = calculate_the_selection ()
            goto_offset (r.start)
            del = get_region_size (r)
          end
          replace_astr (del, AStr (out))
        end
      end
    end

    -- FIXME: We have no way of knowing whether a sub-process caught a
    -- SIGWINCH, so raise one. (Add to lposix.c)
    --posix.raise (posix.SIGWINCH)

    if h then
      h:close ()
    end
    if tempfile then
      os.remove (tempfile)
    end

    return not ok
  end
)
