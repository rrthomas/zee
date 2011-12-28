-- Buffer-oriented functions
--
-- Copyright (c) 2010-2011 Free Software Foundation, Inc.
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

-- Create an empty list, returning a pointer to the list
function line_new ()
  return {}
end

-- Insert a line into list after the given point, returning the new line
function line_insert (l, s)
  local n = {next = l.next, prev = l, text = s}
  if l.next then
    l.next.prev = n
  end
  l.next = n

  return n
end

-- Adjust markers (including point) when line at point is split, or
-- next line is joined on, or where a line is edited.
--   newlp is the line to which characters were moved, oldlp the line
--     moved from (if dir == 0, newlp == oldlp)
--   pointo is point at which oldlp was split (to make newlp) or
--     joined to newlp
--   dir is 1 for split, -1 for join or 0 for line edit (newlp == oldlp)
--   if dir == 0, delta gives the number of characters inserted (>0) or
--     deleted (<0)
local function adjust_markers (newlp, oldlp, pointo, dir, delta)
  local m_pt = point_marker ()

  assert (dir == -1 or dir == 0 or dir == 1)

  for m in pairs (cur_bp.markers) do
    if m.pt.p == oldlp and (dir == -1 or m.pt.o > pointo) then
      m.pt.p = newlp
      m.pt.o = m.pt.o + delta - (pointo * dir)
      m.pt.n = m.pt.n + dir
    elseif m.pt.n > cur_bp.pt.n then
      m.pt.n = m.pt.n + dir
    end
  end

  -- This marker has been updated to new position.
  goto_point (m_pt.pt)
  unchain_marker (m_pt)
end

-- Check the case of a string.
-- Returns "uppercase" if it is all upper case, "capitalized" if just
-- the first letter is, and nil otherwise.
local function check_case (s)
  if string.match (s, "^%u+$") then
    return "uppercase"
  elseif string.match (s, "^%u%U*") then
    return "capitalized"
  end
end

-- Insert the character at the current position and move the text at its right.
-- This function doesn't change the current position of the pointer.
local function intercalate_char (c)
  if warn_if_readonly_buffer () then
    return false
  end

  local as = cur_bp.pt.p.text
  undo_save (UNDO_REPLACE_BLOCK, cur_bp.pt, 0, 1)
  as = string.sub (as, 1, cur_bp.pt.o) .. c .. string.sub (as, cur_bp.pt.o + 1)
  cur_bp.pt.p.text = as
  cur_bp.modified = true

  return true
end

-- Insert the character `c' at the current point position
-- into the current buffer.
function insert_char (c)
  local t = tab_width (cur_bp)

  if warn_if_readonly_buffer () then
    return false
  end

  intercalate_char (c)
  forward_char ()
  adjust_markers (cur_bp.pt.p, cur_bp.pt.p, cur_bp.pt.o, 0, 1)

  return true
end

-- Insert a newline at the current position without moving the cursor.
-- Update markers after point in the split line.
function intercalate_newline ()
  if warn_if_readonly_buffer () then
    return false
  end

  undo_save (UNDO_REPLACE_BLOCK, cur_bp.pt, 0, 1)

  -- Move the text after the point into a new line.
  line_insert (cur_bp.pt.p, string.sub (cur_bp.pt.p.text, cur_bp.pt.o + 1))
  cur_bp.last_line = cur_bp.last_line + 1
  cur_bp.pt.p.text = string.sub (cur_bp.pt.p.text, 1, cur_bp.pt.o)
  adjust_markers (cur_bp.pt.p.next, cur_bp.pt.p, cur_bp.pt.o, 1, 0)

  cur_bp.modified = true
  thisflag.need_resync = true

  return true
end

function delete_char ()
  deactivate_mark ()

  if eobp () then
    minibuf_error ("End of buffer")
    return false
  end

  if warn_if_readonly_buffer () then
    return false
  end

  undo_save (UNDO_REPLACE_BLOCK, cur_bp.pt, 1, 0)

  if eolp () then
    local oldlen = #cur_bp.pt.p.text
    local oldlp = cur_bp.pt.p.next

    -- Join the lines.
    local as = cur_bp.pt.p.text
    local bs = oldlp.text
    as = as .. bs
    cur_bp.pt.p.text = as
    if oldlp.prev then
      oldlp.prev.next = oldlp.next
    end
    if oldlp.next then
      oldlp.next.prev = oldlp.prev
    end

    adjust_markers (cur_bp.pt.p, oldlp, oldlen, -1, 0)
    cur_bp.last_line = cur_bp.last_line - 1
    thisflag.need_resync = true
  else
    local as = cur_bp.pt.p.text
    as = string.sub (as, 1, cur_bp.pt.o) .. string.sub (as, cur_bp.pt.o + 2)
    cur_bp.pt.p.text = as
    adjust_markers (cur_bp.pt.p, cur_bp.pt.p, cur_bp.pt.o, 0, -1)
  end

  cur_bp.modified = true

  return true
end

-- Replace text in the line `lp' with `newtext'. If `replace_case' is
-- true then the new characters will be the same case as the old.
function line_replace_text (lp, offset, oldlen, newtext, replace_case)
  if replace_case and get_variable_bool ("case-replace") then
    local case_type = check_case (string.sub (lp.text, offset + 1, offset + oldlen))
    if case_type then
      newtext = recase (newtext, case_type)
    end
  end

  cur_bp.modified = true
  lp.text = string.sub (lp.text, 1, offset) .. newtext .. string.sub (lp.text, offset + 1 + oldlen)
  adjust_markers (lp, lp, offset, 0, #newtext - oldlen)
end


-- The buffer list
buffers = {}

buffer_name_history = history_new ()


-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new ()
  local bp = {}

  bp.pt = point_new ()
  bp.pt.p = line_new ()
  bp.pt.p.text = ""
  bp.lines = bp.pt.p
  bp.last_line = 0
  bp.markers = {}
  bp.eol = coding_eol_lf
  bp.dir = posix.getcwd () or ""

  -- Insert into buffer list.
  table.insert (buffers, bp)

  init_buffer (bp)

  return bp
end

-- Initialise a buffer
function init_buffer (bp)
  if get_variable_bool ("auto-fill-mode") then
    bp.autofill = true
  end
end

-- Get filename, or buffer name if nil.
function get_buffer_filename_or_name (bp)
  return bp.filename or bp.name
end

function activate_mark ()
  cur_bp.mark_active = true
end

function deactivate_mark ()
  cur_bp.mark_active = false
end

function delete_region (rp)
  local m = point_marker ()

  if warn_if_readonly_buffer () then
    return false
  end

  goto_point (rp.start)
  undo_save (UNDO_REPLACE_BLOCK, rp.start, rp.size, 0)
  undo_nosave = true
  for i = rp.size, 1, -1 do
    delete_char ()
  end
  undo_nosave = false
  goto_point (m.pt)
  unchain_marker (m)

  return true
end

function calculate_buffer_size (bp)
  local size = 0
  local lp = bp.lines
  while lp ~= nil do
    size = size + #lp.text
    lp = lp.next
    if lp == nil then
      break
    end
    size = size + 1
  end

  return size
end

-- Return a safe tab width for the given buffer.
function tab_width (bp)
  return math.max (get_variable_number_bp (bp, "tab-width"), 1)
end

-- Copy a region of text into a string.
function copy_text_block (pt, size)
  local lp = pt.p
  local s = string.sub (lp.text, pt.o + 1) .. "\n"

  lp = lp.next
  while #s < size do
    s = s .. lp.text .. "\n"
    lp = lp.next
  end

  return string.sub (s, 1, size)
end

function in_region (lineno, x, rp)
  if lineno < rp.start.n or lineno > rp.finish.n then
    return false
  elseif rp.start.n == rp.finish.n then
    return x >= rp.start.o and x < rp.finish.o
  elseif lineno == rp.start.n then
    return x >= rp.start.o
  elseif lineno == rp.finish.n then
    return x < rp.finish.o
  else
    return true
  end
  return false
end

local function warn_if_no_mark ()
  if not cur_bp.mark then
    minibuf_error ("The mark is not set now")
    return true
  elseif not cur_bp.mark_active then
    minibuf_error ("The mark is not active now")
    return true
  end
  return false
end

function region_new ()
  return {}
end

-- Calculate the region size between point and mark and set the
-- region.
function calculate_the_region (rp)
  if warn_if_no_mark () then
    return false
  end

  if cmp_point (cur_bp.pt, cur_bp.mark.pt) < 0 then
    -- Point is before mark.
    rp.start = table.clone (cur_bp.pt)
    rp.finish = table.clone (cur_bp.mark.pt)
  else
    -- Mark is before point.
    rp.start = table.clone (cur_bp.mark.pt)
    rp.finish = table.clone (cur_bp.pt)
  end

  local size = rp.finish.o - rp.start.o
  local lp = rp.start.p
  while lp ~= rp.finish.p do
    size = size + #lp.text + 1
    lp = lp.next
  end

  rp.size = size
  return true
end

-- Switch to the specified buffer.
function switch_to_buffer (bp)
  assert (cur_wp.bp == cur_bp)

  -- The buffer is the current buffer; return safely.
  if cur_bp == bp then
    return
  end

  -- Set current buffer.
  cur_bp = bp
  cur_wp.bp = cur_bp

  -- Move the buffer to head.
  for i = 1, #buffers do
    if buffers[i] == bp then
      table.remove (buffers, i)
      table.insert (buffers, bp)
      break
    end
  end

  -- Change to buffer's default directory
  posix.chdir (bp.dir)

  thisflag.need_resync = true
end

-- Create a buffer name using the file name.
local function make_buffer_name (filename)
  local s = posix.basename (filename)

  if not find_buffer (s) then
    return s
  else
    local i = 2
    while true do
      local name = string.format ("%s<%d>", s, i)
      if not find_buffer (name) then
        return name
      end
      i = i + 1
    end
  end
end

-- Search for a buffer named `name'.
function find_buffer (name)
  for _, bp in ipairs (buffers) do
    if bp.name == name then
      return bp
    end
  end
end

-- Set a new filename, and from it a name, for the buffer.
function set_buffer_names (bp, filename)
  if filename[1] ~= '/' then
      filename = posix.getcwd () .. "/" .. filename
      bp.filename = filename
  else
    bp.filename = filename
  end

  bp.name = make_buffer_name (filename)
end

-- Remove the specified buffer from the buffer list.
-- Recreate the scratch buffer when required.
function kill_buffer (kill_bp)
  -- Search for windows displaying the buffer to kill.
  for _, wp in ipairs (windows) do
    if wp.bp == kill_bp then
      wp.topdelta = 0
      wp.saved_pt = nil
    end
  end

  -- Remove the buffer from the buffer list.
  local next_bp = buffers[#buffers]
  for i = 1, #buffers do
    if buffers[i] == kill_bp then
      table.remove (buffers, i)
      next_bp = buffers[i > 1 and i - 1 or #buffers]
      if cur_bp == kill_bp then
        cur_bp = next_bp
      end
      break
    end
  end

  -- If no buffers left, recreate scratch buffer and point windows at
  -- it.
  if #buffers == 0 then
    table.insert (buffers, create_scratch_buffer ())
    cur_bp = buffers[1]
    for _, wp in ipairs (windows) do
      wp.bp = cur_bp
    end
  end

  -- Resync windows that need it.
  for _, wp in ipairs (windows) do
    if wp.bp == kill_bp then
      wp.bp = next_bp
      resync_redisplay (wp)
    end
  end
end

-- Set the specified buffer's temporary flag and move the buffer
-- to the end of the buffer list.
function set_temporary_buffer (bp)
  bp.temporary = true

  for i = 1, #buffers do
    if buffers[i] == bp then
      table.remove (buffers, i)
      break
    end
  end
  table.insert (buffers, 1, bp)
end

-- Print an error message into the echo area and return true
-- if the current buffer is readonly; otherwise return false.
function warn_if_readonly_buffer ()
  if cur_bp.readonly then
    minibuf_error (string.format ("Buffer is readonly: %s", cur_bp.name))
    return true
  end

  return false
end

function activate_mark ()
  cur_bp.mark_active = true
end

function deactivate_mark ()
  cur_bp.mark_active = false
end

-- Check if the buffer has been modified.  If so, asks the user if
-- he/she wants to save the changes.  If the response is positive, return
-- true, else false.
function check_modified_buffer (bp)
  if bp.modified and not bp.nosave then
    while true do
      local ans = minibuf_read_yesno (string.format ("Buffer %s modified; kill anyway? (yes or no) ", bp.name))
      if ans == nil then
        execute_function ("keyboard-quit")
        return false
      elseif not ans then
        return false
      end
      break
    end
  end

  return true
end


-- Basic movement routines

function move_char (dir)
  if (dir > 0 and not eolp ()) or (dir < 0 and not bolp ()) then
    cur_bp.pt.o = cur_bp.pt.o + dir
    return true
  elseif (dir > 0 and not eobp ()) or (dir < 0 and not bobp ()) then
    thisflag.need_resync = true
    if dir > 0 then
      cur_bp.pt.p = cur_bp.pt.p.next
    else
      cur_bp.pt.p = cur_bp.pt.p.prev
    end
    cur_bp.pt.n = cur_bp.pt.n + dir
    if dir > 0 then
      execute_function ("beginning-of-line")
    else
      execute_function ("end-of-line")
    end
    return true
  end

  return false
end

-- Go to the column `goalc'.  Take care of expanding tabulations.
function goto_goalc ()
  local col = 0

  local i = 1
  while i <= #cur_bp.pt.p.text do
    if col == cur_bp.goalc then
      break
    elseif cur_bp.pt.p.text[i] == '\t' then
      local t = tab_width (cur_bp)
      for w = t - col % t, 1, -1 do
        col = col + 1
        if col == cur_bp.goalc then
          break
        end
      end
    else
      col = col + 1
    end
    i = i + 1
  end

  cur_bp.pt.o = i - 1
end

function move_line (n)
  local ok = true
  local dir

  if n == 0 then
    return false
  elseif n > 0 then
    dir = 1
    if n > cur_bp.last_line - cur_bp.pt.n then
      ok = false
      n = cur_bp.last_line - cur_bp.pt.n
    end
  else
    dir = -1
    n = -n
    if n > cur_bp.pt.n then
      ok = false
      n = cur_bp.pt.n
    end
  end

  for i = n, 1, -1 do
    cur_bp.pt.p = cur_bp.pt.p[dir > 0 and "next" or "prev"]
    cur_bp.pt.n = cur_bp.pt.n + dir
  end

  if _last_command ~= "next-line" and _last_command ~= "previous-line" then
    cur_bp.goalc = get_goalc ()
  end
  goto_goalc ()

  thisflag.need_resync = true

  return ok
end


Defun ("kill-buffer",
       {"string"},
[[
Kill buffer BUFFER.
With a nil argument, kill the current buffer.
]],
  true,
  function (buffer)
    local ok = leT

    if not buffer then
      local cp = make_buffer_completion ()
      buffer = minibuf_read (string.format ("Kill buffer (default %s): ", cur_bp.name),
                             "", cp, buffer_name_history)
      if not buffer then
        ok = execute_function ("keyboard-quit")
      end
    end

    local bp
    if buffer and buffer ~= "" then
      bp = find_buffer (buffer)
      if not bp then
        minibuf_error (string.format ("Buffer `%s' not found", buffer))
        ok = leNIL
      end
    else
      bp = cur_bp
    end

    if ok == leT then
      if not check_modified_buffer (bp) then
        ok = leNIL
      else
        kill_buffer (bp)
      end
    end

    return ok
  end
)

local function buffer_next (this_bp)
  for i, bp in ipairs (buffers) do
    if bp == this_bp then
      if i > 1 then
        return buffers[i - 1]
      else
        return buffers[#buffers]
      end
      break
    end
  end
end

Defun ("switch-to-buffer",
       {"string"},
[[
Select buffer @i{buffer} in the current window.
]],
  true,
  function (buffer)
    local ok = leT
    local bp = buffer_next (cur_bp)

    if not buffer then
      local cp = make_buffer_completion ()
      buffer = minibuf_read (string.format ("Switch to buffer (default %s): ", bp.name),
                             "", cp, buffer_name_history)

      if not buffer then
        ok = execute_function ("keyboard-quit")
      end
    end

    if ok == leT then
      if buffer and buffer ~= "" then
        bp = find_buffer (buffer)
        if not bp then
          bp = buffer_new ()
          bp.name = buffer
          bp.needname = true
          bp.nosave = true
        end
      end

      switch_to_buffer (bp)
    end

    return ok
  end
)

function create_auto_buffer (name)
  local bp = buffer_new ()
  bp.name = name
  bp.needname = true
  bp.temporary = true
  bp.nosave = true
  return bp
end

function create_scratch_buffer ()
  return create_auto_buffer ("*scratch*")
end
