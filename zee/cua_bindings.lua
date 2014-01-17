-- CUA key bindings

-- Command execution
key_bind ("Ctrl-Alt-x", "execute-command")
key_bind ("Ctrl-Alt-s", "edit-shell-command")
key_bind ("Ctrl-Alt-z", "file-suspend")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind ("Left", "move-previous-character")
key_bind ("Right", "move-next-character")
key_bind ("Up", "move-previous-line")
key_bind ("Down", "move-next-line")
-- Word/paragraph
key_bind ("Ctrl-Left", "move-previous-word")
key_bind ("Ctrl-Right", "move-next-word")
key_bind ("Alt-[", "move-previous-paragraph") -- FIXME
key_bind ("Alt-]", "move-next-paragraph") -- FIXME
-- Line/page
key_bind ("Home", "move-start-line")
key_bind ("End", "move-end-line")
key_bind ("PageUp", "move-previous-page")
key_bind ("PageDown", "move-next-page")
-- Whole buffer
key_bind ("Ctrl-Home", "move-start-file")
key_bind ("Ctrl-End", "move-end-file")
-- Window
key_bind ("Ctrl-l", "view-refresh")

-- Selection
key_bind ("Ctrl-@", "edit-select-toggle")
key_bind ("Alt-Space", "edit-select-other-end") -- FIXME

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.
key_bind ("Ctrl-Alt-g", "move-goto-line")
key_bind ("Ctrl-Alt-c", "move-goto-column")

-- Save
key_bind ("Ctrl-s", "file-save")
-- Quit
key_bind ("Ctrl-q", "file-quit")

-- Undo
key_bind ("Ctrl-z", "edit-undo")
-- Cut selection to clipboard
key_bind ("Ctrl-x", "edit-cut")
key_bind ("Ctrl-Delete", "edit-delete-word")
-- Copy selection to clipboard
key_bind ("Ctrl-c", "edit-copy")
-- Delete without modifying clipboard
key_bind ("Backspace", "edit-delete-previous-character")
key_bind ("Ctrl-?", "edit-delete-previous-character")
key_bind ("Ctrl-Backspace", "edit-delete-word-backward")
key_bind ("Ctrl-Alt-?", "edit-delete-word-backward")
key_bind ("Delete", "edit-delete-next-character")
key_bind ("Ctrl-Alt-d", "edit-delete-horizontal-space")
-- Paste
key_bind ("Ctrl-v", "edit-paste")

-- Search
key_bind ("Ctrl-f", "edit-find")
key_bind ("Ctrl-Alt-f", "edit-find-backward")
key_bind ("Ctrl-r", "edit-replace")

-- Insert special characters
key_bind ("Alt-Tab", "edit-indent") -- FIXME: Should be Ctrl-Alt-i
key_bind ("Ctrl-Alt-q", "edit-insert-quoted")
key_bind ("Tab", "edit-indent-relative")
key_bind ("Ctrl-Return", "edit-insert-newline") -- FIXME: binding doesn't work
key_bind ("Return", "edit-insert-newline-and-indent")

-- Macros
key_bind ("Ctrl-(", "macro-record")
key_bind ("Ctrl-)", "macro-stop")
key_bind ("Alt-Return", "macro-play") -- FIXME
