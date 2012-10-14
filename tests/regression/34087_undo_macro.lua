-- macro-record foo RET macro-stop call-last-kbd-macro undo
-- file-save file-quit
call_command ("macro-play", "M-x", "m", "a", "c", "r", "o", "-", "r", "e", "c", "o", "r", "d", "RET", "f", "o", "o", "RET", "M-x", "m", "a", "c", "r", "o", "-", "s", "t", "o", "p", "RET", "M-m", "C-_", "M-s", "C-M-q")
