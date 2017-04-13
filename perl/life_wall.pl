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

my $brightness  = 100;
my $saturation  = 1;
my $hue         = 0;
my $hue_change  = .5;
my $trail_speed = .008;

my $sleep     = .05;
my $no_print  = 0;
my $no_serial = 0;
my $port      = '/dev/tty.usbmodem2033001';

my @map;
my $wall;
my ($r, $g, $b, $colour);

GetOptions(
    "noserial|n"      => \$no_serial,
    "no_print|p"      => \$no_print,
    "chance|l=f"      => \$life_chance,
    "device|d=s"      => \$port,
    "colour|c=s"      => \$colour,
    "brightness|v=i"  => \$brightness,
    "speed|s=f"       => \$sleep,
    "hue-change|h=f"  => \$hue_change,
    "trail-speed|t=f" => \$trail_speed,
);

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
                    $map[$x][$y] = [ $hue, $saturation, $brightness / 100 ];
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
    select(undef, undef, undef, $sleep);

    $game->process();
}

sub init_map {
    my $starting = shift;
    return map {
        [ map { $_ ? [ $hue, $saturation, $brightness ] : [ 0, 0, 0 ] } @$_ ]
    } @$starting;
}
