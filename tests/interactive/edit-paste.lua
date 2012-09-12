-- edit-select-on edit-goto-line 3 RET edit-kill-selection move-end-file edit-paste
-- file-save file-quit
call_command ("macro-play", "\\C-@\\M-g3\\r\\C-w\\M->\\C-y\\M-s\\C-\\M-q")
