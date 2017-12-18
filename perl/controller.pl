#!/usr/bin/env perl

use common::sense;
use Mojolicious::Lite;
use File::Path 'make_path';
use Device::SerialPort;
use Scalar::Util 'looks_like_number';

my $LOG_DIR = '/tmp/LEDwall';
my $PORT    = '/dev/ttyACM0';
make_path($LOG_DIR) unless -d $LOG_DIR;

my $wall = Device::SerialPort->new($PORT) || die "Can't open $PORT: $!";
$wall->baudrate(230400);

my %settings = (
    brightness   => 3,
    speed        => 4,
    trail_length => 5,
    hue          => 6,
    hue_speed    => 7,
    density      => 8,
    restart      => 9,
    stats        => 10,
);

# routes
get '/' => sub {
    my $c = shift;
    $c->redirect_to('index.html');
};

get '/set/:setting' => sub {
    my $c       = shift;
    my $setting = $c->stash->{setting};
    my $value   = $c->req->param('value');

    say "$setting: $value";

    if (defined $value) {
        $wall->write(chr($settings{$setting}) . chr($value));
    }
    else {
        $wall->write(chr($settings{$setting}));
    }
    select(undef,undef,undef,0.3);
    my ($count, $raw) = $wall->read(10000);
    $c->render(text => $raw);
};

# set custom secret
app->secrets(['hb3u46rt34878t48bo3fqiu34fvq83498fuv5940yuvto34ty7']);

# use Time::HiRes version of time() for Mojo::Log'ing
app->log->format(
    sub { shift; '[' . time . '] [' . shift() . '] ' . join "\n", @_, ''; });
app->log->path("$LOG_DIR/mojo.log");

# serve static files
app->static->paths(['./public']);

app->start;
