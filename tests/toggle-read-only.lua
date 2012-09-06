call_command ("toggle-read-only")
-- FIXME: The next line causes an error in Emacs so the test fails
--(insert "aaa")
call_command ("toggle-read-only")
call_command ("insert", "a")
call_command ("save-buffer")
call_command ("save-buffers-kill-emacs")
