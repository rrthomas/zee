-- FIXME: Next line should not be needed
call_command ("setq", "sentence-end-double-space", "nil")

-- edit-select-on edit-goto-line 3 RET edit-kill-selection edit-paste edit-paste open-line
-- edit-goto-line 6 RET edit-paste edit-goto-line 9 RET
call_command ("macro-play", "\\C-@\\M-gg3\\r\\C-w\\C-y\\C-y\\C-o\\M-gg6\\r\\C-y\\M-gg9\\r")

-- ESC 3 set-fill-column fill-paragraph ESC -3 edit-goto-line 6 RET
-- ESC 12 set-fill-column fill-paragraph ESC -3 edit-goto-line 2 RET move-previous-character
-- ESC 33 set-fill-column fill-paragraph file-save file-quit
call_command ("macro-play", "\\e3\\C-xf\\M-q\\M-gg6\\r\\e12\\C-xf\\M-q\\M-gg2\\r\\C-b\\e33\\C-xf\\M-q\\C-x\\C-s\\C-x\\C-c")
