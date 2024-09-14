
#include "dump.h"
#include <iostream>
#include <iomanip>
#include <utility>

namespace {
  std::string format_duration(const Duration& duration) {
    const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration - minutes);
    auto ss = std::stringstream();
    ss << std::setfill('0')
      << std::setw(2) << minutes.count() << ":"
      << std::setw(2) << seconds.count();
    return ss.str();
  }
} // namespace

void dump_titles(const std::vector<Title>& titles, bool dump_cells) {
  for (auto i = 0u; i < titles.size(); ++i) {
    const auto& title = titles[i];
    std::cout << "Track " << i <<
      ", Set: " << title.set_number << "." << title.number_in_set <<
      ", Length: " << format_duration(title.duration) <<
      ", Chapters: " << title.chapters.size() <<
      ", Audio streams: " << title.num_audio_streams;

    if (title.duplicate_of)
      std::cout << ", Duplicate of " << title.duplicate_of.value();

    if (!title.subset_of.empty()) {
      std::cout << ", Subset of ";
      auto first = true;
      for (auto j : title.subset_of)
        std::cout << (std::exchange(first, false) ? "" : "/") << j;
    }

    switch (title.rating) {
      case Rating::Title: std::cout << ", Title"; break;
      case Rating::Extra: std::cout << ", Extra"; break;
      case Rating::Duplicate: break;
      case Rating::Subset: break;
      case Rating::Joined: std::cout << ", Joined"; break;
      case Rating::Other: std::cout << ", Other"; break;
    }
    std::cout << "\n";

    if (dump_cells)
      for (const auto& chapter : title.chapters)
        for (const auto& cell : chapter.cells)
          std::cout << "  " << format_duration(cell.duration) << " " <<
            cell.first_sector << " " << cell.last_sector << std::endl;
  }
  std::cout.flush();
}
