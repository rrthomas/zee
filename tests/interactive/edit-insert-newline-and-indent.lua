-- edit-insert-edit-insert-edit-insert-newline-and-indent edit-goto-line 3 RET
-- edit-insert-edit-insert-edit-insert-newline-and-indent edit-goto-line 5 RET
-- edit-insert-edit-insert-edit-insert-newline-and-indent edit-insert-edit-insert-edit-insert-newline-and-indent file-save file-quit
call_command ("macro-play", "\\C-j\\M-gg3\\r\\C-j\\M-gg5\\r\\C-j\\C-j\\C-x\\C-s\\C-x\\C-c")
