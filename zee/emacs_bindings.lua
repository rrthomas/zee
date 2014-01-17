-- Emacs key bindings

-- Command execution
key_bind ("Alt-x", "execute-command")
key_bind ("Alt-!", "edit-shell-command")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind ("Left", "move-previous-character")
key_bind ("Ctrl-b", "move-previous-character")
key_bind ("Right", "move-next-character")
key_bind ("Ctrl-f", "move-next-character")
key_bind ("Up", "move-previous-line")
key_bind ("Ctrl-p", "move-previous-line")
key_bind ("Down", "move-next-line")
key_bind ("Ctrl-n", "move-next-line")
-- Word/paragraph
key_bind ("Ctrl-Left", "move-previous-word")
key_bind ("Alt-Left", "move-previous-word")
key_bind ("Alt-b", "move-previous-word")
key_bind ("Ctrl-Right", "move-next-word")
key_bind ("Alt-Right", "move-next-word")
key_bind ("Alt-f", "move-next-word")
key_bind ("Alt-{", "move-previous-paragraph")
key_bind ("Alt-}", "move-next-paragraph")
-- Line/page
key_bind ("Home", "move-start-line")
key_bind ("Ctrl-a", "move-start-line")
key_bind ("End", "move-end-line")
key_bind ("Ctrl-e", "move-end-line")
key_bind ("PageUp", "move-previous-page")
key_bind ("Alt-v", "move-previous-page")
key_bind ("PageDown", "move-next-page")
key_bind ("Ctrl-v", "move-next-page")
-- Whole buffer
key_bind ("Ctrl-Home", "move-start-file")
key_bind ("Alt-<", "move-start-file")
key_bind ("Ctrl-End", "move-end-file")
key_bind ("Alt->,", "move-end-file")
-- Window
key_bind ("Ctrl-l", "view-refresh")

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.
key_bind ("Ctrl-Alt-g", "move-goto-column")
key_bind ("Alt-g", "move-goto-line")

-- Selection
-- select-other-end
key_bind ("Ctrl-@", "edit-select-toggle")

-- Save
key_bind ("Alt-s", "file-save")
-- Suspend
key_bind ("Ctrl-z", "file-suspend")
-- Quit
key_bind ("Ctrl-Alt-q", "file-quit")

-- Undo
key_bind ("Ctrl-_", "edit-undo")
-- Cut selection to clipboard
key_bind ("Ctrl-w", "edit-cut")
key_bind ("Alt-d", "edit-delete-word")
-- Copy selection to clipboard
key_bind ("Alt-w", "edit-copy")
-- Delete without modifying clipboard
key_bind ("Backspace", "edit-delete-previous-character")
key_bind ("Ctrl-d", "edit-delete-next-character")
key_bind ("Delete", "edit-delete-next-character")
key_bind ("Alt-\\", "edit-delete-horizontal-space")
-- Paste
key_bind ("Ctrl-y", "edit-paste")

-- Search
key_bind ("Ctrl-s", "edit-find")
key_bind ("Ctrl-r", "edit-find-backward")
key_bind ("Alt-%", "edit-replace")

-- Insert special characters
key_bind ("Ctrl-j", "edit-insert-newline")
key_bind ("Return", "edit-insert-newline-and-indent")
key_bind ("Tab", "edit-indent-relative")
key_bind ("Ctrl-q", "edit-insert-quoted")
key_bind ("Alt-i", "edit-indent")

-- Macros
key_bind ("Alt-(", "macro-record")
key_bind ("Alt-)", "macro-stop")
key_bind ("Alt-e", "macro-play")
