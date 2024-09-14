
#include "Rating.h"
#include "Title.h"
#include <algorithm>
#include <numeric>

namespace {
  bool has_sector(const Title& title, int sector) {
    for (const auto& chapter : title.chapters)
      for (const auto& cell : chapter.cells)
        if (sector >= cell.first_sector &&
            sector <= cell.last_sector)
          return true;
    return false;
  }

  bool is_subset_of(const Title& a, const Title& b) {
    if (a.set_number != b.set_number)
      return false;

    for (const auto& chapter : a.chapters)
      for (const auto& cell : chapter.cells)
        if (!has_sector(b, cell.first_sector) ||
            !has_sector(b, cell.last_sector))
          return false;
    return true;
  }
} // namespace

void detect_duplicates(std::vector<Title>& titles) {
  for (auto i = 0u; i < titles.size(); ++i)
    if (auto& title = titles[i];
        !title.chapters.empty()) {

      for (auto j = 0u; j < i; ++j)
        if (const auto& other = titles[j];
            title.set_number == other.set_number &&
            title.chapters == other.chapters) {
          title.duplicate_of = j;
          title.rating = Rating::Duplicate;
          break;
        }
    }
}

void detect_subsets(std::vector<Title>& titles) {
  // when all cells of a title are part of a longer title then it is a subset
  // when a title is made up of subsets only then it is a joined title
  // shorter joined titles can be part of a longer joined title (Looney Tunes All Stars)

  // iterate over titles sorted by duration
  auto title_indices_by_duration = std::vector<size_t>();
  title_indices_by_duration.resize(titles.size());
  std::iota(title_indices_by_duration.begin(), title_indices_by_duration.end(), 0);
  std::sort(title_indices_by_duration.begin(), title_indices_by_duration.end(),
    [&](size_t a, size_t b) { return titles[a].duration < titles[b].duration; });
  for (auto i : title_indices_by_duration)
    if (auto& title = titles[i];
        !title.chapters.empty() &&
        !title.duplicate_of) {

      auto subset_duration = Duration::zero();
      for (auto j = 0u; j < titles.size(); ++j)
        if (auto& other = titles[j];
            j != i &&
            !other.chapters.empty() &&
            !other.duplicate_of &&
            is_subset_of(other, title)) {
          other.subset_of.push_back(i);
          ++title.num_subsets;

          // since shorter titles were processed before it is possible to
          // only accumulate the duration of the first level of subsets
          if (other.num_subsets == 0)
            subset_duration += other.duration;
        }

      const auto subset_ratio = subset_duration / title.duration;
      if (subset_ratio > 0.99 && subset_ratio < 1.01)
        title.rating = Rating::Joined;
    }

  // mark as Subset, when it is a subset of a not Joined title
  for (auto& title : titles)
    for (auto subset_of : title.subset_of)
      if (titles[subset_of].rating != Rating::Joined)
        title.rating = Rating::Subset;
}

void detect_others(std::vector<Title>& titles, const Duration& min_extra_duration) {
  for (auto& title : titles)
    if (title.rating == Rating::Title)
      if (title.chapters.empty() ||
          title.num_audio_streams == 0 ||
          title.duration < min_extra_duration)
        title.rating = Rating::Other;
}

void detect_extras(std::vector<Title>& titles, const Duration& min_title_duration) {
  auto longest_title_duration = Duration::zero();
  auto longest_title = std::add_pointer_t<const Title>{ };
  for (const auto& title : titles)
    if (title.rating == Rating::Title &&
        title.duration > longest_title_duration) {
      longest_title_duration = title.duration;
      longest_title = &title;
    }
  if (!longest_title)
    return;

  // extras are far shorter than longest title
  // have no chapters or only one audio stream
  for (auto& title : titles)
    if (title.rating == Rating::Title)
      if (title.duration < min_title_duration ||
          title.duration < longest_title_duration / 4 ||
          (longest_title->chapters.size() > 1 && title.chapters.size() == 1) ||
          (longest_title->num_audio_streams > 1 && title.num_audio_streams == 1))
        title.rating = Rating::Extra;
}
