call_command ("forward-line", "2")
-- isearch-backward l C-r RET f save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\C-rl\\C-r\\rf\\C-x\\C-s\\C-x\\C-c")
