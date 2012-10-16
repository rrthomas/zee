-- edit-select-on edit-goto-line 3 Return edit-cut move-end-file edit-paste
-- file-save file-quit
call_command ("macro-play", "Ctrl-@", "Alt-g", "3", "Return", "Ctrl-w", "Alt->", "Ctrl-y", "Alt-s", "Ctrl-Alt-q")
