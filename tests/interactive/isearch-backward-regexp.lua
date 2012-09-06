call_command ("forward-line", "2")
-- isearch-backward-regexp e . e RET f save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\C-\\M-re.e\\rf\\C-x\\C-s\\C-x\\C-c")
