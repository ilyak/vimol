static const struct {
	const char *key;
	const char *command;
} keys[] = {
	{ "A", "add-hydrogens" },
	{ "B", "bond" },
	{ "C", "view-center-selection" },
	{ "D", "delete-selection" },
	{ "E", "unselect *" },
	{ "F", "view-zoom 1.15" },
	{ "G", "view-zoom 0.87" },
	{ "H", "view-rotate 0 10 0" },
	{ "I", "toggle id-visible" },
	{ "J", "view-rotate 10 0 0" },
	{ "K", "view-rotate -10 0 0" },
	{ "L", "view-rotate 0 -10 0" },
	{ "M", "measure" },
	{ "N", "select-sphere" },
	{ "O", "toggle origin-visible" },
	{ "P", "paste" },
	{ "Q", "quit" },
	{ "R", "rec" },
	{ "S", "select" },
	{ "T", "replay" },
	{ "U", "unselect" },
	{ "V", "hide-selection" },
	{ "W", "select-molecule" },
	{ "X", "copy-selection; delete-selection" },
	{ "Y", "copy-selection" },
	{ "Z", "show-all" },
	{ "S-A", "" },
	{ "S-B", "reset-bonds" },
	{ "S-C", "view-fit-selection" },
	{ "S-D", "" },
	{ "S-E", "" },
	{ "S-F", "view-rotate 0 0 -10" },
	{ "S-G", "view-rotate 0 0 10" },
	{ "S-H", "view-move -0.1 0 0" },
	{ "S-I", "toggle name-visible" },
	{ "S-J", "view-move 0 0.1 0" },
	{ "S-K", "view-move 0 -0.1 0" },
	{ "S-L", "view-move 0.1 0 0" },
	{ "S-M", "" },
	{ "S-N", "select-box" },
	{ "S-O", "move-selection-to" },
	{ "S-P", "" },
	{ "S-Q", "quit!" },
	{ "S-R", "" },
	{ "S-S", "" },
	{ "S-T", "" },
	{ "S-U", "" },
	{ "S-V", "" },
	{ "S-W", "select-water" },
	{ "S-X", "" },
	{ "S-Y", "" },
	{ "S-Z", "" },
	{ "C-A", "select *" },
	{ "C-B", "" },
	{ "C-C", "copy-selection" },
	{ "C-D", "" },
	{ "C-E", "" },
	{ "C-F", "rotate-selection 0 0 -10" },
	{ "C-G", "rotate-selection 0 0 10" },
	{ "C-H", "rotate-selection 0 10 0" },
	{ "CS-H", "move-selection -0.1 0 0" },
	{ "C-I", "" },
	{ "C-J", "rotate-selection 10 0 0" },
	{ "CS-J", "move-selection 0 0.1 0" },
	{ "C-K", "rotate-selection -10 0 0" },
	{ "CS-K", "move-selection 0 -0.1 0" },
	{ "C-L", "rotate-selection 0 -10 0" },
	{ "CS-L", "move-selection 0.1 0 0" },
	{ "C-M", "" },
	{ "C-N", "" },
	{ "C-O", "" },
	{ "C-P", "" },
	{ "C-Q", "" },
	{ "C-R", "redo" },
	{ "C-S", "" },
	{ "C-T", "" },
	{ "C-U", "" },
	{ "C-V", "paste" },
	{ "C-W", "" },
	{ "C-X", "copy-selection; delete-selection" },
	{ "C-Y", "" },
	{ "C-Z", "undo" },
	{ "Left", "view-rotate 0 10 0" },
	{ "Down", "view-rotate 10 0 0" },
	{ "Up", "view-rotate -10 0 0" },
	{ "Right", "view-rotate 0 -10 0" },
	{ "S-Left", "view-move -0.1 0 0" },
	{ "S-Down", "view-move 0 0.1 0" },
	{ "S-Up", "view-move 0 -0.1 0" },
	{ "S-Right", "view-move 0.1 0 0" },
	{ "C-Left", "rotate-selection 0 10 0" },
	{ "CS-Left", "move-selection -0.1 0 0" },
	{ "C-Down", "rotate-selection 10 0 0" },
	{ "CS-Down", "move-selection 0 0.1 0" },
	{ "C-Up", "rotate-selection -10 0 0" },
	{ "CS-Up", "move-selection 0 -0.1 0" },
	{ "C-Right", "rotate-selection 0 -10 0" },
	{ "CS-Right", "move-selection 0.1 0 0" },
	{ "Delete", "delete-selection" },
	{ "=", "invert-selection" },
	{ "Space", "fullscreen" },
	{ "`", "view-reset" },
	{ "[", "next-frame -1" },
	{ "]", "next-frame 1" },
	{ "S-[", "next-frame -100" },
	{ "S-]", "next-frame 100" },
	{ "S-,", "prev-window" },
	{ "S-.", "next-window" },
};
