-- edit-goto-line 2 Return move-next-word move-next-word move-next-word move-start-line a
-- file-save file-quit
call_command ("macro-play", "Ctrl-Alt-g", "2", "Return", "Ctrl-Right", "Ctrl-Right", "Ctrl-Right", "Home", "a", "Ctrl-s", "Ctrl-q")
