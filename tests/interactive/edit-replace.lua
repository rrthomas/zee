-- query-replace l e RET b d RET ! file-save file-quit
call_command ("macro-play", "\\M-%le\\rbd\\r!\\M-s\\C-\\M-q")
