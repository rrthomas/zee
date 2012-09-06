-- set-mark goto-line 5 RET shell-command-on-region "sort" RET
-- save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\C-@\\M-gg5\\r\\C-u\\M-|sort\\r\\C-x\\C-s\\C-x\\C-c")
