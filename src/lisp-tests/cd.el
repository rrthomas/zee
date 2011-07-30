; execute-extended-command "cd" ENTER ENTER
; save-buffer save-buffers-kill-emacs
(set-variable 'isearch-nonincremental-instead nil)
(execute-kbd-macro "\M-xcd\r\r\C-x\C-s\C-x\C-c")
