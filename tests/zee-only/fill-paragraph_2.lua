-- FIXME: Next line should not be needed
call_command ("setq", "sentence-end-double-space", "nil")
call_command ("move-end-line")
call_command ("insert", " I am now going to make the line sufficiently long that I can wrap it.")
call_command ("edit-wrap-paragraph")
call_command ("edit-undo")
call_command ("insert", "a")
call_command ("file-save")
call_command ("file-quit")
