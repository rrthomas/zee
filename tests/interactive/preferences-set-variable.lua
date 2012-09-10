-- preferences-set-variable "kill-whole-line" RET t RET
-- edit-kill-line file-save file-quit
call_command ("macro-play", "\\M-xpreferences-set-variable\\rkill-whole-line\\rt\\r\\C-k\\C-x\\C-s\\C-x\\C-c")
