-- query-replace l e RET b d RET ! save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\M-%le\\rbd\\r!\\C-x\\C-s\\C-x\\C-c")
