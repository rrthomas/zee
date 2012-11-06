-- macro-record foo Return macro-stop call-last-kbd-macro undo
-- file-save file-quit
call_command ("macro-play", "Ctrl-Alt-x", "m", "a", "c", "r", "o", "-", "r", "e", "c", "o", "r", "d", "Return", "f", "o", "o", "Return", "Ctrl-Alt-x", "m", "a", "c", "r", "o", "-", "s", "t", "o", "p", "Return", "Alt-m", "Ctrl-_", "Ctrl-s", "Ctrl-q")
