-- search-forward "ra" isearch-forward ENTER 2 isearch-forward isearch-forward ENTER 3
-- file-save file-quit
call_command ("macro-play", "Ctrl-f", "r", "a", "Ctrl-f", "Return", "2", "Ctrl-f", "Ctrl-f", "Return", "3", "Ctrl-s", "Ctrl-q")
