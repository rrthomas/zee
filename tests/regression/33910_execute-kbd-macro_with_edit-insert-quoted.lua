-- FIXME: Although F1 usually outputs ^[OP, this test will fail
--        when the terminal outputs something else instead.

-- edit-insert-quoted F1 RETURN
-- file-save file-quit
call_command ("macro-play", "\\C-q\\F1\\RET\\C-x\\C-s\\C-x\\C-c")
