call_command ("global-set-key", "d", "delete-char")
-- d d d d d save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "ddddd\\C-x\\C-s\\C-x\\C-c")
