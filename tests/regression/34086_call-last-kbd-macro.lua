-- macro-record "foo" RET UP macro-stop
-- prefix-cmd 3 call-last-kbd-macro file-save file-quit
call_command ("macro-play", "\\M-xmacro-record\\rfoo\\r\\UP\\M-xmacro-stop\\r\\e3\\M-m\\M-s\\C-\\M-q")
