-- edit-goto-line 3 RET edit-insert-tab edit-insert-tab t a b
-- file-save file-quit)
call_command ("macro-play", "M-g", "3", "RET", "M-i", "M-i", "t", "a", "b", "M-s", "C-M-q")
