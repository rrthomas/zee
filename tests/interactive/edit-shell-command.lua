-- edit-select-on edit-goto-line 5 Return shell-command-on-region "sort" Return
-- file-save file-quit
call_command ("macro-play", "Ctrl-@", "Alt-g", "5", "Return", "Alt-|", "s", "o", "r", "t", "Return", "Alt-s", "Ctrl-Alt-q")
