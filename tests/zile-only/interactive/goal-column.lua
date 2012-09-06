-- ESC 4 forward-char ESC 3 next-line a save-buffer save-buffers-kill-emacs
call_command ("execute-kbd-macro", "\\e4\\C-f\\e3\\C-na\\C-x\\C-s\\C-x\\C-c")
