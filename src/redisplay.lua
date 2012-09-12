-- Terminal independent redisplay routines
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

Defun ("move-redraw",
       {},
[[
Redraw screen.
]],
  function ()
    term_clear ()
    term_redisplay ()
    term_refresh ()
    return true
  end
)

function resize_window ()
  -- Resize window horizontally.
  cur_wp.fwidth = term_width ()
  cur_wp.ewidth = cur_wp.fwidth

  -- Resize window vertically.
  local hdelta = term_height () - 1 - cur_wp.fheight
  cur_wp.fheight = cur_wp.fheight + hdelta
  cur_wp.eheight = cur_wp.eheight + hdelta

  execute_function ("move-redraw")
end
