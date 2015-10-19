//===-- mingw.c - mingw functions ---------------------------------*- C -*-===//

#include "mingw.h"


#include "config.h" // autconf

#include "platform.h" // platform


#ifdef HAVE_WINDOWS

#include <stdbool.h>

#include <windows.h>

#include "memory.h"

int inode_w(const char* path, ino_t* inode) {

	// synthesize inode

	BY_HANDLE_FILE_INFORMATION fi;
	HANDLE h;

	h = CreateFile(path, 0, 0, NULL, OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_READONLY, NULL);

	if (h == INVALID_HANDLE_VALUE)
			return -1;

	if (!GetFileInformationByHandle(h, &fi)) {
			CloseHandle (h);
			return -1;
	}

	*inode = ((uint64_t) fi.nFileIndexHigh << 32) | \
		       ((uint64_t) fi.nFileIndexLow);

	if (!CloseHandle(h))
    return -1;

  return 0;
}


int link_w(const char* old, const char* new) {

  bool e = CreateHardLink(new, old, NULL);

  return e ? 0 : -1;
}

ssize_t getline_w(char** line, size_t* n, FILE* stream) {

  return getdelim_w(line, n, (int) '\n', stream);
}

ssize_t getdelim_w(char** line, size_t* n, int delim, FILE* stream) {

  const size_t lbs = 32;
  size_t cs = 0;

  if ((*line == NULL) || (*n == 0)) {

    *n = lbs;
    *line = fmalloc(*n);
  }

  do {

    if (cs == (*n - 1)) {

      *n += lbs;
      *line = frealloc(*line, *n);
    }

    char c = (char) fgetc(stream);

    if (c == EOF)
      return EOF;

    *(*line + cs) = c;

  } while (*(*line + cs++) != (char) delim);

  *(*line + cs) = '\0';

  return (ssize_t) cs;
}


#endif // HAVE_WINDOWS

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
