(describe-key "\C-f")
(other-window)
(set-mark-command)
(next-line)
(kill-region)
(other-window)
(yank)
(save-buffer)
(save-buffers-kill-emacs)
