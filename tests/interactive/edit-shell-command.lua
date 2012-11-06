-- edit-select-on edit-goto-line 5 Return shell-command-on-region "sort" Return
-- file-save file-quit
call_command ("macro-play", "Ctrl-@", "Ctrl-Alt-g", "5", "Return", "Ctrl-Alt-s", "s", "o", "r", "t", "Return", "Ctrl-s", "Ctrl-q")
