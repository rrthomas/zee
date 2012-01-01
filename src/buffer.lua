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

function get_line_prev (lp)
  if lp.o == 0 then
    return nil
  end
  -- FIXME: Write & use memrmem
  local found = find_substr (get_buffer_text (lp.bp).s, get_buffer_text (lp.bp).eol, 0, lp.o - 1, false, true, true, false, false)
  return {bp = lp.bp, o = found and (found + #get_buffer_text (lp.bp).eol - 1) or 0}
end

function get_line_next (lp)
  local next = string.find (string.sub (get_buffer_text (lp.bp).s, lp.o + 1), get_buffer_text (lp.bp).eol)
  if next == nil then
    return nil
  end
  return {bp = lp.bp, o = lp.o + next - 1 + #get_buffer_text (lp.bp).eol}
end

function get_line_offset (lp)
  return lp.o
end

function point_to_offset (pt)
  return pt.p.o + pt.o
end

-- Adjust markers (including point) at offset `o' by offset `delta'.
local function adjust_markers (o, delta)
  local m_pt = point_marker ()
  for m in pairs (cur_bp.markers) do
    if m.o > o then
      m.o = math.max (o, m.o + delta)
    end
  end

  -- This marker has been updated to new position.
  goto_point (get_marker_pt (m_pt))
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

-- Insert the character `c' at the current point position
-- into the current buffer.
function insert_char (c)
  return replace (0, c)
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
    adjust_markers (cur_bp.pt.p.o + cur_bp.pt.o, -#get_buffer_text (cur_bp).eol)
    cur_bp.es.s = string.sub (get_buffer_text (cur_bp).s, 1, cur_bp.pt.p.o + cur_bp.pt.o) .. string.sub (get_buffer_text (cur_bp).s, cur_bp.pt.p.o + cur_bp.pt.o + 1 + #get_buffer_text (cur_bp).eol)
    cur_bp.last_line = cur_bp.last_line - 1
    thisflag.need_resync = true
  else
    adjust_markers (cur_bp.pt.p.o + cur_bp.pt.o, -1)
    cur_bp.es.s = string.sub (get_buffer_text (cur_bp).s, 1, cur_bp.pt.p.o + cur_bp.pt.o) .. string.sub (get_buffer_text (cur_bp).s, cur_bp.pt.p.o + cur_bp.pt.o + 2)
  end

  cur_bp.modified = true

  return true
end

-- Replace text in the buffer `bp' at offset `offset' with `newtext'.
-- If `replace_case' is true then the new characters will be the same
-- case as the old.
function buffer_replace (bp, offset, oldlen, newtext, replace_case)
  if replace_case and get_variable_bool ("case-replace") then
    local case_type = check_case (string.sub (get_buffer_text (bp).s, offset + 1, offset + oldlen))
    if case_type then
      newtext = recase (newtext, case_type)
    end
  end

  undo_save (UNDO_REPLACE_BLOCK, offset_to_point (bp, offset), oldlen, #newtext)
  bp.modified = true
  adjust_markers (offset, #newtext - oldlen)
  bp.es.s = string.sub (get_buffer_text (bp).s, 1, offset) .. newtext .. string.sub (get_buffer_text (bp).s, offset + 1 + oldlen)
end


-- The buffer list
buffers = {}

buffer_name_history = history_new ()

function insert_buffer (bp)
  undo_save (UNDO_START_SEQUENCE, cur_bp.pt, 0, 0)
  -- Copy text to avoid problems when bp == cur_bp.
  insert_estr (estr_dup (bp.es))
  undo_save (UNDO_END_SEQUENCE, cur_bp.pt, 0, 0)
end

-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new ()
  local bp = {}

  bp.pt = point_new ()
  bp.pt.p = {bp = bp, o = 0, n = 0}
  bp.es = {s = "", eol = coding_eol_lf}
  bp.lines = bp.pt.p
  bp.last_line = 0
  bp.markers = {}
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

function get_buffer_o (bp)
  return bp.pt.p.o
end

function get_buffer_text (bp)
  return bp.es
end

function get_buffer_size (bp)
  return #bp.es.s
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

  goto_point (get_region_start (rp))
  undo_save (UNDO_REPLACE_BLOCK, get_region_start (rp), get_region_size(rp), 0)
  undo_nosave = true
  for i = get_region_size (rp), 1, -1 do
    delete_char ()
  end
  undo_nosave = false
  goto_point (get_marker_pt (m))
  unchain_marker (m)

  return true
end

-- Return a safe tab width for the given buffer.
function tab_width (bp)
  return math.max (get_variable_number_bp (bp, "tab-width"), 1)
end

function get_buffer_line_text (bp, o)
  return string.sub (get_buffer_text (bp).s,
                     estr_start_of_line (get_buffer_text (bp), o) + 1,
                     estr_end_of_line (get_buffer_text (bp), o))
end

-- Copy a region of text into a string.
function get_buffer_region (bp, r)
  return {s = string.sub (get_buffer_text (bp).s, r.start + 1, r.finish), eol = get_buffer_text (bp).eol}
end

function in_region (lineno, x, rp)
  if lineno < get_region_start (rp).n or lineno > get_region_end (rp).n then
    return false
  elseif get_region_start (rp).n == get_region_end (rp).n then
    return x >= get_region_start (rp).o and x < get_region_end (rp).o
  elseif lineno == get_region_start (rp).n then
    return x >= get_region_start (rp).o
  elseif lineno == get_region_end (rp).n then
    return x < get_region_end (rp).o
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

function get_buffer_line_len (bp)
  return estr_end_of_line (get_buffer_text (bp), get_buffer_o (bp)) - get_buffer_o (bp)
end

function region_new ()
  return {}
end

function set_region_start (rp, pt)
  rp.start = point_to_offset (pt)
end

function set_region_end (rp, pt)
  rp.finish = point_to_offset (pt)
end

function get_region_start (rp)
  return offset_to_point (cur_bp, rp.start);
end

function get_region_end (rp)
  return offset_to_point (cur_bp, rp.finish);
end

function get_region_size (rp)
  return rp.finish - rp.start
end


-- Calculate the region size between point and mark and set the
-- region.
function calculate_the_region ()
  if warn_if_no_mark () then
    return nil
  end

  local rp = region_new ()
  if cmp_point (cur_bp.pt, get_marker_pt (cur_bp.mark)) < 0 then
    -- Point is before mark.
    set_region_start (rp, cur_bp.pt)
    set_region_end (rp, get_marker_pt (cur_bp.mark))
  else
    -- Mark is before point.
    set_region_start (rp, get_marker_pt (cur_bp.mark))
    set_region_end (rp, cur_bp.pt)
  end

  return rp
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

function move_char (offset)
  local dir = offset >= 0 and 1 or -1
  for i = 1, math.abs (offset) do
    if (dir > 0 and not eolp ()) or (dir < 0 and not bolp ()) then
      cur_bp.pt.o = cur_bp.pt.o + dir
    elseif (dir > 0 and not eobp ()) or (dir < 0 and not bobp ()) then
      thisflag.need_resync = true
      cur_bp.pt.p = (dir > 0 and get_line_next or get_line_prev) (cur_bp.pt.p)
      assert (cur_bp.pt.p)
      cur_bp.pt.n = cur_bp.pt.n + dir
      execute_function (dir > 0 and "beginning-of-line" or "end-of-line")
    else
      return false
    end
  end

  return true
end

-- Go to the column `goalc'.  Take care of expanding tabulations.
function goto_goalc ()
  local col = 0

  local i = get_buffer_o (cur_bp)
  local lim = estr_next_line (get_buffer_text (cur_bp), get_buffer_o (cur_bp)) or math.huge
  while i < lim do
    if col == get_goalc () then
      break
    elseif get_buffer_text (cur_bp).s[i] == '\t' then
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

  cur_bp.pt.o = i - get_buffer_o (cur_bp)
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
    cur_bp.pt.p = (dir > 0 and get_line_next or get_line_prev) (cur_bp.pt.p)
    assert (cur_bp.pt.p)
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
