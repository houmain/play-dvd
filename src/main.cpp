
#include "Settings.h"
#include "Title.h"
#include "dump.h"
#include "mpv.h"

const auto get_titles_timeout = std::chrono::minutes(2);
const auto min_extra_duration = std::chrono::minutes(1);
const auto min_title_duration = std::chrono::minutes(5);

int main(int argc, const char* argv[]) {
  auto settings = Settings();
  if (!interpret_commandline(settings, argc, argv)) {
    print_help_message(argv[0]);
    return 1;
  }

  auto titles = try_get_titles(&settings.dvd_device,
    get_titles_timeout, settings.verbosity);
  if (titles.empty())
    return 1;

  detect_duplicates(titles);
  detect_subsets(titles);
  detect_others(titles, min_extra_duration);
  detect_extras(titles, min_title_duration);

  if (settings.verbosity > 0)
    dump_titles(titles, settings.verbosity > 1);

  const auto command = make_mpv_command(settings, titles);
  return std::system(command.c_str());
}
