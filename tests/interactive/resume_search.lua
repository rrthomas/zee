-- search-forward "ra" isearch-forward ENTER 2 isearch-forward isearch-forward ENTER 3
-- file-save file-quit
call_command ("preferences-set-variable", "isearch-nonincremental-instead", "nil")
call_command ("macro-play", "\\C-sra\\C-s\\r2\\C-s\\C-s\\r3\\M-s\\C-\\M-q")
