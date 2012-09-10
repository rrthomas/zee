-- edit-goto-line 1 RET indent-for-tab-command
-- edit-goto-line 2 RET indent-for-tab-command
-- edit-goto-line 4 RET indent-for-tab-command
-- indent-for-tab-command file-save file-quit
call_command ("macro-play", "\\M-gg1\\r\\t\\M-gg2\\r\\t\\M-gg4\\r\\t\\C-x\\C-s\\C-x\\C-c")
