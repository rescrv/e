ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(PO6_CFLAGS)
AM_CFLAGS = $(WANAL_CFLAGS)
AM_CXXFLAGS = $(PO6_CFLAGS) $(WANAL_CXXFLAGS)
AM_MAKEFLAGS = --no-print-directory

EXTRA_DIST = LICENSE README THIRD-PARTY

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libe.pc

nobase_include_HEADERS =
nobase_include_HEADERS += e/ao_hash_map.h
nobase_include_HEADERS += e/array_ptr.h
nobase_include_HEADERS += e/atomic.h
nobase_include_HEADERS += e/bitsteal.h
nobase_include_HEADERS += e/buffer.h
nobase_include_HEADERS += e/compat.h
nobase_include_HEADERS += e/convert.h
nobase_include_HEADERS += e/endian.h
nobase_include_HEADERS += e/envconfig.h
nobase_include_HEADERS += e/error.h
nobase_include_HEADERS += e/garbage_collector.h
nobase_include_HEADERS += e/guard.h
nobase_include_HEADERS += e/hazard_ptrs.h
nobase_include_HEADERS += e/intrusive_ptr.h
nobase_include_HEADERS += e/lockfree_fifo.h
nobase_include_HEADERS += e/lockfree_hash_map.h
nobase_include_HEADERS += e/lockfree_hash_set.h
nobase_include_HEADERS += e/locking_fifo.h
nobase_include_HEADERS += e/lookup3.h
nobase_include_HEADERS += e/nonblocking_bounded_fifo.h
nobase_include_HEADERS += e/nwf_hash_map.h
nobase_include_HEADERS += e/popt.h
nobase_include_HEADERS += e/pow2.h
nobase_include_HEADERS += e/safe_math.h
nobase_include_HEADERS += e/slice.h
nobase_include_HEADERS += e/strescape.h
nobase_include_HEADERS += e/striped_lock.h
nobase_include_HEADERS += e/subcommand.h
nobase_include_HEADERS += e/time.h
nobase_include_HEADERS += e/tuple_compare.h
nobase_include_HEADERS += e/unpacker.h
nobase_include_HEADERS += e/varint.h

#################################### Source ####################################

lib_LTLIBRARIES = libe.la
libe_la_SOURCES = atomic.cc buffer.cc garbage_collector.cc lookup3.c lookup3-wrap.cc slice.cc time.cc unpacker.cc varint.cc
libe_la_LDFLAGS = $(LRT_LDFLAGS) -version-info 1:0:0

##################################### Tests ####################################

th_sources = test/runner.cc th.cc th.h

TESTS = $(check_PROGRAMS)
check_PROGRAMS =
check_PROGRAMS += test/array_ptr
check_PROGRAMS += test/bitsteal
check_PROGRAMS += test/buffer
check_PROGRAMS += test/convert
check_PROGRAMS += test/endian
check_PROGRAMS += test/guard
check_PROGRAMS += test/intrusive_ptr
check_PROGRAMS += test/nonblocking_bounded_fifo
check_PROGRAMS += test/safe_math
check_PROGRAMS += test/varint

test_array_ptr_SOURCES = test/array_ptr.cc $(th_sources)
test_bitsteal_SOURCES = test/bitsteal.cc $(th_sources)
test_buffer_SOURCES = test/buffer.cc $(th_sources)
test_buffer_LDADD = libe.la
test_convert_SOURCES = test/convert.cc $(th_sources)
test_endian_SOURCES = test/endian.cc $(th_sources)
test_guard_SOURCES = test/guard.cc $(th_sources)
test_intrusive_ptr_SOURCES = test/intrusive_ptr.cc $(th_sources)
test_nonblocking_bounded_fifo_SOURCES = test/nonblocking_bounded_fifo.cc $(th_sources)
test_safe_math_SOURCES = test/safe_math.cc $(th_sources)
test_varint_SOURCES = test/varint.cc $(th_sources)
test_varint_LDADD = libe.la
