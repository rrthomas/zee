-- query-replace l e RET b d RET ! file-save file-quit
call_command ("macro-play", "\\M-%le\\rbd\\r!\\C-x\\C-s\\C-x\\C-c")
