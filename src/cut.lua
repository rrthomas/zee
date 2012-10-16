-- Cut and paste
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

local cut_buffer_text = AStr ("")

local function copy_or_cut_the_region (cut)
  local r = calculate_the_selection ()
  if not r then
    return true
  end

  cut_buffer_text = get_buffer_region (buf, r)
  if cut then
    if buf.readonly then
      minibuf_error ("Read only text copied to cut buffer")
    else
      delete_region (r)
    end
  end
  execute_command ("edit-select-off")
end

Define ("edit-cut",
[[
Cut the selection.
The text is deleted, unless the buffer is read-only, and saved in the
paste buffer; the `edit-paste' command retrieves it.
]],
  function ()
    return copy_or_cut_the_region (true)
  end
)

Define ("edit-copy",
[[
Copy the selection to the paste buffer.
]],
  function ()
    return copy_or_cut_the_region (false)
  end
)

Define ("edit-paste",
[[
Insert the contents of the paste buffer.
]],
  function ()
    return not insert_string (cut_buffer_text)
  end
)
