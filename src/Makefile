include $(HCLIB_ROOT)/../modules/bale_actor/inc/hclib_bale_actor.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/bale_actor/inc/hclib_bale_actor.post.mak

TARGETS= clean main

SRC := main.cpp

all: clean main main_optimized block block_optimized

main: $(SRC)
	$(CXX) -g -O3 -std=c++17 -DUSE_SHMEM=1 -ftrapv $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)  -lconvey -llibgetput -lhclib_bale_actor -lm

main_optimized: $(SRC)
	$(CXX) -g -O3 -std=c++17 -DOPTIMIZED -DUSE_SHMEM=1 -ftrapv $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)  -lconvey -llibgetput -lhclib_bale_actor -lm

block: $(SRC)
	$(CXX) -g -O3 -std=c++17 -DBLOCK_D -DUSE_SHMEM=1 -ftrapv $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)  -lconvey -llibgetput -lhclib_bale_actor -lm

block_optimized: $(SRC)
	$(CXX) -g -O3 -std=c++17 -DOPTIMIZED -DBLOCK_D -DUSE_SHMEM=1 -ftrapv $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)  -lconvey -llibgetput -lhclib_bale_actor -lm

clean:
	rm -rf main main_optimized block block_optimized