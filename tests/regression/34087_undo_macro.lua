-- macro-record foo RET macro-stop call-last-kbd-macro undo
-- file-save file-quit
call_command ("macro-play", "\\C-x(foo\\r\\C-x)\\M-m\\C-_\\M-s\\C-\\M-q")
