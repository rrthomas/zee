-- shell-command "echo foo" RET save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\C-u\\M-!echo foo\\r\\C-x\\C-s\\C-x\\C-c")
