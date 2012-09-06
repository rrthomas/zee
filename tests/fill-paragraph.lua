-- FIXME: Next line should not be needed
call_command ("setq", "sentence-end-double-space", "nil")
call_command ("end-of-line")
call_command ("insert", " I am now going to make the line sufficiently long that I can wrap it.")
call_command ("fill-paragraph")
call_command ("save-buffer")
call_command ("save-buffers-kill-emacs")
