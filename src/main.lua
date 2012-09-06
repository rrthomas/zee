-- Program invocation, startup and shutdown
--
-- Copyright (c) 2010-2012 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
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
ZILE_VERSION_STRING = "GNU " .. PACKAGE_NAME .. " " .. VERSION

-- Runtime constants
-- The executable name
program_name = posix.basename (arg[0] or PACKAGE)

-- Zi display attributes
display = {}

-- Keyboard handling

GETKEY_DEFAULT = -1
GETKEY_DELAYED = 2000

-- Global variables
main_vars = {}
function X (name, default_value, local_when_set, docstring)
  main_vars[name] = {val = default_value, islocal = local_when_set, doc = texi (docstring)}
end
require "tbl_vars"
X = nil


-- Global flags, stored in thisflag and lastflag.
-- need_resync:    a resync is required.
-- quit:           the user has asked to quit.
-- set_uniarg:     the last command modified the universal arg variable `uniarg'.
-- uniarg_empty:   current universal arg is just C-u's with no number.
-- defining_macro: we are defining a macro.


-- Default waitkey pause in ds
WAITKEY_DEFAULT = 20

-- The current window
cur_wp = nil

-- The current buffer
cur_bp = nil

-- The global editor flags.
thisflag = {}
lastflag = {}


local ZILE_COPYRIGHT_STRING = "Copyright (C) 2012 Free Software Foundation, Inc."

local ZILE_COPYRIGHT_NOTICE = [[
GNU ]] .. PACKAGE_NAME .. [[ comes with ABSOLUTELY NO WARRANTY.
Zile is Free Software--Free as in Freedom--so you can redistribute copies
of Zile and modify it; see the file COPYING. Otherwise, a copy can be
downloaded from http://www.gnu.org/licenses/gpl.html.
]]

local splash_str = "Welcome to GNU " .. PACKAGE_NAME .. [[.

Undo changes	C-x u        Exit ]] .. PACKAGE_NAME .. [[	C-x C-c
(`C-' means use the CTRL key.  `M-' means hold the Meta (or Alt) key.
If you have no Meta key, you may type ESC followed by the character.)
Combinations like `C-x u' mean first press `C-x', then `u'.

Keys not working properly?  See file://]] .. PATH_DOCDIR .. [[/FAQ

]] .. ZILE_VERSION_STRING .. [[

]] .. ZILE_COPYRIGHT_STRING .. [[


]] .. ZILE_COPYRIGHT_NOTICE


-- Documented options table
--
-- Documentation line: "doc", "DOCSTRING"
-- Option: "opt", long name, short name ('\0' for none), argument, argument docstring, docstring)
-- Action: "act", ARGUMENT, DOCSTRING
--
-- Options which take no argument have an optional_argument, so that,
-- as in Emacs, no argument is signalled as extraneous.

local options = {
  {"doc", "Initialization options:"},
  {"doc", ""},
  {"opt", "no-init-file", 'q', "optional", "", "do not load ~/." .. PACKAGE},
  {"opt", "funcall", 'f', "required", "FUNC", "call " .. PACKAGE_NAME .. " function FUNC with no arguments"},
  {"opt", "load", 'l', "required", "FILE", "load " .. PACKAGE_NAME .. " Lua FILE using the load function"},
  {"opt", "help", '\0', "optional", "", "display this help message and exit"},
  {"opt", "version", '\0', "optional", "", "display version information and exit"},
  {"doc", ""},
  {"doc", "Action options:"},
  {"doc", ""},
  {"act", "FILE", "visit FILE"},
  {"act", "+LINE FILE", "visit FILE, then go to line LINE"},
}

-- Options table
local longopts = {}
for _, v in ipairs (options) do
  if v[1] == "opt" then
    table.insert (longopts, {v[2], v[4], string.byte (v[3])})
  end
end


local zarg = {}
local qflag = false

function process_args ()
  -- Leading `-' means process all arguments in order, treating
  -- non-options as arguments to an option with code 1
  -- Leading `:' so as to return ':' for a missing arg, not '?'
  local line = 1
  for c, longindex, optind, optarg in posix.getopt_long (arg, "-:f:l:q", longopts) do
    local this_optind = optind > 0 and optind or 1
    if c == 1 then -- Non-option (assume file name)
      longindex = 5
    elseif c == string.byte ('?') then -- Unknown option
      minibuf_error (string.format ("Unknown option `%s'", arg[this_optind]))
    elseif c == string.byte (':') then -- Missing argument
      io.stderr:write (string.format ("%s: Option `%s' requires an argument\n",
                                      program_name, arg[this_optind]))
      os.exit (1)
    elseif c == string.byte ('q') then
      longindex = 0
    elseif c == string.byte ('f') then
      longindex = 1
    elseif c == string.byte ('l') then
      longindex = 2
    end

    if longindex == 0 then
      qflag = true
    elseif longindex == 1 then
      table.insert (zarg, {'function', optarg})
    elseif longindex == 2 then
      table.insert (zarg, {'loadfile', normalize_path (optarg)})
    elseif longindex == 3 then
      io.write ("Usage: " .. arg[0] .. " [OPTION-OR-FILENAME]...\n" ..
                "\n" ..
                "Run " .. PACKAGE_NAME .. ", the lightweight Emacs clone.\n" ..
                "\n")

      for _, v in ipairs (options) do
        if v[1] == "doc" then
          io.write (v[2] .. "\n")
        elseif v[1] == "opt" then
          local shortopt = string.format (", -%s", v[3])
          local buf = string.format ("--%s%s %s", v[2], v[3] ~= '\0' and shortopt or "", v[5])
          io.write (string.format ("%-24s%s\n", buf, v[6]))
        elseif v[1] == "act" then
          io.write (string.format ("%-24s%s\n", v[2], v[3]))
        end
      end

      io.write ("\n" ..
                "Report bugs to " .. PACKAGE_BUGREPORT .. ".\n")
      os.exit (0)
    elseif longindex == 4 then
      io.write (ZILE_VERSION_STRING .. "\n" ..
                ZILE_COPYRIGHT_STRING .. "\n" ..
                "GNU " .. PACKAGE_NAME .. " comes with ABSOLUTELY NO WARRANTY.\n" ..
                "You may redistribute copies of " .. PACKAGE_NAME .. "\n" ..
                "under the terms of the GNU General Public License.\n" ..
                "For more information about these matters, see the file named COPYING.\n")
      os.exit (0)
    elseif longindex == 5 then
      if optarg[1] == '+' then
        line = tonumber (optarg, 10)
      else
        table.insert (zarg, {'file', normalize_path (optarg), line})
        line = 1
      end
    end
  end
end

local function segv_sig_handler (signo)
  io.stderr:write (program_name .. ": " .. PACKAGE_NAME ..
                   " crashed.  Please send a bug report to <" ..
                   PACKAGE_BUGREPORT .. ">.\r\n")
  zile_exit (true)
end

local function other_sig_handler (signo)
  local msg = program_name .. ": terminated with signal " .. signo .. ".\n" .. debug.traceback ()
  io.stderr:write (msg:gsub ("\n", "\r\n"))
  zile_exit (false)
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
  local scratch_bp

  signal_init ()

  process_args ()

  os.setlocale ("")

  term_init ()

  init_default_bindings ()

  -- Create the `*scratch*' buffer, so that initialisation commands
  -- that act on a buffer have something to act on.
  create_scratch_window ()
  scratch_bp = cur_bp
  insert_string (";; This buffer is for notes you don't want to save.\n;; If you want to create a file, visit that file with C-x C-f,\n;; then enter the text in that file's own buffer.\n\n")
  cur_bp.modified = false

  if not qflag then
    -- local s = os.getenv ("HOME")
    -- if s then
    --   lisp_loadfile (s .. "/." .. PACKAGE)
    -- end
  end

  -- Create the splash buffer & message only if no files, function or
  -- load file is specified on the command line, and there has been no
  -- error.
  if #zarg == 0 and not minibuf_contents and not get_variable_bool ("inhibit-splash-screen") then
    local bp = create_auto_buffer ("*GNU " .. PACKAGE_NAME .. "*")
    switch_to_buffer (bp)
    insert_string (splash_str)
    cur_bp.readonly = true
    execute_function ("beginning-of-buffer")
  end

  -- Load files and load files and run functions given on the command line.
  local ok = true
  for i = 1, #zarg do
    local type, arg, line = zarg[i][1], zarg[i][2], zarg[i][3]

    if type == "function" then
      ok = execute_function (arg)
      if ok == nil then
        minibuf_error (string.format ("Function `%s' not defined", arg))
      end
    elseif type == "loadfile" then
      local f = loadfile (arg) -- FIXME: call load function instead of duplicating code below
      ok = f ~= nil
      if not ok then
        minibuf_error (string.format ("Cannot open load file: %s\n", arg))
      else
        f ()
      end
    elseif type == "file" then
      ok = find_file (arg)
      if ok then
        execute_function ("goto-line", line)
        lastflag.need_resync = true
      end
    end
    if thisflag.quit then
      break
    end
  end

  lastflag.need_resync = true

  -- Set up screen according to number of files loaded.
  if #buffers == 3 then
    -- *scratch* and two files.
    execute_function ("split-window")
    switch_to_buffer (buffers[#buffers -1])
    execute_function ("other-window")
  elseif #buffers > 3 then
    execute_function ("list-buffers")
  end

  -- Reinitialise the scratch buffer to catch settings
  init_buffer (scratch_bp)

  -- Refresh minibuffer in case there was an error that couldn't be
  -- written during startup
  minibuf_refresh ()

  -- Run the main loop.
  while not thisflag.quit do
    if lastflag.need_resync then
      window_resync (cur_wp)
    end
    get_and_run_command ()
  end

  -- Tidy and close the terminal.
  term_finish ()
end

Defun ("debug",
       {},
[[
Enter the debugger.  Zile's debugger is not like Emacs's, principally
in that it is a Lua debugger (the language in which Zile is written).
]],
  true,
  function ()
    term_close ()
    ldb ()
    term_reopen ()
  end
)
