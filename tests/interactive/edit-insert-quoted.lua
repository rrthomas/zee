-- edit-insert-quoted C-a edit-insert-quoted C-h edit-insert-quoted C-z edit-insert-quoted C-q
-- edit-insert-quoted ESC O P edit-insert-quoted TAB file-save file-quit
call_command ("macro-play", "\\C-q\\C-a\\C-q\\C-h\\C-q\\C-z\\C-q\\C-q\\C-q\\eOP\\C-q\\t\\M-s\\C-\\M-q")
