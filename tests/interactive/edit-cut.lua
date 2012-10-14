-- edit-select-on edit-goto-line 3 RET edit-cut
-- file-save file-quit
call_command ("macro-play", "C-@", "M-g", "3", "RET", "C-w", "M-s", "C-M-q")
