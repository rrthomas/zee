-- macro-record open-line foo LEFT LEFT LEFT macro-stop
-- prefix-cmd 3 call-last-kbd-macro file-save file-quit
call_command ("macro-play", "\\C-x(foo\\r\\UP\\C-x)\\e3\\C-xe\\C-x\\C-s\\C-x\\C-c")
