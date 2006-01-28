# Default key bindings

# Navigation. Mostly arrows and things.
# Ideally everything should have a home-keys alternative, but this is
# currently not true.

# Character/line
bind-key "LEFT" edit-navigate-backward-char
bind-key "RIGHT" edit-navigate-forward-char
bind-key "DOWN" edit-navigate-down-line
bind-key "UP" edit-navigate-up-line
# Word/paragraph
bind-key "C-LEFT" backward-word
bind-key "C-RIGHT" forward-word
bind-key "M-{" backward-paragraph
bind-key "M-}" forward-paragraph
# Line/page
bind-key "HOME" beginning-of-line
bind-key "END" end-of-line
bind-key "PGUP" scroll-down
bind-key "PGDN" scroll-up
# Whole buffer
bind-key "C-HOME" beginning-of-buffer
bind-key "C-END" end-of-buffer

# Selection. The goal is that any navigation command can be combined with
# SHIFT to form a selection command.

# FIXME: Not done yet.

# Absolute navigation. These are like navigation commands but they
# don't become selection commands when combined with SHIFT.

bind-key "C-M-g" goto-char
bind-key "M-g" goto-line

# Open, save, close, quit etc.
# Save
bind-key "C-s" file-save
# Quit
bind-key "C-q" file-quit

# Undo, cut, copy, paste, delete etc.

# Undo
bind-key "C--" undo
bind-key "C-z" undo
# Cut selection to clipboard.
bind-key "C-x" kill-region
bind-key "M-DEL" kill-word
# Copy selection to clipboard.
bind-key "C-c" copy-region
# Delete without modifying clipboard.
bind-key "BS" backward-delete-char
bind-key "M-BS" backward-kill-word
bind-key "DEL" delete-char
# Paste.
bind-key "C-v" yank

# Search

# Once the search is underway, "find next" is hard-wired to C-i.
# Having it hard-wired is obviously broken, but something neutral like RET
# would be better.
# The proposed meaning of ESC obviates the current behaviour of RET.
bind-key "C-f" isearch-forward-regexp

bind-key "ESC" keyboard-quit
bind-key "C-g" keyboard-quit

# Insert special characters.

bind-key "C-RET" newline
bind-key "RET" newline-and-indent
bind-key "TAB" indent-relative
# quoted-insert needs a new shortcut? Was C-q.

# Rare commands.
# SUGGESTION: remove these from the default keymap?

bind-key "M-m" back-to-indentation
bind-key "C-M-b" backward-sexp
bind-key "M-e" call-last-kbd-macro
bind-key "M-c" capitalize-word
bind-key "M-\" delete-horizontal-space
bind-key "M- " just-one-space
bind-key "M-l" downcase-word
bind-key "M-)" end-kbd-macro
bind-key "M-q" fill-paragraph
bind-key "C-M-f" forward-sexp
bind-key "C-r" isearch-backward-regexp
bind-key "C-M-k" kill-sexp
bind-key "M-h" mark-paragraph
bind-key "C-M-@" mark-sexp
bind-key "M-@" mark-word
bind-key "C-j" newline
bind-key "M-%" query-replace-regexp
bind-key "C-l" recenter
bind-key "C-@" set-mark-command
bind-key "M-!" shell-command
bind-key "M-(" start-kbd-macro
bind-key "M-i" tab-to-tab-stop
bind-key "C-u" universal-argument
bind-key "M-u" upcase-word
