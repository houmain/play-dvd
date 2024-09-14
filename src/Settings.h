#pragma once

#include <filesystem>
#include <vector>
#include <string>

struct Settings {
  int verbosity{ };
  bool titles{ };
  bool extras{ };
  bool others{ };
  bool all{ };

  bool resume_playback{ true };
  std::string dvd_device;
  std::string watch_later_directory;
  std::vector<std::string> mpv_arguments;
};

bool interpret_commandline(Settings& settings, int argc, const char* argv[]);
void print_help_message(const char* argv0);
