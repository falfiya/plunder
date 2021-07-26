#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <winternl.h>

#define STATUS_SUCCESS 0
__kernel_entry NTSTATUS NTAPI NtGetNextProcess(
   _In_  HANDLE ProcessHandle,
   _In_  ACCESS_MASK DesiredAccess,
   _In_  ULONG HandleAttributes,
   _In_  ULONG Flags,
   _Out_ PHANDLE NewProcessHandle
);

inline PWSTR GetLocalAppData() {
   PWSTR out;
   if (SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &out) != S_OK) {
      ExitProcess(1);
   }
   return out;
}

#define DWORD_PTR *(DWORD *)
#define QWORD_PTR *(size_t *)

int start(void) {
   // 16 KiB heap should be enough, what's that, like 4 pages?
   #define HEAP_SIZE 0x5000
   HANDLE const hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, HEAP_SIZE, 0);

   PWSTR iconcaches = HeapAlloc(
      hHeap,
      HEAP_GENERATE_EXCEPTIONS,
      sizeof(WCHAR[0x4000])
   );
   PWSTR iconcaches_curr = iconcaches;
   size_t iconcaches_count = 0;

   PWSTR delimiters[20];
   size_t delimiters_count = 0;

   HANDLE const hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   HANDLE const hStdin = GetStdHandle(STD_INPUT_HANDLE);
   if (hStdout == INVALID_HANDLE_VALUE || hStdout == INVALID_HANDLE_VALUE) {
      return GetLastError();
   }

   {
      #define mPrompt L"Going to delete these files:\n"
      DWORD cw;
      WriteConsoleW(
         hStdout,
         mPrompt,
         sizeof(mPrompt) / sizeof(WCHAR),
         &cw,
         NULL
      );
   }

   // looks something like this "C:\Users\User\AppData\Local", 0
   PWSTR const local = GetLocalAppData();

   PWSTR local_end = local;
   while (*local_end) {
      local_end++;
   }

   PWSTR cache;
   {
      #define mSubdir L"\\Microsoft\\Windows\\Explorer\\*"
      size_t cache_len = (local_end - local) * sizeof(WCHAR) + sizeof(mSubdir);

      cache = _alloca(cache_len);
      PWSTR cache_curr = cache;
      PWSTR subdir = mSubdir;

      PCWSTR local_curr = local;
      while (local_curr != local_end) {
         *cache_curr = *local_curr;
         *iconcaches_curr = *local_curr;
         local_curr++;
         cache_curr++;
         iconcaches_curr++;
      }

      QWORD_PTR iconcaches_curr = QWORD_PTR L"\\Ico"; iconcaches_curr += 4;
      QWORD_PTR iconcaches_curr = QWORD_PTR L"nCac" ; iconcaches_curr += 4;
      QWORD_PTR iconcaches_curr = QWORD_PTR L"he.d" ; iconcaches_curr += 4;
      DWORD_PTR iconcaches_curr = DWORD_PTR L"b\n"  ; iconcaches_curr += 2;

      delimiters[delimiters_count++] = iconcaches_curr - 1;

      while (*subdir) {
         *cache_curr = *subdir;
         subdir++;
         cache_curr++;
      }
   }

   WIN32_FIND_DATAW file;
   HANDLE hFind = FindFirstFileW(cache, &file);

   if (hFind == INVALID_HANDLE_VALUE) {
      // means the directory doesn't exist
      return GetLastError();
   }

   do {
      PCWSTR c = file.cFileName;

      if (QWORD_PTR c != QWORD_PTR L"icon") continue; c += 4;
      if (QWORD_PTR c != QWORD_PTR L"cach") continue; c += 4;
      if (DWORD_PTR c != DWORD_PTR L"e_"  ) continue; c += 2;

      while (*c) c++;

      if (QWORD_PTR (c - 3) != QWORD_PTR L".db") continue;

      {
         PCWSTR local_curr = local;
         while (local_curr != local_end) {
            *iconcaches_curr = *local_curr;
            iconcaches_curr++;
            local_curr++;
         }
      }

      *iconcaches_curr++ = L'\\';

      {
         c = file.cFileName;
         while (*c) {
            *iconcaches_curr = *c;
            iconcaches_curr++;
            c++;
         }
      }

      *iconcaches_curr = L'\n';
      delimiters[delimiters_count++] = iconcaches_curr;

      iconcaches_count += 2;
   } while (FindNextFileW(hFind, &file));

   QWORD_PTR iconcaches_curr = QWORD_PTR L"Clos" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"e ex" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"plor" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"er a" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"nd d" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"elet" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"e th" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"e ic" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"on c" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"ache" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"?\nPr"; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"ess " ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"'y' " ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"to c" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"onti" ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"nue." ; iconcaches_curr += 4;
   QWORD_PTR iconcaches_curr = QWORD_PTR L"..\n" ; iconcaches_curr += 3;

   DWORD num_chars_written;
   WriteConsoleW(
      hStdout,
      iconcaches,
      iconcaches_curr - iconcaches,
      &num_chars_written,
      NULL
   );

   while (delimiters_count --> 0) {
      *delimiters[delimiters_count] = L'\0';
   }

   SetConsoleMode(hStdin, 0
      | ENABLE_INSERT_MODE
      | ENABLE_PROCESSED_INPUT
   );

   {
      WCHAR input;
      DWORD chars_verbed;
      if (!ReadConsoleW(hStdin, &input, 1, &chars_verbed, NULL)) {
         return GetLastError();
      }

      if (input != L'y') {
         #define mCanceled "Canceled...\n"
         WriteConsoleW(
            hStdout,
            L"Canceled...\n",
            sizeof("Canceled...\n") - 1,
            &chars_verbed,
            NULL
         );
      }
   }

   SetConsoleMode(hStdin, 0
      | ENABLE_ECHO_INPUT
      | ENABLE_INSERT_MODE
      | ENABLE_LINE_INPUT
      | ENABLE_MOUSE_INPUT
      | ENABLE_PROCESSED_INPUT
      | ENABLE_QUICK_EDIT_MODE
      | ENABLE_VIRTUAL_TERMINAL_INPUT
   );

   {
      HANDLE hProcess;
      while (NtGetNextProcess(hProcess, MAXIMUM_ALLOWED, 0, 0, &hProcess) == STATUS_SUCCESS) {
         char buf[sizeof(UNICODE_STRING) + MAX_PATH];
      }
   }
   // system("taskkill /f /im explorer.exe");

   // for (auto &&iconcache : to_delete) {
   //    fs::remove(iconcache);
   //    cout << "deleted " << iconcache << endl;
   // }

   // system("start explorer");
   return 0;
}
