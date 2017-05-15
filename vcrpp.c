/*******************************************************************\
 * VCRplus+ Encoding, Decoding and regression test code.
 * Works with standard USA codes, 1 through 8 digits long,
 * without leading zeroes.
 *
 * This program is released to the public domain.
\*******************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define		KEY001 (68150631)   /* initial scramble (key not used) */
#define		KEY002	(9371)      /* inverse of above, for encoding */
#define		NDIGITS 10          /* Size of all digit arrays */

int g_iflag;
int g_debug;
int g_verbose;      /* More typeout, but not debug stuff */
int g_encode;	    /* 0 = decode, 1 = encode */

int	g_newspaper;    	/* arg to decode, result for encode */
char    *g_progname;
char    *g_channel_name;
int	g_year_today;
int	g_month_today;
int	g_day_today;
int	g_channel;
int	g_starttime;
int	g_starttimem;
int	g_duration;
int	g_durationm;

unsigned char *inv_ttbl = NULL;

/* Forward ref definitions */
void    main_debug(void);
void    clear_ndigits(unsigned char *);
void    split_digits(int n, unsigned char *a);
int     hhmm2mmmm(int);
int     mmmm2hhmm(int);
int     count_digits(int);
int     set_pwr(int);

/* Tables */


/* regression test codes, to make sure things didn't get broken */

typedef struct test {
    int code;
    int year;
    int month; 
    int day;
    int chan;
    int start;
    int duration;
} _test;

static _test tests[] = {
    {    3316, 1991,  5,     10,  4, 2100, 120},
    {   21362, 1992,  3,     11, 24, 1930,  30},
    {       0,    0,  0,      0 , 0,   0,    0}
};

/* List of month names, so we can speak English */
static char monthname[][12] = {
	{ "January" },
	{ "February" },
	{ "March" },
	{ "April" },
	{ "May" },
	{ "June" },
	{ "July" },
	{ "August" },
	{ "September" },
	{ "October" },
	{ "November" },
	{ "December" }
};

/* Channel assignments */
typedef struct {
        int num;
	char * name;
} numname;

/*These need to be edited for local stations */

static
numname chan_to_name[] = {
	{02,  "WXXA2"},
	{03,  "WXXB3"},
	{04,  "WXXC4"},
	{05,  "WXXD5"},
	{06,  "WXXE6"},
	{07,  "WXXF7"},
	{33,  "HBO"},
	{34,  "ESPN"},
	{35,  "AMC"},
	{37,  "DSC"},
	{38,  "NIK"},
	{39,  "A&E"},
	{41,  "SHO"},
	{42,  "CNN"},
	{43,  "TBS"},
	{44,  "USA"},
	{45,  "MAX"},
	{46,  "LIF"},
	{47,  "FAM"},
	{48,  "MTV"},
	{49,  "TNN"},
	{51,  "TLC"},
	{52,  "TNT"},
	{53,  "DIS"},
	{54,  "BRV"},
	{57,  "BET"},
	{58,  "TMC"},
	{59,  "FSN"},
	{62,  "VH1"},
	{63,  "E"},
	{69,  "H&G"},
	{70,  "APL"},
	{73,  "HIS"},
	{75,  "COM"},
	{76,  "TCM"},
	{79,  "CNBC"},
	{80,  "STARZ"},
	{85,  "FX"},
	{86,  "CRT"},
	{88,  "ES2"},
	{89,  "SCI"},
	{90,  "MSNBC"},
	{91,  "FNC"},
	{99,  "QVC"},
	{103, "HAL"},
	{-1,  "???"},
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

unsigned char LengthEightTable[31] = {
    0x14, 0x02, 0x23, 0x03, 0x41, 0x14, 0x14, 0x32,
    0x34, 0x03, 0x32, 0x40, 0x10, 0x01, 0x12, 0x32,
    0x40, 0x21, 0x24, 0x41, 0x23, 0x14, 0x23, 0x41,
    0x02, 0x04, 0x30, 0x24, 0x41, 0x40, 0x01
};

unsigned char LengthSevenTable[31] = {
    0x03, 0x31, 0x12, 0x12, 0x30, 0x03, 0x03, 0x21,
    0x23, 0x12, 0x21, 0x31, 0x03, 0x30, 0x01, 0x21,
    0x31, 0x10, 0x13, 0x30, 0x12, 0x03, 0x12, 0x30,
    0x31, 0x13, 0x21, 0x13, 0x30, 0x31, 0x30
};



void dumpnames(void) {
    numname *nnp;

    for (nnp = chan_to_name; ; nnp++) {
	if (nnp->num < 0) break;
	printf("%d\t%s\n", nnp->num, nnp->name);
    }
}

char *
channame(int chan)
{
    numname *nnp;
    for (nnp = chan_to_name; ; nnp++) {
	if (nnp->num < 0) break;
	if (nnp->num == chan) break;
    }
    return(nnp->name);
}

static void
usage (void) {
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
usagex (void) {
    usage();
    exit (-1);
}

static void errdie(char *s)
{
    printf("ERROR: %s\n", s);
    usagex();
}

/* Convert minutes of the day into friendly time with AM/PM */
/* The string that is returned is overwritten each time we're called */
/* Example return strings: "9 AM", "5:30 PM", "12:15 PM", "12 noon", "12 midnight" */
/* <pab@rainbow.cse.nau.edu> */

static char *
timestr (int t) {
    int hour,min;
    static char str[256];
    char ampm[10], hm[10];

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
count_digits(int val)
{
    int ndigits;

    if (val < 0) {
	printf("Error: code 0 or negative\n");
	ndigits = 0;
    }
    else if (val < 1) ndigits = 0;
    else if (val < 10) ndigits = 1;
    else if (val < 100) ndigits = 2;
    else if (val < 1000) ndigits = 3;
    else if (val < 10000) ndigits = 4;
    else if (val < 100000) ndigits = 5;
    else if (val < 1000000) ndigits = 6;
    else if (val < 10000000) ndigits = 7;
    else if (val < 100000000) ndigits = 8;
    else ndigits = 9;

    if (ndigits > 8) {
	printf ("ERROR: %d has more than 8 digits (it has %d digits)\n",
	        val, ndigits);
	usagex();
    }
    return ndigits;
}

int
set_pwr(int ndigits)
{
    int pwr = 0x7fffffff;

    if (0 == ndigits)  pwr = 1;
    if (1 == ndigits)  pwr = 10;
    if (2 == ndigits)  pwr = 100;
    if (3 == ndigits)  pwr = 1000;
    if (4 == ndigits)  pwr = 10000;
    if (5 == ndigits)  pwr = 100000;
    if (6 == ndigits)  pwr = 1000000;
    if (7 == ndigits)  pwr = 10000000;
    if (8 == ndigits)  pwr = 100000000;
    if (9 == ndigits)  pwr = 1000000000;
    return pwr;
}

void
clear_ndigits(unsigned char *a)
{
    int i;
    for (i = 0; i < NDIGITS; i++) {
	a[i] = 0;
    }
}

void
split_digits(int n, unsigned char *a)
{
    int i;
    unsigned char digit;

    clear_ndigits(a);

    for (i = 0; i < NDIGITS; i++) {
	digit = n % 10;
	a[i] = digit;
	n = (n - digit) / 10;
    }
}

int hhmm2mmmm(int hhmm)
{
    int hh, mm;

    hh = hhmm / 100;
    mm = hhmm % 100;
    if (mm >59) {
	printf("ERROR: Minutes too large in %d:%2d - truncated to %d:00\n",
	       hh, mm, hh);
	mm = 0;
    }
    return (hh * 60) + mm;
}

int mmmm2hhmm(int mmmm)
{
    int hh, mm;

    hh = mmmm / 60;
    mm = mmmm % 60;
    return (hh * 100) + mm;
}


int
func1(int code)
{
    int     x = code;
    int     nd;
    unsigned char a[NDIGITS];
    int     sum;
    int     i, j;
    int	    ndigits;

    split_digits(x, a);
    ndigits = count_digits(x);
    nd = ndigits - 1;       /* Max idx of digit array */

    do {
	i = 0;
	do {
	    j = 1;
	    if (nd >= 1) {
		do {
		    a[j] = (a[j-1] + a[j]) % 10;
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
int	x, y;
{
    int	    i, j, digit, sum;
    int	    a[9], b[9], out[17];

    for (i=0; i<9; i++) {
	digit = x % 10;
	a[i] = digit;
	x = (x - digit) / 10;
    }

    for (i=0; i<9; i++) {
	digit = y % 10;
	b[i] = digit;
	y = (y - digit) / 10;
    }

    for (i=0; i<17; i++) {
	out[i] = 0;
    }

    for (i=0; i<=8; i++) {
	for (j=0; j<=8; j++) {
	    out[i+j] += b[j] * a[i];
	}
    }

    j = 1;
    sum = 0;
    for (i=0; i<=8; i++) {
	sum += j * (out[i] % 10);
	j *= 10;
    }
    return (sum);
}


static int
encfunc1 (val)
int val;
{
    int	    ndigits, pwr;

    ndigits = 0;
    pwr = 1;
    while (val >= pwr) {
	ndigits++;
	pwr *= 10;
    }
    if (ndigits > 8) {
	printf ("ERROR: %d has more than 8 digits (it has %d digits)\n", val, ndigits);
	usagex();
    }
    pwr /= 10;

    if (0 == g_iflag) {
	do {
	    val = encode_final_transform(val, KEY002) % (pwr * 10);
	    if (g_debug & 1) 
		printf("Initial transform returned %d\n", val);
	} while (val < pwr);
    }
    return (val);
}

/* Input tidx is in range 0-16127 */

void
twiddle_tt(int tidx, int *tp, int *dp, int *x1p, int *x2p)
{
    int tt, x1, x2, t, d;
    int t1, t2, b;

    x1 = x2 = t1 = t2 = d = b = t = 0;
    tt = tidx;

    if ((tt >= 768) && (tt < (768 +2880))) {
/* Case of one, but not both, deltas nonzero and base durn thru 3 hrs */

	t1 = ((tt - 768) % 10) + 1;     /* t1 = (0-9) + 1 */
	t2 = t1 * 5;                    /* t2 5-50 */
	if (t1 >= 6) {                  /* More than a 25 min delta durn */
	    x2 = t2 - 25;               /* Start time +delta 5-25 */
	} else {
	    x1 = t2;                    /* Duration -delta 5-25 */
	}
	tt -= 768;	                /* Base durn back to 0-3 half hrs */
	tt = tt / 10;                   /* Durn 0-287, thru 3 hrs */
    } else
    if ((tt >= 3648) && (tt <= 6527)) {
/* Case of quarter-hour durn OR half-hour durn w/ any start,
   and base durn 3-8 hrs */
	t1 = ((tt - 3648) % 6) + 1;     /* 1-6 */
	if (t1 == 1) {
	    x1 = 15;                    /* Delta durn = 1 mins */
	} else {
	    x2 = (5 * t1) - 5;          /* Or delta start = 5-25 */
	}
	tt = ((tt - 3648) / 6) + 288;   /* 288-767 (3-8 hrs base durn */
    } else
    if ((tt > 6527) && (tt <= 13727)) { /* 7200 = 288 * 25 */
/* Case of base durn 0-3 hrs, nonzero deltas of both start and durn */
	x1 = 5 + ((((tt-6528) % 25) % 5) * 5);
	x2 = 5 + ((((tt-6528) % 25) / 5) * 5);
	tt -= 6528;
	tt = tt / 25;
    } else
    if (tt > 13727) {
/* Arbitrary start, long (> 3 hrs) durn, durn rounded to 15 mins */
	x2 = 5 + (((tt - 13728) % 5) * 5);
	x1 = 15;
	tt -= 13728;
	tt = (tt / 5) + 288;
    }

    if (tt < 192) {                 /* Simple 0 - 2 hrs base durn */
	/* Lookup in table */
	b = ttbl[tt];
	t = b % 48;  		    /* Half hours after midnight  */
	d = b / 48;         	    /* Half hours of duration - 1 */

    } else {                        /* More than 2 hrs base durn */
	t = 47 - ((tt - 192) % 48); /* Half hours after midnight  */
	d = tt / 48;                /* Half hours of duration - 1 */
    }

    *tp = t;
    *dp = d;
    *x1p = x1;                      /* Shorter duration  0 - 25 */
    *x2p = x2;                      /* Later start time  0 - 25 */
}


int
inv_twiddle_tt(int startm, int durnm)
{
    int tt = 0;
    int x1, x2, t, d, b, t1, t2, i;
    int bstart, bdurn;              /* Basic (half-hour) start and duration */
    int basett;                     /* Table index for base start/durn */

    x1 = x2 = t1 = t2 = d = b = t = 0;

    if ((durnm < 5) ||
        (durnm > (60*8)) ||
    	(startm < 0) ||
	(startm > (23*60)+59) ) {
	printf("args to inv_twiddle_tt(%d, %d) out of range\n", startm, durnm);
	usagex();
    }

    if (durnm  % 5) { durnm  = durnm + 5  - (durnm  % 5); }
    if (durnm  > (8 * 60)) { durnm  = 8 * 60; }
    if (startm % 5) { startm = startm - (startm % 5); }

    bstart = startm - (startm % 30);    /* Start is a base + 0-25 */
    bdurn  = durnm + 29;                /* Durn increments are negative, so ... */
    bdurn  = bdurn - (bdurn % 30);      /* Basic duration, mult of half hour */
    t = bstart / 30;                    /* 0-47 half hours after midnight, start */
    d = bdurn / 30;                     /* 1-18 half hours duration */

    x1 = bdurn - durnm;                 /* 0-25 (by 5s) minutes shorter durn */
    x2 = startm - bstart;               /* 0-25 (by 5s) later start */

    if (d <= 4) {
	for (i = 0; i < 192; i++) {
	    if (ttbl[i] == (t + ((d - 1) * 48))) {
		    basett = i;
		    break;
	    }
	}
    } else {
	basett = (48 * (d - 1)) + 47 - t;
    }

    if ((x1 == 0) && (x2 == 0)) {
	return basett;
    } else

    if ((d <= 6) && ((x1 == 0) || (x2 == 0))) {
	tt = 768 + (10 * basett);
	if (x1) {
	    tt += (x1 / 5) - 1;
	} else
	if (x2) {
	    tt += (x2 / 5) + 4;
	}
	return tt;
    } else

    if ((d > 6) && ( ((x1 == 15) && (x2 == 0)) ||
                     ((x1 ==  0) && (x2 != 0)) ) ) {

	tt = 3648 + (6 * (basett - 288));
	tt += (x2 / 5);
	return tt;
    } else

    if ((d <= 6) && (x1) && (x2)) {
	tt = 6528 + (25 * basett);
	tt += (x1 / 5) - 1;
	tt += ((x2 / 5) - 1) * 5;
	return tt;
    } else

    if ((d > 6) && (x1) && (x2)) {
	tt = 13728 + (5 * (basett - 288));
	tt += (x2 / 5) - 1;
	return tt;
    }
    errdie("inv_twiddle fell through all cases!\n");
    return -1;                      /* t index for this start/durn */
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

    if (rem  & (1 <<  0)) tt |= (1 <<  0);
    if (rem  & (1 <<  2)) tt |= (1 <<  1);
    if (rem  & (1 <<  4)) tt |= (1 <<  2);
    if (top5 & (1 <<  0)) tt |= (1 <<  3);
    if (top5 & (1 <<  3)) tt |= (1 <<  4);
    if (top5 & (1 <<  4)) tt |= (1 <<  5);
    if (top5 & (1 <<  5)) tt |= (1 <<  6);
    if (top5 & (1 <<  7)) tt |= (1 <<  7);
    if (top5 & (1 <<  9)) tt |= (1 <<  8);
    if (top5 & (1 << 10)) tt |= (1 <<  9);
    if (top5 & (1 << 11)) tt |= (1 << 10);
    if (top5 & (1 << 13)) tt |= (1 << 11);
    if (top5 & (1 << 14)) tt |= (1 << 12);
    if (top5 & (1 << 15)) tt |= (1 << 13);

    if (rem  & (1 <<  1)) cc |= (1 <<  0);
    if (rem  & (1 <<  3)) cc |= (1 <<  1);
    if (top5 & (1 <<  1)) cc |= (1 <<  2);
    if (top5 & (1 <<  2)) cc |= (1 <<  3);
    if (top5 & (1 <<  6)) cc |= (1 <<  4);
    if (top5 & (1 <<  8)) cc |= (1 <<  5);
    if (top5 & (1 << 12)) cc |= (1 <<  6);
    if (top5 & (1 << 16)) cc |= (1 <<  7);

    if (top5 & (1 << 16)) {
    	if (top5 & (1 << 12)) tt |= (1 << 11);
    	if (top5 & (1 << 13)) tt |= (1 << 12);
    	if (top5 & (1 << 14)) tt |= (1 << 13);
    	if (top5 & (1 << 15)) cc |= (1 <<  6);
    }

    if (g_debug & 3) {
        printf("Debug: top5 = 0x%X = %d., b16 = %d for code %d\n",
                top5, top5, top5 & (1 << 16) ? 1 : 0, code);
        printf("Debug: rem  = 0x%X = %d. for code %d\n", rem, rem, code);
        printf("Debug: tt   = 0x%X = %d. for code %d\n", tt, tt, code);
        printf("Debug: cc   = 0x%X = %d. for code %d\n", cc, cc, code);
    }

    twiddle_tt(tt, &t, &d, &x1, &x2);

    outtime = (30 * t) + x2;
    outdur  = ((d + 1) * 30) - x1;

    *cval = cc + 1;
    *dval = outdur;     /* As minutes */
    *tval = outtime;    /* As minutes after midnight */
}

static void
interleave (tval, cval, top5out, bot3out)
int		tval, cval;
int	       *top5out;
int	       *bot3out;
{
    int top5 = 0;
    int bot3  = 0;

    if (tval & (1 <<  0)) (bot3 |= (1 <<  0));
    if (cval & (1 <<  0)) (bot3 |= (1 <<  1));
    if (tval & (1 <<  1)) (bot3 |= (1 <<  2));
    if (cval & (1 <<  1)) (bot3 |= (1 <<  3));
    if (tval & (1 <<  2)) (bot3 |= (1 <<  4));

    if (tval & (1 <<  3)) (top5 |= (1 <<  0));
    if (cval & (1 <<  2)) (top5 |= (1 <<  1));
    if (cval & (1 <<  3)) (top5 |= (1 <<  2));
    if (tval & (1 <<  4)) (top5 |= (1 <<  3));
    if (tval & (1 <<  5)) (top5 |= (1 <<  4));
    if (tval & (1 <<  6)) (top5 |= (1 <<  5));
    if (cval & (1 <<  4)) (top5 |= (1 <<  6));
    if (tval & (1 <<  7)) (top5 |= (1 <<  7));
    if (cval & (1 <<  5)) (top5 |= (1 <<  8));
    if (tval & (1 <<  8)) (top5 |= (1 <<  9));
    if (tval & (1 <<  9)) (top5 |= (1 << 10));
    if (tval & (1 << 10)) (top5 |= (1 << 11));
    if (cval & (1 <<  6)) (top5 |= (1 << 12));
    if (tval & (1 << 11)) (top5 |= (1 << 13));
    if (tval & (1 << 12)) (top5 |= (1 << 14));
    if (tval & (1 << 13)) (top5 |= (1 << 15));
    if (cval & (1 <<  7)) (top5 |= (1 << 16));
/* Untested below. Channels over 127. */
    if (top5 & (1 << 16)) {
    	if (tval & (1 << 11)) (top5 |= (1 << 12));
    	if (tval & (1 << 12)) (top5 |= (1 << 13));
    	if (tval & (1 << 13)) (top5 |= (1 << 14));
    	if (cval & (1 <<  6)) (top5 |= (1 << 15));
    }
/* Untested above. Channels over 127. */
    *top5out = top5;
    *bot3out = bot3;
}


void
map_top(year, month, day, top5, rem, mtoutp, remoutp)
int year;
int month;
int day;
int top5;
int rem;
int *mtoutp;
int *remoutp;
{
    int year_mod16, year_mod100;
    int ndigits, nd, j, k, t1, t2, t3, ym, mtout;

    unsigned char n[NDIGITS];

    year_mod100 = year % 100;
    year_mod16 = year_mod100 % 16;

    split_digits(top5, n);
    ndigits = count_digits(top5) + 3;
    nd = ndigits - 3 - 1;   /* Max array idx of top5 */

    rem  = (rem + n[0] + n[1] + n[2] + n[3] + n[4]) % 32;

    if (ndigits <= 6) {
	if (nd >= 0) {
            do {
    	        k = 0;
                do {
    	            n[nd] = (n[nd] + day) % 10;
    	            if (nd > 0) {
    		        for (j = nd - 1; j >= 0; j--) {
    		            n[j] = (n[j] + n[j+1]) % 10;
    		        }
    	            }
    		    rem += n[0];
                } while (++k <= year_mod16);
            } while (n[nd] == 0);
	}
        rem = (rem + (day * (month + 1))) % 32;

    } else { /* ndigits > 6 */

        ym = (year_mod100 * 12) + month;   /* 1 through 1200 */

        do {
	    k = 1;
    	    do {
		t1 = (ym + 310 - k) % 31;
    		t1 = (ndigits == 7) ?
		     LengthSevenTable[t1] : LengthEightTable[t1];
                t2 = (t1 & 0x0f);
                t3 = ((t1 >> 4) & 0x0f);
    	        t1 = n[t2] + (10 * n[t3]);
		do {
		    t1 = (ttbl[t1] - ym + 1920) % 192;
		} while (t1 > 99);
    		n[t2] = t1 % 10;
    		n[t3] = t1 / 10;
    	    } while (++k <= 31);
        } while (n[nd] == 0);
    }   /* End code for lengths 7 and 8 */

    mtout =  10000*n[4] + 1000*n[3] + 100*n[2] + 10*n[1] + n[0];
    *mtoutp = mtout;
    *remoutp = rem;
}

int
inv_map_top(year, month, day, top5)
int year;
int month;
int day;
int top5;
{
    int year_mod16, year_mod100;
    int ndigits, nd, i, j, k, t1, t2, t3, ym, mtout;
    unsigned char n[NDIGITS];

    year_mod100 = year % 100;
    year_mod16 = year_mod100 % 16;

    split_digits(top5, n);
    ndigits = count_digits(top5) + 3;
    nd = ndigits - 3 - 1;

    switch (ndigits) {
      case 0:
      case 1:
      case 2:
      case 3:
	mtout = top5;
	break;

      case 4:
      case 5:
      case 6:
        do {
    	    k = 0;
            do {
		if (nd > 0) {
		    for (j = 0; j < nd; j++) {
			n[j] = (n[j] + 100 - n[j+1]) % 10;
		    }
		}
    	        n[nd] = (n[nd] + 100 - day) % 10;
            } while (++k <= year_mod16);
        } while (n[nd] == 0);

        mtout =  10000*n[4] + 1000*n[3] + 100*n[2] + 10*n[1] + n[0];
	break;

      case 7:
      case 8:
	ym = (year_mod100 * 12) + month;   /* 1 through 1200 */
	if (NULL == inv_ttbl) {
	    inv_ttbl = (unsigned char *)malloc(100);
	    if (NULL == inv_ttbl) {
		errdie("malloc failed in inv_map_top");
	    }
	}

	for (i = 0; i < 100; i++) {
	    j = i;
	    do {
		j = (ttbl[j] - ym + 1920) % 192;
	    } while (j > 99);
	    inv_ttbl[j] = i;
	}
        do {
	    k = 31;
    	    do {
		t1 = (ym + 310 - k) % 31;
    		t1 = (ndigits == 7) ?
		     LengthSevenTable[t1] : LengthEightTable[t1];
                t2 = (t1 & 0x0f);
                t3 = ((t1 >> 4) & 0x0f);
    	        t1 = n[t2] + (10 * n[t3]);
		t1 = inv_ttbl[t1];
    		n[t2] = t1 % 10;
    		n[t3] = t1 / 10;
    	    } while (--k >= 1);
        } while (n[nd] == 0);
        mtout =  10000*n[4] + 1000*n[3] + 100*n[2] + 10*n[1] + n[0];
        break;

      default:
	errdie("Bad length in inv_map_top");
	mtout = (-1);
    }
    return mtout;
}


void
decode_main (int month_today, int day_today, int year_today, int newspaper,
		int * day_ret, int *channel_ret,
		int *starttime_ret, int *duration_ret)
{
    int	    s1_out, bot3, top5, quo, rem;
    int	    mtout, tval, dval, cval;
    int	    day_out, channel_out;
    int	    starttime_out, duration_out;
    int     modnews;

    year_today = year_today % 100;

    if (month_today<1 || month_today>12) {
	    printf ("Invalid month\n");
	    usagex();
    }

    if (day_today<1 || day_today>31) {
	    printf ("Invalid day of the month\n");
	    usagex();
    }

    if (newspaper < 1) {
	printf("DON'T TRY NUMBERS LESS THAN 1!  Furrfu!\n");
	usagex();
    }

    s1_out = func1(newspaper);

    bot3 = (s1_out % 1000);
    quo = (bot3 - 1) / 32;
    rem = (bot3 - 1) % 32;
    day_out = quo + 1;

    top5 = s1_out / 1000;

    map_top(year_today, month_today, day_out, top5, rem, &mtout, &rem);

    modnews = mtout * 1000;
    modnews += (day_out << 5) + rem - 31;

    bit_shuffle(modnews, &tval, &dval, &cval);

    starttime_out = tval;
    duration_out = dval;
    channel_out = cval;

    *day_ret       = day_out;
    *channel_ret   = channel_out;
    *starttime_ret = starttime_out;
    *duration_ret  = duration_out;
    return;
}


int
encode_main (int month,
             int day,
	     int year,
	     int channel,
	     int starttimem,
	     int durationm)
{
    int	    j = 0;
    int	    s1_out = 0;
    int     bot3 = 0;
    int     top5 = 0;
    int     quo = 0;
    int     rem = 0;
    int     newspaper = 0;
    int     cval = 0;
    int     tval = 0;
    int     big_top5 = 0;
    int     big_rem = 0;
    int     mtout = 0;

    year = year % 100;

    cval = channel - 1;
    tval = inv_twiddle_tt(starttimem, durationm);   /* was revluk */
    if (tval < 0) {
	    return (-1);
    }

    interleave (tval, cval, &big_top5, &big_rem);

    top5 = 0;
    j = 0;

    top5 = inv_map_top(year, month, day, big_top5);
    if (top5 < 0) {
	errdie("inv_map_top returned error");
    }

    rem = 0;
    map_top(year, month, day, top5, rem, &mtout, &rem);

    quo = day - 1;
    rem = (big_rem + 320 - rem) % 32;
    bot3 = rem + 1 + (32 * quo);
    s1_out = bot3 + (1000 * top5);

    newspaper = encfunc1(s1_out);
    if (g_debug & 3) {
        printf ("Final transform: s1_out=%d => newspaper=%d\n",
	         s1_out, newspaper);
    }
    return (newspaper);
}


void
main_debug()
{
    int i, j, regresserrs;
    int day_out;
    int channel_out,starttime_out, duration_out;
    struct test *tp;
    int gt, gd, gx1, gx2, gstartm, gdurnm;

    if (g_debug & 16) {
	printf(" 192-entry lookup table:\n");
	for (i = 0; i < 192; i++) {
	    j = ttbl[i];
	    printf(" %2.2X", j);
	    if ((i & 0x0f) == 0x0f) {
		printf("\n");
	    }
	}
	printf("\n");
	for (i = 0; i < 192; i++) {
	    j = ttbl[i];
	    printf(" %3.3o", j);
	    if ((i & 0x0f) == 0x0f) {
		printf("\n");
	    }
	}
	printf("\n");
/*	dumpnames(); */
    }

    if (g_debug & 32) {     /* Regression tests */
	tp = tests;
	regresserrs = 0;

	while (tp->code > 0) {
	    g_newspaper = tp->code;
	    g_day_today = 1;
	    g_month_today = tp->month;
	    g_year_today = tp->year;


            decode_main (g_month_today,
	                 g_day_today,
			 g_year_today,
			 g_newspaper,
			 &day_out,
		         &channel_out,
			 &starttime_out,
			 &duration_out);

	    starttime_out = mmmm2hhmm(starttime_out);

	    if ((day_out == tp->day) &&
		(channel_out == tp->chan) &&
		(starttime_out == tp->start) &&
		(duration_out == tp->duration)) {
		if (g_verbose) {
         	    printf ("Decode: %d %d-%d should be %d, %d, %d, %d\n",
        	            g_newspaper, tp->year, tp->month,
        		    tp->day, tp->chan, tp->start, tp->duration);
        	    printf("Result: Day %d, Chan %d, Start %d, Length %d\n",
        	            day_out, channel_out, starttime_out, duration_out );
    		    printf("Ok.\n\n");
		}
	    } else {
     	        printf ("Decode: %d %d-%d should be %d, %d, %d, %d\n",
    	                g_newspaper, tp->year, tp->month,
    		        tp->day, tp->chan, tp->start, tp->duration);
    	        printf("Result: Day %d, Chan %d, Start %d, Length %d\n",
    	                day_out, channel_out, starttime_out, duration_out );
		printf("***** ERROR *****\n\n");
        	regresserrs++;
	    }

	    g_newspaper =
	    encode_main(tp->month,
                        tp->day,
			tp->year,
			tp->chan,
			hhmm2mmmm(tp->start),
			tp->duration
			);
	    if (g_verbose) {
         	printf ("Encode %d-%d-%d Chan %d Start %d, Durn %d"
		        " should be %d\n",
        	        tp->year, tp->month,
        		tp->day, tp->chan,
			tp->start, tp->duration, tp->code);
            	printf("Result: Day %d, Chan %d, Start %d, Length %d\n",
        	        day_out, channel_out, starttime_out, duration_out );
	    }
	    if (tp->code == g_newspaper) {
		if (g_verbose) {
        	    printf("Ok.\n\n");
		}

	    } else {
         	printf ("Encode %d-%d-%d Chan %d Start %d, Durn %d"
		        " should be %d\n",
        	        tp->year, tp->month,
        		tp->day, tp->chan,
			tp->start, tp->duration, tp->code);
            	printf("Result: Newspaper: %d\n",
        	        g_newspaper);
		printf("***** ERROR *****\n\n");
        	regresserrs++;
	    }
	    tp++;
	}
        if (regresserrs) {
	    printf("*** ERRORS in regression tests ***\n");
        } else {
	    printf("Regression tests passed.\n");
        }
    }


    if (g_debug & 128) {      /* Test inv_twiddle_tt */
	for (i = 0; i < 16127 ; i++) {
	    twiddle_tt(i, &gt, &gd, &gx1, &gx2);
	    gstartm = (30 * gt) + gx2;
	    gdurnm  = (30 * gd) - gx1 + 30;

	    if ( (gstartm >= (24 * 60)) ||
	         (gdurnm > (8 * 60))) {

		printf("twiddle_tt(%d) gives %d, %d, %d, %d\n", i, gt, gd, gx1, gx2);
    	        printf(" for startm = %d, durnm = %d\n", gstartm, gdurnm);
	     }
	    j = inv_twiddle_tt(gstartm, gdurnm);
	    if (i != j) {
		printf("inv_twiddle_tt %d not equal %d\n", j, i);
	    }
	}
    }

    if (g_debug & 512) {
	i = 5;
	map_top(3, 11, 18, i, 0, &j, &gd);
	gt = inv_map_top(3, 11, 18, j);
	i = 94;
	map_top(3, 11, 16, i, 0, &j, &gd);
	gt = inv_map_top(3, 11, 16, j);

    }

}


int
main (argc, argv)
int	argc;
char	*argv[];
{
    char    ch;
    int	    rem;
    int	    day_out, channel_out;
    int	    starttime_out, duration_out;

    int s_year_today;
    int s_month_today;
    int s_day_today;
    int s_channel;
    int s_starttime;
    int s_duration;


    struct tm *tm;
    time_t t;


    g_progname = argv[0];
    g_iflag = 0;
    g_debug = 0;
    g_encode = 0;
    g_starttime = s_starttime = 1930;
    g_duration = s_duration = 30;
    g_starttimem = hhmm2mmmm(g_starttime);
    g_durationm = hhmm2mmmm(g_duration);
    g_channel = s_channel = 2;

    t = time (NULL);
    tm = localtime (&t);
    s_month_today = g_month_today = tm->tm_mon + 1;
    s_day_today = g_day_today = tm->tm_mday;
    s_year_today = g_year_today = tm->tm_year + 1900;

    while ((ch = getopt(argc, argv, "veg:y:m:d:c:t:l:i")) != -1) {
      switch (ch) {
	case 'g':       /* as in -g to compiler for debuGging */
	    g_debug = strtol(optarg, (char **)NULL, 0);
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
	    s_year_today = strtol(optarg, (char **)NULL, 0);
	    if ((s_year_today < 0) || (s_year_today > 2999))
		errdie("Year out of range");
	    break;
	case 'm':
	    s_month_today = strtol(optarg, (char **)NULL, 0);
	    if ((s_month_today < 1) || (s_month_today > 12))
		errdie("Month out of range 1-12");
	    break;
	case 'd':
	    s_day_today = strtol(optarg, (char **)NULL, 0);
	    if ((s_day_today < 1) || (s_day_today > 31))
		errdie("Day out of range 1-31");
	    break;
	case 'c':
	    s_channel = strtol(optarg, (char **)NULL, 0);
	    if ((s_channel < 1) || (s_channel > 128))
		errdie("Channel out of range 1-128");
	    break;
	case 't':
	    s_starttime = atoi(optarg);     /* Decimal only for times */
	    rem = s_starttime % 100;
	    if ((rem % 5) ||
		(rem < 0) ||
		(rem > 55) ||
	        (s_starttime < 0) ||
	        (s_starttime > 2355)
		)
		errdie("Start time not an even five minutes from 0000 to 2355");
	    break;
	case 'l':
	    s_duration = atoi(optarg);      /* Decimal only for times */
	    rem = s_duration % 100;
	    if ((rem % 5) ||
	        (s_duration < 5) ||
	        (s_duration > 800)
		)
		errdie("Duration not a multiple of 5 from 5 to 800");
	    break;
        case 'h':
	    usage();
	    exit(0);
	    break;
      }
    }
    argc -= optind;
    argv += optind;

    if (g_debug) {
	main_debug();       /* do debugging stuff first */
    }

    g_year_today = s_year_today;
    g_month_today = s_month_today;
    g_day_today = s_day_today;
    g_channel = s_channel;
    g_starttime = s_starttime;
    g_duration = s_duration;
    g_starttimem = hhmm2mmmm(g_starttime);
    g_durationm = hhmm2mmmm(g_duration);

    if (0 == g_encode) {
	if (argc != 1) {
	    if (g_debug) {
		exit(0);    /* Don't complain if debug routines were run */
	    }
	    printf("ERROR: decoding requires a code as single argument.\n");
	    usagex();
	}
	g_newspaper = strtol(argv[0], (char **)NULL, 0);
    	if (g_verbose) {        /* Announce what we're decoding */
	    printf ("Today's date: %s %d %d\n", monthname[g_month_today-1],
		    g_day_today, g_year_today);
        }

        decode_main (g_month_today, g_day_today, g_year_today, g_newspaper,
		     &day_out, &channel_out,
     		     &starttime_out, &duration_out);

	if (g_year_today < 50)		/* window: <50=year 2000's, >50=year 1900's */
		g_year_today += 100;
	if (g_year_today < 1900)
		g_year_today += 1900;
	g_channel_name = channame(channel_out);
	printf ("Program: %s %d %d, channel %d (%s), at %s, duration %d:%2.2d.\n",
		monthname[g_month_today-1], day_out, g_year_today, channel_out,
		g_channel_name, timestr(mmmm2hhmm(starttime_out)),
		duration_out / 60, duration_out % 60);

    } else {
        g_newspaper = encode_main (g_month_today, g_day_today, g_year_today,
				   g_channel, g_starttimem, g_durationm);

    	if (g_verbose) {        /* Announce what we're decoding */
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
