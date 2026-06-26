# make -C libs/
# as lib.s -o lib.o
# rm lib.s

make -C libs/io
ar -cvqs lib.a libs/io/*.o
make clean -C libs/io