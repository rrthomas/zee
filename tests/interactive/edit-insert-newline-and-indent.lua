-- edit-insert-newline-and-indent edit-goto-line 3 RET
-- edit-insert-newline-and-indent edit-goto-line 5 RET
-- edit-insert-newline-and-indent edit-insert-newline-and-indent file-save file-quit
call_command ("macro-play", "C-j", "M-g", "3", "RET", "C-j", "M-g", "5", "RET", "C-j", "C-j", "M-s", "C-M-q")
