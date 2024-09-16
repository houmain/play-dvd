# play-dvd

Opens a DVD with the video player [mpv](https://mpv.io).

So basically all it does is to call:
```python
mpv dvd://0 dvd://1 dvd://2 dvd://3 ...
```

As you might have experienced, it is challenging to find the title numbers you want to see. Therefore `play-dvd` was created, which has the following features:

- by default it only enqueues the main title(s).
- it allows to [resume playback](https://mpv.io/manual/master/#resuming-playback) of each DVD, by managing the [`watch-later-dir`](https://mpv.io/manual/master/#watch-later) (in `~/.cache/play-dvd/watch_later`).
- it waits until a just inserted DVD can be successfully opened.
- when installed, it adds a _"DVD Playback"_ entry to your application launchers.
- the launcher has a second launch mode to play the extras (trailers, featurettes, ...).

These make it arguably more convenient to use than a dedicated DVD player (with its irritating menus, intros, pirating nag screens...).

## Tips

You may want to add the following settings to your [mpv.conf](https://mpv.io/manual/master/#configuration-files):

```bash
# select the preferred audio languages
alang=de,en

# enable the resume playback feature
save-position-on-quit
```

## Commandline arguments

```
Usage: play-dvd [-options] [--mpv-arguments]
  -t, --titles     playback titles (default).
  -e, --extras     playback extras.
  -o, --others     playback other tracks.
  -a, --all        playback all tracks.
  -v, --verbose    enable verbose messages.
  -h, --help       print this help.
```

## Building

A C++17 conforming compiler is required. A script for the
[CMake](https://cmake.org) build system is provided.

**Installing dependencies on Debian Linux and derivatives:**

```
sudo apt install build-essential git cmake mpv libdvdread-dev
```

**Checking out the source:**

```
git clone https://github.com/houmain/play-dvd
```

**Building:**

```
cd play-dvd
cmake -B build
cmake --build build
```

## License

**play-dvd** is released under the GNU GPLv3. It comes with absolutely no warranty. Please see `LICENSE` for license details.
