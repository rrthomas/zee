-- Regression:
-- crash after accepting an empty argument to insert

-- insert RET
-- save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\M-xinsert\\r\\C-g\\C-x\\C-s\\C-x\\C-c")
