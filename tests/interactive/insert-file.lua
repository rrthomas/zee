-- insert-file "insert-file.input" RET save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\C-xiinsert-file.input\\r\\C-x\\C-s\\C-x\\C-c")
