set PATH=C:\msys64\bin;%PATH%
set PATH=C:\msys64\usr\bin;%PATH%
set PATH=C:\msys64\mingw64\bin;%PATH%

set CC=gcc

REM echo useful info
bash --login -c "$CC -v"

REM appveyor msys configure workaround "exec 0</dev/null"
bash --login -c "cd `cygpath '%CD%'`; exec 0</dev/null; ./autoconf.sh"
bash --login -c "cd `cygpath '%CD%'`; make"
