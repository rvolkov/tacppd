pgsql.so	- PostgreSQL module
mysql.so	- MySQL module
msql.so		- miniSQL module
none.so		- void module

in development (doesn't ready):
oci.cc		- Oracle Call Interface (libclntst8.a) (Oracle8i)
occi.cc		- Oracle C++ Call Interface (Oracle 9i)
odbc.cc		- ODBC
xml-db.cc	- XML plain file db see README.xml-db for more details

you can compile module again with command like:

g++ -shared -O2 -DMODULE -DDB_PGSQL -o pgsql.so pgsql.cc -I/usr/local/pgsql/include -L/usr/local/pgsql/lib -lpq -lc
(on SOLARIS add -DSOLARIS)


The documentation how to create own database drivers will be available soon


For modules testing better to use foreground tacppd mode (see START.sh),
and use printf() to output error messages to console
