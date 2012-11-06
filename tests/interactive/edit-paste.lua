-- edit-select-on edit-goto-line 3 Return edit-cut move-end-file edit-paste
-- file-save file-quit
call_command ("macro-play", "Ctrl-@", "Ctrl-Alt-g", "3", "Return", "Ctrl-x", "Ctrl-End", "Ctrl-v", "Ctrl-s", "Ctrl-q")
