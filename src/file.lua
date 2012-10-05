-- Disk file handling
--
-- Copyright (c) 2009-2012 Free Software Foundation, Inc.
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

-- FIXME: Reload when file changes on disk

function exist_file (filename)
  if posix.stat (filename) then
    return true
  end
  local _, err = posix.errno ()
  return err ~= posix.ENOENT
end

local function is_regular_file (filename)
  local st = posix.stat (filename)
  return st and st.type == "regular"
end

-- Return nonzero if file exists and can be written.
local function check_writable (filename)
  local ok = posix.euidaccess (filename, "w")
  return ok and ok >= 0
end

-- This functions makes the passed path an absolute path:
--
--  * expands `~/' and `~name/' expressions;
--  * replaces `//' with `/' (restarting from the root directory);
--  * removes `..' and `.' entries.
--
-- Returns normalized path, or nil if a password entry could not be
-- read
function normalize_path (path)
  local comp = io.splitdir (path)
  local ncomp = {}

  -- Prepend cwd if path is relative
  if comp[1] ~= "" then
    comp = list.concat (io.splitdir (posix.getcwd () or ""), comp)
  end

  -- Deal with `~[user]', `..', `.', `//'
  for i, v in ipairs (comp) do
    if v == "" and i > 1 and i < #comp then -- `//'
      ncomp = {}
    elseif v == ".." then -- `..'
      table.remove (ncomp)
    elseif v ~= "." then -- not `.'
      if v[1] == "~" then -- `~[user]'
        ncomp = {}
        v = posix.getpasswd (v:match ("^~(.+)$"), "dir")
        if v == nil then
          return nil
        end
      end
      table.insert (ncomp, v)
    end
  end

  return io.catdir (unpack (ncomp))
end

-- Write buffer to given file name with given mode.
alien.default.write:types ("ptrdiff_t", "int", "pointer", "size_t")
local function write_file (filename, mode)
  local h = posix.creat (filename, mode)
  if h then
    local s = get_buffer_pre_point (buf)
    local ret = alien.default.write (h, s.buf.buffer:topointer (), #s)
    if ret == #s then
      s = get_buffer_post_point (buf)
      ret = alien.default.write (h, s.buf.buffer:topointer (), #s)
      if ret == #s then
        ret = true
      end
    end

    local ret2 = posix.close (h)
    if ret == true and ret2 ~= 0 then -- writing error takes precedence over closing error
      ret = ret2
    end

    return ret
  end
end

-- Write the buffer contents to a file.
local function save_buffer ()
  local ret = write_file (buf.filename, "rw-rw-rw-")
  if not ret then
    return minibuf_error (string.format ("Error writing `%s'%s", buf.filename,
                                         ret == -1 and ": " .. posix.errno () or ""))
  end

  minibuf_write ("Wrote " .. buf.filename)
  buf.modified = false
  undo_set_unchanged (buf.last_undop)
  return true
end

Command ("file-save",
[[
Save buffer in visited file.
]],
  function ()
    return save_buffer ()
  end
)

Command ("file-quit",
[[
Offer to save the file, then delete this process.
]],
  function ()
    if buf.modified then
      local ans = minibuf_read_yn (string.format ("Save file %s? (y, n) ", get_buffer_filename (buf)))
      if ans == nil then
        return ding ()
      elseif ans then
        save_buffer ()
      end
    else
      minibuf_write ("(The file does not need saving)")
    end

    thisflag.quit = true
  end
)

function find_file (filename)
  if exist_file (filename) and not is_regular_file (filename) then
    return minibuf_error (string.format ("File `%s' exists but could not be read", filename))
  else
    buf = buffer_new ()
    set_buffer_name (buf, filename)

    local s = io.slurp (filename)
    if s then
      buf.readonly = not check_writable (filename)
    else
      s = ""
    end
    buf.text = AStr (s)

    -- Reset undo history
    buf.next_undop = nil
    buf.last_undop = nil
    buf.modified = false
  end

  -- Change to buffer's default directory
  posix.chdir (posix.dirname (buf.filename))

  thisflag.need_resync = true

  return true
end

-- Function called on unexpected error or crash (SIGSEGV).
-- Attempts to save modified buffer.
-- If doabort is true, aborts to allow core dump generation;
-- otherwise, exit.
-- FIXME: Reset the terminal?
function editor_exit (doabort)
  io.stderr:write ("Trying to save buffer (if modified)...\r\n")

  if buf and buf.modified then
    local file = buf.filename .. string.upper (PACKAGE) .. "SAVE"
    io.stderr:write (string.format ("Saving %s...\r\n", file))
    write_to_disk (file, "rw-------")
  end

  if doabort then
    posix.abort ()
  else
    posix._exit (2)
  end
end
