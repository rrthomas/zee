// Table of variables

/*
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("true" or "false");
 *   - "" (empty string) for non-fixed format.
 * The third column specifies the default value.
 * The fourth column gives the variable's documentation.
 */

X("tab_display_width", "", "8", "Number of spaces displayed for a tab character.")
X("indent_width", "", "2", "Number per indentation for `edit_insert_tab'.")
X("wrap_mode", "b", "false", "Whether Wrap Mode is automatically enabled.")
X("wrap_column", "", "72", "Column beyond which wrapping occurs in Wrap Mode.")
X("case_replace", "b", "true", "Whether `edit_replace' should preserve case.")
