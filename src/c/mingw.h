//===-- mingw.h - mingw functions ---------------------------------*- C -*-===//

#ifndef MINGW_H
#define MINGW_H

#include "config.h" // autoconf

#include "platform.h" // platform

#ifdef HAVE_WINDOWS

#include <stdint.h>

#include <stdio.h>

#include <sys/stat.h>

#define ino_t uint64_t

#define lstat stat

#define link link_w

#define getline getline_w

#define getdelim getdelim_w

int inode_w(const char* path, ino_t* inode);

int link_w(const char* old, const char* new);

ssize_t getline_w(char** line, size_t* n, FILE* stream);

ssize_t getdelim_w(char** line, size_t* n, int delim, FILE* stream);

#endif

#ifndef HAVE_WINDOWS
const char _have_windows = 'n'; // dummy
#endif

#endif // WINDOWS_H

// vim:ft=c:et:ts=2:sts=2:sw=2:tw=80
