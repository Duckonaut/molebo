run: build
    desmume molebo.nds

rebuild:
    make clean
    make tools
    make assets
    make

build:
    make
