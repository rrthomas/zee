-- Regression:
-- crash after accepting an empty argument to insert

-- insert RET
-- file-save file-quit
call_command ("macro-play", "\\M-xinsert\\r\\C-g\\C-x\\C-s\\C-x\\C-c")
