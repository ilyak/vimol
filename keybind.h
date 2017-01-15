static const struct {
	const char *key;
	const char *command;
} keys[] = {
	{ "A", "chain" },
	{ "B", "view-zoom 0.87" },
	{ "C", "view-center" },
	{ "D", "nop" },
	{ "E", "select *" },
	{ "F", "view-zoom 1.15" },
	{ "G", "nop" },
	{ "H", "view-rotate 0 10 0" },
	{ "I", "toggle id.visible" },
	{ "J", "view-rotate 10 0 0" },
	{ "K", "view-rotate -10 0 0" },
	{ "L", "view-rotate 0 -10 0" },
	{ "M", "nop" },
	{ "N", "select-within 5.0" },
	{ "O", "toggle origin.visible" },
	{ "P", "paste" },
	{ "Q", "q" },
	{ "R", "play" },
	{ "S", "select-next 1" },
	{ "T", "unselect-next 1" },
	{ "U", "undo" },
	{ "V", "hide; unselect" },
	{ "W", "select-bonded" },
	{ "X", "copy-selection a; delete-selection" },
	{ "Y", "copy-selection a" },
	{ "Z", "select-name H" },
	{ "shift-A", "ring" },
	{ "shift-B", "nop" },
	{ "shift-C", "view-center; view-fit" },
	{ "shift-D", "clear" },
	{ "shift-E", "unselect *" },
	{ "shift-F", "view-rotate 0 0 -10" },
	{ "shift-G", "view-rotate 0 0 10" },
	{ "shift-H", "view-move -0.1 0 0" },
	{ "shift-I", "toggle name.visible" },
	{ "shift-J", "view-move 0 0.1 0" },
	{ "shift-K", "view-move 0 -0.1 0" },
	{ "shift-L", "view-move 0.1 0 0" },
	{ "shift-M", "nop" },
	{ "shift-N", "select-within 2.0" },
	{ "shift-O", "toggle statusbar.visible" },
	{ "shift-P", "count" },
	{ "shift-Q", "q!" },
	{ "shift-R", "rec a" },
	{ "shift-S", "select-next -1" },
	{ "shift-T", "unselect-next -1" },
	{ "shift-U", "nop" },
	{ "shift-V", "show *" },
	{ "shift-W", "select-molecule" },
	{ "shift-X", "copy-selection a; delete-selection" },
	{ "shift-Y", "nop" },
	{ "shift-Z", "select-water" },
	{ "ctrl-A", "select *" },
	{ "ctrl-B", "nop" },
	{ "ctrl-C", "copy-selection a" },
	{ "ctrl-D", "dist?" },
	{ "ctrl-E", "nop" },
	{ "ctrl-F", "rotate 0 0 -10" },
	{ "ctrl-G", "get-path" },
	{ "ctrl-H", "rotate 0 10 0" },
	{ "ctrl-I", "nop" },
	{ "ctrl-J", "rotate 10 0 0" },
	{ "ctrl-K", "rotate -10 0 0" },
	{ "ctrl-L", "rotate 0 -10 0" },
	{ "ctrl-M", "nop" },
	{ "ctrl-N", "nop" },
	{ "ctrl-O", "nop" },
	{ "ctrl-P", "pos?" },
	{ "ctrl-Q", "nop" },
	{ "ctrl-R", "redo" },
	{ "ctrl-S", "nop" },
	{ "ctrl-T", "torsion?" },
	{ "ctrl-U", "nop" },
	{ "ctrl-V", "paste" },
	{ "ctrl-W", "nop" },
	{ "ctrl-X", "copy-selection a; delete-selection" },
	{ "ctrl-Y", "nop" },
	{ "ctrl-Z", "undo" },
	{ "Up", "view-rotate -10 0 0" },
	{ "Down", "view-rotate 10 0 0" },
	{ "Left", "view-rotate 0 10 0" },
	{ "Right", "view-rotate 0 -10 0" },
	{ "shift-Up", "view-move 0 -0.1 0" },
	{ "shift-Down", "view-move 0 0.1 0" },
	{ "shift-Left", "view-move -0.1 0 0" },
	{ "shift-Right", "view-move 0.1 0 0" },
	{ "Delete", "delete-selection" },
	{ "=", "invert-selection *" },
	{ "Space", "fullscreen" },
	{ "`", "view-reset" },
	{ "[", "next-frame -1" },
	{ "]", "next-frame 1" },
	{ "shift-[", "next-frame -100" },
	{ "shift-]", "next-frame 100" },
	{ "shift-;", "edit" },
	{ ",", "prev-window" },
	{ ".", "next-window" },
	{ "shift-,", "first-window" },
	{ "shift-.", "last-window" },
};
