-- FIXME: Although F1 usually outputs ^[OP, this test will fail
--        when the terminal outputs something else instead.

-- edit-insert-quoted F1 ReturnURN
-- file-save file-quit
call_command ("macro-play", "Ctrl-q", "F1", "Return", "Alt-s", "Ctrl-Alt-q")
