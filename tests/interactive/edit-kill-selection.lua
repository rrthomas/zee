-- edit-select-on edit-goto-line 3 RET edit-kill-selection
-- file-save file-quit
call_command ("macro-play", "\\C-@\\M-gg3\\r\\C-w\\C-x\\C-s\\C-x\\C-c")
