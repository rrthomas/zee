-- edit-select-on edit-goto-line 5 RET shell-command-on-region "sort" RET
-- file-save file-quit
call_command ("macro-play", "C-@", "M-g", "5", "RET", "M-|", "s", "o", "r", "t", "RET", "M-s", "C-M-q")
