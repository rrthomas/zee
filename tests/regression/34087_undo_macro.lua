-- macro-record foo RET macro-stop call-last-kbd-macro undo
-- file-save file-quit
call_command ("macro-play", "\\C-x(foo\\r\\C-x)\\C-xe\\C-_\\C-x\\C-s\\C-x\\C-c")
