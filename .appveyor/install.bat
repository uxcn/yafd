set PATH=C:\msys64\bin;%PATH%
set PATH=C:\msys64\usr\bin;%PATH%
set PATH=C:\msys64\mingw64\bin;%PATH%

REM echo useful info
bash --login -c "env"
bash --login -c "echo $PATH"
bash --login -c "pacman -Q"

bash --login -c "pacman -S --noconfirm python"

bash --login -c "curl https://bootstrap.pypa.io/get-pip.py | python"
bash --login -c "pip install cram"
