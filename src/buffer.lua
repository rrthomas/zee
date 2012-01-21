-- Buffer-oriented functions
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


-- Buffer methods that know about the gap.

function get_buffer_pre_point (bp)
  return bp.text.s:sub (1, get_buffer_pt (bp))
end

function get_buffer_post_point (bp)
  return bp.text.s:sub (get_buffer_pt (bp) + bp.gap + 1)
end

function get_buffer_pt (bp)
  return bp.pt
end

function set_buffer_pt (bp, o)
  local len = #bp.text.s
  if o < bp.pt then
    bp.text.s = bp.text.s:sub (1, o) .. string.rep ('\0', bp.gap) .. bp.text.s:sub (o + 1, bp.pt) .. bp.text.s:sub (bp.pt + bp.gap + 1)
    assert (len == #bp.text.s)
  elseif o > bp.pt then
    bp.text.s = bp.text.s:sub (1, bp.pt) .. bp.text.s:sub (bp.pt + bp.gap + 1, bp.gap + o) .. string.rep ('\0', bp.gap) .. bp.text.s:sub (bp.gap + o + 1)
    assert (len == #bp.text.s)
  end
  bp.pt = o
end

local function realo_to_o (bp, o)
  if o == nil then
    return o
  elseif o < bp.pt + bp.gap then
    return math.min (o, bp.pt)
  end
  return o - bp.gap
end

local function o_to_realo (bp, o)
  return o < bp.pt and o or o + bp.gap
end

function get_buffer_size (bp)
  return realo_to_o (bp, #bp.text.s)
end

function buffer_line_len (bp, o)
  o = o or get_buffer_line_o (bp)
  return realo_to_o (bp, estr_end_of_line (bp.text, o_to_realo (bp, o))) -
    realo_to_o (bp, estr_start_of_line (bp.text, o_to_realo (bp, o)))
end

-- Replace `oldlen' chars after point `bp' with `newtext'.
local min_gap = 1024 -- Minimum gap size after resize
local max_gap = 4096 -- Maximum permitted gap size
function replace (oldlen, newtext)
  undo_save_block (cur_bp.pt, oldlen, #newtext)

  -- Ensure gap fits #newtext; if gap becomes zero, set to min_gap.
  if cur_bp.gap + oldlen < #newtext then
    cur_bp.text.s = cur_bp.text.s:sub (1, cur_bp.pt) .. string.rep ("\0", (#newtext + min_gap) - (cur_bp.gap + oldlen)) .. cur_bp.text.s:sub (cur_bp.pt + 1)
    cur_bp.gap = #newtext + min_gap - oldlen
  end

  -- Remove oldlen chars, zeroing some if necessary.
  if oldlen > #newtext then
    cur_bp.text.s = cur_bp.text.s:sub (1, cur_bp.pt + cur_bp.gap) .. string.rep ('\0', oldlen - #newtext) .. cur_bp.text.s:sub (cur_bp.pt + cur_bp.gap + 1 + oldlen - #newtext)
  end
  cur_bp.gap = cur_bp.gap + oldlen

  -- Insert #newtext chars.
  cur_bp.text.s = cur_bp.text.s:sub (1, cur_bp.pt + cur_bp.gap - #newtext) .. newtext .. cur_bp.text.s:sub (cur_bp.pt + cur_bp.gap + 1)
  cur_bp.gap = cur_bp.gap - #newtext

  -- Ensure gap doesn't get too big.
  if cur_bp.gap > max_gap then
    cur_bp.text.s = cur_bp.text.s:sub (1, cur_bp.pt) .. cur_bp.text.s:sub (cur_bp.pt + cur_bp.gap - max_gap + 1)
    cur_bp.gap = max_gap
  end

  -- Adjust markers.
  for m in pairs (cur_bp.markers) do
    if m.o > cur_bp.pt then
      m.o = math.max (cur_bp.pt, m.o + #newtext - oldlen)
    end
  end

  cur_bp.modified = true
  thisflag.need_resync = true
end

function replace_estr (del, es)
  if warn_if_readonly_buffer () then
    return false
  end

  undo_start_sequence ()
  replace (del, "")
  local p = 1
  while p <= #es.s do
    local next = string.find (es.s, es.eol, p)
    local line_len = (next or #es.s + 1) - p
    replace (0, string.sub (es.s, p, p + line_len - 1))
    local eol_len, buf_eol_len = #es.eol, #get_buffer_eol (cur_bp)
    set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + line_len)
    p = p + line_len
    if next then
      replace (0, get_buffer_eol (cur_bp))
      set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + buf_eol_len)
      thisflag.need_resync = true
      p = p + eol_len
    end
  end
  undo_end_sequence ()

  return true
end

function insert_estr (es)
  return replace_estr (0, es)
end

function get_buffer_char (bp, o)
  return bp.text.s[o_to_realo (bp, o) + 1]
end

function buffer_prev_line (bp, o)
  return realo_to_o (bp, estr_prev_line (bp.text, o_to_realo (bp, o)))
end

function buffer_next_line (bp, o)
  return realo_to_o (bp, estr_next_line (bp.text, o_to_realo (bp, o)))
end

function buffer_start_of_line (bp, o)
  return realo_to_o (bp, estr_start_of_line (bp.text, o_to_realo (bp, o)))
end

function buffer_end_of_line (bp, o)
  return realo_to_o (bp, estr_end_of_line (bp.text, o_to_realo (bp, o)))
end

function get_buffer_line_o (bp)
  return realo_to_o (bp, estr_start_of_line (bp.text, o_to_realo (bp, bp.pt)))
end


-- Buffer methods that don't know about the gap.

function get_buffer_eol (bp)
  return bp.text.eol
end

-- Copy a region of text into an estr.
function get_buffer_region (bp, r)
  local s = ""
  if r.start < get_buffer_pt (bp) then
    s = s .. get_buffer_pre_point (bp):sub (r.start + 1, math.min (r.finish, get_buffer_pt (bp)))
  end
  if r.finish > get_buffer_pt (bp) then
    local from = math.max (r.start - get_buffer_pt (bp), 0)
    s = s .. get_buffer_post_point (bp):sub (from + 1, r.finish - get_buffer_pt (bp))
  end
  return {s = s, eol = get_buffer_eol (bp)}
end

-- Insert the character `c' at the current point position
-- into the current buffer.
function insert_char (c)
  return replace_estr (0, {s = c, eol = coding_eol_lf})
end

function delete_char ()
  deactivate_mark ()

  if eobp () then
    return minibuf_error ("End of buffer")
  end

  if warn_if_readonly_buffer () then
    return false
  end

  if eolp () then
    replace (#get_buffer_eol (cur_bp), "")
    thisflag.need_resync = true
  else
    replace (1, "")
  end

  cur_bp.modified = true

  return true
end

function insert_buffer (bp)
  -- Copy text to avoid problems when bp == cur_bp.
  insert_estr ({s = get_buffer_pre_point (bp) .. get_buffer_post_point (bp), eol = get_buffer_eol (bp)})
end


-- The buffer list
buffers = {}

buffer_name_history = history_new ()

-- Allocate a new buffer, set the default local variable values, and
-- insert it into the buffer list.
function buffer_new ()
  local bp = {}

  bp.pt = 0
  bp.gap = 0
  bp.text = {s = "", eol = coding_eol_lf}
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

-- Set a new filename, and from it a name, for the buffer.
function set_buffer_names (bp, filename)
  if filename[1] ~= '/' then
    filename = string.format ("%s/%s", posix.getcwd(), filename)
  end
  bp.filename = filename

  local s = posix.basename (filename)
  local name = s
  local i = 2
  while find_buffer (name) do
    name = string.format ("%s<%d>", s, i)
    i = i + 1
  end
  bp.name = name
end

-- Search for a buffer named `name'.
function find_buffer (name)
  for _, bp in ipairs (buffers) do
    if bp.name == name then
      return bp
    end
  end
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

-- Print an error message into the echo area and return true
-- if the current buffer is readonly; otherwise return false.
function warn_if_readonly_buffer ()
  if cur_bp.readonly then
    minibuf_error (string.format ("Buffer is readonly: %s", cur_bp.name))
    return true
  end

  return false
end

function warn_if_no_mark ()
  if not cur_bp.mark then
    minibuf_error ("The mark is not set now")
    return true
  elseif not cur_bp.mark_active then
    minibuf_error ("The mark is not active now")
    return true
  end
  return false
end

-- Make a region from two offsets
function region_new (o1, o2)
  return {start = math.min (o1, o2), finish = math.max (o1, o2)}
end

function get_region_size (rp)
  return rp.finish - rp.start
end

-- Return the region between point and mark.
function calculate_the_region ()
  if warn_if_no_mark () then
    return nil
  end

  return region_new (cur_bp.pt, cur_bp.mark.o)
end

function delete_region (r)
  if warn_if_readonly_buffer () then
    return false
  end

  local m = point_marker ()
  goto_offset (r.start)
  replace (get_region_size (r), "")
  goto_offset (m.o)
  unchain_marker (m)

  return true
end

function in_region (o, x, r)
  return o + x >= r.start and o + x < r.finish
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

function activate_mark ()
  cur_bp.mark_active = true
end

function deactivate_mark ()
  cur_bp.mark_active = false
end

-- Return a safe tab width for the given buffer.
function tab_width (bp)
  return math.max (get_variable_number_bp (bp, "tab-width"), 1)
end

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

Defun ("kill-buffer",
       {"string"},
[[
Kill buffer BUFFER.
With a nil argument, kill the current buffer.
]],
  true,
  function (buffer)
    local ok = true

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
        ok = false
      end
    else
      bp = cur_bp
    end

    if ok then
      if not check_modified_buffer (bp) then
        ok = false
      else
        kill_buffer (bp)
      end
    end

    return ok
  end
)

function make_buffer_completion ()
  local cp = completion_new ()
  for _, bp in ipairs (buffers) do
    table.insert (cp.completions, bp.name)
  end

  return cp
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
      set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + dir)
    elseif (dir > 0 and not eobp ()) or (dir < 0 and not bobp ()) then
      thisflag.need_resync = true
      set_buffer_pt (cur_bp, get_buffer_pt (cur_bp) + #get_buffer_eol (cur_bp) * dir)
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

  local i = get_buffer_line_o (cur_bp)
  local lim = get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp)
  while i < lim do
    if col == cur_bp.goalc then
      break
    elseif get_buffer_char (cur_bp, i) == '\t' then
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

  set_buffer_pt (cur_bp, i)
end

function move_line (n)
  local func = buffer_next_line
  if n < 0 then
    n = -n
    func = buffer_prev_line
  end

  if _last_command ~= "next-line" and _last_command ~= "previous-line" then
    cur_bp.goalc = get_goalc ()
  end

  while n > 0 do
    local o = func (cur_bp, cur_bp.pt)
    if o == nil then
      break
    end
    set_buffer_pt (cur_bp, o)
    n = n - 1
  end

  goto_goalc ()
  thisflag.need_resync = true

  return n == 0
end

function offset_to_line (bp, offset)
  local n = 0
  local o = 0
  while buffer_end_of_line (bp, o) and buffer_end_of_line (bp, o) < offset do
    n = n + 1
    o = buffer_next_line (bp, o)
    assert (o)
  end
  return n
end

function goto_offset (o)
  local old_lineo = get_buffer_line_o (cur_bp)
  set_buffer_pt (cur_bp, o)
  if get_buffer_line_o (cur_bp) ~= old_lineo then
    cur_bp.goalc = get_goalc ()
    thisflag.need_resync = true
  end
end

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
    local ok = true
    local bp = buffer_next (cur_bp)

    if not buffer then
      local cp = make_buffer_completion ()
      buffer = minibuf_read (string.format ("Switch to buffer (default %s): ", bp.name),
                             "", cp, buffer_name_history)

      if not buffer then
        ok = execute_function ("keyboard-quit")
      end
    end

    if ok then
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
