# Default key bindings

# Navigation. Mostly arrows and things.
# Ideally everything should have a home-keys alternative, but this is
# currently not true.

# Character/line
set-key "LEFT" edit-navigate-backward-char
set-key "RIGHT" edit-navigate-forward-char
set-key "DOWN" edit-navigate-down-line
set-key "UP" edit-navigate-up-line
# Word/paragraph
set-key "C-LEFT" backward-word
set-key "C-RIGHT" forward-word
set-key "M-{" backward-paragraph
set-key "M-}" forward-paragraph
# Line/page
set-key "HOME" beginning-of-line
set-key "END" end-of-line
set-key "PGUP" scroll-down
set-key "PGDN" scroll-up
# Whole buffer
set-key "C-HOME" beginning-of-buffer
set-key "C-END" end-of-buffer

# Selection. The goal is that any navigation command can be combined with
# SHIFT to form a selection command.

# FIXME: Not done yet.

# Absolute navigation. These are like navigation commands but they
# don't become selection commands when combined with SHIFT.

set-key "C-M-g" goto-char
set-key "M-g" goto-line

# Open, save, close, quit etc.
# Save
set-key "C-s" file-save
# Quit
set-key "C-q" file-quit

# Undo, cut, copy, paste, delete etc.

# Undo
set-key "C--" undo
set-key "C-z" undo
# Cut selection to clipboard.
set-key "C-x" kill-region
set-key "M-DEL" kill-word
# Copy selection to clipboard.
set-key "C-c" copy-region
# Delete without modifying clipboard.
set-key "BS" backward-delete-char
set-key "M-BS" backward-kill-word
set-key "DEL" delete-char
# Paste.
set-key "C-v" yank

# Search

# Once the search is underway, "find next" is hard-wired to C-i.
# Having it hard-wired is obviously broken, but something neutral like RET
# would be better.
# The proposed meaning of ESC obviates the current behaviour of RET.
set-key "C-f" isearch-forward-regexp

set-key "ESC" keyboard-quit
set-key "C-g" keyboard-quit

# Insert special characters.

set-key "C-RET" newline
set-key "RET" newline-and-indent
set-key "TAB" indent-relative
# quoted-insert needs a new shortcut? Was C-q.

# Rare commands.
# SUGGESTION: remove these from the default keymap?

set-key "M-m" back-to-indentation
set-key "C-M-b" backward-sexp
set-key "M-e" call-last-kbd-macro
set-key "M-c" capitalize-word
set-key "M-\" delete-horizontal-space
set-key "M- " just-one-space
set-key "M-l" downcase-word
set-key "M-)" end-kbd-macro
set-key "M-q" fill-paragraph
set-key "C-M-f" forward-sexp
set-key "C-r" isearch-backward-regexp
set-key "C-M-k" kill-sexp
set-key "M-h" mark-paragraph
set-key "C-M-@" mark-sexp
set-key "M-@" mark-word
set-key "C-j" newline
set-key "M-%" query-replace-regexp
set-key "C-l" recenter
set-key "C-@" set-mark-command
set-key "M-!" shell-command
set-key "M-(" start-kbd-macro
set-key "M-i" tab-to-tab-stop
set-key "C-u" universal-argument
set-key "M-u" upcase-word
