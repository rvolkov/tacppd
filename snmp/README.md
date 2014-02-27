Common modules:
cisco.so  - Cisco snmp parser
cisco-trf.so - Cisco snmp parser with reading ifInOctets/ifOutOctets
interface information

Specialised modules for special devices with less SNMP queries:
cisco-asy.so - Cisco devices with async ports (2511/2509)
cisco-e1.so - Cisco devices with E1 connections (AS5300/36xx with E1)

I have tested it for 2511/2509 and AS5300 (analog and ISDN users),
(have tested for 120 and 240-ports AS5300).
I think, platforms like 26xx or 2522 with modules or universal
ports can have problems. Hope to fix it soon.
No way to detect/drop users via SNMP on FXO/FXS VoIP ports -
you should use other way - tcl scripts, etc.

none.so - void module


Parsers avaiable in sources

the API documentation for own parser will be available soon
