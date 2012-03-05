if HAVE_GTEST
AM_CPPFLAGS = -I$(abs_top_srcdir)/include $(PO6_CFLAGS) $(GTEST_CPPFLAGS)
AM_LDFLAGS = $(GTEST_LDFLAGS)

TESTS = $(check_PROGRAMS)
check_PROGRAMS = bitfield bitsteal buffer convert guard intrusive_ptr \
				 locking_iterable_fifo profiler
bitfield_SOURCES = runner.cc bitfield.cc
bitfield_LDADD = ../src/libe.la ${GTEST_LIBS}
bitsteal_SOURCES = runner.cc bitsteal.cc
bitsteal_LDADD = $(GTEST_LIBS)
buffer_SOURCES = runner.cc buffer.cc
buffer_LDADD = ../src/libe.la $(GTEST_LIBS)
convert_SOURCES = runner.cc convert.cc
convert_LDADD = $(GTEST_LIBS)
guard_SOURCES = runner.cc guard.cc
guard_LDADD = $(GTEST_LIBS)
intrusive_ptr_SOURCES = runner.cc intrusive_ptr.cc
intrusive_ptr_LDADD = $(GTEST_LIBS)
locking_iterable_fifo_SOURCES = runner.cc locking_iterable_fifo.cc
locking_iterable_fifo_LDADD = $(GTEST_LIBS)
profiler_SOURCES = runner.cc profiler.cc
profiler_LDADD = -lrt $(GTEST_LIBS)
endif
