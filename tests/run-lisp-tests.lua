-- run-lisp-tests
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

require "posix"
require "std"

-- N.B. Tests that use execute-kbd-macro must note that keyboard input
-- is only evaluated once the script has finished running.

-- The following are defined in the environment for a build
local srcdir = os.getenv ("srcdir") or "."
local abs_srcdir = os.getenv ("abs_srcdir") or "."
local builddir = os.getenv ("builddir") or "."
local EMACSPROG = os.getenv ("EMACSPROG") or ""

local zile_pass = 0
local zile_fail = 0
local emacs_pass = 0
local emacs_fail = 0

zile_cmd = io.catfile (builddir, "src", "zile")

function run_test (test, name, editor_name, edit_file, cmd, args)
  posix.system ("cp", io.catfile (srcdir, "tests", "test.input"), edit_file)
  posix.system ("chmod", "+w", edit_file)
  local status = posix.system (cmd, unpack (args))
  if status == 0 then
    if posix.system ("diff", test .. ".output", edit_file) == 0 then
      posix.system ("rm", "-f", edit_file, edit_file .. "~")
      return true
    else
      print ("Emacs " .. name .. " failed to produce correct output")
    end
  else
    print ("Emacs " .. name .. " failed to run with error code " .. tostring (status))
  end
end

for _, name in ipairs (arg) do
  local test = name:gsub ("%.el$", "")
  name = posix.basename (test)
  local edit_file = io.catfile (builddir, "tests", name .. ".input")
  local args = {"--quick", "--batch", "--no-init-file", edit_file, "--load", io.catfile (abs_srcdir, (string.gsub (test .. ".el", "^" .. srcdir .. "/", "")))}

  posix.system ("mkdir", "-p", posix.dirname (edit_file))

  if EMACSPROG ~= "" then
    if run_test (test, name, "Emacs", edit_file, EMACSPROG, args) then
      emacs_pass = emacs_pass + 1
    else
      emacs_fail = emacs_fail + 1
      os.rename (edit_file, edit_file .. "-emacs")
      os.rename (edit_file .. "~", edit_file .. "-emacs~")
    end
  end

  if run_test (test, name, "Zile", edit_file, zile_cmd, args) then
    zile_pass = zile_pass + 1
  else
    zile_fail = zile_fail + 1
  end
end

print (string.format ("Zile: %d pass(es) and %d failure(s)", zile_pass, zile_fail))
print (string.format ("Emacs: %d pass(es) and %d failure(s)", emacs_pass, emacs_fail))

os.exit (zile_fail + emacs_fail)
