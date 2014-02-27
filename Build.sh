#!/bin/sh
mkdir build
cd build
../configure --with-snmp-shared --with-pgsql --with-mysql
make
