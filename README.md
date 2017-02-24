# Vimol 2.0

Vimol is a powerful molecular viewer and editor. Vimol features
vi-like key bindings, fast and lightweight design, editing in multiple tabs,
visualization of trajectories, unlimited undo/redo, command record/replay,
and much more! Vimol does not require the mouse, and most things
can be accomplished in less than 5 keystrokes. Use **h**/**j**/**k**/**l** keys
to rotate the molecule and **q** to exit the program. Viewing and editing of
multi-frame **xyz** and **pdb** files are supported. For the detailed
documentation consult the [vimol(1)](https://vimol.github.io/vimol.1.html)
manual page.

### Compilation from sources

Get vimol source code by cloning [git](https://git-scm.com) repository:

	git clone https://github.com/ilyak/vimol

To compile vimol from source you will need an ANSI C complaint compiler and a
make utility. You will also need the following dependencies installed along
with their corresponding development packages:

 * Cairo graphics library (https://cairographics.org). Version 1.12.0 or
   newer is recommended.

  - Fedora: `yum install cairo-devel`
  - FreeBSD: `pkg install cairo`
  - OpenBSD: `pkg_add cairo`
  - Ubuntu: `apt-get install libcairo-dev`

 * Simple direct-media layer (https://libsdl.org). Version 2.0.1 or newer is
   recommended. Make sure that the video support is turned on if building SDL
   from source.

  - Fedora: `yum install SDL2-devel`
  - FreeBSD: `pkg install sdl2`
  - OpenBSD: `pkg_add sdl2`
  - Ubuntu: `apt-get install libsdl2-dev`

After installing all dependencies, compile vimol by issuing:

	make

To install vimol, issue as root:

	make install
