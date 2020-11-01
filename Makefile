CPP=clang++

saam : src/Array.hpp src/fb_wrapper@*.cc src/imageArray.hpp src/main.cc src/media.hpp src/saam.hpp src/saam_data.hpp src/saam_graphic.cc src/saam_search.cc src/tuiapp.cc src/tuiapp.hpp src/impl@posix/fb_impl@*.cc src/impl@posix/ld_impl@*.cc src/impl@posix/md_impl@*.cc
	mkdir -p ./make/posix; cd ./make/posix; $(CPP) -Wall -O3 ../../src/*.cc ../../src/impl@posix/*.cc -o saam -lpthread `sdl2-config --cflags --libs` -lSDL2_image -lSDL2_mixer
