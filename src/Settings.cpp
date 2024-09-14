
#include "Settings.h"
#include <iostream>

bool interpret_commandline(Settings& settings, int argc, const char* argv[]) {
  for (auto i = 1; i < argc; i++) {
    const auto argument = std::string_view(argv[i]);
    if (argument == "-t" || argument == "--titles") {
      settings.titles = true;
    }
    else if (argument == "-e" || argument == "--extras") {
      settings.extras = true;
    }
    else if (argument == "-o" || argument == "--others") {
      settings.others = true;
    }
    else if (argument == "-a" || argument == "--all") {
      settings.all = true;
    }
    else if (argument == "-v" || argument == "--verbose") {
      ++settings.verbosity;
    }
    else if (argument == "-h" || argument == "--help") {
      return false;
    }
    else if (argument.find("--dvd-device=") == 0) {
      settings.dvd_device = argument.substr(13);
    }
    else if (argument.find("--resume-playback=") == 0) {
      settings.resume_playback = (argument.substr(18) == "yes");
    }
    else if (argument.find("--watch-later-dir=") == 0) {
      settings.watch_later_directory = argument.substr(18);
    }
    else if (argument.find("--") == 0) {
      settings.mpv_arguments.push_back(std::string(argument));
    }
    else {
      return false;
    }
  }
  if (!settings.titles && !settings.extras && !settings.others)
    settings.titles = true;

  return true;
}

void print_help_message(const char* argv0) {
  auto program = std::string_view(argv0);
  if (auto i = program.rfind('/'); i != std::string::npos)
    program = program.substr(i + 1);
  if (auto i = program.rfind('.'); i != std::string::npos)
    program = program.substr(0, i);

  const auto version =
#if __has_include("_version.h")
# include "_version.h"
  " ";
#else
  "";
#endif

  std::cout <<
    "play-dvd " << version << "(c) 2024 by Albert Kalchmair\n"
    "\n"
    "Usage: " << program << " [-options] [--mpv-arguments]\n"
    "  -t, --titles     playback titles (default).\n"
    "  -e, --extras     playback extras.\n"
    "  -o, --others     playback other tracks.\n"
    "  -a, --all        playback all tracks.\n"
    "  -v, --verbose    enable verbose messages.\n"
    "  -h, --help       print this help.\n"
    "\n"
    "All Rights Reserved.\n"
    "This program comes with absolutely no warranty.\n"
    "See the GNU General Public License, version 3 for details.\n"
    "\n";
}
