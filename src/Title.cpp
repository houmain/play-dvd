
#include "Title.h"

// some documentation on libdvdread:
// https://dvds.beandog.org/doku.php?id=libdvdread

#include <dvdread/ifo_read.h>
#include <dvdread/dvd_reader.h>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <numeric>
#include <thread>
#include <filesystem>

namespace {
  Duration to_duration(dvd_time_t *dt) {
    auto duration = Duration::zero();
    duration += std::chrono::hours(  ((dt->hour   & 0xf0) >> 3) * 5 + (dt->hour   & 0x0f));
    duration += std::chrono::minutes(((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f));
    duration += std::chrono::seconds(((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f));

    const double frames_per_second[4] = { -1.0, 25.00, -1.0, 29.97 };
    const auto fps = frames_per_second[(dt->frame_u & 0xc0) >> 6];
    if(fps > 0)
      duration += Duration{ (((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f)) } / fps;

    return duration;
  }

  pgc_t* get_pgc(ifo_handle_t& vtsi, int vts_ttn) {
    return vtsi.vts_pgcit->pgci_srp[
      vtsi.vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;
  }

  int get_num_audio_streams(pgc_t& pgc) {
    auto count = 0;
    for (auto i = 0; i < 8; ++i)
      if (pgc.audio_control[i] & 0x8000)
        ++count;
    return count;
  }

  std::vector<Chapter> get_chapters(pgc_t& pgc) {
    auto chapters = std::vector<Chapter>();
    for (auto i = 0; i < pgc.nr_of_programs; ++i) {
      auto& chapter = chapters.emplace_back();
      auto& cells = chapter.cells;
      const auto cells_begin = pgc.program_map[i] - 1;
      const auto cells_end = (i + 1 < pgc.nr_of_programs ?
        pgc.program_map[i + 1] - 1 : pgc.nr_of_cells);

      for (auto j = cells_begin; j < cells_end; ++j) {
        auto& cell = pgc.cell_playback[j];

        // only use first cell of multi-angle cells
        if (cell.block_mode > 1)
          continue;

        cells.push_back({
          static_cast<int>(cell.first_sector),
          static_cast<int>(cell.last_sector),
          to_duration(&cell.playback_time),
        });

        // merge adjacent cells
        if (chapter.cells.size() >= 2) {
          auto& last = *std::prev(cells.end(), 2);
          auto& current = *std::prev(cells.end(), 1);
          if (last.last_sector + 1 == current.first_sector) {
            last.last_sector = current.last_sector;
            last.duration += current.duration;
            cells.pop_back();
          }
        }
      }

      // remove almost empty cells
      cells.erase(std::remove_if(cells.begin(), cells.end(),
        [](const Cell& cell) {
          return cell.duration <= std::chrono::seconds(1);
        }), cells.end());

      // remove almost empty chapters
      const auto chapter_duration =
        std::accumulate(cells.begin(), cells.end(), Duration::zero(),
          [](const Duration& total, const Cell& cell) {
            return total + cell.duration;
          });
      if (chapter_duration <= std::chrono::seconds(5))
        chapters.pop_back();
    }
    return chapters;
  }

  std::vector<Title> get_titles(const std::vector<ifo_handle_t*>& ifos) {
    if (ifos.empty())
      return { };
    const auto& vmgi = *ifos.front();

    auto titles = std::vector<Title>();
    const auto num_titles = vmgi.tt_srpt->nr_of_srpts;
    for (auto i = 0; i < num_titles; ++i) {
      const auto& title = vmgi.tt_srpt->title[i];
      auto& vtsi = *ifos[title.title_set_nr];
      if (!vtsi.vtsi_mat || !vtsi.vts_pgcit || !vtsi.vts_ptt_srpt)
        continue;

      const auto pgc = get_pgc(vtsi, title.vts_ttn);
      if (!pgc || !pgc->cell_playback || !pgc->program_map)
        continue;

      titles.push_back({
        title.title_set_nr,
        title.vts_ttn,
        get_num_audio_streams(*pgc),
        to_duration(&pgc->playback_time),
        get_chapters(*pgc),
      });
    }
    return titles;
  }

  std::vector<std::string> find_dvd_devices(const std::string& dvd_device) {
    auto error = std::error_code{ };
    if (std::filesystem::exists(dvd_device, error))
      return { dvd_device };

    auto dvd_devices = std::vector<std::string>();
    for (auto device : {
        "/dev/dvd",
        "/dev/sr0",
        "/dev/sr1",
        "/dev/sr2"
      })
      if (std::filesystem::exists(device, error))
        dvd_devices.push_back(device);

    return dvd_devices;
  }

  void log_callback(void* pverbosity, dvd_logger_level_t level,
      const char* format, va_list args) {
    [[maybe_unused]] const auto verbosity = *static_cast<int*>(pverbosity);
  }

  dvd_reader_t* try_open_dvd(std::string* dvd_device,
      std::chrono::seconds timeout, int verbosity) {
    if (!dvd_device)
      return nullptr;

    // find DVD devices when none is specified
    const auto dvd_devices = find_dvd_devices(*dvd_device);
    if (dvd_devices.empty())
      return nullptr;

    const auto logger = dvd_logger_cb { &log_callback };
    const auto timeout_end = std::chrono::system_clock::now() + timeout;
    for (;;) {
      // try to open any of the devices for a while
      for (const auto& device : dvd_devices)
        if (auto dvd = DVDOpen2(&verbosity, &logger, device.c_str())) {
          // return the one that could be opened
          *dvd_device = device;
          return dvd;
        }

      if (std::chrono::system_clock::now() > timeout_end)
        return nullptr;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  std::vector<ifo_handle_t*> open_ifos(dvd_reader_t* dvd) {
    auto vmgi = ifoOpen(dvd, 0);
    if (!vmgi)
      return { };

    const auto num_vts = vmgi->vts_atrt->nr_of_vtss;
    auto ifos = std::vector<ifo_handle_t*>(num_vts + 1);
    ifos[0] = vmgi;

    for (auto i = 1; i <= num_vts; ++i) {
      ifos[i] = ifoOpen(dvd, i);
      if (!ifos[i])
        return { };
    }
    return ifos;
  }

  void close_ifos(const std::vector<ifo_handle_t*>& ifos) {
    for (auto ifo : ifos)
      ifoClose(ifo);
  }
} // namespace

std::vector<Title> try_get_titles(std::string* dvd_device,
    std::chrono::seconds timeout, int verbosity) {

  auto dvd = try_open_dvd(dvd_device, timeout, verbosity);
  if (!dvd)
    return { };

  auto titles = std::vector<Title>();
  for (auto i = 0; i < 5; ++i) {
    auto ifos = open_ifos(dvd);
    if (ifos.empty()) {
      // can fail right after inserting
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    titles = get_titles(ifos);
    close_ifos(ifos);
    break;
  }
  DVDClose(dvd);
  return titles;
}
