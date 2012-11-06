-- Regression:
-- crash after accepting an empty argument to insert

-- insert Return
-- file-save file-quit
call_command ("macro-play", "Ctrl-Alt-x", "i", "n", "s", "e", "r", "t", "Return", "Ctrl-g", "Ctrl-s", "Ctrl-q")
