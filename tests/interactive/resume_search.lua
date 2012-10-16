-- search-forward "ra" isearch-forward ENTER 2 isearch-forward isearch-forward ENTER 3
-- file-save file-quit
call_command ("macro-play", "Ctrl-s", "r", "a", "Ctrl-s", "Return", "2", "Ctrl-s", "Ctrl-s", "Return", "3", "Alt-s", "Ctrl-Alt-q")
