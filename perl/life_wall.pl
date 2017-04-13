#!/usr/bin/env perl

use common::sense;
use Game::Life;
use Getopt::Long;

my $size        = [ 25, 32 ];
my $keeplife    = [3];
my $breedlife   = [ 2, 3, 7 ];
my $life_chance = 0.252;
my $brightness  = 100;
my $no_serial   = 0;
my $port        = '/dev/tty.usbmodem2033001';
# my $colour      = 'ffffff';
my ($red, $green, $blue) = (255,255,255);
my ($ored, $ogreen, $oblue);
my $sleep       = .05;
my $wall;

GetOptions(
    "noserial|n"     => \$no_serial,
    "chance|l=f"     => \$life_chance,
    "device|d=s"     => \$port,
    "red|r=i"        => \$red,
    "green|g=i"      => \$green,
    "blue|b=i"       => \$blue,
    "brightness|v=i" => \$brightness,
    "speed|s=f"      => \$sleep,
);

unless ($no_serial) {
    require Device::SerialPort;
    $wall = Device::SerialPort->new($port) || die "Can't open $port: $!";;
    $wall->baudrate(230400);
}

my $game = Game::Life->new($size, $keeplife, $breedlife);
my $starting = [
    map {
        [ map { int($life_chance + rand) } 1 .. 32 ]
    } 1 .. 25
];

$game->place_points(0, 0, $starting);
while (1) {
    my $grid = $game->get_grid();

    # send new frame header
    $wall->write(chr(1)) unless $no_serial;

    # clear terminal screen
    print "\033[2J\033[0;0H";

    my $r = int($red * ($brightness / 100));
    my $g = int($green * ($brightness / 100));
    my $b = int($blue * ($brightness / 100));
    $ored   = $r - 10;
    $ogreen = $g - 0;
    $oblue  = $b - 10;

    my $line_count = 0;
    foreach my $line (@$grid) {
        unless ($no_serial) {
            $line_count++;

            my $row = join(
                '',
                map {
                    $_
                        # cell is alive
                        ? $line_count % 2
                            # invert colour order every second line because of
                            # snake style LED strip wrapping
                            ? chr($g) . chr($r) . chr($b)
                            : chr($b) . chr($r) . chr($g)

                        # cell is dead (use previous iteration colours)
                        # : $line_count % 2
                        #     # invert colour order every second line because of
                        #     # snake style LED strip wrapping
                        #     ? chr($ogreen) . chr($ored) . chr($oblue)
                        #     : chr($oblue) . chr($ored) . chr($ogreen)
                        : chr(0) . chr(0) . chr(0)
                } @$line
            );
            $row = scalar reverse $row unless ($line_count % 2);
            $wall->write($row);
        }
        print map { $_ ? 'O  ' : '   ' } @$line;
        print "\n";
    }

    # store new colours as old colours
    # ($ored, $ogreen, $oblue) = ($red, $green, $blue);

    # slow down processing
    select(undef, undef, undef, $sleep);

    $game->process();
}
