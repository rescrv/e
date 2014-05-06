// Header file for lookup3.c by Bob Jenkins
// lookup3.c, by Bob Jenkins, May 2006, Public Domain.

#ifndef e_lookup3_h_
#define e_lookup3_h_

// C
#include <stdint.h>

namespace e
{

// These functions wrap the lookup3.c functions provided by Bob Jenkins.  The
// library exposes those functions directly, but I prefer using this wrapper
// instead:
uint64_t lookup3_64(uint64_t in);

} // namespace e

#endif // e_lookup3_h_
