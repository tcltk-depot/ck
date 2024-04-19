TARGET=/opt/mingw64/bin/x86_64-w64-mingw32- make -C wincon WIDE=Y
mv wincon/pdcurses.a libpdcurses.a
make -C wincon clean
