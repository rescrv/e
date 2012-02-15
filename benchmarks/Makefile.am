AM_CPPFLAGS = -I$(abs_top_srcdir)/include $(PO6_CFLAGS)
noinst_PROGRAMS = lockfree_fifo lockfree_hash_map lockfree_hash_set locking_iterable_fifo

lockfree_fifo_SOURCES = lockfree_fifo.cc
lockfree_fifo_LDADD = -lpthread
lockfree_hash_map_SOURCES = lockfree_hash_map.cc
lockfree_hash_map_LDADD = -lpthread
lockfree_hash_set_SOURCES = lockfree_hash_set.cc
lockfree_hash_set_LDADD = -lpthread
locking_iterable_fifo_SOURCES = locking_iterable_fifo.cc
locking_iterable_fifo_LDADD = -lpthread -lrt
