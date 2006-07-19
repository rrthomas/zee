/* Table of variables */

/*
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("true" or "false");
 *   - "" (empty string) for non-fixed format.
 * The third column specifies the default value.
 * The fourth column specifies the variable documentation.
 */

X("wrap_mode",				"b", "false", "\
If enabled, the Wrap Mode is automatically enabled.")
X("case_replace",			"b", "true", "\
Non-nil means `edit_find_and_replace' should preserve case in replacements.")
X("wrap_column",			"", "72", "\
Column beyond which automatic line-wrapping should happen.")
X("tab_width",				"", "8", "\
Distance between tab stops (for display of tab characters), in columns.")
