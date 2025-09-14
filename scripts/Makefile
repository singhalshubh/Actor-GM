include $(HCLIB_ROOT)/../modules/bale_actor/inc/hclib_bale_actor.pre.mak
include $(HCLIB_ROOT)/include/hclib.mak
include $(HCLIB_ROOT)/../modules/bale_actor/inc/hclib_bale_actor.post.mak

all: clean rgg_to_mtx

rgg_to_mtx: rgg_to_mtx.cpp 
	$(CXX) -g -O3 -std=c++17 -DUSE_SHMEM=1 -ftrapv $(HCLIB_CFLAGS) $(HCLIB_LDFLAGS) -o $@ $^ $(HCLIB_LDLIBS)  -lconvey -llibgetput -lhclib_bale_actor -lm

clean:
	rm -rf rgg_to_mtx