-- forward-word SPC SPC SPC just-one-space insert "a"
-- save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\M-f   \\M- a\\C-x\\C-s\\C-x\\C-c")
