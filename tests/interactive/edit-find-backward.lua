call_command ("move-next-line")
call_command ("move-next-line")
-- edit-find-backward e . e RET f file-save file-quit
call_command ("macro-play", "C-r", "e", ".", "e", "RET", "f", "M-s", "C-M-q")
