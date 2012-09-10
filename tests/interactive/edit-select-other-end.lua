-- ESC 4 move-next-character edit-select-on forward-line forward-line
-- exchange-point-and-mark f file-save file-quit
call_command ("macro-play", "\\e4\\C-f\\C-@f\\M->\\C-x\\C-x\\C-x\\C-s\\C-x\\C-c")

-- With a direct translation, Emacs exits 255 with 'end of buffer' error
-- for some reason?!
--(macro-play "\e4\C-f\C-@\C-n\C-n\C-x\C-xf\C-x\C-s\C-x\C-c")
