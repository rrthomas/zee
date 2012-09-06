-- FIXME: Use next-line!
call_command ("forward-line", "3")
call_command ("insert", "a")
call_command ("save-buffer")
call_command ("save-buffers-kill-emacs")
