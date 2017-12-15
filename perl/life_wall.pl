#!/usr/bin/env perl

# use Modern::Perl;
use common::sense;
use Game::Life;
use Getopt::Long;
use Convert::Color;

# use Data::Printer { escape_chars => 'all' };

my $size         = [ 27, 32 ];
my $keeplife     = [3];
my $breedlife    = [ 2, 3 ];
my $life_chance  = .252;
my $life_entropy = .999;

my $brightness  = 0.1;
my $saturation  = 0;
my $hue         = 0;
my $hue_change  = .5;
my $trail_speed = .008;

my $delay     = .05;
my $no_print  = 0;
my $no_serial = 0;
my $port      = '/dev/tty.usbmodem3781501';

my @map;
my $wall;
my ($r, $g, $b, $colour);
my $help;

GetOptions(
    "help|?"           => \$help,
    "no-serial|n"      => \$no_serial,
    "no-print|p"       => \$no_print,
    "life-chance|l=f"  => \$life_chance,
    "device|d=s"       => \$port,
    "colour|c=s"       => \$colour,
    "brightness|v=f"   => \$brightness,
    "delay|s=f"        => \$delay,
    "hue-change|h=f"   => \$hue_change,
    "trail-speed|t=f"  => \$trail_speed,
    "life-entropy|e=f" => \$life_entropy,
);

if ($help) {
    say "Usage: $0 <options>";
    say "";
    say "  --no-serial, -n         disable serial port output";
    say "                          Default: enabled";
    say "  --no-print, -p          disable terminal print output";
    say "                          this is required for faster processing";
    say "                          Default: enabled";
    say "  --help, -?              print this usage help";
    say
        "  --life-chance, -l       float value between 0-1 used as chance for life";
    say "                          for each cell in the initial setup";
    say "                          Default: $life_chance";
    say "  --device, -d            serial port to use";
    say "                          Default: $port";
    say "  --colour, -c            string used by Convert::Colour constructor";
    say "                          to set the inital colour.";
    say "                          Default: hsv:$hue,$saturation,$brightness";
    say "  --brightness, -v        float value between 0-1 to set the overall";
    say "                          brightness or all pixels";
    say "                          Default: $brightness";
    say "  --hue-change, -h        float value between 0-1 used to change the";
    say "                          hue value on each line of iteration";
    say "                          Default: $hue_change";
    say
        "  --trail-speed, -t       float value between 0-1 used to decrease brightness";
    say "                          of dead cells after each iteration";
    say "                          Default: $trail_speed";
    say
        "  --delay, -s             float value in seconds to delay processing after each iteration";
    say "                          Default: $delay";
    say
        " --life-entropy -e        float value between 0.9 and 1 to increase random cell birth,";
    say "                          sparking life";
    say "                          Default: $life_entropy";
    exit;
}

my $starting = [
    map {
        [ map { int($life_chance + rand) } 1 .. $size->[0] ]
    } 1 .. $size->[1] ];

unless ($no_serial) {
    require Device::SerialPort;
    $wall = Device::SerialPort->new($port) || die "Can't open $port: $!";
    $wall->baudrate(230400);
    
    $SIG{TERM} = sub { trap() };
    $SIG{INT} = sub { trap() };
    $SIG{QUIT} = sub { trap() };

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
    
    # my ($count, $raw) = $wall->read(100);
    # print "$raw\n" if $count;

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

                # add very low random chance for each pixel to be born parentless
                if (rand > $life_entropy) {
                    $game->set_point($x, $y);
                    $map[$x][$y] = [ $hue, $saturation, $brightness ];
                    $life_entropy -= 0.00001;
                    $life_entropy = .99999 if $life_entropy < 0.95;
                    # say "\033[2J\033[0;0H" . $life_entropy;    #debug
                }
            }

            my @row = map {
                my $c = Convert::Color::HSV->new(@$_)->as_rgb8;
                join('', map { chr($_) } $c->green, $c->red, $c->blue)
            } @{ $map[$x] };
            $wall->write(join('', $x % 2 ? @row : reverse @row));
        }
        unless ($no_print) {
            print map { $_ ? 'O  ' : '   ' } @line;
            say "|";
        }

        $hue += $hue_change;
    }
    say "---" x $size->[0] unless $no_print;

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

sub trap {
    $wall->write(chr(2) x ($size->[0] * $size->[1] + 1)) unless $no_serial;
    say "EXITING";
    exit;
}
