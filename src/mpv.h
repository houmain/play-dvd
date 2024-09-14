
#pragma once

#include "Title.h"
#include "Settings.h"
#include <string>

std::string make_mpv_command(const Settings& settings,
  const std::vector<Title>& titles);
