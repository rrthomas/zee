-- edit-insert-newline-and-indent edit-goto-line 3 Return
-- edit-insert-newline-and-indent edit-goto-line 5 Return
-- edit-insert-newline-and-indent edit-insert-newline-and-indent file-save file-quit
call_command ("macro-play", "Ctrl-j", "Alt-g", "3", "Return", "Ctrl-j", "Alt-g", "5", "Return", "Ctrl-j", "Ctrl-j", "Alt-s", "Ctrl-Alt-q")
