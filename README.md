oscr
====

oscr is a simple local router for Open Sound Control (OSC) [1].
It allows multiple hosts on the same machine to receive
messages from OSC clients via a unified port.

Usage
-----

oscr reads routing information from a config file. The
syntax of the config file is very simple:

    # comments
    path -> port
    # or alternatively
    port <- path

For example,

    # example config
    /1/push1 -> 10001
    /1/push2 -> 10002

This config above routes messages from `/1/push1` to
`localhost:10001` and messages from `/1/push2` to
`localhost:10002`.

You can also use wildcard patterns, for example,

    /1/* -> 10001
    /2/* -> 10002

but it requires a cutting-edge version of liblo. See *Compile
liblo* section below for details.

After setting up a config file, you can start the router on
port 10000 with the default UDP protocol by executing

    $ oscr -p10000 config

or alternatively with TCP protocol

    $ oscr -t -p10000 config

You can also use UNIX domain sockets with the `-U` option

    $ oscr -U/tmp/osc.sock config

After the router is running, any messages sent to the
designated port or socket (in this case 10000 or
`/tmp/osc.sock`) will be dispatched to corresponding hosts
specified in the config.

See also `oscr(1)`.

Build
-----

oscr currently depends on liblo [2]. I might remove the
dependency eventually, but for the time being I see no
motivation to do so.

For Arch-based distros, it can be installed by

    $ pacman -S liblo

For Debian-based distros, install by

    $ apt-get install liblo-dev

or alternatively you can compile from source.

You also need a C99-compatible C compiler and POSIX make.
Run

    $ make

to build oscr and

    # make install

to install it on your system.

Compile liblo
-------------

I sent a patch to liblo a while ago to fix its wildcard
pattern support. Although it has been merged in to the main
branch, there hasn't been any new releases.

Therefore, if you want to use the wildcard pattern support,
you need to compile liblo from source.

First clone the repository:

    $ git clone https://github.com/radarsat1/liblo.git
    $ cd liblo

Then build and install it to your system:

    $ ./autogen.sh
    # make install

Finally, follow the instruction in previous section to
rebuild oscr.


[1]: http://opensoundcontrol.org/
[2]: http://liblo.sourceforge.net/
