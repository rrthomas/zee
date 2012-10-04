-- CUA key bindings

-- Command execution
key_bind("Alt-x", "execute_command")
key_bind("Ctrl-u", "edit_repeat")
key_bind("Alt-s", "edit_shell_command")

-- Navigation. Mostly arrows and things.

-- Character/line
key_bind("LEFT", "move_previous_character")
key_bind("RIGHT", "move_next_character")
key_bind("UP", "move_previous_line")
key_bind("DOWN", "move_next_line")
-- Word/paragraph
key_bind("Ctrl-LEFT", "move_previous_word")
key_bind("Ctrl-RIGHT", "move_next_word")
key_bind("Alt-{", "move_previous_paragraph")
key_bind("Alt-}", "move_next_paragraph")
-- Line/page
key_bind("HOME", "move_start_line")
key_bind("END", "move_end_line")
key_bind("PGUP", "move_previous_page")
key_bind("PGDN", "move_next_page")
-- Whole buffer
key_bind("Ctrl-HOME", "move_start_file")
key_bind("Ctrl-END", "move_end_file")
-- Window
key_bind("Ctrl-l", "move_redraw")

-- Selection
-- select_other_end
key_bind("Ctrl-@", "edit_select_toggle")

-- Absolute navigation. These are like navigation commands but they
-- don't become selection commands when combined with SHIFT.

key_bind("Ctrl-Alt-g", "edit_goto_column")
key_bind("Alt-g", "edit_goto_line")

-- Save
key_bind("Ctrl-s", "file_save")
-- Quit
key_bind("Ctrl-q", "file_quit")

-- Undo
key_bind("Ctrl-z", "edit_undo")
-- Cut selection to clipboard
-- FIXME: Following is approximate! We don't have a proper edit_cut
-- command yet.
key_bind("Ctrl-x", "edit_delete_selection")
key_bind("Alt-DEL", "edit_delete_word")
-- Copy selection to clipboard
key_bind("Ctrl-c", "edit_copy")
-- Delete without modifying clipboard")
key_bind("BS", "edit_delete_previous_character")
key_bind("Alt-BS", "edit_delete_word_backward")
key_bind("DEL", "edit_delete_next_character")
-- Paste
key_bind("Ctrl-v", "edit_paste")

-- Search
key_bind("Ctrl-f", "edit_find")
key_bind("Ctrl-Alt-f", "edit_find_backward")
key_bind("Ctrl-r", "edit_replace")

-- Insert special characters
key_bind("Ctrl-RET", "edit_insert_newline")
key_bind("RET", "edit_insert_newline_and_indent")
key_bind("Alt-i", "edit_insert_quoted")

-- Macros
key_bind("Alt-(", "macro_record")
key_bind("Alt-)", "macro_stop")
key_bind("Alt-e", "macro_play")

-- Wrap
key_bind("Ctrl-j", "edit_wrap_paragraph")
