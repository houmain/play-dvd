#pragma once

#include "Rating.h"
#include <string>
#include <optional>
#include <tuple>

struct Cell {
  int first_sector;
  int last_sector;
  Duration duration;
};

struct Chapter {
  std::vector<Cell> cells;
};

struct Title {
  int set_number;
  int number_in_set;
  int num_audio_streams;
  Duration duration;
  std::vector<Chapter> chapters;
  std::optional<int> duplicate_of;
  std::vector<int> subset_of;
  int num_subsets;
  Rating rating;
};

std::vector<Title> try_get_titles(std::string* dvd_device,
  std::chrono::seconds timeout, int verbosity);
