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

  if st and st.type == "regular" then
    return true
  end
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

-- Return a `~/foo' like path if the user is under his home directory,
-- else the unmodified path.
-- If the user's home directory cannot be read, nil is returned.
function compact_path (path)
  local home = posix.getpasswd (nil, "dir")
  -- If we cannot get the home directory, return empty string
  if home == nil then
    return ""
  end

  -- Replace `^$HOME' (if found) with `~'.
  return (string.gsub (path, "^" .. home, "~"))
end

Defun ("insert-file",
[[
Insert contents of file FILENAME into buffer after point.
Set mark after the inserted text.
]],
  function (file)
    local ok = true

    if warn_if_readonly_buffer () then
      return false
    end

    if not file then
      file = minibuf_read_filename ("Insert file: ", cur_bp.dir)
      if not file then
        ok = execute_function ("keyboard-quit")
      end
    end

    if not file or file == "" then
      ok = false
    end

    if ok then
      local s = io.slurp (file)
      if s then
        insert_astr (AStr (s))
      else
        ok = minibuf_error ("%s: %s", file, posix.errno ())
      end
    end

    return ok
  end
)

-- Write buffer to given file name with given mode.

alien.default.write:types ("ptrdiff_t", "int", "pointer", "size_t")


local function write_to_disk (filename, mode)
  local h = posix.creat (filename, mode)
  if h then
    local s = get_buffer_pre_point (cur_bp)
    local ret = alien.default.write (h, s.buf.buffer:topointer (), #s)
    if ret == #s then
      s = get_buffer_post_point (cur_bp)
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
local function write_file ()
  local ret = write_to_disk (cur_bp.filename, "rw-rw-rw-")
  if ret == true then
    return true
  end

  return minibuf_error (string.format ("Error writing `%s'%s", cur_bp.filename,
                                       ret == -1 and ": " .. posix.errno () or ""))
end

local function save_buffer ()
  if cur_bp.modified then
    if not write_file () then
      return false
    end
    minibuf_write ("Wrote " .. cur_bp.filename)
    cur_bp.modified = false
    undo_set_unchanged (cur_bp.last_undop)
  else
    minibuf_write ("(No changes need to be saved)")
  end
  return true
end

Defun ("file-save",
[[
Save buffer in visited file if modified.
]],
  function ()
    return save_buffer ()
  end
)

Defun ("file-quit",
[[
Offer to save the file, then kill this process.
]],
  function ()
    if cur_bp.modified then
      while true do
        minibuf_write (string.format ("Save file %s? (y, n) ", get_buffer_filename_or_name (cur_bp)))
        local c = getkey (GETKEY_DEFAULT)
        minibuf_clear ()

        if c == keycode "C-g" then
          execute_function ("keyboard-quit")
          return false
        end
        if c == keycode "y" then
          save_buffer ()
        end
        if c == keycode "y" or c == keycode "n" then
          break
        else
          minibuf_error ("Please answer y or n.")
          waitkey (WAITKEY_DEFAULT)
        end
      end
    else
      minibuf_write ("(The file does not need saving)")
    end

    if cur_bp.modified then
      while true do
        local ans = minibuf_read_yn ("The file is modified; exit anyway? (y or n) ")
        if ans == nil then
          return execute_function ("keyboard-quit")
        elseif not ans then
          return false
        end
      end
    end

    thisflag.quit = true
  end
)

function find_file (filename)
  if exist_file (filename) and not is_regular_file (filename) then
    return minibuf_error ("File exists but could not be read")
  else
    cur_bp = buffer_new ()
    set_buffer_names (cur_bp, filename)
    cur_bp.dir = posix.dirname (filename)

    local s = io.slurp (filename)
    if s then
      cur_bp.readonly = not check_writable (filename)
    else
      s = ""
    end
    cur_bp.text = AStr (s)

    -- Reset undo history
    cur_bp.next_undop = nil
    cur_bp.last_undop = nil
    cur_bp.modified = false
  end

  -- Change to buffer's default directory
  posix.chdir (cur_bp.dir)

  thisflag.need_resync = true

  return true
end

-- Function called on unexpected error or crash (SIGSEGV).
-- Attempts to save modified buffer.
-- If doabort is true, aborts to allow core dump generation;
-- otherwise, exit.
function editor_exit (doabort)
  io.stderr:write ("Trying to save buffer (if modified)...\r\n")

  if cur_bp and cur_bp.modified then
    local buf = (cur_bp.filename or cur_bp.name) .. string.upper (PACKAGE) .. "SAVE"
    io.stderr:write (string.format ("Saving %s...\r\n", buf))
    write_to_disk (buf, "rw-------")
  end

  if doabort then
    posix.abort ()
  else
    posix._exit (2)
  end
end
