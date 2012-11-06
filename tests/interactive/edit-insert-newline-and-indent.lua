-- edit-insert-newline-and-indent edit-goto-line 3 Return
-- edit-insert-newline-and-indent edit-goto-line 5 Return
-- edit-insert-newline-and-indent edit-insert-newline-and-indent file-save file-quit
call_command ("macro-play", "Return", "Ctrl-Alt-g", "3", "Return", "Return", "Ctrl-Alt-g", "5", "Return", "Return", "Return", "Ctrl-s", "Ctrl-q")
