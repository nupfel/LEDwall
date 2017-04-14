#!/usr/bin/env perl

use common::sense;
use Game::Life;
use Getopt::Long;
use Convert::Color;
# use Data::Printer { escape_chars => 'all' };

my $size        = [ 25, 32 ];
my $keeplife    = [3];
my $breedlife   = [ 2, 3, 7 ];
my $life_chance = .252;

my $brightness  = 0.1;
my $saturation  = 0;
my $hue         = 0;
my $hue_change  = .5;
my $trail_speed = .008;

my $delay     = .05;
my $no_print  = 0;
my $no_serial = 0;
my $port      = '/dev/tty.usbmodem2033001';

my @map;
my $wall;
my ($r, $g, $b, $colour);
my $help;

GetOptions(
    "help|?"          => \$help,
    "no-serial|n"     => \$no_serial,
    "no-print|p"      => \$no_print,
    "life-chance|l=f" => \$life_chance,
    "device|d=s"      => \$port,
    "colour|c=s"      => \$colour,
    "brightness|v=f"  => \$brightness,
    "delay|s=f"       => \$delay,
    "hue-change|h=f"  => \$hue_change,
    "trail-speed|t=f" => \$trail_speed,
);

if ($help) {
    print "Usage: $0 <options>\n";
    print "\n";
    print "  --no-serial, -n         disable serial port output\n";
    print "                          Default: enabled\n";
    print "  --no-print, -p          disable terminal print output\n";
    print "                          this is required for faster processing\n";
    print "                          Default: enabled\n";
    print "  --help, -?              print this usage help\n";
    print "  --life-chance, -l       float value between 0-1 used as chance for life\n";
    print "                          for each cell in the initial setup\n";
    print "                          Default: $life_chance\n";
    print "  --device, -d            serial port to use\n";
    print "                          Default: $port\n";
    print "  --colour, -c            string used by Convert::Colour constructor\n";
    print "                          to set the inital colour.\n";
    print "                          Default: hsv:$hue,$saturation,$brightness\n";
    print "  --brightness, -v        float value between 0-1 to set the overall\n";
    print "                          brightness or all pixels\n";
    print "                          Default: $brightness\n";
    print "  --hue-change, -h        float value between 0-1 used to change the\n";
    print "                          hue value on each line of iteration\n";
    print "                          Default: $hue_change\n";
    print "  --trail-speed, -t       float value between 0-1 used to decrease brightness\n";
    print "                          of dead cells after each iteration\n";
    print "                          Default: $trail_speed\n";
    print "  --delay, -s             float value in seconds to delay processing after each iteration\n";
    print "                          Default: $delay\n";
    exit;
}

my $starting = [
    map {
        [ map { int($life_chance + rand) } 1 .. 25 ]
    } 1 .. 32
];

unless ($no_serial) {
    require Device::SerialPort;
    $wall = Device::SerialPort->new($port) || die "Can't open $port: $!";
    $wall->baudrate(230400);

    $colour = Convert::Color->new($colour
            // ('hsv:' . join(',', $hue, $saturation, $brightness)));
    ($hue, $saturation) = $colour->hsv;

    @map = init_map($starting);
}

my $game = Game::Life->new($size, $keeplife, $breedlife);

$game->place_points(0, 0, $starting);
undef $starting;
while (1) {
    my @grid = @{ $game->get_grid() };

    # send new frame header
    $wall->write(chr(1)) unless $no_serial;

    # clear terminal screen
    print "\033[2J\033[0;0H" unless $no_print;

    for (my $x = 0; $x <= $#grid; $x++) {
        my @line = @{ $grid[$x] };

        unless ($no_serial) {
            for (my $y = 0; $y <= $#line; $y++) {
                if ($grid[$x][$y]) {
                    $map[$x][$y] = [ $hue, $saturation, $brightness ];
                }
                else {
                    $map[$x][$y][2] -= $trail_speed;
                    $map[$x][$y][2] = 0 if ($map[$x][$y][2] <= 0);
                }
            }

            my @row = map {
                my $c = Convert::Color::HSV->new(@$_)->as_rgb8;
                join('',
                    map { chr($_) }
                        $c->green, $c->red, $c->blue)
            } @{ $map[$x] };
            $wall->write(join('', $x % 2 ? @row : reverse @row));
        }
        unless ($no_print) {
            print map { $_ ? 'O  ' : '   ' } @line;
            print "|\n";
        }

        $hue += $hue_change;
    }
    print "---" x $size->[0] . $/ unless $no_print;

    # slow down processing
    select(undef, undef, undef, $delay);

    $game->process();
}

sub init_map {
    my $starting = shift;
    return map {
        [ map { $_ ? [ $hue, $saturation, $brightness ] : [ 0, 0, 0 ] } @$_ ]
    } @$starting;
}
