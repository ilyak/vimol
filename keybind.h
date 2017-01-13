static const struct {
	const char *key;
	const char *command;
} keys[] = {
	{ "A", "chain" },
	{ "B", "autobond" },
	{ "C", "view.center" },
	{ "D", "delete" },
	{ "E", "unselect *" },
	{ "F", "view.zoom 1.15" },
	{ "G", "view.zoom 0.87" },
	{ "H", "view.rotate [0 10 0]" },
	{ "I", "toggle id.visible" },
	{ "J", "view.rotate [10 0 0]" },
	{ "K", "view.rotate [-10 0 0]" },
	{ "L", "view.rotate [0 -10 0]" },
	{ "M", "nop" },
	{ "N", "select.within 5.0" },
	{ "O", "toggle origin.visible" },
	{ "P", "paste" },
	{ "Q", "quit" },
	{ "R", "play" },
	{ "S", "select.next 1" },
	{ "T", "unselect.next 1" },
	{ "U", "undo" },
	{ "V", "hide; unselect" },
	{ "W", "select.bonded" },
	{ "X", "copy a; delete" },
	{ "Y", "copy a" },
	{ "Z", "select.name H" },
	{ "shift-A", "ring" },
	{ "shift-B", "nop" },
	{ "shift-C", "view.center; view.fit" },
	{ "shift-D", "clear" },
	{ "shift-E", "hyd" },
	{ "shift-F", "view.rotate [0 0 -10]" },
	{ "shift-G", "view.rotate [0 0 10]" },
	{ "shift-H", "view.pos+ [-0.1 0 0]" },
	{ "shift-I", "toggle name.visible" },
	{ "shift-J", "view.pos+ [0 0.1 0]" },
	{ "shift-K", "view.pos+ [0 -0.1 0]" },
	{ "shift-L", "view.pos+ [0.1 0 0]" },
	{ "shift-M", "nop" },
	{ "shift-N", "select.within 2.0" },
	{ "shift-O", "nop" },
	{ "shift-P", "count" },
	{ "shift-Q", "quit!" },
	{ "shift-R", "rec a" },
	{ "shift-S", "select.next -1" },
	{ "shift-T", "unselect.next -1" },
	{ "shift-U", "nop" },
	{ "shift-V", "show *" },
	{ "shift-W", "select.molecule" },
	{ "shift-X", "nop" },
	{ "shift-Y", "copy b" },
	{ "shift-Z", "select.water" },
	{ "ctrl-A", "select *" },
	{ "ctrl-B", "nop" },
	{ "ctrl-C", "copy a" },
	{ "ctrl-D", "dist?" },
	{ "ctrl-E", "nop" },
	{ "ctrl-F", "rotate [0 0 -10]" },
	{ "ctrl-G", "rotate [0 0 10]" },
	{ "ctrl-H", "rotate [0 10 0]" },
	{ "ctrl-I", "path?" },
	{ "ctrl-J", "rotate [10 0 0]" },
	{ "ctrl-K", "rotate [-10 0 0]" },
	{ "ctrl-L", "rotate [0 -10 0]" },
	{ "ctrl-M", "nop" },
	{ "ctrl-N", "nop" },
	{ "ctrl-O", "nop" },
	{ "ctrl-P", "pos?" },
	{ "ctrl-Q", "nop" },
	{ "ctrl-R", "redo" },
	{ "ctrl-S", "nop" },
	{ "ctrl-T", "tors?" },
	{ "ctrl-U", "nop" },
	{ "ctrl-V", "paste" },
	{ "ctrl-W", "nop" },
	{ "ctrl-X", "copy a; delete" },
	{ "ctrl-Y", "nop" },
	{ "ctrl-Z", "undo" },
	{ "alt-A", "nop" },
	{ "alt-B", "nop" },
	{ "alt-C", "atom C" },
	{ "ctrl-alt-C", "name C" },
	{ "alt-D", "nop" },
	{ "alt-E", "nop" },
	{ "alt-F", "nop" },
	{ "alt-G", "nop" },
	{ "alt-H", "pos+ [-0.1 0 0]" },
	{ "alt-I", "nop" },
	{ "alt-J", "pos+ [0 0.1 0]" },
	{ "alt-K", "pos+ [0 -0.1 0]" },
	{ "alt-L", "pos+ [0.1 0 0]" },
	{ "alt-M", "nop" },
	{ "alt-N", "atom N" },
	{ "ctrl-alt-N", "name N" },
	{ "alt-O", "atom O" },
	{ "ctrl-alt-O", "name O" },
	{ "alt-P", "atom P" },
	{ "ctrl-alt-P", "name P" },
	{ "alt-Q", "nop" },
	{ "alt-R", "nop" },
	{ "alt-S", "atom S" },
	{ "ctrl-alt-S", "name S" },
	{ "alt-T", "nop" },
	{ "alt-U", "nop" },
	{ "alt-V", "nop" },
	{ "alt-W", "nop" },
	{ "alt-X", "nop" },
	{ "alt-Y", "nop" },
	{ "alt-Z", "nop" },
	{ "Up", "view.rotate [-10 0 0]" },
	{ "Down", "view.rotate [10 0 0]" },
	{ "Left", "view.rotate [0 10 0]" },
	{ "Right", "view.rotate [0 -10 0]" },
	{ "shift-Up", "view.pos+ [0 -0.1 0]" },
	{ "shift-Down", "view.pos+ [0 0.1 0]" },
	{ "shift-Left", "view.pos+ [-0.1 0 0]" },
	{ "shift-Right", "view.pos+ [0.1 0 0]" },
	{ "ctrl-Up", "rotate [-10 0 0]" },
	{ "ctrl-Down", "rotate [10 0 0]" },
	{ "ctrl-Left", "rotate [0 10 0]" },
	{ "ctrl-Right", "rotate [0 -10 0]" },
	{ "alt-Up", "pos+ [0 -0.1 0]" },
	{ "alt-Down", "pos+ [0 0.1 0]" },
	{ "alt-Left", "pos+ [-0.1 0 0]" },
	{ "alt-Right", "pos+ [0.1 0 0]" },
	{ "Delete", "delete" },
	{ "=", "invert *" },
	{ "Space", "fullscreen" },
	{ "`", "view.reset" },
	{ "-", "nop" },
	{ ";", "nop" },
	{ "[", "frame+ -1" },
	{ "]", "frame+ 1" },
	{ "shift-[", "frame+ -100" },
	{ "shift-]", "frame+ 100" },
	{ "shift-=", "nop" },
	{ "shift-`", "nop" },
	{ "shift--", "nop" },
	{ "shift-;", "edit" },
	{ ",", "prev" },
	{ ".", "next" },
	{ "shift-,", "first" },
	{ "shift-.", "last" },
};
