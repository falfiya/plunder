#include <conio.h>
#include <ShlObj.h>
#include <string>
#include <filesystem>
#include <iostream>
#include <vector>
#include <corecrt_terminate.h>
using namespace std;
namespace fs = std::filesystem;

inline fs::path get_localappdata() {
   PWSTR suffering_string{};
   if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &suffering_string) != S_OK) {
      terminate();
   }

   return suffering_string;
}

inline bool is_icon_cache(fs::directory_entry const dirent) noexcept {
   if (!dirent.is_regular_file()) {
      return false;
   }

   auto path{dirent.path()};
   if (path.extension() != ".db") {
      return false;
   }

   if (path.filename().string().find("iconcache_") != 0) {
      return false;
   }

   return true;
}

int main() {
   cout << "Going to delete these files:\n";

   auto localappdata{get_localappdata()};
   auto explorer{localappdata / "Microsoft" / "Windows" / "Explorer"};

   vector<fs::path> to_delete{};
   to_delete.reserve(16);
   to_delete.emplace_back(localappdata / "IconCache.db");

   for (auto &&dirent : fs::directory_iterator(explorer)) {
      if (is_icon_cache(dirent)) {
         cout << dirent.path() << '\n';
         to_delete.emplace_back(dirent.path());
      }
   }

   cout
      << "Close explorer and delete the icon cache?\nPress 'y' to continue..."
      << endl;

   if (_getch_nolock() != L'y') {
      _cputs("Canceled...\n");
      return 0;
   }

   system("taskkill /f /im explorer.exe");

   for (auto &&iconcache : to_delete) {
      fs::remove(iconcache);
      cout << "deleted " << iconcache << endl;
   }

   system("start explorer");
}
