-- edit-goto-line 2 RET move-next-word move-next-word move-next-word move-start-line a
-- file-save file-quit
call_command ("macro-play", "\\M-gg2\\r\\M-f\\M-f\\M-f\\C-aa\\C-x\\C-s\\C-x\\C-c")
