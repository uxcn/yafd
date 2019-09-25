# yafd #

[![build status](https://travis-ci.org/uxcn/yafd.svg?branch=master)](https://travis-ci.org/uxcn/yafd)
[![build status](https://ci.appveyor.com/api/projects/status/tfikjw9me77nvuw5?svg=true)](https://ci.appveyor.com/project/uxcn/yafd)
[![coverage status](https://coveralls.io/repos/github/uxcn/yafd/badge.svg?branch=master)](https://coveralls.io/github/uxcn/yafd?branch=master)
[![issues](https://img.shields.io/github/issues/uxcn/yafd.svg)](https://github.com/uxcn/yafd/issues)

yafd is a (yet another) file deduplicator.

## Usage ##

For detailed info, see [USAGE](https://github.com/uxcn/yafd/blob/master/USAGE.md) or the
manpage (`man yafd`).

The easiest way to use yafd is to pass a directory or a set of directories to
it.

    jason@io ~ yafd .

It can recurse as well.

    jason@io ~ yafd -r .

Another easy way to use yafd is passing files to check as arguments.  Shell
globbing helps.

    jason@io ~ yafd **/*.c **/*.h

You can also pipe paths to yafd via stdin.  This makes it easy to limit the sets
of files to check.

    jason@io ~ find . -size +1M | yafd

The output can also be piped to other commands to do things with the duplicate files.

    jason@io ~ find /usr/src -size +1M | yafd | xargs du -b | awk '{ x+=$1; } END { print x; }'
    12659698

## Performance ##

As of yet, yafd is not always the fastest deduplicator (see hdd performance).
If performance is a concern, it may be worth considering another deduplicator
like [rmlint](https://github.com/sahib/rmlint).  Performance can be optimized
using command arguments (`--bytes`, `--blocksize`, `--threads`, etc...),
although yafd with defaults should be usable for most tasks.

Here are some metrics for reference.

**SSD (btrfs)**

<table>
<tr><th></th><th>time</th><th>throughput</th><th>throughput (dup)</th></tr>
<tr><td><code>yafd</code></td><td>4.30s</td><td>267.88 MiB/s</td><td>175.70 MiB/s</td></tr>
<tr><td><code>rmlint</code></td><td>7.43s</td><td>155.13 MiB/s</td><td>101.74 MiB/s</td></tr>
<tr><td><code>fdupes</code></td><td>30.34s</td><td>37.99 MiB/s</td><td>24.92 MiB/s</td></tr>
<tr><td><code>duff</code></td><td>25.14s</td><td>45.86 MiB/s</td><td>30.08 MiB/s</td></tr>
<tr><td><code>yafd (cached)</code></td><td>0.61s</td><td>1.84 GiB/s</td><td>1.20 GiB/s</td></tr>
<tr><td><code>rmlint (cached)</code></td><td>2.46s</td><td>466.21 MiB/s</td><td>307.40 MiB/s</td></tr>
<tr><td><code>fdupes (cached)</code></td><td>12.27s</td><td>93.94 MiB/s</td><td>61.61 MiB/s</td></tr>
<tr><td><code>duff (cached)</code></td><td>6.51s</td><td>176.17 MiB/s</td><td>116.12 MiB/s</td></tr>
</table>

**HDD (ext4)**

<table>
<tr><th></th><th>time</th><th>throughput</th><th>throughput (dup)</th></tr>
<tr><td><code>yafd</code></td><td>1087.59s</td><td>1.05 MiB/s</td><td>711.99 KiB/s</td></tr>
<tr><td><code>rmlint</code></td><td>65.03s</td><td>163.46 MiB/s</td><td>107.21 MiB/s</td></tr>
<tr><td><code>fdupes</code></td><td>322.57s</td><td>3.57 MiB/s</td><td>2.34 MiB/s</td></tr>
<tr><td><code>duff</code></td><td>954.70s</td><td>1.20 MiB/s</td><td>811.10 KiB/s</td></tr>
<tr><td><code>yafd (cached)</code></td><td>7.05s</td><td>163.46 MiB/s</td><td>107.21 MiB/s</td></tr>
<tr><td><code>rmlint (cached)</code></td><td>2.84s</td><td>406.37 MiB/s</td><td>266.53 MiB/s</td></tr>
<tr><td><code>fdupes (cached)</code></td><td>12.44s</td><td>92.64 MiB/s</td><td>60.76 MiB/s</td></tr>
<tr><td><code>duff (cached)</code></td><td>6.56s</td><td>175.76 MiB/s</td><td>115.28 MiB/s</td></tr>
</table>

**NFS (v4)**

<table>
<tr><th></th><th>time</th><th>throughput</th><th>throughput (dup)</th></tr>
<tr><td><code>yafd</code></td><td>197.08s</td><td>5.85 MiB/s</td><td>3.83 MiB/s</td></tr>
<tr><td><code>rmlint</code></td><td>461.26s</td><td>2.49 MiB/s</td><td>1.63 MiB/s</td></tr>
<tr><td><code>fdupes</code></td><td>648.24s</td><td>1.77 MiB/s</td><td>1.16 MiB/s</td></tr>
<tr><td><code>duff</code></td><td>466.69s</td><td>2.47 MiB/s</td><td>1.62 MiB/s</td></tr>
<tr><td><code>yafd (cached)</code></td><td>95.04s</td><td>12.13 MiB/s</td><td>7.95 MiB/s</td></tr>
<tr><td><code>rmlint (cached)</code></td><td>423.90s</td><td>2.71 MiB/s</td><td>1.78 MiB/s</td></tr>
<tr><td><code>fdupes (cached)</code></td><td>611.19s</td><td>1.88 MiB/s</td><td>1.23 MiB/s</td></tr>
<tr><td><code>duff (cached)</code></td><td>403.72s</td><td>2.85 MiB/s</td><td>1.87 MiB/s</td></tr>
</table>

(1) The linux sources were searched for identical files (4.3, 4.4)

(2) For an equivalent comparison, the following command arguments were used
(also [see](https://github.com/uxcn/yafd/tree/master/perf/src/python/benchmark))

    yafd --recurse --zero
    rmlint --algorithm=paranoid --hidden -o fdupes:stdout
    fdupes --recurse
    duff -rpta -f#

(3) Linux 4.4.0 and Intel Ivy Bridge (i7-3632QM) were used for benchmarks

## Install ##

You can download a copy of the source
[here](https://github.com/uxcn/yafd/releases) or you can clone the repository
using git.

    jason@io ~ git clone git://github.com:uxcn/yafd.git

It's a good idea to check out a specific release.

    jason@io ~/yafd git checkout v0.1

In the project directory, run the autoconf script.

    jason@io ~/yafd ./autoconf.sh CFLAGS='-march=native -mtune=native -O2'

Adding the architecture allows algorithms that rely on architecutre specific
implementations to be used.  The easiest way to do this is normally
`-march=native`.   You can also explicitly enable instruction sets
via autoconf.

    jason@io ~/yafd ./autoconf.sh --enable-sse4_2

To install to a directory other than /usr/local, you can manually configure the
prefix.  If you do, make sure your `PATH` and `MANPATH` are set correctly.

    jason@io ~/yafd ./autoconf.sh --prefix=$HOME

Run `make install` to compile and install.

    jason@io ~/yafd $ make install

Currently yafd compiles and is tested on Linux, FreeBSD, OSX, and Windows.
Although, patches and pull requests for others are definitely welcome.

## Versions ##

0.1 - alpha release

## FAQ ##

Why write another file deduplicater?

*A lot of the current ones were more complicated than I wanted, didn't perform
well, or weren't portable.*

Why doesn't yafd do *X*?

*Most likely nobody asked for X yet.  If you think something's missing, send a
feature [request](https://github.com/uxcn/yafd/issues) or even better, a [pull
request](https://github.com/uxcn/yafd/pull/new/master).*

How does yafd work?

*The basic algorithm is to group files by their sizes, compute a hash on a small
(random) chunk of each file, and then compare files that have the same hash.
This is a bit of an oversimplicification though.  For a better understanding, it
may help to try reading the
[code](https://github.com/uxcn/yafd/blob/master/src/c/worker.c).*

## other deduplicators ##

* [rmlint](https://github.com/sahib/rmlint)
* [fdupes](https://github.com/adrianlopezroche/fdupes)
* [duff](https://github.com/elmindreda/duff)
* others...
