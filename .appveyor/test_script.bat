set PATH=C:\msys64\bin;%PATH%
set PATH=C:\msys64\usr\bin;%PATH%
set PATH=C:\msys64\mingw64\bin;%PATH%

bash --login -c "cd `cygpath '%CD%'`; make test"
