-- edit-goto-line 3 RET edit-insert-tab edit-insert-tab t a b
-- file-save file-quit)
call_command ("macro-play", "\\M-g3\\r\\M-i\\M-itab\\M-s\\C-\\M-q")
