-- Program invocation, startup and shutdown
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

-- Derived constants
VERSION_STRING = PACKAGE_NAME .. " " .. VERSION

local COPYRIGHT_STRING = "Copyright (C) 2012 Free Software Foundation, Inc."

prog = {
  name = posix.basename (arg[0] or PACKAGE),
  banner = VERSION_STRING .. " by Reuben Thomas <rrt@sc3d.org>",
  purpose = "An editor.",
  notes = COPYRIGHT_STRING .. "\n" ..
    PACKAGE_NAME .. " comes with ABSOLUTELY NO WARRANTY.\n" ..
    "You may redistribute copies of " .. PACKAGE_NAME .. "\n" ..
    "under the terms of the GNU General Public License.\n" ..
    "For more information about these matters, see the file named COPYING.\n" ..
    "Report bugs to " .. PACKAGE_BUGREPORT .. ".",
}


-- Runtime constants

-- Display attributes
display = {}

-- Keyboard handling

GETKEY_DEFAULT = -1
GETKEY_DELAYED = 2000


-- Global flags, stored in thisflag and lastflag.
-- need_resync:    a resync is required.
-- quit:           the user has asked to quit.
-- defining_macro: we are defining a macro.


-- The current window
win = nil

-- The current buffer
buf = nil

-- The global editor flags.
thisflag = {}
lastflag = {}


options = {
  Option {{"no-init-file", 'q'}, "do not load ~/." .. PACKAGE},
  Option {{"eval", 'e'}, "evaluate Lua chunk CHUNK", "Req", "CHUNK"},
  Option {{"line", 'n'}, "start editing at line LINE", "Req", "LINE"},
}

local function segv_sig_handler (signo)
  io.stderr:write (prog.name .. ": " .. PACKAGE_NAME ..
                   " crashed.  Please send a bug report to <" ..
                   PACKAGE_BUGREPORT .. ">.\r\n")
  editor_exit (true)
end

local function other_sig_handler (signo)
  local msg = prog.name .. ": terminated with signal " .. signo .. ".\n" .. debug.traceback ()
  io.stderr:write (msg:gsub ("\n", "\r\n"))
  editor_exit (false)
end

local function signal_init ()
  -- Set up signal handling
  posix.signal(posix.SIGSEGV, segv_sig_handler)
  posix.signal(posix.SIGBUS, segv_sig_handler)
  posix.signal(posix.SIGHUP, other_sig_handler)
  posix.signal(posix.SIGINT, other_sig_handler)
  posix.signal(posix.SIGTERM, other_sig_handler)
end

function main ()
  signal_init ()
  getopt.processArgs ()

  if #arg ~= 1 and not (#arg == 0 and getopt.opt.eval) then
    getopt.usage ()
    os.exit (1)
  end

  os.setlocale ("")
  term_init ()
  init_default_bindings ()
  create_window ()

  if not getopt.opt["no-init-file"] then
    local s = os.getenv ("HOME")
    if s then
      execute_command ("load", s .. "/." .. PACKAGE)
    end
  end

  -- Load file
  local ok = true
  if #arg == 1 then
    ok = find_file (normalize_path (arg[1]))
    if ok then
      execute_command ("edit-goto-line", getopt.opt.line and getopt.opt.line[#getopt.opt.line] or 1)

      -- Evaluate Lua chunks given on the command line.
      for _, c in ipairs (getopt.opt.eval or {}) do
        if not execute_command ("eval", c) then
          break
        end
        if thisflag.quit then
          break
        end
      end

      lastflag.need_resync = true

      -- Refresh minibuffer in case there's a pending error message.
      minibuf_refresh ()

      -- Leave cursor in correct position.
      term_redraw_cursor ()

      -- Run the main loop.
      while not thisflag.quit do
        if lastflag.need_resync then
          window_resync (win)
        end
        get_and_run_command ()
      end
    end
  end

  -- Tidy and close the terminal.
  term_finish ()

  -- Print any error message.
  if not ok then
    io.stderr:write (minibuf_contents .. "\n")
  end
end
