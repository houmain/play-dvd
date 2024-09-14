#pragma once

#include <vector>
#include <chrono>

using Duration = std::chrono::duration<double>;
class Title;

enum class Rating {
  Title,
  Extra,
  Duplicate,
  Subset,
  Joined,
  Other,
};

void detect_duplicates(std::vector<Title>& titles);
void detect_subsets(std::vector<Title>& titles);
void detect_others(std::vector<Title>& titles,
  const Duration& min_extra_duration);
void detect_extras(std::vector<Title>& titles,
  const Duration& min_title_duration);
