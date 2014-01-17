-- Run shell commands
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
      cmd = minibuf_read ("Shell command: ")
    end
    if cmd == "" then
      return ding ()
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
        local out = pipe:read ("*a")
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
          replace_string (del, out)
        end
      end
    end

    -- A sub-process may have caught a SIGWINCH, so assume one did.
    curses.ungetch (curses.KEY_RESIZE)

    if h then
      h:close ()
    end
    if tempfile then
      os.remove (tempfile)
    end

    return not ok
  end
)
