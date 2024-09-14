#pragma once

#include "Rating.h"
#include <string>
#include <optional>
#include <tuple>

struct Cell {
  int first_sector;
  int last_sector;
  Duration duration;

  friend bool operator==(const Cell& a, const Cell& b) {
    return std::tie(a.first_sector, a.last_sector) ==
           std::tie(b.first_sector, b.last_sector);
  }
};

struct Chapter {
  std::vector<Cell> cells;

  friend bool operator==(const Chapter& a, const Chapter& b) {
    return std::tie(a.cells) ==
           std::tie(b.cells);
  }
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
