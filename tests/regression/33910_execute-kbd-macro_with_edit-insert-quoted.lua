-- FIXME: Although F1 usually outputs ^[OP, this test will fail
--        when the terminal outputs something else instead.

-- edit-insert-quoted F1 RETURN
-- file-save file-quit
call_command ("macro-play", "\\C-q\\F1\\RET\\M-s\\C-\\M-q")
