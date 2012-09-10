-- prefix-argument 2 edit-kill-line edit-goto-line 2 RET edit-paste
-- file-save file-quit
call_command ("macro-play", "\\M-2\\C-k\\M-gg2\\r\\C-y\\C-x\\C-s\\C-x\\C-c")
