ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(PO6_CFLAGS)
AM_CFLAGS = $(PTHREAD_CFLAGS) $(WANAL_CFLAGS)
AM_CXXFLAGS = $(PO6_CFLAGS) $(PTHREAD_CFLAGS) $(WANAL_CXXFLAGS)
AM_MAKEFLAGS = --no-print-directory

EXTRA_DIST = LICENSE README THIRD-PARTY

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libe.pc

nobase_include_HEADERS =
nobase_include_HEADERS += e/ao_hash_map.h
nobase_include_HEADERS += e/arena.h
nobase_include_HEADERS += e/array_ptr.h
nobase_include_HEADERS += e/atomic.h
nobase_include_HEADERS += e/base64.h
nobase_include_HEADERS += e/bitsteal.h
nobase_include_HEADERS += e/buffer.h
nobase_include_HEADERS += e/compat.h
nobase_include_HEADERS += e/daemon.h
nobase_include_HEADERS += e/daemonize.h
nobase_include_HEADERS += e/endian.h
nobase_include_HEADERS += e/error.h
nobase_include_HEADERS += e/flagfd.h
nobase_include_HEADERS += e/garbage_collector.h
nobase_include_HEADERS += e/guard.h
nobase_include_HEADERS += e/hazard_ptrs.h
nobase_include_HEADERS += e/identity.h
nobase_include_HEADERS += e/intrusive_ptr.h
nobase_include_HEADERS += e/lockfile.h
nobase_include_HEADERS += e/lockfree_fifo.h
nobase_include_HEADERS += e/lockfree_hash_map.h
nobase_include_HEADERS += e/lockfree_mpsc_fifo.h
nobase_include_HEADERS += e/lookup3.h
nobase_include_HEADERS += e/nwf_hash_map.h
nobase_include_HEADERS += e/popt.h
nobase_include_HEADERS += e/pow2.h
nobase_include_HEADERS += e/safe_math.h
nobase_include_HEADERS += e/seqno_collector.h
nobase_include_HEADERS += e/serialization.h
nobase_include_HEADERS += e/slice.h
nobase_include_HEADERS += e/state_hash_table.h
nobase_include_HEADERS += e/strescape.h
nobase_include_HEADERS += e/subcommand.h
nobase_include_HEADERS += e/tuple_compare.h
nobase_include_HEADERS += e/varint.h

noinst_HEADERS =
noinst_HEADERS += file_lock_table.h

#################################### Source ####################################

lib_LTLIBRARIES = libe.la
libe_la_SOURCES  =
libe_la_SOURCES += arena.cc
libe_la_SOURCES += atomic.cc
libe_la_SOURCES += base64.cc
libe_la_SOURCES += buffer.cc
libe_la_SOURCES += endian.cc
libe_la_SOURCES += error.cc
libe_la_SOURCES += file_lock_table.cc
libe_la_SOURCES += flagfd.cc
libe_la_SOURCES += garbage_collector.cc
libe_la_SOURCES += identity.cc
libe_la_SOURCES += lockfile.cc
libe_la_SOURCES += lookup3.c
libe_la_SOURCES += lookup3-wrap.cc
libe_la_SOURCES += seqno_collector.cc
libe_la_SOURCES += serialization.cc
libe_la_SOURCES += slice.cc
libe_la_SOURCES += strescape.cc
libe_la_SOURCES += varint.cc
libe_la_LIBADD =
libe_la_LIBADD += $(PO6_LIBS)
libe_la_LIBADD += $(PTHREAD_LIBS)
libe_la_LDFLAGS = -version-info 5:0:0

##################################### Tests ####################################

th_sources = test/runner.cc th.cc th.h

TESTS = $(check_PROGRAMS)
check_PROGRAMS =
check_PROGRAMS += test/array_ptr
check_PROGRAMS += test/bitsteal
check_PROGRAMS += test/buffer
check_PROGRAMS += test/endian
check_PROGRAMS += test/guard
check_PROGRAMS += test/intrusive_ptr
check_PROGRAMS += test/pow2
check_PROGRAMS += test/safe_math
check_PROGRAMS += test/seqno_collector
check_PROGRAMS += test/varint

test_array_ptr_SOURCES = test/array_ptr.cc $(th_sources)
test_bitsteal_SOURCES = test/bitsteal.cc $(th_sources)
test_buffer_SOURCES = test/buffer.cc $(th_sources)
test_buffer_LDADD = libe.la
test_endian_SOURCES = test/endian.cc $(th_sources)
test_endian_LDADD = libe.la
test_guard_SOURCES = test/guard.cc $(th_sources)
test_intrusive_ptr_SOURCES = test/intrusive_ptr.cc $(th_sources)
test_pow2_SOURCES = test/pow2.cc $(th_sources)
test_safe_math_SOURCES = test/safe_math.cc $(th_sources)
test_seqno_collector_SOURCES = test/seqno_collector.cc $(th_sources)
test_seqno_collector_LDADD = libe.la
test_varint_SOURCES = test/varint.cc $(th_sources)
test_varint_LDADD = libe.la
