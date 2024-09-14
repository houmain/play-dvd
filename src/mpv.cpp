
#include "mpv.h"
#include <filesystem>
#include <sstream>

namespace {
  std::string get_dvd_hash(const std::string& dvd_device) {
    auto file = std::fopen(dvd_device.c_str(), "rb");
    if (!file)
      return { };

    struct Guard {
      FILE* file;
      ~Guard() { std::fclose(file); }
    } guard{ file };

    if (std::fseek(file, 65536, SEEK_SET))
      return { };

    auto buffer = std::string(2048, ' ');
    if (std::fread(buffer.data(), 1, buffer.size(), file) != buffer.size())
      return { };

    return std::to_string(std::hash<std::string>{ }(buffer));
  }

  std::filesystem::path get_cache_directory() {
    if (auto cache = std::getenv("XDG_CACHE_HOME"))
      return cache;
    if (auto home = std::getenv("HOME"))
      return std::filesystem::path(home) / ".cache";
    return { };
  }

  std::filesystem::path get_watch_later_directory(const std::string& dvd_device) {
    const auto cache = get_cache_directory();
    const auto hash = get_dvd_hash(dvd_device);
    if (cache.empty() || hash.empty())
      return { };
    return cache / "play-dvd" / "watch_later" / hash;
  }
} // namespace

std::string make_mpv_command(const Settings& settings,
    const std::vector<Title>& titles) {

  auto ss = std::stringstream();
  ss << "mpv ";
  ss << " --dvd-device=" << settings.dvd_device;

  if (settings.resume_playback) {
    // set custom watch later directory, which depends on DVD hash
    auto dir = std::filesystem::path(settings.watch_later_directory);
    if (dir.empty() && settings.dvd_device.find("/dev/") == 0)
      dir = get_watch_later_directory(settings.dvd_device);
    if (!dir.empty()) {
      std::filesystem::create_directories(dir);
      ss << " --watch-later-dir=" << dir.string();
    }
  }
  else {
    ss << " --resume-playback=no";
  }

  // set requested titles
  for (auto i = 0u; i < titles.size(); ++i) {
    const auto& title = titles[i];
    if (settings.all ||
        (settings.titles && title.rating == Rating::Title) ||
        (settings.extras && title.rating == Rating::Extra) ||
        (settings.others && title.rating == Rating::Other))
      ss << " dvd://" << i;
  }

  for (const auto& argument : settings.mpv_arguments)
    ss << " " << argument;

  // silence mpv when outputting self
  if (settings.verbosity > 0)
    ss << " > /dev/null 2>&1";

  return ss.str();
}
