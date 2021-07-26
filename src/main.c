#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <winternl.h>

#define STATUS_SUCCESS 0
// undocumented system call moment
__kernel_entry NTSTATUS NTAPI NtGetNextProcess(
   _In_  HANDLE      ProcessHandle,
   _In_  ACCESS_MASK DesiredAccess,
   _In_  ULONG       HandleAttributes,
   _In_  ULONG       Flags,
   _Out_ PHANDLE     NewProcessHandle
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
   HANDLE const hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   HANDLE const hStdin = GetStdHandle(STD_INPUT_HANDLE);

   // 16 KiB heap should be enough, what's that, like 4 pages?
   #define HEAP_SIZE 0x5000
   HANDLE const hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, HEAP_SIZE, 0);

   if (0
      || hStdout == INVALID_HANDLE_VALUE
      || hStdout == INVALID_HANDLE_VALUE
      || hHeap   == INVALID_HANDLE_VALUE
   ) return GetLastError();

   PWSTR iobuf = HeapAlloc(
      hHeap,
      HEAP_GENERATE_EXCEPTIONS,
      HEAP_SIZE
   );
   /* iobuf write head */
   PWSTR iobuf_wh = iobuf;

   // Before we start deleting files, we print out the list of files that are
   // going to be deleted. Each icon cache path is going to be stored in the
   // iobuf and will be delimited by newlines. Since newlines can occur in
   // paths, though the really shouldn't in this case, we're going to store a
   // list of pointers to the beginning of each icon cache in here.
   // this will start at iobuf offset 0x4000.
   PWSTR *iconcache_end = (PWSTR *) (iobuf + 0x4000 / sizeof(WCHAR));
   size_t iconcache_count = 0;

   QWORD_PTR iobuf_wh = QWORD_PTR L"Goin"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"g to"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L" del"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"ete "; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"thes"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"e fi"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"les:"; iobuf_wh += 4;
   *iobuf_wh++ = L'\n';

   PWSTR iconcache_start = iobuf_wh;

   {
      PWSTR search_path;
      {
         PCWSTR const local_appdata = GetLocalAppData();
         // local is "C:\Users\User\AppData\Local"

         #define mSubdir L"\\Microsoft\\Windows\\Explorer\\*"
         {
            // unfortunately, we do need to figure out the length of this since
            // it's going to be part of search_path.
            PCWSTR local_end = local_appdata;
            while (*local_end) local_end++;

            search_path = _alloca(0
               + (local_end - local_appdata) * sizeof(WCHAR)
               + sizeof(mSubdir)
            );
         }
         {
            PWSTR search_path_wh = search_path;
            PWSTR subdir_rh = mSubdir;

            for (
               PCWSTR local_appdata_rh = local_appdata;
               *local_appdata_rh;
               *search_path_wh++ = *local_appdata_rh++
            );

            while (*subdir_rh) *search_path_wh++ = *subdir_rh++;
         }
      }

      // search_path is "C:\Users\User\AppData\Local\Microsoft\Windows\Explorer\*"

      WIN32_FIND_DATAW file;
      HANDLE hFind = FindFirstFileW(search_path, &file);

      if (hFind == INVALID_HANDLE_VALUE) {
         // means the directory doesn't exist
         return GetLastError();
      }

      do {
         PCWSTR name_rh = file.cFileName;

         if (QWORD_PTR name_rh != QWORD_PTR L"icon") continue; name_rh += 4;
         if (QWORD_PTR name_rh != QWORD_PTR L"cach") continue; name_rh += 4;
         if (DWORD_PTR name_rh != DWORD_PTR L"e_"  ) continue; name_rh += 2;

         while (*name_rh) name_rh++;

         if (QWORD_PTR (name_rh - 3) != QWORD_PTR L".db") continue;

         for (
            PCWSTR search_path_rh = search_path;
            *search_path_rh != L'*';
            *iobuf_wh++ = *search_path_rh++
         );

         for (
            name_rh = file.cFileName;
            *name_rh;
            *iobuf_wh++ = *name_rh++
         );

         *iobuf_wh = L'\n';
         iconcache_end[iconcache_count++] = iobuf_wh++;
      } while (FindNextFileW(hFind, &file));
   }

   QWORD_PTR iobuf_wh = QWORD_PTR L"Clos" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"e ex" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"plor" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"er a" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"nd d" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"elet" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"e th" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"e ic" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"on c" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"ache" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"?\nPr"; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"ess " ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"'y' " ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"to c" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"onti" ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"nue." ; iobuf_wh += 4;
   QWORD_PTR iobuf_wh = QWORD_PTR L"..\n" ; iobuf_wh += 3;

   DWORD num_chars_written;
   WriteConsoleW(
      hStdout,
      iobuf,
      iobuf_wh - iobuf,
      &num_chars_written,
      NULL
   );

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
         return 0;
      }
   }

   SetConsoleMode(hStdin, 0
      | ENABLE_ECHO_INPUT
      | ENABLE_INSERT_MODE
      | ENABLE_LINE_INPUT
      | ENABLE_MOUSE_INPUT
      | ENABLE_PROCESSED_INPUT
      | ENABLE_QUICK_EDIT_MODE
   );

   // kill explorer
   {
      #define fsize sizeof(UNICODE_STRING) + MAX_PATH
      UNICODE_STRING *f = _alloca(fsize);

      HANDLE hProcess = NULL;
      while (
         // We could use PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE but we
         // might stumble across a process that doesn't allow these two before
         // we reach the explorer.exe process. In that case, we wouldn't be able
         // to tell the difference between the end of the process list and the
         // os refusing to give us a handle with those perms. It's better to
         // just fail within the loop when we try to get it's executable.
         NtGetNextProcess(hProcess, MAXIMUM_ALLOWED, 0, 0, &hProcess)
         == STATUS_SUCCESS
      ) {
         ULONG return_length;
         if (NtQueryInformationProcess(
            hProcess,
            ProcessImageFileName,
            f,
            fsize,
            &return_length
         ) != STATUS_SUCCESS) continue;

         while (*f->Buffer) {
            f->Buffer++;
         }

         if (QWORD_PTR (f->Buffer - 4 * 1) != QWORD_PTR L".exe" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 2) != QWORD_PTR L"orer" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 3) != QWORD_PTR L"expl" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 4) != QWORD_PTR L"ows\\") continue;
         if (QWORD_PTR (f->Buffer - 4 * 5) != QWORD_PTR L"Wind" ) continue;

         TerminateProcess(hProcess, 0);
      }
   }

   while (iconcache_count --> 0) {
      **iconcache_end = L'\0';
      DeleteFileW(iconcache_start);
      iconcache_start = *iconcache_end + 1;
      iconcache_end++;
   }

   return 0;
}
