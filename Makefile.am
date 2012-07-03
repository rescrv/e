ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(abs_top_srcdir) $(PO6_CFLAGS)
AM_CXXFLAGS = $(WANAL_CXXFLAGS)
AM_LDFLAGS =

EXTRA_DIST = LICENSE README THIRD-PARTY

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libe.pc

nobase_include_HEADERS = e/array_ptr.h e/atomic.h e/bitfield.h e/bitsteal.h e/buffer.h \
						 e/bufferio.h e/convert.h e/endian.h e/envconfig.h \
						 e/guard.h e/hazard_ptrs.h e/intrusive_ptr.h \
						 e/lockfree_fifo.h e/lockfree_hash_map.h \
						 e/lockfree_hash_set.h e/locking_fifo.h \
						 e/locking_iterable_fifo.h \
						 e/microbench.h \
						 e/nonblocking_bounded_fifo.h e/pow2.h e/profiler.h \
						 e/safe_math.h e/slice.h e/striped_lock.h e/timer.h \
						 e/tuple_compare.h e/worker_barrier.h

#################################### Source ####################################

lib_LTLIBRARIES = libe.la
libe_la_SOURCES = atomic.cc buffer.cc slice.cc

##################################### Tests ####################################

if HAVE_GTEST
AM_CPPFLAGS += $(GTEST_CPPFLAGS)
AM_LDFLAGS += $(GTEST_LDFLAGS)

TESTS = $(check_PROGRAMS)
check_PROGRAMS = \
			test/array_ptr \
			test/bitfield \
			test/bitsteal \
			test/buffer \
			test/convert \
			test/endian \
			test/guard \
			test/intrusive_ptr \
			test/locking_iterable_fifo \
			test/nonblocking_bounded_fifo \
			test/profiler \
			test/safe_math

test_array_ptr_SOURCES = test/runner.cc test/array_ptr.cc
test_array_ptr_LDADD = libe.la ${GTEST_LIBS}
test_bitfield_SOURCES = test/runner.cc test/bitfield.cc
test_bitfield_LDADD = libe.la ${GTEST_LIBS}
test_bitsteal_SOURCES = test/runner.cc test/bitsteal.cc
test_bitsteal_LDADD = $(GTEST_LIBS)
test_buffer_SOURCES = test/runner.cc test/buffer.cc
test_buffer_LDADD = libe.la $(GTEST_LIBS)
test_convert_SOURCES = test/runner.cc test/convert.cc
test_convert_LDADD = $(GTEST_LIBS)
test_endian_SOURCES = test/runner.cc test/endian.cc
test_endian_LDADD = $(GTEST_LIBS)
test_guard_SOURCES = test/runner.cc test/guard.cc
test_guard_LDADD = $(GTEST_LIBS)
test_intrusive_ptr_SOURCES = test/runner.cc test/intrusive_ptr.cc
test_intrusive_ptr_LDADD = $(GTEST_LIBS)
test_locking_iterable_fifo_SOURCES = test/runner.cc test/locking_iterable_fifo.cc
test_locking_iterable_fifo_LDADD = $(GTEST_LIBS)
test_nonblocking_bounded_fifo_SOURCES = test/runner.cc test/nonblocking_bounded_fifo.cc
test_nonblocking_bounded_fifo_LDADD = $(GTEST_LIBS)
test_profiler_SOURCES = test/runner.cc test/profiler.cc
test_profiler_LDADD = -lrt $(GTEST_LIBS)
test_safe_math_SOURCES = test/runner.cc test/safe_math.cc
test_safe_math_LDADD = -lrt $(GTEST_LIBS)
endif
