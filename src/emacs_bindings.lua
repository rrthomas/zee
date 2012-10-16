-- CUA key bindings

-- Command execution
key_bind("Alt-x", "execute-command")
key_bind("Ctrl-u", "edit-repeat")
key_bind("Alt-!", "edit-shell-command")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind("LEFT", "move-previous-character")
key_bind("Ctrl-b", "move-previous-character")
key_bind("RIGHT", "move-next-character")
key_bind("Ctrl-f", "move-next-character")
key_bind("UP", "move-previous-line")
key_bind("Ctrl-p", "move-previous-line")
key_bind("DOWN", "move-next-line")
key_bind("Ctrl-n", "move-next-line")
-- Word/paragraph
key_bind("Ctrl-LEFT", "move-previous-word")
key_bind("Alt-b", "move-previous-word")
key_bind("Ctrl-RIGHT", "move-next-word")
key_bind("Alt-f", "move-next-word")
-- Line/page
key_bind("HOME", "move-start-line")
key_bind("Ctrl-a", "move-start-line")
key_bind("END", "move-end-line")
key_bind("Ctrl-e", "move-end-line")
key_bind("PGUP", "move-previous-page")
key_bind("Alt-v", "move-previous-page")
key_bind("PGDN", "move-next-page")
key_bind("Ctrl-v", "move-next-page")
-- Whole buffer
key_bind("Ctrl-HOME", "move-start-file")
key_bind("Alt-<", "move-start-file")
key_bind("Ctrl-END", "move-end-file")
key_bind("Alt->," "move-end-file")
-- Window
key_bind("Ctrl-l", "view-refresh")

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.
key_bind("Ctrl-Alt-g", "edit-goto-column")
key_bind("Alt-g", "edit-goto-line")

-- Save
key_bind("Alt-s", "file-save")
-- Suspend
key_bind("Ctrl-z", "file-suspend")
-- Quit
key_bind("Ctrl-Alt-q", "file-quit")

-- Undo
key_bind("Ctrl--", "edit-undo")
-- Cut selection to clipboard
key_bind("Ctrl-k", "edit-cut")
key_bind("Alt-d", "edit-delete-word")
-- Copy selection to clipboard
key_bind("Alt-w", "edit-copy")
-- Delete without modifying clipboard
key_bind("Ctrl-d", "edit-delete-next-character")
-- Paste
key_bind("Ctrl-y", "edit-paste")

-- Search
key_bind("Ctrl-s", "edit-find")
key_bind("Ctrl-r", "edit-find-backward")
key_bind("Alt-%", "edit-replace")

-- Insert special characters
key_bind("Ctrl-Return", "edit-insert-newline")
key_bind("Return", "edit-insert-newline-and-indent")
key_bind("Tab", "indent-relative")
key_bind("Ctrl-q", "edit-insert-quoted")
key_bind("Alt-i", "edit-indent")

-- Macros
key_bind("Alt-(", "macro-record")
key_bind("Alt-)", "macro-stop")
key_bind("Alt-e", "macro-play")
