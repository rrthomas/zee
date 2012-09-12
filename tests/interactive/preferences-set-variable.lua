-- preferences-set-variable "case-fold-search" RET nil RET
-- edit-find "I" RET a file-save file-quit
call_command ("macro-play", "\\M-xpreferences-set-variable\\rcase-fold-search\\rnil\\r\\C-sI\\ra\\M-s\\C-\\M-q")
