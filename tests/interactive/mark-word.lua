-- mark-word kill-region end-of-line yank save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\M-@\\C-w\\C-e\\C-y\\C-x\\C-s\\C-x\\C-c")
