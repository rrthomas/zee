/*	$Id$	*/

/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you do any modification, please remember to keep in sync with the
 * documentation in the `../doc/zee.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the variable type.
 *   - "b" for boolean ("true" or "false");
 *   - "" (empty string) for non-fixed format.
 * The third column specifies the default value.
 * The forth column specifies the variable documentation.
 */

X("auto-fill-mode",			"b", "false", "\
If enabled, the Auto Fill Mode is automatically enabled.")
X("beep",				"b", "true", "\
If enabled, a sound will be emitted on any error.")
X("case-replace",			"b", "true", "\
Non-nil means `query-replace' should preserve case in replacements.")
X("expand-tabs",			"b", "false", "\
If disabled, Zee will insert hard tabs (the character `\\t'),\n\
otherwise it will insert spaces.")
X("fill-column",			"", "72", "\
Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes buffer-local when set in any fashion.")
X("highlight-nonselected-windows",	"b", "false", "\
If enabled, highlight region even in nonselected windows.")
X("standard-indent",			"", "4", "\
Default number of columns for margin-changing functions to indent.")
X("tab-width",				"", "8", "\
Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes buffer-local when set in any fashion.")
X("transient-mark-mode",		"b", "true", "\
If enabled, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
