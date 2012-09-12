-- edit-insert-newline-and-indent edit-goto-line 3 RET
-- edit-insert-newline-and-indent edit-goto-line 5 RET
-- edit-insert-newline-and-indent edit-insert-newline-and-indent file-save file-quit
call_command ("macro-play", "\\C-j\\M-g3\\r\\C-j\\M-g5\\r\\C-j\\C-j\\M-s\\C-\\M-q")
