-- macro-record "foo" RET UP macro-stop
-- prefix-cmd 3 call-last-kbd-macro file-save file-quit
-- FIXME: Get this working with edit-repeat
-- macro_play ("M-x", "m", "a", "c", "r", "o", "-", "r", "e", "c", "o", "r", "d", "RET", "f", "o", "o", "RET", "UP", "M-x", "m", "a", "c", "r", "o", "-", "s", "t", "o", "p", "RET", "ESC", "3", "M-m", "M-s", "C-M-q")
macro_play ("M-x", "m", "a", "c", "r", "o", "-", "r", "e", "c", "o", "r", "d", "RET", "f", "o", "o", "RET", "UP", "M-x", "m", "a", "c", "r", "o", "-", "s", "t", "o", "p", "RET", "M-m", "M-m", "M-m", "M-s", "C-M-q")
