-- help-key \C-f other-window edit-select-on edit-goto-line 2 RET edit-copy
-- other-window edit-paste file-save file-quit
call_command ("macro-play", "\\M-xhelp-key\\r\\C-f\\C-xo\\C-@\\M-gg2\\r\\M-w\\C-xo\\C-y\\C-x\\C-s\\C-x\\C-c")
