#!/usr/bin/env perl

use common::sense;
use Device::SerialPort;
use Scalar::Util 'looks_like_number';

my $setting = shift @ARGV;
my $value = shift @ARGV;

my %command = (
    brightness => 3,
    speed => 4,
    trail_length => 5,
    hue_speed => 6,
    density => 7,
    restart => 8,
    stats => 9,
);

die unless exists $command{$setting};

my $w = Device::SerialPort->new("/dev/tty.usbmodem3781501");
$w->baudrate(230400);
if ($setting =~ m/^(restart|stats)$/) {
    $w->write(chr($command{$setting}));
}
else {
    die unless looks_like_number($value);
    $w->write(chr($command{$setting}).chr($value));
}
select(undef, undef, undef, 0.2);
my ($c, $r) = $w->read(10000);
say "$c: $r" if $c;
