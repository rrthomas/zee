call_command ("preferences-toggle-read-only")
-- FIXME: The next line causes an error in Emacs so the test fails
--(insert "aaa")
call_command ("preferences-toggle-read-only")
call_command ("insert", "a")
call_command ("file-save")
call_command ("file-quit")
