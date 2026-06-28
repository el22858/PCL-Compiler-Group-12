# make -C libs/
# as lib.s -o lib.o
# rm lib.s

# make -C libs/cast
make -C libs/io
# make -C libs/maths
ar -cvqs lib.a libs/io/*.o #libs/maths/*.o libs/cast/*.o
# make clean -C libs/maths
make clean -C libs/io
# make clean -C libs/cast