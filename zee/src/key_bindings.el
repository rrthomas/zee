# Default key bindings

# Navigation. Mostly arrows and things.
# Ideally everything should have a home keys alternative, but this is
# currently not true.

# Character/line
key_bind "LEFT" edit_navigate_previous_character
key_bind "RIGHT" edit_navigate_next_character
key_bind "UP" edit_navigate_previous_line
key_bind "DOWN" edit_navigate_next_line
# Word/paragraph
key_bind "C-LEFT" edit_navigate_previous_word
key_bind "C-RIGHT" edit_navigate_next_word
key_bind "M-{" edit_navigate_previous_paragraph # FIXME Doesn't work!
key_bind "M-}" edit_navigate_next_paragraph # FIXME Doesn't work!
# Line/page
key_bind "HOME" edit_navigate_start_line
key_bind "END" edit_navigate_end_line
key_bind "PGUP" edit_navigate_previous_page
key_bind "PGDN" edit_navigate_next_page
# Whole buffer
key_bind "C-HOME" edit_navigate_start_file
key_bind "C-END" edit_navigate_end_file

# Selection.
# select_other_end
key_bind "M-h" edit_select_paragraph
key_bind "C-@" edit_select_on
key_bind "C-g" edit_select_off
key_bind "M-@" edit_select_word

# Absolute navigation.

key_bind "C-M-g" edit_goto_column
key_bind "M-g" edit_goto_line

# Open, save, close, quit etc.
# Save
key_bind "C-s" file_save
# Quit
key_bind "C-q" file_quit

# Undo, cut, copy, paste, delete etc.

# Undo
key_bind "C-_" edit_undo
key_bind "C-z" edit_undo
# Cut selection to clipboard.
# Following is approximate! We don't have a proper "edit_cut" command yet.
key_bind "C-x" edit_kill_selection
key_bind "M-DEL" edit_kill_word
# Copy selection to clipboard.
key_bind "C-c" edit_copy
# Delete without modifying clipboard.
key_bind "BS" edit_delete_previous_character
bind_key "M-BS" edit_kill_word_backward
key_bind "DEL" edit_delete_next_character
# Paste.
key_bind "C-v" edit_paste

# Search

# Once the search is underway, "find next" is hard-wired to C-s.
# Having it hard-wired is obviously broken, but something neutral like RET
# would be better.
# The proposed meaning of ESC obviates the current behaviour of RET.
key_bind "C-f" edit_find
key_bind "C-S-f" edit_find_backwards
key_bind "C-r" edit_find_and_replace

# Insert special characters.

key_bind "C-RET" edit_insert_newline
key_bind "RET" edit_insert_newline_and_indent
key_bind "TAB" indent_relative # FIXME: edit_insert_tab?
# edit_quoted_insert needs a new shortcut? Was C-q.

# Rare commands.
# SUGGESTION: remove these from the default keymap?

key_bind "M-m" edit_navigate_start_line_text
key_bind "M-e" macro_play
key_bind "M-c" edit_case_capitalize
key_bind "M-l" edit_case_lower
key_bind "M-)" macro_stop
key_bind "M-q" edit_wrap_paragraph
key_bind "C-j" edit_insert_newline
key_bind "M-%" edit_find_and_replace
key_bind "C-l" recenter
key_bind "M-!" edit_shell_command
key_bind "M-(" macro_record
key_bind "M-i" edit_insert_tab
key_bind "C-u" edit_repeat
key_bind "M-u" edit_case_upper
