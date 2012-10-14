-- query-replace l e RET b d RET ! file-save file-quit
call_command ("macro-play", "M-%", "l", "e", "RET", "b", "d", "RET", "!", "M-s", "C-M-q")
