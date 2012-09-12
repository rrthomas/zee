-- FIXME: Next line should not be needed
call_command ("setq", "sentence-end-double-space", "nil")

-- move-end-line
-- " I am now going to make the line sufficiently long that I can wrap it."
-- fill-paragraph file-save file-quit
call_command ("macro-play", "\\C-e I am now going to make the line sufficiently long that I can wrap it.\\M-q\\M-s\\C-\\M-q")
