#!/usr/bin/perl -w
# Code derived from the C version at <http://www.cryptome.org/vcrpp.c>
# plus improvements
# Yes, this is just translated C code.  A perl guru can make it more perlish.

use Tk;
require Tk::BrowseEntry;

my $g_decyear;
my $g_decmonth;
my $g_decday;
my $g_decpluscode;
my $g_decchannel;
my $g_decchannelname;
my $g_decstart;
my $g_decstartm;
my $g_decduration;
my $g_decdurationm;

my $g_encyear;
my $g_encmonth;
my $g_encday;
my $g_encpluscode;
my $g_encchannel;
my $g_encchannelname;

my $g_today_year;
my $g_today_month;
my $g_today_day;

my $i;

my $NDIGITS=10;

$g_encpluscode = "";
$g_encchannel = "2";
$g_encstart = "1900";
$g_encduration = "30";
$g_decpluscode = "3113";


# The following tables belong in the VCRPLUS work section, but
# are here so they get initialized before getting into main.
# This should get fixed when the work routines become a module.

# First, the two (well, three) tables
# Primary table of 4 * 48 = 192 entries
# Index is "magic" from the pluscode
# Value is 24*2*<hour of start> + (0 or 1) half-hours of start +
# half-hours of duration - 1 [= 0-3]) * 48
# for durations = 30, 60, 90, 120 minutes and starts = any half hour 0000-2330
# Also used as a quasi-random number generator in the 7 and 8 digit codes.

my @g_ttbl = (
 0x25, 0x20, 0x27, 0x21, 0x1F, 0x23, 0x24, 0x1D,
 0x26, 0x52, 0x1C, 0x29, 0x22, 0xB0, 0x28, 0x1E,
 0xB8, 0xBA, 0x58, 0xB4, 0x56, 0x5C, 0x5A, 0xAC,
 0x4E, 0xBC, 0x17, 0x16, 0x2E, 0x50, 0x8A, 0x2A,
 0x19, 0x1B, 0x13, 0x4A, 0x2B, 0x48, 0xA4, 0x54,
 0x2C, 0x18, 0x10, 0x11, 0xB2, 0x12, 0x2D, 0x15,
 0xB6, 0x0F, 0x5E, 0x44, 0x0E, 0x1A, 0x9E, 0x46,
 0x4C, 0x14, 0xA0, 0x2F, 0xAA, 0xA8, 0xA2, 0x0D,
 0x84, 0x0C, 0x0B, 0x00, 0xBF, 0x8C, 0x7A, 0x42,
 0x81, 0x80, 0x7D, 0x88, 0x85, 0x3C, 0x78, 0x01,
 0x93, 0x30, 0x82, 0x90, 0x40, 0x3E, 0xBB, 0x0A,
 0x7F, 0xA7, 0xA6, 0x71, 0x8D, 0x72, 0x8B, 0xB1,
 0x5F, 0x92, 0x7C, 0x03, 0x97, 0x7E, 0xAE, 0xBE,
 0x86, 0x70, 0x09, 0x06, 0xAB, 0x74, 0x6E, 0x02,
 0x8F, 0x07, 0x04, 0xBD, 0x08, 0x9C, 0x98, 0x05,
 0x6D, 0x31, 0x5D, 0x32, 0x91, 0x8E, 0x51, 0x41,
 0x60, 0xB7, 0xA3, 0x89, 0x3A, 0x53, 0x94, 0x87,
 0x73, 0xB3, 0x9D, 0x55, 0x4D, 0x77, 0x61, 0xA1,
 0x75, 0xAD, 0x62, 0x9F, 0xB9, 0x66, 0x96, 0x7B,
 0x79, 0x65, 0x5B, 0x47, 0xB5, 0x3D, 0x3B, 0x34,
 0xAF, 0x3F, 0x6C, 0x83, 0x38, 0x6F, 0x69, 0x39,
 0x63, 0xA9, 0x33, 0x95, 0x57, 0x36, 0xA5, 0x64,
 0x37, 0x9A, 0x43, 0x35, 0x59, 0x68, 0x4F, 0x99,
 0x4B, 0x49, 0x67, 0x45, 0x6A, 0x9B, 0x6B, 0x76
);

# Second table controls 7 and 8 digit pluscodes

my @g_LengthEightTable = (
 0x14, 0x02, 0x23, 0x03, 0x41, 0x14, 0x14, 0x32,
 0x34, 0x03, 0x32, 0x40, 0x10, 0x01, 0x12, 0x32,
 0x40, 0x21, 0x24, 0x41, 0x23, 0x14, 0x23, 0x41,
 0x02, 0x04, 0x30, 0x24, 0x41, 0x40, 0x01
);

my @g_LengthSevenTable = (
 0x03, 0x31, 0x12, 0x12, 0x30, 0x03, 0x03, 0x21,
 0x23, 0x12, 0x21, 0x31, 0x03, 0x30, 0x01, 0x21,
 0x31, 0x10, 0x13, 0x30, 0x12, 0x03, 0x12, 0x30,
 0x31, 0x13, 0x21, 0x13, 0x30, 0x31, 0x30
);

my @inv_ttbl;

# Localize this table with local station names/mapped-channels

my @g_chan_names;
$g_chan_names[ 2] = "WXXA2";
$g_chan_names[ 3] = "WXXB3";
$g_chan_names[ 4] = "WXXC4";
$g_chan_names[ 5] = "WXXD5";
$g_chan_names[ 6] = "WXXE6";
$g_chan_names[ 7] = "WXXF7";
$g_chan_names[ 8] = "WXXG9";
$g_chan_names[ 9] = "WXXH9";
$g_chan_names[10] = "WXXI10";
$g_chan_names[11] = "WXXJ11";
$g_chan_names[12] = "WXXK12";
$g_chan_names[12] = "WXXL13";
$g_chan_names[33] = "HBO";
$g_chan_names[34] = "ESPN";
$g_chan_names[35] = "AMC";
$g_chan_names[37] = "DSC";
$g_chan_names[38] = "NIK";
$g_chan_names[39] = "A&E";
$g_chan_names[41] = "SHO";
$g_chan_names[42] = "CNN";
$g_chan_names[43] = "TBS";
$g_chan_names[44] = "USA";
$g_chan_names[45] = "MAX";
$g_chan_names[46] = "LIF";
$g_chan_names[47] = "FAM";
$g_chan_names[48] = "MTV";
$g_chan_names[49] = "TNN";
$g_chan_names[51] = "TLC";
$g_chan_names[52] = "TNT";
$g_chan_names[53] = "DIS";
$g_chan_names[54] = "BRV";
$g_chan_names[57] = "BET";
$g_chan_names[58] = "TMC";
$g_chan_names[59] = "FSN";
$g_chan_names[62] = "VH1";
$g_chan_names[63] = "E";
$g_chan_names[69] = "H&G";
$g_chan_names[70] = "APL";
$g_chan_names[73] = "HIS";
$g_chan_names[75] = "COM";
$g_chan_names[76] = "TCM";
$g_chan_names[79] = "CNBC";
$g_chan_names[80] = "STARZ";
$g_chan_names[82] = "NEC";
$g_chan_names[85] = "FX";
$g_chan_names[86] = "CRT";
$g_chan_names[89] = "SCI";
$g_chan_names[90] = "MSNBC";
$g_chan_names[91] = "FNC";
$g_chan_names[99] = "QVC";
$g_chan_names[103] ="HAL";
;

# Code starts here

($g_today_year, $g_today_month, $g_today_day) = (localtime) [5,4,3];
$g_today_year += 1900;
$g_today_month += 1;

$g_encyear      = $g_today_year;
$g_encmonth     = $g_today_month;
$g_encday       = $g_today_day;

$g_decyear      = $g_today_year;
$g_decmonth     = $g_today_month;

$mw = MainWindow->new;
$mw->title("VCRplus+ Decoder/Encoder");
$mw->Label(-text =>"VCRplus+ Decoder/Encoder")->pack(
                                                 -side => 'top',
						 -anchor => 'center');;

$mw->Button(-text => "Quit",
    -command => sub {exit})->pack(-side => 'bottom',
                                  -expand => 1,
				  -fill => 'x');

my $df = $mw->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'left',
		     -anchor => 'n',
	             -padx => 10,
		     -pady => 10);

my $ef = $mw->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'right',
	             -anchor => 'n',
       		     -padx => 10,
       		     -pady => 10);

# The Decode frame

$df->Label(-text => "Decode from a Pluscode")
           ->pack(-side => "top",
	   -anchor=> 'w');

my $be_d_y = $df->BrowseEntry(
	-label => "Year:",
	-width => 4,
	-variable => \$g_decyear);

foreach $i (1990..2037) {
    $be_d_y->insert('end', "$i");
}

$be_d_y->pack(
    -side => 'top',
    -anchor => 'w');


my $be_d_m = $df->BrowseEntry(
	-label => "Month:",
	-width => 2,
	-variable => \$g_decmonth);

foreach $i (1..12) {
    $be_d_m->insert('end', "$i");
}

$be_d_m->pack(
    -side => 'top',
    -expand => 1,
    -anchor => 'w');



my $df1 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


$df->Button(-text => "Decode",
    -command => \&do_decode_gui )->pack(-side => 'top',
				    -pady => 10);



my $df2 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $df3 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $df4 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');



my $df5 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');



my $df6 = $df->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $lab_d_pc = $df1->Label(
        -text => "PlusCode:")->pack(
	    -side => "left",
	    -expand => 1,
	    -anchor=> 'w');
my $ent_d_pc = $df1->Entry(
	-width => 8,
	-textvariable => \$g_decpluscode)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');


my $lab_d_ch = $df2->Label(-text => "Channel:")->pack(-side => "left", -anchor=> 'w');
my $ent_d_ch = $df2->Entry(
	-width => 3,
	-textvariable => \$g_decchannel)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');


my $lab_d_chnam = $df3->Label(-text => "Channel Name:")->pack(-side => "left", -anchor=> 'w');
my $ent_d_chnam = $df3->Entry(
	-width => 8,
	-textvariable => \$g_decchannelname)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');

my $lab_d_day = $df4->Label(-text => "Day:")->pack(-side => "left", -anchor=> 'w');
my $ent_d_day = $df4->Entry(
	-width => 2,
	-textvariable => \$g_decday)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');



my $lab_d_start = $df5->Label(-text => "Start (hhmm):")->pack(-side => "left", -anchor=> 'w');
my $ent_d_start = $df5->Entry(
	-width => 4,
	-textvariable => \$g_decstart)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');

my $lab_d_duration = $df6->Label(-text => "Duration (hmm):")->pack(-side => "left", -anchor=> 'w');
my $ent_d_duration = $df6->Entry(
	-width => 3,
	-textvariable => \$g_decduration)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');


# The Encode frame

$ef->Label(-text => "Encode to a Pluscode")->pack(-side => "top", -anchor=> 'n');



my $ef1 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $ef2 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $ef3 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $ef4 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');

my $ef5 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $be_e_ch = $ef->BrowseEntry(
	-label => "Channel:",
	-width => 3,
	-variable => \$g_encchannel);

foreach $i (1..127) {
    $be_e_ch->insert('end', "$i");
}

$be_e_ch->pack(
    -side => 'top',
    -anchor => 'w');

$ef->Button(-text => "Encode",
    -command => \&do_encode_gui )->pack(-side => 'top',
				    -pady => 10);

my $ef6 = $ef->Frame(-borderwidth => 2,
                    -relief => 'groove')
	      ->pack(-side => 'top',
		     -expand => 1,
	             -anchor => 'w');


my $lab_e_pc = $ef6->Label(
        -text => "PlusCode:")->pack(
	    -side => "left",
	    -expand => 1,
	    -anchor=> 'w');
my $ent_e_pc = $ef6->Entry(
	-width => 8,
	-textvariable => \$g_encpluscode)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');

my $be_e_y = $ef1->BrowseEntry(
	-label => "Year:",
	-width => 4,
	-variable => \$g_encyear);

foreach $i (1990..2037) {
    $be_e_y->insert('end', "$i");
}

$be_e_y->pack(
    -side => 'top',
    -anchor => 'w');

my $be_e_m = $ef2->BrowseEntry(
	-label => "Month:",
	-width => 2,
	-variable => \$g_encmonth);

foreach $i (1..12) {
    $be_e_m->insert('end', "$i");
}

$be_e_m->pack(
    -side => 'top',
    -anchor => 'w');

my $be_e_d = $ef3->BrowseEntry(
	-label => "Day:",
	-width => 2,
	-variable => \$g_encday);

foreach $i (1..31) {
    $be_e_d->insert('end', "$i");
}

$be_e_d->pack(
    -side => 'top',
    -anchor => 'w');

my $lab_e_start = $ef4->Label(-text => "Start (hhmm):")->pack(-side => "left", -anchor=> 'w');
my $ent_e_start = $ef4->Entry(
	-width => 4,
	-textvariable => \$g_encstart)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');

my $lab_e_duration = $ef5->Label(-text => "Duration (hmm):")->pack(-side => "left", -anchor=> 'w');
my $ent_e_duration = $ef5->Entry(
	-width => 3,
	-textvariable => \$g_encduration)->pack(
	    -side => 'right',
	    -expand => 1,
	    -anchor => 'e');

$ent_d_pc->focus;
MainLoop;


# Above is all the GUI stuff.  The VCRPLUS logic follows.


# Here are the real work routines, taking args and returning
# results as per the usual subroutine interface.
# This is what should be split off into a module.

# Glue routines to move args/vals from/to globals

sub do_decode_gui {

    ($g_decday, $g_decstartm, $g_decdurationm, $g_decchannel) =
	vcrpp_decode($g_decyear, $g_decmonth, $g_decpluscode);

    $g_decstart = (100 * (int ($g_decstartm / 60))) +
		     ($g_decstartm % 60);

    $g_decduration = (100 * (int ($g_decdurationm / 60))) +
		     ($g_decdurationm % 60);

# Report name of channel, if known
    $g_decchannelname = ("Ch " . $g_decchannel);
    if ($g_chan_names[$g_decchannel]) {
        $g_decchannelname = $g_chan_names[$g_decchannel];
    }
}

sub do_encode_gui {

    $g_encpluscode =
	vcrpp_encode($g_encyear, $g_encmonth, $g_encday,
	             $g_encstart, $g_encduration, $g_encchannel);
}

#******************************************************************
#* Utility routines
#******************************************************************

sub
count_digits()
{
    my $val;
    my $ndigits;
    ($val) = @_;

    if    ($val < 1) {$ndigits = 0}
    elsif ($val < 10) {$ndigits = 1}
    elsif ($val < 100) {$ndigits = 2}
    elsif ($val < 1000) {$ndigits = 3}
    elsif ($val < 10000) {$ndigits = 4}
    elsif ($val < 100000) {$ndigits = 5}
    elsif ($val < 1000000) {$ndigits = 6}
    elsif ($val < 10000000) {$ndigits = 7}
    else                     {$ndigits = 8};

    $ndigits;
}

sub
set_pwr()
{
    my $ndigits = $_;
    my $pwr = 0x7fffffff;

    if (0 == $ndigits)  {$pwr = 1};
    if (1 == $ndigits)  {$pwr = 10};
    if (2 == $ndigits)  {$pwr = 100};
    if (3 == $ndigits)  {$pwr = 1000}; 
    if (4 == $ndigits)  {$pwr = 10000};
    if (5 == $ndigits)  {$pwr = 100000};
    if (6 == $ndigits)  {$pwr = 10000000};
    if (7 == $ndigits)  {$pwr = 100000000}; 
    if (8 == $ndigits)  {$pwr = 1000000000};
    if (9 == $ndigits)  {$pwr = 10000000000};
    $pwr;
}

# Split the code into an array of digits, lsb at low index

sub
split_digits()
{
    my $n;
    my @a;
    my $aref;

    ($n, $aref) = @_;

    my $i;
    my $digit;;

    for ($i = 0; $i < $NDIGITS; $i++) {
       	$$aref[$i] = 0;
    }

    for ($i = 0; $i < $NDIGITS; $i++) {
	$digit = $n % 10;
	$$aref[$i] = $digit;
	$n = ($n - $digit) / 10;
    }
}

# Inverse of the decoder's initial transform.  For encoder.

sub
encfunc1 ()
{
#input/output
    my $val;

#locals
    my $ndigits;
    my $pwr;

    ($val) = @_;

    $ndigits = 0;
    $pwr = 1;
    while ($val >= $pwr) {
	$ndigits++;
	$pwr *= 10;
    }
    if ($ndigits > 8) {
	return 0;
    }
    $pwr /= 10;

    do {
	$val = &encode_final_transform($val, 9371) % ($pwr * 10);
    } while ($val < $pwr);

    return ($val);
}

sub
twiddle_tt()
{
#input
    my $tt;

#outputs
    my $t;
    my $d;
    my $x1;
    my $x2;

# locals
    my $t1;
    my $t2;
    my $b;

    $tt = $_[0];

    $x1 = 0;
    $x2 = 0;
    $t1 = 0;
    $t1 = 0;
    $d  = 0;
    $t  = 0;
    $b  = 0;

    if (($tt >= 768) && ($tt <= 3647)) {
	$t1 = (($tt - 768) % 10) + 1;
	$t2 = $t1 * 5;
	if ($t1 >= 6) {
	    $x2 = $t2 - 25;
	} else {
	    $x1 = $t2;
	}
	$tt -= 768;
	$tt = int ($tt / 10);
    } 
    elsif (($tt >= 3648) && ($tt <= 6527)) {
	$t1 = (($tt - 3648) % 6) + 1;
	if ($t1 == 1) {
	    $x1 = 15;
	} else {
	    $x2 = (5 * $t1) - 5;
	}
	$tt = int (($tt - 3648) / 6) + 288;
    }
    elsif (($tt > 6527) && ($tt <= 13727)) {
	$x1 = 5 + (((($tt-6528) % 25) % 5) * 5);
	$x2 = 5 + (int((($tt-6528) % 25) / 5) * 5);
	$tt -= 6528;
	$tt = int ($tt / 25);
    }
    elsif ($tt > 13727) {
	$x2 = 5 + ((($tt - 13728) % 5) * 5);
	$x1 = 15;
	$tt -= 13728;
	$tt = int ($tt / 5) + 288;
    }

    if ($tt < 192) {
	# Lookup in table
	$b = $g_ttbl[$tt];
	$t = $b % 48;  		# Half hours after midnight
	$d = int ($b / 48);     # Half hours of duration - 1

    } else {
	$t = 47 - (($tt - 192) % 48);   # Half hours after midnight
	$d = int ($tt / 48);            # Half hours of duration - 1
    }

    @_ = ($t, $d, $x1, $x2);
}

sub
interleave ()
{
#inputs
    my $tval;
    my $cval;

    ($tval, $cval) = @_;

#outputs
    my $top5 = 0;
    my $bot3  = 0;

    if ($tval & (1 <<  0)) {($bot3 |= (1 <<  0))};
    if ($cval & (1 <<  0)) {($bot3 |= (1 <<  1))};
    if ($tval & (1 <<  1)) {($bot3 |= (1 <<  2))};
    if ($cval & (1 <<  1)) {($bot3 |= (1 <<  3))};
    if ($tval & (1 <<  2)) {($bot3 |= (1 <<  4))};

    if ($tval & (1 <<  3)) {($top5 |= (1 <<  0))};
    if ($cval & (1 <<  2)) {($top5 |= (1 <<  1))};
    if ($cval & (1 <<  3)) {($top5 |= (1 <<  2))};
    if ($tval & (1 <<  4)) {($top5 |= (1 <<  3))};
    if ($tval & (1 <<  5)) {($top5 |= (1 <<  4))};
    if ($tval & (1 <<  6)) {($top5 |= (1 <<  5))};
    if ($cval & (1 <<  4)) {($top5 |= (1 <<  6))};
    if ($tval & (1 <<  7)) {($top5 |= (1 <<  7))};
    if ($cval & (1 <<  5)) {($top5 |= (1 <<  8))};
    if ($tval & (1 <<  8)) {($top5 |= (1 <<  9))};
    if ($tval & (1 <<  9)) {($top5 |= (1 << 10))};
    if ($tval & (1 << 10)) {($top5 |= (1 << 11))};
    if ($cval & (1 <<  6)) {($top5 |= (1 << 12))};
    if ($tval & (1 << 11)) {($top5 |= (1 << 13))};
    if ($tval & (1 << 12)) {($top5 |= (1 << 14))};
    if ($tval & (1 << 13)) {($top5 |= (1 << 15))};
    if ($cval & (1 <<  7)) {($top5 |= (1 << 16))};

# Untested below. Channels over 127.
    if ($top5 & (1 << 16)) {
    	if ($tval & (1 << 11)) {($top5 |= (1 << 12))};
    	if ($tval & (1 << 12)) {($top5 |= (1 << 13))};
    	if ($tval & (1 << 13)) {($top5 |= (1 << 14))};
    	if ($cval & (1 <<  6)) {($top5 |= (1 << 15))};
    }
# Untested above. Channels over 127.

# Return shuffled bits
    @_ = ($top5, $bot3);
}

sub
inv_twiddle_tt(int startm, int durnm)
{
#inputs
    my $startm;
    my $durnm;
#output
    my $tt = 0;;
#locals
    my $x1 = 0;
    my $x2 = 0;
    my $t1 = 0;
    my $t2 = 0;
    my $t = 0;
    my $d = 0;
    my $b = 0;
    my $i = 0;
    my $bstart = 0;
    my $bdurn = 0;
    my $basett = 0;

    ($startm, $durnm) = @_;

    if (($durnm < 5) ||
        ($durnm > (60*8)) ||
    	($startm < 0) ||
	($startm > (23*60)+59) ) {
	    return (-1);
    }

    if ($durnm  % 5) { $durnm  = $durnm + 5  - ($durnm  % 5); }
    if ($durnm  > (8 * 60)) { $durnm  = 8 * 60; }
    if ($startm % 5) { $startm = $startm - ($startm % 5); }

    $bstart = $startm - ($startm % 30);
    $bdurn  = $durnm + 29;
    $bdurn  = $bdurn - ($bdurn % 30);
    $t = int($bstart / 30);
    $d = int($bdurn / 30);

    $x1 = $bdurn - $durnm;
    $x2 = $startm - $bstart;

    if ($d <= 4) {
	for ($i = 0; $i < 192; $i++) {
	    $basett = $i;
	    last if ($g_ttbl[$i] == ($t + (($d - 1) * 48)));
	}
    } else {
	$basett = (48 * ($d - 1)) + 47 - $t;
    }

    if (($x1 == 0) && ($x2 == 0)) {
	return $basett;
    }

    elsif (($d <= 6) && (($x1 == 0) || ($x2 == 0))) {
	$tt = 768 + (10 * $basett);
	if ($x1) {
	    $tt += int($x1 / 5) - 1;
	}
	elsif ($x2) {
	    $tt += int($x2 / 5) + 4;
	}
	return $tt;
    }

    elsif (($d > 6) && ( (($x1 == 15) && ($x2 == 0)) ||
                     (($x1 ==  0) && ($x2 != 0)) ) ) {

	$tt = 3648 + (6 * ($basett - 288));
	$tt += int($x2 / 5);
	return $tt;
    }

    elsif (($d <= 6) && ($x1) && ($x2)) {
	$tt = 6528 + (25 * $basett);
	$tt += int($x1 / 5) - 1;
	$tt += (int($x2 / 5) - 1) * 5;
	return $tt;
    }

    elsif (($d > 6) && ($x1) && ($x2)) {
	$tt = 13728 + (5 * ($basett - 288));
	$tt += int($x2 / 5) - 1;
	return $tt;
    }
    return -1;
}

sub
map_top()
{
#inputs
    my $year;
    my $month;
    my $day;
    my $top5;
    my $rem;
#outputs
    my $mtout;
    my $remout;
#locals
    my $year_mod16;
    my $year_mod100;
    my $ndigits;
    my $nd;
    my $j;

    my $k;
    my $t1;
    my $t2;
    my $t3;
    my $ym;
    my $datum;
    my $year_today;
    my @n;
    my $nref;

    ($year, $month, $day, $top5, $rem) = @_;


    $nref = \@n;

    $year_today = $year;

    $year_mod100 = $year % 100;
    $year_mod16  = $year_mod100 % 16;

    &split_digits($top5, $nref);
    $ndigits = &count_digits($top5) + 3;
    $nd = $ndigits - 3 - 1;

    $rem  = ($rem + $n[0] + $n[1] + $n[2] + $n[3] + $n[4]) & 0x1f;

    if ($ndigits <= 6) {
	if ($nd >= 0) {
            do {
    	        $k = 0;
                do {
    	            $n[$nd] = ($n[$nd] + $day) % 10;
    	            if ($nd > 0) {
    		        for ($j = $nd - 1; $j >= 0; $j--) {
    		            $n[$j] = ($n[$j] + $n[$j+1]) % 10;
    		        }
    	            }
    		    $rem += $n[0];
                } while (++$k <= $year_mod16);
            } while ($n[$nd] == 0);
	}
        $rem = ($rem + ($day * ($month + 1))) & 0x1f;

    } else { # ndigits > 6

        $ym = ($year_mod100 * 12) + $month;   # 1 to 1200

        do {
	    $k = 1;
    	    do {
		$t1 = ($ym + 310 - $k) % 31;
    		$t1 = ($ndigits == 7) ?
		      $g_LengthSevenTable[$t1] : $g_LengthEightTable[$t1];
                $t2 = ($t1 & 0x0f);
                $t3 = (($t1 >> 4) & 0x0f);
    	     	$t1 = $n[$t2] + (10 * $n[$t3]);
		do {
		    $t1 = ($g_ttbl[$t1] - $ym + 1920) % 192;
		} while ($t1 > 99);
    		$n[$t2] = $t1 % 10;
    		$n[$t3] = int ($t1 / 10);
    	    } while (++$k <= 31);
        } while ($n[$nd] == 0);
    }

    $mtout =  10000*$n[4] + 1000*$n[3] + 100*$n[2] + 10*$n[1] + $n[0];
    @_ = ($mtout, $rem);
}


sub
inv_map_top(year, month, day, top5)
{
#inputs
    my $year;
    my $month;
    my $day;
    my $top5;

# output
    my $mtout;

#locals
    my $year_mod16;
    my $year_mod100;
    my $ndigits;
    my $nd;
    my $i;
    my $j;
    my $k;
    my $t1;
    my $t2;
    my $t3;
    my $ym;
    my @n;
    my $nref;

    ($year, $month, $day, $top5) = @_;
    $nref = \@n;

    $year_mod100 = $year % 100;
    $year_mod16  = $year_mod100 % 16;
    &split_digits($top5, $nref);
    $ndigits = &count_digits($top5) + 3;
    $nd = $ndigits - 3 - 1;

    SWITCH: {
      if (($ndigits >= 0) && ($ndigits <= 3)) {
	$mtout = $top5;
	last SWITCH;
      }
      elsif (($ndigits >= 4) && ($ndigits <= 6)) {
        do {
    	    $k = 0;
            do {
		if ($nd > 0) {
		    for ($j = 0; $j < $nd; $j++) {
			$n[$j] = ($n[$j] + 100 - $n[$j+1]) % 10;
		    }
		}
    	        $n[$nd] = ($n[$nd] + 100 - $day) % 10;
            } while (++$k <= $year_mod16);
        } while ($n[$nd] == 0);

        $mtout =  10000*$n[4] + 1000*$n[3] + 100*$n[2] + 10*$n[1] + $n[0];
	last SWITCH;
      }
      elsif (($ndigits >= 7) && ($ndigits <= 8)) {
	$ym = ($year_mod100 * 12) + $month;   # 1 through 1200

	for ($i = 0; $i < 100; $i++) {
	    $j = $i;
	    do {
		$j = ($g_ttbl[$j] - $ym + 1920) % 192;
	    } while ($j > 99);
	    $inv_ttbl[$j] = $i;
	}
        do {
	    $k = 31;
    	    do {
		$t1 = ($ym + 310 - $k) % 31;
    		$t1 = ($ndigits == 7) ?
		     $g_LengthSevenTable[$t1] : $g_LengthEightTable[$t1];
                $t2 = ($t1 & 0x0f);
                $t3 = (($t1 >> 4) & 0x0f);
    	        $t1 = $n[$t2] + (10 * $n[$t3]);
		$t1 = $inv_ttbl[$t1];
    		$n[$t2] = $t1 % 10;
    		$n[$t3] = int($t1 / 10);
    	    } while (--$k >= 1);
        } while ($n[$nd] == 0);
        $mtout =  10000*$n[4] + 1000*$n[3] + 100*$n[2] + 10*$n[1] + $n[0];
        last SWITCH;
      }
      else {
	$mtout = (-1);
	last SWITCH;
      }
    }
    return $mtout;
}

sub
bit_shuffle ()
{
#inputs
    my $code;

#outputs
    my $tval;
    my $dval;
    my $cval;

#locals
    my  $tt;
    my	$cc;
    my  $x1;
    my  $x2;
    my  $nn;
    my  $top5;
    my  $rem;
    my  $t;
    my  $d;
    my  $outtime;
    my  $outdur;

# Setup arg and locals
    $code = $_[0];
    $tt = 0;
    $cc = 0;
    $x1 = 0;
    $x2 = 0;
    $nn = $code - 1;
    $top5 = int ($nn / 1000);
    $rem = int ($nn % 1000)  & 0x1f;

    if ($rem  & (1 <<  0)) { $tt |= (1 <<  0)};
    if ($rem  & (1 <<  2)) { $tt |= (1 <<  1)};
    if ($rem  & (1 <<  4)) { $tt |= (1 <<  2)};
    if ($top5 & (1 <<  0)) { $tt |= (1 <<  3)};
    if ($top5 & (1 <<  3)) { $tt |= (1 <<  4)};
    if ($top5 & (1 <<  4)) { $tt |= (1 <<  5)};
    if ($top5 & (1 <<  5)) { $tt |= (1 <<  6)};
    if ($top5 & (1 <<  7)) { $tt |= (1 <<  7)};
    if ($top5 & (1 <<  9)) { $tt |= (1 <<  8)};
    if ($top5 & (1 << 10)) { $tt |= (1 <<  9)};
    if ($top5 & (1 << 11)) { $tt |= (1 << 10)};
    if ($top5 & (1 << 13)) { $tt |= (1 << 11)};
    if ($top5 & (1 << 14)) { $tt |= (1 << 12)};
    if ($top5 & (1 << 15)) { $tt |= (1 << 13)};

    if ($rem  & (1 <<  1)) { $cc |= (1 <<  0)};
    if ($rem  & (1 <<  3)) { $cc |= (1 <<  1)};
    if ($top5 & (1 <<  1)) { $cc |= (1 <<  2)};
    if ($top5 & (1 <<  2)) { $cc |= (1 <<  3)};
    if ($top5 & (1 <<  6)) { $cc |= (1 <<  4)};
    if ($top5 & (1 <<  8)) { $cc |= (1 <<  5)};
    if ($top5 & (1 << 12)) { $cc |= (1 <<  6)};
    if ($top5 & (1 << 16)) { $cc |= (1 <<  7)};

# Following not verified - haven't seen a code with the high bit on here
    if ($top5 & (1 << 16)) {
    	if ($top5 & (1 << 12)) { $tt |= (1 << 11)};
    	if ($top5 & (1 << 13)) { $tt |= (1 << 12)};
    	if ($top5 & (1 << 14)) { $tt |= (1 << 13)};
    	if ($top5 & (1 << 15)) { $cc |= (1 <<  6)};
    }

    ($t, $d, $x1, $x2) = &twiddle_tt($tt);

    $outtime = (30 * $t) + $x2;
    $outdur  = (($d + 1) * 30) - $x1;

    @_ = ($cc + 1, $outdur, $outtime);
}

sub vcrpp_decode {
#args
    my $year;
    my $month;
    my $pluscode;

#results
    my $channel;
    my $day;
    my $start;
    my $duration;

#locals
    my $s1_out;
    my $bot3;
    my $top5;
    my $quo;
    my $rem;
    my $mtout;
    my $tval;
    my $dval;
    my $cval;
    my $day_out;
    my $modnews;

# Read the inouts
    ($year, $month, $pluscode) = @_;

    $year = $year % 100;

    if ($month<1 || $month>12) {
	die "Invalid month\n";
    }

    if (($pluscode < 1) || ($pluscode > 99999999)) {
	die "Invalid pluscode(TM), not 1 thru 99999999\n";
    }
    $s1_out = func1($pluscode);
    $bot3 = ($s1_out % 1000);
    $quo = ($bot3 - 1) >> 5;
    $rem = ($bot3 - 1) & 0x1f;
    $day = $quo + 1;
    $top5 = int ($s1_out / 1000);

    ($mtout, $rem) =  &map_top($year, $month, $day, $top5, $rem);
    $modnews = $mtout * 1000;
    $modnews += ($day << 5) + $rem - 31;
    ($cval, $dval, $tval) = &bit_shuffle($modnews);

    @_ = ($day, $tval, $dval, $cval);
}


sub vcrpp_encode {

#args
    my $year;
    my $month;
    my $day;
    my $starttime;
    my $starttimem;
    my $duration;
    my $durationm;
    my $channel;

#results
    my $newspaper;

#locals
    my $j = 0;
    my $s1_out = 0;
    my $bot3 = 0;
    my $top5 = 0;
    my $quo = 0;
    my $rem = 0;
    my $cval = 0;
    my $tval = 0;
    my $big_top5 = 0;
    my $big_rem = 0;
    my $mtout = 0;

    ($year, $month, $day, $starttime, $duration, $channel) = @_;
    $year = $year % 100;

    return 0 if $starttime > 2355;
    return 0 if $starttime < 0;
    return 0 if ($starttime % 100) > 55;
    return 0 if ($starttime % 5) != 0;
    return 0 if $duration > 800;
    return 0 if $duration < 0;
    return 0 if ($duration % 100) > 55;
    return 0 if ($duration % 5) != 0;
    return 0 if $day < 1;
    return 0 if $day > 31;
    return 0 if $month < 1;
    return 0 if $month > 12;
    return 0 if $year < 0;
    return 0 if $channel < 1;
    return 0 if $channel > 127;

    $starttimem = (60 * int($starttime / 100) + ($starttime % 100));
    $durationm = (60 * int($duration / 100) + ($duration % 100));

    $cval = $channel - 1;
    $tval = &inv_twiddle_tt($starttimem, $durationm);
    if ($tval < 0) {
	    return (0);
    }

    ($big_top5, $big_rem) = &interleave ($tval, $cval);

    $top5 = &inv_map_top($year, $month, $day, $big_top5);
    if ($top5 < 0) {
	return 0;
    }
    ($mtout, $rem) = &map_top($year, $month, $day, $top5, 0);
    $quo = $day - 1;
    $rem = ($big_rem + 320 - $rem) % 32;
    $bot3 = $rem + 1 + (32 * $quo);
    $s1_out = $bot3 + (1000 * $top5);

    $newspaper = &encfunc1($s1_out);
    return ($newspaper);
}

# func1, for decoding only

sub
func1()
{
    my $code;
    my $x;
    my $aref;
    my @a;

    my $sum;
    my $i;
    my $j;
    my $ndigits = -1;
    my $nd = 0;

    ($code)= @_;
    $x= $code;
    $aref = \@a;

    &split_digits($x, $aref);
    $ndigits = &count_digits($x);
    $nd = $ndigits - 1;

    do {
	if ($nd >= 0) {
    	    $i = 0;
    	    do {
    	        $j = 1;
    	        if ($nd >= 1) {
    		    do {
    		        $a[$j] = ($a[$j-1] + $a[$j]) % 10;
        	    } while (++$j <= $nd);
    	        }
    	    } while (++$i <= 2);
	}
    } while ($a[$nd] == 0);

    $sum = 0;
    $j = 1;
    for ($i = 0; $i < $NDIGITS; $i++) {
	$sum += $j * $a[$i];
	$j *= 10;
    }
    return $sum;
}
 
sub
encode_final_transform ()
{
    my $x;
    my $y;

    ($x, $y) = @_;

    my     ($i, $j, $digit, $sum);
    my     (@a, @b, @out);

    for ($i=0; $i<9; $i++) {
	$digit = $x % 10;
	$a[$i] = $digit;
	$x = ($x - $digit) / 10;
    }

    for ($i=0; $i<9; $i++) {
	$digit = $y % 10;
	$b[$i] = $digit;
	$y = ($y - $digit) / 10;
    }

    for ($i=0; $i<17; $i++) {
	$out[$i] = 0;
    }

    for ($i=0; $i<=8; $i++) {
	for ($j=0; $j<=8; $j++) {
	    $out[$i+$j] += $b[$j] * $a[$i];
	}
    }

    $j = 1;
    $sum = 0;
    for ($i=0; $i<=8; $i++) {
	$sum += $j * ($out[$i] % 10);
	$j *= 10;
    }
    return ($sum);
}
