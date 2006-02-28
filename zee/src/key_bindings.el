# Default key bindings

# Navigation. Mostly arrows and things.
# Ideally everything should have a home keys alternative, but this is
# currently not true.

# Character/line
bind_key "LEFT" edit_navigate_backward_char
bind_key "RIGHT" edit_navigate_forward_char
bind_key "DOWN" edit_navigate_down_line
bind_key "UP" edit_navigate_up_line
# Word/paragraph
bind_key "C-LEFT" backward_word
bind_key "C-RIGHT" forward_word
bind_key "M-{" backward_paragraph
bind_key "M-}" forward_paragraph
# Line/page
bind_key "HOME" beginning_of_line
bind_key "END" end_of_line
bind_key "PGUP" scroll_down
bind_key "PGDN" scroll_up
# Whole buffer
bind_key "C-HOME" beginning_of_buffer
bind_key "C-END" end_of_buffer

# Selection. The goal is that any navigation command can be combined with
# SHIFT to form a selection command.

# FIXME: Not done yet.

# Absolute navigation. These are like navigation commands but they
# don't become selection commands when combined with SHIFT.

bind_key "C-M-g" goto_column
bind_key "M-g" goto_line

# Open, save, close, quit etc.
# Save
bind_key "C-s" file_save
# Quit
bind_key "C-q" file_quit

# Undo, cut, copy, paste, delete etc.

# Undo
bind_key "C-_" undo
bind_key "C-z" undo
# Cut selection to clipboard.
bind_key "C-x" kill_region
bind_key "M-DEL" kill_word
# Copy selection to clipboard.
bind_key "C-c" copy
# Delete without modifying clipboard.
bind_key "BS" backward_delete_char
bind_key "M-BS" backward_kill_word
bind_key "DEL" delete_char
# Paste.
bind_key "C-v" paste

# Search

# Once the search is underway, "find next" is hard-wired to C-s.
# Having it hard-wired is obviously broken, but something neutral like RET
# would be better.
# The proposed meaning of ESC obviates the current behaviour of RET.
bind_key "C-f" isearch_forward

#bind_key "ESC" cancel
bind_key "C-g" cancel

# Insert special characters.

bind_key "C-RET" newline
bind_key "RET" newline_and_indent
bind_key "TAB" indent_relative
# quoted_insert needs a new shortcut? Was C-q.

# Rare commands.
# SUGGESTION: remove these from the default keymap?

bind_key "M-m" back_to_indentation
bind_key "M-e" call_last_kbd_macro
bind_key "M-c" capitalize_word
bind_key "M-l" downcase_word
bind_key "M-)" end_kbd_macro
bind_key "M-q" fill_paragraph
bind_key "C-r" isearch_backward
bind_key "M-h" mark_paragraph
bind_key "M-@" mark_word
bind_key "C-j" newline
bind_key "M-%" query_replace
bind_key "C-l" recenter
bind_key "C-@" set_mark
bind_key "M-!" shell_command
bind_key "M-(" start_kbd_macro
bind_key "M-i" tab_to_tab_stop
bind_key "C-u" universal_argument
bind_key "M-u" upcase_word
