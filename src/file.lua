-- Disk file handling
--
-- Copyright (c) 2009-2011 Free Software Foundation, Inc.
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

-- FIXME: Warn when file changes on disk


-- Formats of end-of-line
coding_eol_lf = "\n"
coding_eol_crlf = "\r\n"
coding_eol_cr = "\r"


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
        v = posix.getpasswd (string.match (v, "^~(.+)$"), "dir")
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
  return string.gsub (path, "^" .. home, "~")
end

Defun ("find-file",
       {"string"},
[[
Edit the specified file.
Switch to a buffer visiting the file,
creating one if none already exists.
]],
  true,
  function (filename)
    local ok = leNIL

    if not filename then
      filename = minibuf_read_filename ("Find file: ", cur_bp.dir)
    end

    if not filename then
      ok = execute_function ("keyboard-quit")
    elseif "" ~= filename then
      ok = find_file (filename)
    end

    return ok
  end
)

Defun ("find-file-read-only",
       {"string"},
[[
Edit the specified file but don't allow changes.
Like `find-file' but marks buffer as read-only.
Use @kbd{M-x toggle-read-only} to permit editing.
]],
  true,
  function (filename)
    local ok = execute_function ("find-file", filename)
    if ok == leT then
      cur_bp.readonly = true
    end
  end
)

Defun ("find-alternate-file",
       {},
[[
Find the file specified by the user, select its buffer, kill previous buffer.
If the current buffer now contains an empty file that you just visited
(presumably by mistake), use this command to visit the file you really want.
]],
  true,
  function ()
    local buf = cur_bp.filename
    local base, ms, as

    if not buf then
      buf = cur_bp.dir
    else
      base = posix.basename (buf)
    end
    ms = minibuf_read_filename ("Find alternate: ", buf, base)

    local ok = leNIL
    if not ms then
      ok = execute_function ("keyboard-quit")
    elseif ms ~= "" and check_modified_buffer (cur_bp ()) then
      kill_buffer (cur_bp)
      ok = bool_to_lisp (find_file (ms))
    end

    return ok
  end
)

-- Insert file contents into current buffer.
-- Return quietly if the file doesn't exist, or other error.
local function insert_file (filename)
  if exist_file (filename) then
    local h = io.open (filename, "r")
    if h then
      local buf = h:read ("*a")
      h:close ()
      if #buf >= 1 then
        undo_save (UNDO_REPLACE_BLOCK, cur_bp.pt, 0, size)
        undo_nosave = true
        insert_string (buf) -- FIXME: Detect coding of file.
        undo_nosave = false
      end
      return true
    end
  end
  return false
end

Defun ("insert-file",
       {"string"},
[[
Insert contents of file FILENAME into buffer after point.
Set mark after the inserted text.
]],
  true,
  function (file)
    local ok = leT

    if warn_if_readonly_buffer () then
      return leNIL
    end

    if not file then
      file = minibuf_read_filename ("Insert file: ", cur_bp.dir)
      if not file then
        ok = execute_function ("keyboard-quit")
      end
    end

    if not file or file == "" then
      ok = leNIL
    end

    if ok ~= leNIL then
      if not insert_file (file) then
        ok = leNIL
        minibuf_error ("%s: %s", file, posix.errno ())
      end
    else
      set_mark_interactive ()
    end

    return ok
  end
)

-- Write buffer to given file name with given mode.
local function raw_write_to_disk (bp, filename, mode)
  local ret = true
  local h = posix.creat (filename, mode)

  if not h then
    return false
  end

  local len = get_buffer_size (bp)
  local written = posix.write (h, get_buffer_text (bp), len)
  if written < 0 or written ~= len then
    ret = written
  end

  if posix.close (h) ~= 0 then
    ret = false
  end

  return ret
end

-- Create a backup filename according to user specified variables.
local function create_backup_filename (filename, backupdir)
  local res

  -- Prepend the backup directory path to the filename
  if backupdir then
    local buf = backupdir
    if buf[-1] ~= '/' then
      buf = buf .. '/'
      filename = gsub (filename, "/", "!")

      if not normalize_path (buf) then
        buf = nil
      end
      res = buf
    end
  else
    res = filename
  end

  return res .. "~"
end

-- Copy a file.
local function copy_file (source, dest)
  local ifd = io.open (source, "r")
  if not ifd then
    minibuf_error (string.format ("%s: unable to backup", source))
    return false
  end

  local ofd, tname = posix.mkstemp (dest .. "XXXXXX")
  if not ofd then
    ifd:close ()
    minibuf_error (string.format ("%s: unable to create backup", dest))
    return false
  end

  local written = posix.write (ofd, ifd:read ("*a"))
  ifd:close ()
  posix.close (ofd)

  if not written then
    minibuf_error (string.format ("Unable to write to backup file `%s'", dest))
    return false
  end

  local st = posix.stat (source)

  -- Recover file permissions and ownership.
  if st then
    posix.chmod (tname, st.mode)
    posix.chown (tname, st.uid, st.gid)
  end

  if st then
    local ok, err = os.rename (tname, dest)
    if not ok then
      minibuf_error (string.format ("Cannot rename temporary file `%s'", err))
      os.remove (tname)
      st = nil
    end
  elseif unlink (tname) == -1 then
    minibuf_error (string.format ("Cannot remove temporary file `%s'", err))
  end

  -- Recover file modification time.
  if st then
    posix.utime (dest, st.mtime, st.atime)
  end

  return st ~= nil
end

-- Write the buffer contents to a file.
-- Create a backup file if specified by the user variables.
local function write_to_disk (bp, filename)
  local backup = get_variable_bool ("make-backup-files")
  local backupdir = get_variable_bool ("backup-directory") and get_variable ("backup-directory")

  -- Make backup of original file.
  if not bp.backup and backup then
    local h = io.open (filename, "r+")
    if h then
      h:close ()
      local bfilename = create_backup_filename (filename, backupdir)
      if bfilename and copy_file (filename, bfilename) then
        bp.backup = true
      else
        minibuf_error (string.format ("Cannot make backup file: %s", posix.errno ()))
        waitkey ()
      end
    end
  end

  local ret, err = raw_write_to_disk (bp, filename, "rw-rw-rw-")
  if ret ~= true then
    if ret == -1 then
      minibuf_error (string.format ("Error writing `%s': %s", filename, err))
    else
      minibuf_error (string.format ("Error writing `%s'", filename))
    end
    return false
  end

  return true
end

local function write_buffer (bp, needname, confirm, name, prompt)
  local ans = true
  local ok = leT

  if needname then
    name = minibuf_read_filename (prompt, "")
    if not name then
      return execute_function ("keyboard-quit")
    end
    if name == "" then
      return leNIL
    end
    confirm = true
  end

  if confirm and exist_file (name) then
    ans = minibuf_read_yn (string.format ("File `%s' exists; overwrite? (y or n) ", name))
    if ans == -1 then
      execute_function ("keyboard-quit")
    elseif ans == false then
      minibuf_error ("Canceled")
    end
    if ans ~= true then
      ok = leNIL
    end
  end

  if ans == true then
    if name ~= bp.filename then
      set_buffer_names (bp, name)
    end
    bp.needname = false
    bp.temporary = false
    bp.nosave = false
    if write_to_disk (bp, name) then
      minibuf_write ("Wrote " .. name)
      bp.modified = false
      undo_set_unchanged (bp.last_undop)
    else
      ok = leNIL
    end
  end

  return ok
end

local function save_buffer (bp)
  if not bp.modified then
    minibuf_write ("(No changes need to be saved)")
    return leT
  else
    return write_buffer (bp, bp.needname, false, bp.filename, "File to save in: ")
  end
end

Defun ("save-buffer",
       {},
[[
Save current buffer in visited file if modified. By default, makes the
previous version into a backup file if this is the first save.
]],
  true,
  function ()
    return save_buffer (cur_bp)
  end
)

Defun ("write-file",
       {},
[[
Write current buffer into file @i{filename}.
This makes the buffer visit that file, and marks it as not modified.

Interactively, confirmation is required unless you supply a prefix argument.
]],
  true,
  function ()
    return write_buffer (cur_bp, true,
                         _interactive and not lastflag.set_uniarg,
                         nil, "Write file: ")
  end
)

local function save_some_buffers ()
  local none_to_save = true
  local noask = false

  for _, bp in ripairs (buffers) do
    if bp.modified and not bp.nosave then
      local fname = get_buffer_filename_or_name (bp)

      none_to_save = false

      if noask then
        save_buffer (bp)
      else
        while true do
          minibuf_write (string.format ("Save file %s? (y, n, !, ., q) ", fname))
          local c = getkey (GETKEY_DEFAULT)
          minibuf_clear ()

          if c == KBD_CANCEL then -- C-g
            execute_function ("keyboard-quit")
            return false
          elseif c == string.byte ('q') then
            bp = nil
            break
          elseif c == string.byte ('.') then
            save_buffer (bp)
            return true
          elseif c == string.byte ('!') then
            noask = true
          end
          if c == string.byte ('!') or c == string.byte (' ') or c == string.byte ('y') then
            save_buffer (bp)
          end
          if c == string.byte ('!') or c == string.byte (' ') or c == string.byte ('y') or c == string.byte ('n') or c == KBD_RET or c == KBD_DEL then
            break
          else
            minibuf_error ("Please answer y, n, !, . or q.")
            waitkey (WAITKEY_DEFAULT)
          end
        end
      end
    end
  end

  if none_to_save then
    minibuf_write ("(No files need saving)")
  end

  return true
end

Defun ("save-some-buffers",
       {},
[[
Save some modified file-visiting buffers.  Asks user about each one.
]],
  true,
  function ()
    return bool_to_lisp (save_some_buffers ())
  end
)

Defun ("save-buffers-kill-emacs",
       {},
[[
Offer to save each buffer, then kill this Zile process.
]],
  true,
  function ()
    if not save_some_buffers () then
      return leNIL
    end

    for _, bp in ipairs (buffers) do
      if bp.modified and not bp.needname then
        while true do
          local ans = minibuf_read_yesno ("Modified buffers exist; exit anyway? (yes or no) ")
          if ans == nil then
            return execute_function ("keyboard-quit")
          elseif not ans then
            return leNIL
          end
          break -- We have found a modified buffer, so stop.
        end
      end
    end

    thisflag.quit = true
  end
)

Defun ("cd",
       {"string"},
[[
Make DIR become the current buffer's default directory.
]],
  true,
  function (dir)
    if not dir and _interactive then
      dir = minibuf_read_filename ("Change default directory: ", cur_bp.dir)
    end

    if not dir then
      return execute_function ("keyboard-quit")
    end

    if dir ~= "" then
      local st = posix.stat (dir)
      if not st or not st.type == "directory" then
        minibuf_error (string.format ("`%s' is not a directory", dir))
      elseif posix.chdir (dir) == -1 then
        minibuf_write (string.format ("%s: %s", dir, posix.errno ()))
      else
        cur_bp.dir = dir
        return true
      end
    end
  end
)

Defun ("insert-buffer",
       {"string"},
[[
Insert after point the contents of BUFFER.
Puts mark after the inserted text.
]],
  true,
  function (buffer)
    local ok = leT

    local def_bp = buffers[#buffers]
    for i = 2, #buffers do
      if buffers[i] == cur_bp then
        def_bp = buffers[i - 1]
        break
      end
    end

    if warn_if_readonly_buffer () then
      return leNIL
    end

    if not buffer then
      local cp = make_buffer_completion ()
      buffer = minibuf_read (string.format ("Insert buffer (default %s): ", def_bp.name),
                             "", cp, buffer_name_history)
      if not buffer then
        ok = execute_function ("keyboard-quit")
      end
    end

    if ok == leT then
      local bp

      if buffer and buffer ~= "" then
        bp = find_buffer (buffer)
        if not bp then
          minibuf_error (string.format ("Buffer `%s' not found", buffer))
          ok = leNIL
        end
      else
        bp = def_bp
      end

      insert_buffer (bp)
      set_mark_interactive ()
    end

    return ok
  end
)

function find_file (filename)
  for _, bp in ipairs (buffers) do
    if bp.filename == filename then
      switch_to_buffer (bp)
      return true
    end
  end

  if exist_file (filename) and not is_regular_file (filename) then
    minibuf_error ("File exists but could not be read")
    return false
  end

  local bp = buffer_new ()
  set_buffer_names (bp, filename)

  switch_to_buffer (bp)

  if insert_file (filename) then
    if not check_writable (filename) then
      cur_bp.readonly = true
    end
    buffer_set_eol_type (cur_bp)

    -- Reset undo history
    cur_bp.next_undop = nil
    cur_bp.last_undop = nil
  end

  bp.modified = false
  bp.dir = posix.dirname (filename)
  posix.chdir (bp.dir) -- FIXME: Call cd instead of last two lines

  thisflag.need_resync = true

  return true
end

-- Function called on unexpected error or Zile crash (SIGSEGV).
-- Attempts to save modified buffers.
-- If doabort is true, aborts to allow core dump generation;
-- otherwise, exit.
function zile_exit (doabort)
  io.stderr:write ("Trying to save modified buffers (if any)...\r\n")

  for _, bp in ipairs (buffers) do
    if bp.modified and not bp.nosave then
      local buf, as = ""
      local i
      local fname = bp.filename or bp.name
      buf = fname .. string.upper (PACKAGE) .. "SAVE"
      io.stderr:write (string.format ("Saving %s...\r\n", buf))
      raw_write_to_disk (bp, buf, "rw-------")
    end
  end

  if doabort then
    posix.abort ()
  else
    posix._exit (2)
  end
end
