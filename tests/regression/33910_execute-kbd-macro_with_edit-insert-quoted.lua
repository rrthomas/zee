-- FIXME: Although F1 usually outputs ^[OP, this test will fail
--        when the terminal outputs something else instead.

-- edit-insert-quoted F1 Return
-- file-save file-quit
call_command ("macro-play", "Ctrl-Alt-q", "F1", "Return", "Ctrl-s", "Ctrl-q")
