          TACPPD

TACPPD this is Tacacs+ plus daemon (TACacs Plus Plus Daemon).

This is AAA server (authentication,authorization,accounting) for network
devices. The main goal - create distributed AAA environment with
full database support + integration with billing system and with real-time user sessions control.

THis is "all-in-one" system (tacacs+ with telnet server,
with http server, with NetFlow collector, with SNMP poller and other)
to make it later in hardware box or VM with telnet/web control and AAA server
functionality. Tacppd core use C++ and web/billing interface
uses Perl. It can be compiled for any *nix system with POSIX threads.
Most information about network devices valid only for Cisco equipment.
For your device, see vendor documentation. Now i am thinking about raise

For Documentation, see doc/ subdirectory. All installation information you
will find in User Guide.

The first lines of tacppd code was initially written by me when i had been
working for Khabarovsk TTS (this is regional long distance telephone company
in the Far East of Russia) in ISP department. The goal was create AAA/billing
system for dial-up access. This project was closed due to lack of resources,
and source code was GPLed and later released under BSD-style license.

With best wishes,
Roman Volkov
