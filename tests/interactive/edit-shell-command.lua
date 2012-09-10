-- edit-select-on edit-goto-line 5 RET shell-command-on-region "sort" RET
-- file-save file-quit
call_command ("macro-play", "\\C-@\\M-gg5\\r\\C-u\\M-|sort\\r\\C-x\\C-s\\C-x\\C-c")
