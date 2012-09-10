-- ESC 1 ESC ! "echo foo" \M-x RET
-- file-save file-quit
call_command ("macro-play", "\\e1\\e|echo foo\\M-x\\r\\C-x\\C-s\\C-x\\C-c")
