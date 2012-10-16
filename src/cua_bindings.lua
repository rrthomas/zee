-- CUA key bindings

-- Command execution
key_bind("Alt-x", "execute-command")
key_bind("Ctrl-u", "edit-repeat")
key_bind("Alt-s", "edit-shell-command")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind("LEFT", "move-previous-character")
key_bind("RIGHT", "move-next-character")
key_bind("UP", "move-previous-line")
key_bind("DOWN", "move-next-line")
-- Word/paragraph
key_bind("Ctrl-LEFT", "move-previous-word")
key_bind("Ctrl-RIGHT", "move-next-word")
key_bind("Alt-{", "move-previous-paragraph")
key_bind("Alt-}", "move-next-paragraph")
-- Line/page
key_bind("HOME", "move-start-line")
key_bind("END", "move-end-line")
key_bind("PGUP", "move-previous-page")
key_bind("PGDN", "move-next-page")
-- Whole buffer
key_bind("Ctrl-HOME", "move-start-file")
key_bind("Ctrl-END", "move-end-file")
-- Window
key_bind("Ctrl-l", "view-refresh")

-- Selection
-- select-other-end
key_bind("Ctrl-@", "edit-select-toggle")

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.

key_bind("Ctrl-Alt-g", "edit-goto-column")
key_bind("Alt-g", "edit-goto-line")

-- Save
key_bind("Ctrl-s", "file-save")
-- Quit
key_bind("Ctrl-q", "file-quit")

-- Undo
key_bind("Ctrl-z", "edit-undo")
-- Cut selection to clipboard
key_bind("Ctrl-x", "edit-cut")
key_bind("Alt-DEL", "edit-delete-word")
-- Copy selection to clipboard
key_bind("Ctrl-c", "edit-copy")
-- Delete without modifying clipboard")
key_bind("BS", "edit-delete-previous-character")
key_bind("Alt-BS", "edit-delete-word-backward")
key_bind("DEL", "edit-delete-next-character")
-- Paste
key_bind("Ctrl-v", "edit-paste")

-- Search
key_bind("Ctrl-f", "edit-find")
key_bind("Ctrl-Alt-f", "edit-find-backward")
key_bind("Ctrl-r", "edit-replace")

-- Insert special characters
key_bind("Ctrl-Return", "edit-insert-newline")
key_bind("Return", "edit-insert-newline-and-indent")
key_bind("Alt-i", "edit-insert-quoted")

-- Macros
key_bind("Alt-(", "macro-record")
key_bind("Alt-)", "macro-stop")
key_bind("Alt-e", "macro-play")
