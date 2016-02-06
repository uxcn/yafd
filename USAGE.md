yafd - yet another file deduplicator

## SYNOPSIS

`yafd` [ *options* ]  [ *path* ... ]

## DESCRIPTION

`yafd` identifies duplicate files in *path* lists.  If no *path* is given, `yafd`
will process standard input as a list of values.  By default, `yafd` prints
duplicate files to standard output.

## OPTIONS

* `-h ,  --help`: Print a help summary.
* `-v, --version`: Print the version number.
* `-r, --recurse`: Recurse into sub-directories.
* `-q, --quiet`: Be quiet, don't output anything to standard output.
* `-0, --null`: Use a zero byte (ASCII `NUL`) instead of a newline to delimit
  files.  If filenames can contain whitespace, this can be used to remove
  ambiguity.
* `-f, --force`: Don't prompt before doing action (`-d/--delete`, `-l/--link`,
  etc...).  By default, any action requires confirmation.
* `-l, --link`: Merge duplicate files by hard linking them.  This will reduce
  the amount of disk needed by ((`num_files - 1) x file_size`) for each group of
  duplicate files found for each device.  Note, hard links do not have
  copy-on-write semantics, and duplicate file names will refer to the same
  files on each device after being hard linked.
* `-p, --print`: Print duplicate files (default).
* `-i, --interactive`: Prompt to choose an action for each duplicate file...
  (d)elete, (l)ink, (n)othing, etc.
* `-z, --zero`: Process zero size files as duplicate.  By default, zero sized
  files are ignored.
* `-t, --threads`: The number of concurrent system threads to use.  By default,
  this value is (`num_cpus x 2`), which is meant to allow half of the threads to
  block on I/O at any time.  For most workloads and hardware this is a good
  default, although different workloads and hardware may have better performance
  with different values.  It is safe to leave this value unset.  Set this to 0
  to run without any threads.
* `-b, --bytes`: The number of bytes to use in the digest.  This value defaults
  to the block size of the filesystem, which is generally enough data to
  distinguish between files.  However, if files have more data in common than
  differs, using a larger value may give better performance.  It is safe to
  leave this value unset.  Set this value to 0 to avoid computing digests.
* `-k, --blocksize`: The default blocksize to use.  If the block size for a file
  cannot be determined, this value will be used.  It is safe to leave this value
  unset.  Valid values are generally multiples of two (4096, 8192, etc...).
* `-o, --offset`: A static offset into files to use for digests.  This value
  should not be set unless you have specific information about the contents of
  the files being processed.  By default, the offset is randomized which limits
  pathological performance and digest collision problems.  It is safe to leave
  this value unset.  Set to a negative value to define an offset relative to
  file ends.
* `-m, --mmap`: Use memory mapped file I/O.  This may improve performance for
  unusually large files or extremely high file counts, but it is slightly
  dangerous and might be removed in a future version.  Consider guarding your
  pets.
* `-c, --crc32`: Use CRC-32 to digest bytes.  This may be faster on
  architectures that support a CRC-32 instruction, namely the SSE4.2 instruction
  set.
* `-N, --fnv32a`: Use FNV-1a (32 bit) to digest bytes.
* `-V, --fnv64a`: Use FNV-1a (64 bit) to digest bytes.
* `-F, --frm64`: Use FarmHash (64 bit) to digest bytes.
* `-M, --frm128`: Use FarmHash (128 bit) to digest bytes.
* `-S, --sky64`: Use SpookyHash (64 bit) to digest bytes.
* `-P, --sky128`: Use SpookyHash (128 bit) to digest bytes.

## EXAMPLES

    yafd a b c d

Print duplicate files in the group of files a, b, c, and d.

    yafd -r -z .

Recursively find duplicated files in the current directory, including zero sized files.

    yafd -i src/*/**/*.hpp src/*/**/*.cpp

Interactively process duplicated source files.

    find /multimedia -size +1G | yafd -N -b 1048576 | xargs du -b | awk '{ x+=$1; } END { print x; }'

Find duplicate files larger than 1GiB using FNV-1a (64 bit) with 1MiB for digest and print the total bytes consumed.

## COPYRIGHT
Copyright (c) 2015-2016 Jason Schulz &lt;https://github.com/uxcn&gt;.

## BUGS
Send any bug reports to the github issue tracker (https://github.com/uxcn/yafd/issues)

## SEE ALSO
fdupes(1), find(1), xargs(1)
