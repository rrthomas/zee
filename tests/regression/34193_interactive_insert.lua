-- Regression:
-- crash after accepting an empty argument to insert

-- insert RET
-- file-save file-quit
macro_play ("M-x", "i", "n", "s", "e", "r", "t", "RET", "C-g", "M-s", "C-M-q")
