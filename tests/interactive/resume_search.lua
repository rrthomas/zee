-- search-forward "ra" isearch-forward ENTER 2 isearch-forward isearch-forward ENTER 3
-- file-save file-quit
call_command ("macro-play", "C-s", "r", "a", "C-s", "RET", "2", "C-s", "C-s", "RET", "3", "M-s", "C-M-q")
