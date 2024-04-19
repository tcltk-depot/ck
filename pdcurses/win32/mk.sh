TARGET=/opt/mingw/bin/i386-mingw32msvc- make -C wincon WIDE=Y INFOEX=N
mv wincon/pdcurses.a libpdcurses.a
make -C wincon clean
