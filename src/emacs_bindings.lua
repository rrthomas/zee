-- CUA key bindings

-- Command execution
key_bind("Alt-x", "execute_command")
key_bind("Ctrl-u", "edit_repeat")
key_bind("Alt-!", "edit_shell_command")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind("LEFT", "move_previous_character")
key_bind("Ctrl-b", "move_previous_character")
key_bind("RIGHT", "move_next_character")
key_bind("Ctrl-f", "move_next_character")
key_bind("UP", "move_previous_line")
key_bind("Ctrl-p", "move_previous_line")
key_bind("DOWN", "move_next_line")
key_bind("Ctrl-n", "move_next_line")
-- Word/paragraph
key_bind("Ctrl-LEFT", "move_previous_word")
key_bind("Alt-b", "move_previous_word")
key_bind("Ctrl-RIGHT", "move_next_word")
key_bind("Alt-f", "move_next_word")
-- Line/page
key_bind("HOME", "move_start_line")
key_bind("Ctrl-a", "move_start_line")
key_bind("END", "move_end_line")
key_bind("Ctrl-e", "move_end_line")
key_bind("PGUP", "move_previous_page")
key_bind("Alt-v", "move_previous_page")
key_bind("PGDN", "move_next_page")
key_bind("Ctrl-v", "move_next_page")
-- Whole buffer
key_bind("Ctrl-HOME", "move_start_file")
key_bind("Alt-<", "move_start_file")
key_bind("Ctrl-END", "move_end_file")
key_bind("Alt->," "move_end_file")
-- Window
key_bind("Ctrl-l", "move_redraw")

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.
key_bind("Ctrl-Alt-g", "edit_goto_column")
key_bind("Alt-g", "edit_goto_line")

-- Save
key_bind("Alt-s", "file_save")
-- Suspend
key_bind("Ctrl-z", "file_suspend")
-- Quit
key_bind("Ctrl-Alt-q", "file_quit")

-- Undo
key_bind("Ctrl-_", "edit_undo")
-- Cut selection to clipboard
key_bind("Ctrl-k", "edit_delete_selection")
key_bind("Alt-d", "edit_delete_word")
-- Copy selection to clipboard
key_bind("Alt-w", "edit_copy")
-- Delete without modifying clipboard
key_bind("Ctrl-d", "edit_delete_next_character")
-- Paste
key_bind("Ctrl-y", "edit_paste")

-- Search
key_bind("Ctrl-s", "edit_find")
key_bind("Ctrl-r", "edit_find_backward")
key_bind("Alt-%", "edit_replace")

-- Insert special characters
key_bind("Ctrl-RET", "edit_insert_newline")
key_bind("RET", "edit_insert_newline_and_indent")
key_bind("TAB", "indent_relative")
key_bind("Ctrl-q", "edit_insert_quoted")
key_bind("Alt-i", "edit_insert_tab")

-- Macros
key_bind("Alt-(", "macro_record")
key_bind("Alt-)", "macro_stop")
key_bind("Alt-e", "macro_play")

-- Wrap
key_bind("Alt-q", "edit_wrap_paragraph")
