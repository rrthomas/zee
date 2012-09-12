-- ESC 1 ESC ! "echo foo" \M-x RET
-- file-save file-quit
call_command ("macro-play", "\\e1\\e|echo foo\\M-x\\r\\M-s\\C-\\M-q")
