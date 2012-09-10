-- edit-goto-line 3 RET edit-insert-tab edit-insert-tab t a b
-- file-save file-quit)
call_command ("macro-play", "\\M-gg3\\r\\M-i\\M-itab\\C-x\\C-s\\C-x\\C-c")
