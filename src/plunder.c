/*
author - coalpha

Just some quick info before we start.

_rh - read  head: a pointer that we'll be reading from
_wh - write head: a pointer that we'll be writing to

Why are you using so many code blocks?
I dunno. It doesn't actually do anything. Originally it was for pointless stack
control but when you think about how it gets translated into llvm-ir, it seems
absolutely useless.
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>   // only for SHGetKnownFolderPath
#include <winternl.h>

#define STATUS_SUCCESS 0
NTSYSAPI NTSTATUS NTAPI ZwGetNextProcess(
   _In_  HANDLE      ProcessHandle,
   _In_  ACCESS_MASK DesiredAccess,
   _In_  ULONG       HandleAttributes,
   _In_  ULONG       Flags,
   _Out_ PHANDLE     NewProcessHandle
);

NTSYSAPI NTSTATUS NTAPI ZwTerminateProcess(
   _In_ HANDLE       ProcessHandle OPTIONAL,
   _In_ NTSTATUS     ExitStatus
);

#define DWORD_PTR *(DWORD *)
#define QWORD unsigned long long
#define QWORD_PTR *(QWORD *)

void start(void) {
   HANDLE const hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   HANDLE const hStdin = GetStdHandle(STD_INPUT_HANDLE);
   HANDLE const hHeap = GetProcessHeap();

   if (0
      || hStdout == INVALID_HANDLE_VALUE
      || hStdout == INVALID_HANDLE_VALUE
      || hHeap   == INVALID_HANDLE_VALUE
   ) {
      goto BAD_END;
   }

   // replace this with page alloc
   PWSTR obuf = HeapAlloc(hHeap, 0, 0x5000);
   if (obuf == NULL) {
      goto BAD_END;
   }
   PWSTR obuf_wh = obuf;

   // Before we start deleting files, we print out the list of files that are
   // going to be deleted. Each icon cache path is going to be stored in the
   // iobuf and will be delimited by newlines. Since newlines can occur in
   // paths, though the really shouldn't in this case, we're going to store a
   // list of pointers to each newline that should be changed into null when the
   // time comes to actually delete them.
   PWSTR *files_to_delete_end = (PWSTR *) (obuf + 0x4000 / sizeof(WCHAR));
   size_t files_to_delete_count = 0;

   // "Going to delete these files:\n"
   QWORD_PTR obuf_wh = QWORD_PTR L"Goin"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"g to"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L" del"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"ete "; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"thes"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"e fi"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"les:"; obuf_wh += 4;
   *obuf_wh++ = L'\n';

   PWSTR files_to_delete_start = obuf_wh;

   {
      PWSTR search_path;
      {
         PWSTR local_appdata;
         if (SHGetKnownFolderPath(&FOLDERID_LocalAppData, 0, NULL, &local_appdata) != S_OK) {
            ExitProcess(1);
            __builtin_unreachable();
         }

         // About to do a little string concatenation
         #define mIC     L"\\IconCache.db"
         #define mSubdir L"\\Microsoft\\Windows\\Explorer\\*"
         // Print out the first one and get length
         {
            // unfortunately, we do need to figure out the length of this since
            // it's going to be part of search_path.
            PCWSTR local_end = local_appdata;
            while (TRUE) {
               WCHAR curr = *local_end++;
               if (curr == '\0') {
                  break;
               }

               *obuf_wh++ = curr;
            }

            // NOTE(2025):
            // Just because you're right, it doesn't make this clear.
            size_t local_appdata_sz = (local_end - local_appdata) * sizeof(WCHAR);

            // Includes the null ptr
            search_path = __builtin_alloca(local_appdata_sz + sizeof(mSubdir));
         }

         QWORD_PTR obuf_wh = QWORD_PTR L"\\Ico"; obuf_wh += 4;
         QWORD_PTR obuf_wh = QWORD_PTR L"nCac" ; obuf_wh += 4;
         QWORD_PTR obuf_wh = QWORD_PTR L"he.d" ; obuf_wh += 4;
         DWORD_PTR obuf_wh = DWORD_PTR L"b\n"  ; obuf_wh += 2;

         // copy it into the new memory
         {
            PWSTR search_path_wh = search_path;

            for (
               PCWSTR local_appdata_rh = local_appdata;
               *local_appdata_rh;
               *search_path_wh++ = *local_appdata_rh++
            );

            // search_path += mSubdir
            // for (
            //    PWSTR subdir_rh = mSubdir;
            //    *subdir_rh;
            //    *search_path_wh++ = *subdir_rh++
            // );
            QWORD_PTR search_path_wh = QWORD_PTR L"\\Mic"; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"roso" ; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"ft\\W"; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"indo" ; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"ws\\E"; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"xplo" ; search_path_wh += 4;
            QWORD_PTR search_path_wh = QWORD_PTR L"rer\\"; search_path_wh += 4;
            DWORD_PTR search_path_wh = DWORD_PTR L"*"    ; search_path_wh += 2; // includes null ptr
         }
         // I sure hope COM frees the memory when the process exits.
         // It would be pretty stupid if it didn't.
         //CoTaskMemFree(local_appdata);
      }

      // search_path is "C:\Users\User\AppData\Local\Microsoft\Windows\Explorer\*"

      // Replace with NtQueryDirectoryFile
      WIN32_FIND_DATAW file;
      HANDLE hFind = FindFirstFileW(search_path, &file);

      if (hFind == INVALID_HANDLE_VALUE) {
         // means the directory doesn't exist
         goto BAD_END;
      }

      do {
         PCWSTR name_rh = file.cFileName;

         // if file.cFileName does not start with "iconcache_" skip it
         if (QWORD_PTR name_rh != QWORD_PTR L"icon") continue; name_rh += 4;
         if (QWORD_PTR name_rh != QWORD_PTR L"cach") continue; name_rh += 4;
         if (DWORD_PTR name_rh != DWORD_PTR L"e_"  ) continue; name_rh += 2;

         while (*name_rh) name_rh++;

         // if file.cFileName does not end with ".db"
         if (QWORD_PTR (name_rh - 3) != QWORD_PTR L".db") continue;

         for (
            PCWSTR search_path_rh = search_path;
            *search_path_rh != L'*';
            *obuf_wh++ = *search_path_rh++
         );

         for (
            name_rh = file.cFileName;
            *name_rh;
            *obuf_wh++ = *name_rh++
         );

         *obuf_wh = L'\n';
         files_to_delete_end[files_to_delete_count++] = obuf_wh++;
      } while (FindNextFileW(hFind, &file));
   }

   // "Close explorer and delete the icon cache?\n"
   // "Press 'y' to continue...\n"
   QWORD_PTR obuf_wh = QWORD_PTR L"Clos" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"e ex" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"plor" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"er a" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"nd d" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"elet" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"e th" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"e ic" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"on c" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"ache" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"?\nPr"; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"ess " ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"'y' " ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"to c" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"onti" ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"nue." ; obuf_wh += 4;
   QWORD_PTR obuf_wh = QWORD_PTR L"..\n>"; obuf_wh += 4;

   {
      DWORD chars_written;
      // we don't actually really care if this fails
      WriteConsoleW(
         hStdout,
         obuf,
         obuf_wh - obuf,
         &chars_written,
         NULL
      );
   }

   // unbuffered input
   SetConsoleMode(hStdin, 0
      | ENABLE_INSERT_MODE
      | ENABLE_PROCESSED_INPUT
   );

   {
      WCHAR input;
      DWORD charsRead;
      if (!ReadConsoleW(hStdin, &input, 1, &charsRead, NULL)) {
         goto BAD_END;
      }

      if (input != L'y') {
         #define mCanceled L"Canceled...\n"
         WriteConsoleW(
            hStdout,
            mCanceled,
            sizeof(mCanceled) / sizeof(wchar_t) - 1,
            NULL,
            NULL
         );
         goto GOOD_END;
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

   // kill explorer using undocumented system calls
   {
      #define fsize sizeof(UNICODE_STRING) + MAX_PATH
      UNICODE_STRING *const f = __builtin_alloca(fsize);

      HANDLE hProcess = NULL;
      while (
         // We could use PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE but we
         // might stumble across a process that doesn't allow these two before
         // we reach the explorer.exe process. In that case, we wouldn't be able
         // to tell the difference between the end of the process list and the
         // os refusing to give us a handle with those perms. It's better to
         // just fail within the loop when we try to get its executable.
         ZwGetNextProcess(hProcess, MAXIMUM_ALLOWED, 0, 0, &hProcess)
         == STATUS_SUCCESS
      ) {
         ULONG return_length;
         NTSTATUS res = NtQueryInformationProcess(
            hProcess,
            ProcessImageFileName,
            f,
            fsize,
            &return_length
         );
         if (res != STATUS_SUCCESS) continue;

         while (*f->Buffer) {
            f->Buffer++;
         }

         // if process executable path does not end with "Windows\explorer.exe"
         if (QWORD_PTR (f->Buffer - 4 * 5) != QWORD_PTR L"Wind" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 4) != QWORD_PTR L"ows\\") continue;
         if (QWORD_PTR (f->Buffer - 4 * 3) != QWORD_PTR L"expl" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 2) != QWORD_PTR L"orer" ) continue;
         if (QWORD_PTR (f->Buffer - 4 * 1) != QWORD_PTR L".exe" ) continue;

         ZwTerminateProcess(hProcess, 0);
      }
   }

   // full paths truncated in this visualization:
   // v files_to_delete_start
   // v
   // \Explorer\iconcache_16.db↵\Explorer\iconcache_32.db↵
   //                          ^                         ^
   //         files_to_delete_end[0] ^        files_to_delete_end[1] ^
   while (files_to_delete_count --> 0) {
      // previously, these were newlines.
      // DeleteFileW wants the path to be null terminated.
      **files_to_delete_end = L'\0';
      DeleteFileW(files_to_delete_start);

      // The character after each files_to_delete_end is the start of the next icon
      // cache path.
      files_to_delete_start = *files_to_delete_end + 1;

      // Instead of using a subscript, we're just going to directly increment
      // the pointer.
      files_to_delete_end++;
   }

   GOOD_END:
   ExitProcess(0);
   __builtin_unreachable();

   BAD_END:
   ExitProcess(GetLastError());
   __builtin_unreachable();
}
