-- describe-bindings other-window edit-select-on edit-goto-line 2 RET edit-copy
-- other-window edit-paste file-save file-quit
call_command ("macro-play", "\\M-xdescribe-bindings\\r\\C-xo\\C-@\\M-gg2\\r\\ew\\C-xo\\C-y\\C-x\\C-s\\C-x\\C-c")
