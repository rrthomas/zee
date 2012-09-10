-- FIXME: Use move-next-line!
call_command ("forward-line", "3")
call_command ("insert", "a")
call_command ("file-save")
call_command ("file-quit")
