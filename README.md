          TACPPD

TACPPD this is Tacacs+ plus daemon (TACacs Plus Plus Daemon).

This is AAA server (authentication,authorization,accounting) for network
devices. The main goal - create distributed AAA environment with
full database support + integration with billing system. Also we try to
do mechanism for real-time user sessions control with the really true
information for it (if you have network with some unstable links,
you can loss accounting packets from network devices).
We are using model "all-in-one" (tacacs+ with telnet server,
with http server, with NetFlow collector, with SNMP poller and other)
for make it later in hardware box with telnet/web control and AAA server
functionality. Tacppd core use C++ and web/billing interface
use Perl. It can be compiled for any *nix system with POSIX threads.
Most information about network devices valid only for Cisco equipment.
For your device, see vendor documentation. Now i am thinking about raise
tacppd to some set of API or may be separate libraries for doing
applications with integrated CLI, http, log and debug. Network applications
which can be easy integrated to hardware linux-based boxes and use
external storage like NAS (network attached storage) devices.
For example, smtp/pop3 server, http server, etc. Now i am thinking for
application-independend core of tacppd, which can be separated from
tacacs+ or any other application code to separated API or library.

For Documentation, see doc/ subdirectory. All installation information you
will find in User Guide.

Also Documentation available by http://tacppd.org

The first lines of tacppd code was initially written by me when i had been
working for Khabarovsk TTS (this is regional long distance telephone company
in the Far East of Russia) in ISP department. The goal was create AAA/billing
system for dial-up access. This project was closed due to lack of resources,
and source code was GPLed. After that from time to time i have working with it.
We have some developers and contributors, but while we have not have timeline.
So if you have needs in something similar, join us. Our community will
help you. Also i will appreciate your ideas about better developing cycle
organization and any information about tacppd using experience.

Also it will be very nice if somebody provide us with access to hardware -
i need a Solaris system for testing and building. Also you can help us with
access to Cisco equipment for testing and continue developing.

With best wishes,
Roman Volkov
