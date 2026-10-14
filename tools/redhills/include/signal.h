/*
 * <signal.h> for the RedHills (Green Hills ABI) build.
 *
 * The Green Hills C library's <signal.h> names its highest signal number _SIGMAX (as SIGUSR2), and the
 * Cafe libsys port (source/cafe/libsys/ind_sgnl.cpp) sizes its handler table with it. newlib's header
 * has no such name; this one adds it on top of newlib's numbering.
 */

#ifndef REDHILLS_SIGNAL_H
#define REDHILLS_SIGNAL_H

#include_next <signal.h>

#ifndef _SIGMAX
#define _SIGMAX SIGUSR2
#endif

#endif
