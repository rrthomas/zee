-- Produce dotzile.sample
--
-- Copyright (c) 2012 Free Software Foundation, Inc.
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

require "std"
require "lib"

-- Load variables
vars = {}
function X (name, default_value, local_when_set, docstring)
  table.insert (vars, {name = name, val = default_value, islocal = local_when_set, doc = texi (docstring)})
end
require "tbl_vars"

local h = io.open ("src/dotzile.sample", "w")
if not h then
  error ()
end

h:write (
  [[
;;;; .]] .. os.getenv ("PACKAGE") .. [[ configuration

;; Rebind keys with:
;; (global-set-key "key" 'func)

]])

-- Don't note where the contents of this file comes from or that it's
-- auto-generated, because it's ugly in a user configuration file.

for i, v in ipairs (vars) do
  h:writelines ("; " .. v.doc:gsub ("\n", "\n; "),
                "; Default value is " .. v.val .. ".",
                "(setq " .. v.name .. " " .. v.val .. ")",
                "")
end
