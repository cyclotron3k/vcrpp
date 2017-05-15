25 July 2003

---------------------------------------------------------------------

From: Anonymous
Subject: VCRplus+ Code that wants to be free

I'm not an expert on anonymous posting/distribution, so I'm
requesting your help with this.

Background:

In 1992, Ken Shirriff, Curt Welch and Andrew Kinsman published
a partial analysis of the VCRplus+ "PlusCode" coding scheme
in Cryptologia 16(3) July 1992, pp 227-234.

They figured out the codes of length 1 through 6, but didn't
break the 7 and 8 digit codes.

Later, programs implementing their logic appeared on the net,
adding encoding as well as decoding.

New stuff:

This program expands on those programs, and does the 7 and 8
digit codes, which includes TV/cable channels over 64 and
times not an even multiple of 30 minutes, and/or not lasting
an even multiple of a half hour.

It is therefore useful for personal recording of local sources at
non-round-number times or on non-broadcast channels, and for
generating codes that aren't in the newspaper.

The (encoding) algorithm is not fast enough to be used for
commercial purposes, having a brute-force section in it.  But
someone might be able to improve on that.

I am not breaking any confidentiality agreements.  I have no
access to inside information on the system.  This was done
by reverse engineering.

Please consider posting this on cryptome, under the crypto
category.



/*******************************************************************\
 * VCRplus+ Encoding, Decoding and regression test code.
 * Works with standard USA codes, 1 through 8 digits long,
 * without leading zeroes.
 *
 * This program is released to the public domain.
\*******************************************************************/


/*******************************************************************\
 * System include files
\*******************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


/*******************************************************************\
 * Preprocessor constants
\*******************************************************************/

#define		KEY001 (68150631)
#define		KEY002	(9371)
#define		NDIGITS 10

/*******************************************************************\
 * Globals
\*******************************************************************/

int             g_iflag;	/* Omit initial scramble on decode/encode */
int             g_debug;	/* Bits enable debug output */
int             g_verbose;	/* More typeout, but not debug stuff */
int             g_encode;	/* 0 = decode, 1 = encode */

int             g_newspaper;
char           *g_progname;
char           *g_channel_name;
int             g_year_today;
int             g_month_today;
int             g_day_today;
int             g_channel;
int             g_starttime;
int             g_starttimem;
int             g_duration;
int             g_durationm;

void            main_debug (void);
void            clear_ndigits (unsigned char *);
void            split_digits (int n, unsigned char *a);
int             hhmm2mmmm (int);
int             mmmm2hhmm (int);
int             count_digits (int);
int             set_pwr (int);
/*******************************************************************\
 * Tables
\*******************************************************************/

/* regression test codes, to make sure things didn't get broken */
/* Inconsistent: starts are hhmm, durations are mmmm */
/* Table trimmed for anonymity */

typedef struct test {
    int             code;
    int             year;
    int             month;
    int             day;
    int             chan;
    int             start;
    int             duration;
}               _test;

static _test    tests[] = {
    {3316, 1991, 5, 10, 4, 2100, 120},
    {21362, 1992, 3, 11, 24, 1930, 30},
/* ... */
    {0, 0, 0, 0, 0, 0, 0}
};
/* General utilities and data structures */

/* List of month names, so we can speak English */
static char     monthname[][12] = {
    {"January"},
    {"February"},
    {"March"},
    {"April"},
    {"May"},
    {"June"},
    {"July"},
    {"August"},
    {"September"},
    {"October"},
    {"November"},
    {"December"}
};
/* TV guide channel assignments */
typedef struct {
    int             num;
    char           *name;
}               numname;

static
numname         chan_to_name[] = {	/* Trimmed for anonymity */
    {02, "WAAB2"},
    {03, "WAAC3"},
    {04, "WAAD4"},
    {05, "WAAE5"},
    {06, "WAAF6"},
    {07, "WAAG7"},
    {8, "WAAH8"},
    {9, "WAAI9"},
    {10, "WAAJ10"},
    {11, "WAAK11"},
    {12, "WAAL12"},
    {13, "WAAM13"},
    {33, "HBO"},
    {35, "AMC"},
    {37, "DSC"},
    {38, "NIK"},
    {39, "A&E"},
    {41, "SHO"},
    {42, "CNN"},
    {43, "TBS"},
    {44, "USA"},
    {45, "MAX"},
    {46, "LIF"},
    {58, "TMC"},
    {62, "VH1"},
    {63, "E"},
    {75, "COM"},
    {80, "STARZ"},
    {89, "SCI"},
    {90, "MSNBC"},
    {99, "QVC"},
    {103, "HAL"},
    {-1, "???"},
};

unsigned char ttbl[192] = {
	(18 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(16 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(19 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(16 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(15 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(17 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(18 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(14 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(19 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(17 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(14 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(20 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(17 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(16 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(20 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(15 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(20 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(21 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(20 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(18 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(19 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(22 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(21 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(14 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(15 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(22 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(11 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(11 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(23 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(16 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(21 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(21 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(12 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(13 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 9 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(13 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(21 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(12 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(10 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(18 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(22 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(12 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 8 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 8 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(17 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 9 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(22 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(10 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(19 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 7 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(23 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(10 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 7 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(13 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 7 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(11 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(14 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(10 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 8 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(23 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(13 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(12 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 9 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 6 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(18 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 6 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 5 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 0 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(23 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(22 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(13 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 9 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(16 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(16 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(14 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(20 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(18 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 6 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(12 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 0 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 1 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 0 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(17 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 0 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 8 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 7 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(21 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 5 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(15 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(11 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(11 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 8 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(22 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 9 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(21 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(16 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(23 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 1 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(14 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 1 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 3 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(15 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(15 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(23 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(19 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 8 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 4 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 3 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(13 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(10 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 7 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 1 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(23 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 3 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 2 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	(22 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 4 * 2) + (00 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 6 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 4 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 2 * 2) + (30 ? 1 : 0) + ((( 30 - 30)/30) * 48) ,
	( 6 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 0 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(22 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 1 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 0 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(23 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(16 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 8 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 0 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(19 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 9 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(20 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 5 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(17 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 2 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(19 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 9 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(17 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 6 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(18 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(14 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(11 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 0 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 8 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(10 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(14 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 1 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 7 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(20 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 3 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 3 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(13 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(12 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 2 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(21 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(11 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(18 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 6 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 5 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 2 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(15 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 7 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 6 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(17 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 4 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 7 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 4 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 4 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 1 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(12 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 1 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 2 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(19 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 3 * 2) + (00 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(10 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 2 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 3 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 5 * 2) + (00 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 9 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 2 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(20 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 4 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(15 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 4 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	(13 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	(12 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 3 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(10 * 2) + (30 ? 1 : 0) + ((( 60 - 30)/30) * 48) ,
	( 5 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	( 5 * 2) + (30 ? 1 : 0) + (((120 - 30)/30) * 48) ,
	( 5 * 2) + (30 ? 1 : 0) + ((( 90 - 30)/30) * 48) ,
	(11 * 2) + (00 ? 1 : 0) + ((( 90 - 30)/30) * 48)
};

/* A table of additional scrambling constants,
 * used only for codes of length 7 and 8.
 */

unsigned char   OverSixTable[31] = {
    0x25, 0x13, 0x34, 0x14, 0x52, 0x25, 0x25, 0x43,
    0x45, 0x14, 0x43, 0x51, 0x21, 0x12, 0x23, 0x43,
    0x51, 0x32, 0x35, 0x52, 0x34, 0x25, 0x34, 0x52,
    0x13, 0x15, 0x41, 0x35, 0x52, 0x51, 0x12
};
/*******************************************************************\
 * Utility routines
\*******************************************************************/

void 
dumpnames (void)
{
    numname        *nnp;
    for (nnp = chan_to_name;; nnp++) {
	if (nnp->num < 0)
	    break;
	printf ("%d\t%s\n", nnp->num, nnp->name);
    }
}

char           *
channame (int chan)
{
    numname        *nnp;
    for (nnp = chan_to_name;; nnp++) {
	if (nnp->num < 0)
	    break;
	if (nnp->num == chan)
	    break;
    }
    return (nnp->name);
}
/* Usage message and exit */
static void
usage (void)
{
    printf (
	    "Usage: %s [-eivd] <flags> [vcrplus+_code]\n"
	    "\tflags:\n"
	    "\t[-y year] [-m month] [-d day]\n"
	    "\t[-c channel] [-t hhmm] [-l hhmm]\n",
	    g_progname);
    printf ("\tDefaults: year, month, day = today. time = 1930, length = 30\n");
    printf ("\tExample decode: %s -y 1991 -m 5 3316\n", g_progname);
    printf ("\tExample encode: %s -e -y 1991 -m 5 -d 10 -c 4 -t 2100 -l 200\n",
	    g_progname);
}

static void
usagex (void)
{
    usage ();
    exit (-1);
}

static void 
errdie (char *s)
{
    printf ("ERROR: %s\n", s);
    usagex ();
}
/* Convert minutes of the day into friendly time with AM/PM */
/* The string that is returned is overwritten each time we're called */
/* Example return strings: "9 AM", "5:30 PM", "12:15 PM", "12 noon", "12 midnight" */
/* <xxx@xxx.xxx.xxx> */

static char    *
timestr (int t)
{
    int             hour, min;
    static char     str[256];
    char            ampm[10], hm[10];
    hour = t / 100;
    min = t % 100;
    if (hour < 0 || hour > 24) {
	fprintf (stderr, "timestr: I'm confused, time %d implies hour = %d which is bad!\n", t, hour);
	exit (-1);
    }
    if (min < 0 || min > 59) {
	fprintf (stderr, "timestr: I'm confused, time %d implies minute = %d which is bad!\n", t, min);
	exit (-1);
    }
    if (hour == 24 && min > 0) {
	fprintf (stderr, "timestr: I'm confused, time %d is beyond 24 hours range!\n", t, min);
	exit (-1);
    }
    if (hour < 12) {
	strcpy (ampm, "AM");
    } else {
	strcpy (ampm, "PM");
    }
    if (min == 0) {
	if (hour == 0 || hour == 24) {
	    strcpy (ampm, "midnight");
	} else if (hour == 12) {
	    strcpy (ampm, "noon");
	}
    }
    if (hour == 0)
	hour = 12;
    if (hour > 12)
	hour -= 12;
    if (min == 0)
	sprintf (hm, "%d", hour);
    else
	sprintf (hm, "%d:%02d", hour, min);

    sprintf (str, "%s %s", hm, ampm);
    return (str);
}

int
count_digits (int val)
{
    int             ndigits;
    if (val < 0) {
	printf ("Error: code 0 or negative\n");
	ndigits = 0;
    } else if (val < 1)
	ndigits = 0;
    else if (val < 10)
	ndigits = 1;
    else if (val < 100)
	ndigits = 2;
    else if (val < 1000)
	ndigits = 3;
    else if (val < 10000)
	ndigits = 4;
    else if (val < 100000)
	ndigits = 5;
    else if (val < 1000000)
	ndigits = 6;
    else if (val < 10000000)
	ndigits = 7;
    else if (val < 100000000)
	ndigits = 8;
    else
	ndigits = 9;

    if (ndigits > 8) {
	printf ("ERROR: %d has more than 8 digits (it has %d digits)\n",
		val, ndigits);
	usagex ();
    }
    return ndigits;
}

int
set_pwr (int ndigits)
{
    int             pwr = 0x7fffffff;
    if (0 == ndigits)
	pwr = 1;
    if (1 == ndigits)
	pwr = 10;
    if (2 == ndigits)
	pwr = 100;
    if (3 == ndigits)
	pwr = 1000;
    if (4 == ndigits)
	pwr = 10000;
    if (5 == ndigits)
	pwr = 100000;
    if (6 == ndigits)
	pwr = 1000000;
    if (7 == ndigits)
	pwr = 10000000;
    if (8 == ndigits)
	pwr = 100000000;
    if (9 == ndigits)
	pwr = 1000000000;
    return pwr;
}

void
clear_ndigits (unsigned char *a)
{
    int             i;
    for (i = 0; i < NDIGITS; i++) {
	a[i] = 0;
    }
}

void
split_digits (int n, unsigned char *a)
{
    int             i;
    unsigned char   digit;
    clear_ndigits (a);

    for (i = 0; i < NDIGITS; i++) {
	digit = n % 10;
	a[i] = digit;
	n = (n - digit) / 10;
    }
}

int 
hhmm2mmmm (int hhmm)
{
    int             hh, mm;
    hh = hhmm / 100;
    mm = hhmm % 100;
    if (mm > 59) {
	printf ("ERROR: Minutes too large in %d:%2d - truncated to %d:00\n",
		hh, mm, hh);
	mm = 0;
    }
    return (hh * 60) + mm;
}

int 
mmmm2hhmm (int mmmm)
{
    int             hh, mm;
    hh = mmmm / 60;
    mm = mmmm % 60;
    return (hh * 100) + mm;
}
/*******************************************************************\
 * Routines for VCRplus+ encoding/decoding
\*******************************************************************/

/* func1, for decoding only */

int
func1 (int code)
{
    int             x = code;
    int             nd;
    unsigned char   a[NDIGITS];
    int             sum;
    int             i, j;
    int             ndigits;
    split_digits (x, a);
    ndigits = count_digits (x);
    nd = ndigits - 1;

    do {
	i = 0;
	do {
	    j = 1;
	    if (nd >= 1) {
		do {
		    a[j] = (a[j - 1] + a[j]) % 10;
		} while (++j <= nd);
	    }
	} while (++i <= 2);
    } while (a[nd] == 0);

    sum = 0;
    j = 1;
    for (i = 0; i < NDIGITS; i++) {
	sum += j * a[i];
	j *= 10;
    }
    return sum;
}


static int
encode_final_transform (x, y)
int             x, y;
{
    int             i, j, digit, sum;
    int             a[9], b[9], out[17];

    for (i = 0; i < 9; i++) {
	digit = x % 10;
	a[i] = digit;
	x = (x - digit) / 10;
    }

    for (i = 0; i < 9; i++) {
	digit = y % 10;
	b[i] = digit;
	y = (y - digit) / 10;
    }

    for (i = 0; i < 17; i++) {
	out[i] = 0;
    }

    for (i = 0; i <= 8; i++) {
	for (j = 0; j <= 8; j++) {
	    out[i + j] += b[j] * a[i];
	}
    }

    j = 1;
    sum = 0;
    for (i = 0; i <= 8; i++) {
	sum += j * (out[i] % 10);
	j *= 10;
    }
    return (sum);
}


static int
encfunc1 (val)
int             val;
{
    int             ndigits, pwr;
    ndigits = 0;
    pwr = 1;
    while (val >= pwr) {
	ndigits++;
	pwr *= 10;
    }
    if (ndigits > 8) {
	printf ("ERROR: %d has more than 8 digits (it has %d digits)\n", val, ndigits);
	usagex ();
    }
    pwr /= 10;

    if (0 == g_iflag) {
	do {
	    val = encode_final_transform (val, KEY002) % (pwr * 10);
	} while (val < pwr);
    }
    return (val);
}
/* Extra modifications to the time/duration for 7 and 8 digit codes */

void
twiddle_tt (int tidx, int *tp, int *dp, int *x1p, int *x2p)
{
    int             tt, x1, x2, t, d;
    int             t1, t2, b;
    x1 = x2 = t1 = t2 = d = b = t = 0;
    tt = tidx;

    if ((tt >= 768) && (tt <= 3647)) {
	t1 = ((tt - 768) % 10) + 1;
	t2 = t1 * 5;
	if (t1 >= 6) {
	    x2 = t2 - 25;
	} else {
	    x1 = t2;
	}
	tt -= 768;
	tt = tt / 10;
    } else if ((tt >= 3648) && (tt <= 6527)) {
	t1 = ((tt - 3648) % 6) + 1;
	if (t1 == 1) {
	    x1 = 15;
	} else {
	    x2 = (5 * t1) - 5;
	}
	tt = ((tt - 3648) / 6) + 288;
    } else if ((tt >= 6528) && (tt <= 13727)) {
	x1 = 5 + ((((tt - 6528) % 25) % 5) * 5);
	x2 = 5 + ((((tt - 6528) % 25) / 5) * 5);
	tt -= 6528;
	tt = tt / 25;
    } else if (tt >= 13728) {
	x2 = 5 + (((tt - 13728) % 5) * 5);
	x1 = 15;
	tt -= 13728;
	tt = (tt / 5) + 288;
    }
    if (tt < 192) {
	/* Lookup in table */
	b = ttbl[tt];
	t = b % 48;		/* Half hours after midnight  */
	d = b / 48;		/* Half hours of duration - 1 */
    } else {
	t = 47 - ((tt - 192) % 48);	/* Half hours after midnight  */
	d = tt / 48;		/* Half hours of duration - 1 */
    }

    *tp = t;
    *dp = d;
    *x1p = x1;			/* Shorter duration */
    *x2p = x2;			/* Later start time */
}

static void
bit_shuffle (code, tval, dval, cval)
int             code;
unsigned int   *tval;
unsigned int   *dval;
unsigned int   *cval;
{
    unsigned int    tt, cc;
    int             x1, x2;
    int             nn;
    int             top5;
    int             bot3;
    int             rem;
    int             t, d, outtime, outdur;
    tt = 0;
    cc = 0;
    x1 = x2 = 0;
    nn = code - 1;
    top5 = nn / 1000;
    bot3 = nn % 1000;
    rem = bot3 & 0x1f;

/* Time/duration */
    if (rem & (1 << 0))
	tt |= (1 << 0);
    if (rem & (1 << 2))
	tt |= (1 << 1);
    if (rem & (1 << 4))
	tt |= (1 << 2);
    if (top5 & (1 << 0))
	tt |= (1 << 3);
    if (top5 & (1 << 3))
	tt |= (1 << 4);
    if (top5 & (1 << 4))
	tt |= (1 << 5);
    if (top5 & (1 << 5))
	tt |= (1 << 6);
    if (top5 & (1 << 7))
	tt |= (1 << 7);
    if (top5 & (1 << 9))
	tt |= (1 << 8);
    if (top5 & (1 << 10))
	tt |= (1 << 9);
    if (top5 & (1 << 11))
	tt |= (1 << 10);
    if (top5 & (1 << 13))
	tt |= (1 << 11);
    if (top5 & (1 << 14))
	tt |= (1 << 12);
    if (top5 & (1 << 15))
	tt |= (1 << 13);

/* Channel */
    if (rem & (1 << 1))
	cc |= (1 << 0);
    if (rem & (1 << 3))
	cc |= (1 << 1);
    if (top5 & (1 << 1))
	cc |= (1 << 2);
    if (top5 & (1 << 2))
	cc |= (1 << 3);
    if (top5 & (1 << 6))
	cc |= (1 << 4);
    if (top5 & (1 << 8))
	cc |= (1 << 5);
    if (top5 & (1 << 12))
	cc |= (1 << 6);
    if (top5 & (1 << 16))
	cc |= (1 << 7);

/* Following not verified - haven't seen a code with the high bit on here */
    if (top5 & (1 << 16)) {
	if (top5 & (1 << 12))
	    tt |= (1 << 11);
	if (top5 & (1 << 13))
	    tt |= (1 << 12);
	if (top5 & (1 << 14))
	    tt |= (1 << 13);
	if (top5 & (1 << 15))
	    cc |= (1 << 6);
    }
    twiddle_tt (tt, &t, &d, &x1, &x2);

/* Return as minutes after midnight, minutes of duration. */
    outtime = (30 * t) + x2;
    outdur = ((d + 1) * 30) - x1;

    *cval = cc + 1;
    *dval = outdur;		/* As minutes */
    *tval = outtime;		/* As minutes after midnight */
}

static void
interleave (tval, cval, top5out, bot3out)
int             tval, cval;
int            *top5out;
int            *bot3out;
{
    int             top5 = 0;
    int             bot3 = 0;
    if (tval & (1 << 0))
	(bot3 |= (1 << 0));
    if (cval & (1 << 0))
	(bot3 |= (1 << 1));
    if (tval & (1 << 1))
	(bot3 |= (1 << 2));
    if (cval & (1 << 1))
	(bot3 |= (1 << 3));
    if (tval & (1 << 2))
	(bot3 |= (1 << 4));

    if (tval & (1 << 3))
	(top5 |= (1 << 0));
    if (cval & (1 << 2))
	(top5 |= (1 << 1));
    if (cval & (1 << 3))
	(top5 |= (1 << 2));
    if (tval & (1 << 4))
	(top5 |= (1 << 3));
    if (tval & (1 << 5))
	(top5 |= (1 << 4));
    if (tval & (1 << 6))
	(top5 |= (1 << 5));
    if (cval & (1 << 4))
	(top5 |= (1 << 6));
    if (tval & (1 << 7))
	(top5 |= (1 << 7));
    if (cval & (1 << 5))
	(top5 |= (1 << 8));
    if (tval & (1 << 8))
	(top5 |= (1 << 9));
    if (tval & (1 << 9))
	(top5 |= (1 << 10));
    if (tval & (1 << 10))
	(top5 |= (1 << 11));
    if (cval & (1 << 6))
	(top5 |= (1 << 12));
    if (tval & (1 << 11))
	(top5 |= (1 << 13));
    if (tval & (1 << 12))
	(top5 |= (1 << 14));
    if (tval & (1 << 13))
	(top5 |= (1 << 15));
    if (cval & (1 << 7))
	(top5 |= (1 << 16));
/* Untested below. Channels over 127. */
    if (top5 & (1 << 16)) {
	if (tval & (1 << 11))
	    (top5 |= (1 << 12));
	if (tval & (1 << 12))
	    (top5 |= (1 << 13));
	if (tval & (1 << 13))
	    (top5 |= (1 << 14));
	if (cval & (1 << 6))
	    (top5 |= (1 << 15));
    }
/* Untested above. Channels over 127. */
    *top5out = top5;
    *bot3out = bot3;
}

int
revluk (int starttimem, int durationm)
{				/* Both in minutes. */
    int             t, d, x1, x2;
    int             tidx, startm, dur;
    int             startd = starttimem;
    for (tidx = 0; tidx < 16384; tidx++) {
	twiddle_tt (tidx, &t, &d, &x1, &x2);
	dur = 30 + (d * 30) - x1;
	startm = (t * 30) + x2;
	if ((dur == durationm) && (startm == startd)) {
	    return tidx;
	}
    }
    return -1;
}


void
map_top (year, month_out, day_out, top5, rem, mtoutp, remoutp)
int             year;
int             month_out;
int             day_out;
int             top5;
int             rem;
int            *mtoutp;
int            *remoutp;
{
    int             year_mod16, year_mod100;
    int             ndigits, nd, flag7, j, k, t1, t2, t3, ym, datum, mtout;
    int             month_today = month_out;
    int             year_today = year;
    unsigned char   n[NDIGITS];
    year_mod100 = year % 100;
    year_mod16 = year_mod100 % 16;

    split_digits (top5, n);
    ndigits = count_digits (top5) + 3;
    nd = ndigits - 3 - 1;

    rem = (rem + n[0] + n[1] + n[2] + n[3] + n[4]) % 32;

    if (ndigits <= 6) {

	do {
	    k = 0;
	    do {
		n[nd] = (n[nd] + day_out) % 10;
		if (nd > 0) {
		    for (j = nd - 1; j >= 0; j--) {
			n[j] = (n[j] + n[j + 1]) % 10;
		    }
		}
		rem += n[0];
	    } while (++k <= year_mod16);
	} while (n[nd] == 0);

	rem = (rem + (day_out * (month_today + 1))) % 32;

    } else {			/* ndigits > 6 */

	flag7 = (ndigits == 7) ? 1 : 0;
	ym = (year_today * 12) + month_today;

	do {
	    k = 1;
	    do {
		t1 = (ym + 310 - k) % 31;
		t1 = OverSixTable[t1];
		t2 = (t1 & 0x0f) - flag7;
		t3 = ((t1 >> 4) & 0x0f) - flag7;
		if ((t2 == 0) && (t3 < 3)) {
		    t2 = 4;
		} else if ((t2 == 0) && (t3 >= 3)) {
		    t2 = 2;
		} else if (t3 == 0) {
		    t3 = (t2 >= 3) ? 2 : 4;
		}
		t1 = n[--t2] + (10 * n[--t3]);

		do {
		    datum = ttbl[t1] - ym;
		    while ((datum < 0) || (datum >= 192)) {
			datum += 192;
		    }
		    if (datum > 99)
			t1 = datum;
		} while (datum > 99);
		if ((t2 >= 0) && (t2 < 5) && (t3 >= 0) && (t3 < 5)) {
		    n[t2] = datum % 10;
		    n[t3] = datum / 10;
		} else {
		    errdie ("ERROR: internal table index wild!\n");
		}
	    } while (++k <= 31);
	} while (n[nd] == 0);
    }				/* End code for lengths 7 and 8 */

    mtout = 10000 * n[4] + 1000 * n[3] + 100 * n[2] + 10 * n[1] + n[0];
    *mtoutp = mtout;
    *remoutp = rem;
}
/*******************************************************************\
 * The decode routine
\*******************************************************************/

void
decode_main (int month_today, int day_today, int year_today, int newspaper,
	     int *day_ret, int *channel_ret,
	     int *starttime_ret, int *duration_ret)
{
    int             s1_out, bot3, top5, quo, rem;
    int             mtout, tval, dval, cval;
    int             day_out, channel_out;
    int             starttime_out, duration_out;
    int             modnews;
    year_today = year_today % 100;

    if (month_today < 1 || month_today > 12) {
	printf ("Invalid month\n");
	usagex ();
    }
    if (day_today < 1 || day_today > 31) {
	printf ("Invalid day of the month\n");
	usagex ();
    }
    if (newspaper < 1) {
	printf ("DON'T TRY NUMBERS LESS THAN 1!\n");
	usagex ();
    }
    if (g_iflag) {
	s1_out = newspaper;
    } else {
	s1_out = func1 (newspaper);
    }

    top5 = s1_out / 1000;
    bot3 = (s1_out % 1000);
    quo = (bot3 - 1) / 32;
    rem = (bot3 - 1) % 32;
    day_out = quo + 1;

    map_top (year_today, month_today, day_out, top5, rem, &mtout, &rem);

    modnews = mtout * 1000;
    modnews += (day_out << 5) + rem - 31;

    bit_shuffle (modnews, &tval, &dval, &cval);

    starttime_out = tval;
    duration_out = dval;
    channel_out = cval;

    *day_ret = day_out;
    *channel_ret = channel_out;
    *starttime_ret = starttime_out;
    *duration_ret = duration_out;
    return;
}
/*******************************************************************\
 * The encode routine
\*******************************************************************/

/*
 * Returns VCRplus code (range 0..99999999), or -1 if error.
 */

int
encode_main (int month,
	     int day,
	     int year,
	     int channel,
	     int starttimem,
	     int durationm)
{
    int             j = 0;
    int             s1_out = 0;
    int             bot3 = 0;
    int             top5 = 0;
    int             quo = 0;
    int             rem = 0;
    int             newspaper = 0;
    int             cval = 0;
    int             tval = 0;
    int             big_top5 = 0;
    int             big_rem = 0;
    int             mtout = 0;
    year = year % 100;

    cval = channel - 1;
    tval = revluk (starttimem, durationm);
    if (tval < 0) {
	return (-1);
    }
    interleave (tval, cval, &big_top5, &big_rem);

    top5 = 0;
    j = 0;
    /*
     * Brute-force search to invert top5. Need closed-form solution. This was
     * OK for 6-digit codes, but is slow for 8-digits.
     */

    for (top5 = 0; top5 < 100000; top5++) {
	rem = 0;
	map_top (year, month, day,
		 top5, rem, &mtout, &rem);
	if (mtout == big_top5) {
	    break;
	}
    }

    quo = day - 1;
    rem = (big_rem + 320 - rem) % 32;
    bot3 = rem + 1 + (32 * quo);
    s1_out = bot3 + (1000 * top5);

    newspaper = encfunc1 (s1_out);
    return (newspaper);
}
/*******************************************************************\
 * Debug and regression test routine - mostly removed for anonymity
\*******************************************************************/

void
main_debug ()
{
    int             i, j, regresserrs;
    int             day_out;
    int             channel_out, starttime_out, duration_out;
    struct test    *tp;
    if (g_debug & 16) {
	printf (" 192-entry lookup table:\n");
	for (i = 0; i < 192; i++) {
	    j = ttbl[i];
	    printf (" %2.2X", j);
	    if ((i & 0x0f) == 0x0f) {
		printf ("\n");
	    }
	}
	printf ("\n");
	for (i = 0; i < 192; i++) {
	    j = ttbl[i];
	    printf (" %3.3o", j);
	    if ((i & 0x0f) == 0x0f) {
		printf ("\n");
	    }
	}
	printf ("\n");
/*	dumpnames(); */
    }
    if (g_debug & 32) {		/* Regression tests */
	tp = tests;
	regresserrs = 0;

	while (tp->code > 0) {
	    g_newspaper = tp->code;
	    g_day_today = 1;
	    g_month_today = tp->month;
	    g_year_today = tp->year;

/* First, test that set of data for decoding */

	    decode_main (g_month_today,
			 g_day_today,
			 g_year_today,
			 g_newspaper,
			 &day_out,
			 &channel_out,
			 &starttime_out,
			 &duration_out);

	    starttime_out = mmmm2hhmm (starttime_out);

	    if ((day_out == tp->day) &&
		(channel_out == tp->chan) &&
		(starttime_out == tp->start) &&
		(duration_out == tp->duration)) {
		if (g_verbose) {
		    printf ("Code: %d %d-%d should be %d, %d, %d, %d\n",
			    g_newspaper, tp->year, tp->month,
			    tp->day, tp->chan, tp->start, tp->duration);
		    printf ("Result: Day %d, Chan %d, Start %d, Length %d\n",
			 day_out, channel_out, starttime_out, duration_out);
		    printf ("Ok.\n\n");
		}
	    } else {
		printf ("Code: %d %d-%d should be %d, %d, %d, %d\n",
			g_newspaper, tp->year, tp->month,
			tp->day, tp->chan, tp->start, tp->duration);
		printf ("Result: Day %d, Chan %d, Start %d, Length %d\n",
			day_out, channel_out, starttime_out, duration_out);
		printf ("***** ERROR *****\n\n");
		regresserrs++;
	    }

/* Now, reverse it and see if the encode gives the newspaper */

	    g_newspaper =
		encode_main (tp->month,
			     tp->day,
			     tp->year,
			     tp->chan,
			     hhmm2mmmm (tp->start),
			     tp->duration
		);
	    if (g_verbose) {
		printf ("Encode %d-%d-%d Chan %d Start %d, Durn %d"
			" should be %d\n",
			tp->year, tp->month,
			tp->day, tp->chan,
			tp->start, tp->duration, tp->code);
		printf ("Result: Day %d, Chan %d, Start %d, Length %d\n",
			day_out, channel_out, starttime_out, duration_out);
	    }
	    if (tp->code == g_newspaper) {
		if (g_verbose) {
		    printf ("Ok.\n\n");
		}
	    } else {
		printf ("Encode %d-%d-%d Chan %d Start %d, Durn %d"
			" should be %d\n",
			tp->year, tp->month,
			tp->day, tp->chan,
			tp->start, tp->duration, tp->code);
		printf ("Result: Newspaper: %d\n",
			g_newspaper);
		printf ("***** ERROR *****\n\n");
		regresserrs++;
	    }
	    tp++;
	}
	if (regresserrs) {
	    printf ("*** ERRORS in regression tests ***\n");
	} else {
	    printf ("Regression tests passed.\n");
	}
    }
}
/*******************************************************************\
 * The MAIN main() routine
\*******************************************************************/

int
main (argc, argv)
int             argc;
char           *argv[];
{
    char            ch;
    int             rem;
    int             day_out, channel_out;
    int             starttime_out, duration_out;

    int             s_year_today;
    int             s_month_today;
    int             s_day_today;
    int             s_channel;
    int             s_starttime;
    int             s_duration;

    struct tm      *tm;
    time_t          t;
    /* defaults */
    g_progname = argv[0];
    g_iflag = 0;
    g_debug = 0;
    g_encode = 0;
    g_starttime = s_starttime = 1930;
    g_duration = s_duration = 30;
    g_starttimem = hhmm2mmmm (g_starttime);
    g_durationm = hhmm2mmmm (g_duration);
    g_channel = s_channel = 2;

    t = time (NULL);
    tm = localtime (&t);
    s_month_today = g_month_today = tm->tm_mon + 1;
    s_day_today = g_day_today = tm->tm_mday;
    s_year_today = g_year_today = tm->tm_year + 1900;

    while ((ch = getopt (argc, argv, "veg:y:m:d:c:t:l:i")) != -1) {
	switch (ch) {
	case 'g':		/* as in -g to compiler for debuGging */
	    g_debug = strtol (optarg, (char **)NULL, 0);
	    break;
	case 'i':
	    g_iflag++;
	    break;
	case 'e':
	    g_encode++;
	    break;
	case 'v':
	    g_verbose++;
	    break;
	case 'y':
	    s_year_today = strtol (optarg, (char **)NULL, 0);
	    if ((s_year_today < 0) || (s_year_today > 2999))
		errdie ("Year out of range");
	    break;
	case 'm':
	    s_month_today = strtol (optarg, (char **)NULL, 0);
	    if ((s_month_today < 1) || (s_month_today > 12))
		errdie ("Month out of range 1-12");
	    break;
	case 'd':
	    s_day_today = strtol (optarg, (char **)NULL, 0);
	    if ((s_day_today < 1) || (s_day_today > 31))
		errdie ("Day out of range 1-31");
	    break;
	case 'c':
	    s_channel = strtol (optarg, (char **)NULL, 0);
	    if ((s_channel < 1) || (s_channel > 128))
		errdie ("Channel out of range 1-128");
	    break;
	case 't':
	    s_starttime = atoi (optarg);	/* Decimal only for times */
	    rem = s_starttime % 100;
	    if ((rem % 5) ||
		(rem < 0) ||
		(rem > 55) ||
		(s_starttime < 0) ||
		(s_starttime > 2355)
		)
		errdie ("Start time not an even five minutes from 0000 to 2355");
	    break;
	case 'l':
	    s_duration = atoi (optarg);	/* Decimal only for times */
	    rem = s_duration % 100;
	    if ((rem % 5) ||
		(s_duration < 5) ||
		(s_duration > 300)
		)
		errdie ("Duration not a multiple of 5 from 5 to 300");
	    break;
	case 'h':
	    usage ();
	    exit (0);
	    break;
	}
    }
    argc -= optind;
    argv += optind;

    if (g_debug) {
	main_debug ();		/* do debugging stuff first */
    }
    g_year_today = s_year_today;
    g_month_today = s_month_today;
    g_day_today = s_day_today;
    g_channel = s_channel;
    g_starttime = s_starttime;
    g_duration = s_duration;
    g_starttimem = hhmm2mmmm (g_starttime);
    g_durationm = hhmm2mmmm (g_duration);


    if (0 == g_encode) {
	if (argc != 1) {
	    if (g_debug) {
		exit (0);
	    }
	    printf ("ERROR: decoding requires a code as single argument.\n");
	    usagex ();
	}
	g_newspaper = strtol (argv[0], (char **)NULL, 0);
	if (g_verbose) {
	    printf ("Today's date: %s %d %d\n", monthname[g_month_today - 1],
		    g_day_today, g_year_today);
	}
	decode_main (g_month_today, g_day_today, g_year_today, g_newspaper,
		     &day_out, &channel_out,
		     &starttime_out, &duration_out);

	if (g_year_today < 50)	/* window: <50=year 2000's, >50=year 1900's */
	    g_year_today += 100;
	if (g_year_today < 1900)/* convert to 4-digit year */
	    g_year_today += 1900;
	g_channel_name = channame (channel_out);
	printf ("Program: %s %d %d, channel %d (%s), at %s, duration %d:%2.2d.\n",
	   monthname[g_month_today - 1], day_out, g_year_today, channel_out,
		g_channel_name, timestr (mmmm2hhmm (starttime_out)),
		duration_out / 60, duration_out % 60);

    } else {
/* Encoding */
	g_newspaper = encode_main (g_month_today, g_day_today, g_year_today,
				   g_channel, g_starttimem, g_durationm);

	if (g_verbose) {	/* Announce what we're decoding */
	    printf ("%2d %2d %2d %3d %4d %3d  VCR++ code=%8d\n",
		    g_month_today, g_day_today, g_year_today, g_channel,
		    g_starttime, g_duration, g_newspaper);
	} else {
	    printf ("%d\n", g_newspaper);
	}
	exit (0);
    }

    exit (0);
    return 0;
}
