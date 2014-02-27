! tacppd configuration file
!  this is example configuration with extra comments and all features
!  enabled, in real configuration you can do not see some features
!  and comments (it depends from compilation keys)
!
! config word must be here
config

 ! set debug
 
 ! output to file
 debug file
  facility tacacs
  facility db
  facility aaa
  exit
 ! output to local syslog server
! debug syslog
!  facility db
!  exit
 ! output to remote syslog server
! debug 10.1.1.1
!  facility aaa
!  facility tacacs
!  exit

 ! access lists
 access 1
  permit ^127.0.0.1$
  deny ^127.0.0
  permit ^192.168.0
  exit
 access 2
  permit ^127.0.0
  permit ^192.168.0
  exit
 access 3
  permit ^127.0.0
  permit ^192.168.0
  exit
 access 4
  permit ^239.1.1
  exit

 ! config managers
 manager admin
  password admin_password
  type terminal
  acl 1
  exit
 manager webmin
  password 'webmin'
  type http
  acl 1
  exit
 manager tcius
  password 'tcipw'
  type tci
  acl 1
  exit

 ! listeners configuration
 listener 2222
  type terminal
  maxconnect 2
  acl 2
  exit
 listener 10000
  type tacacs
  maxconnect 5
  acl 3
  exit
 listener 8888
  type http
  maxconnect 10
  acl 2
  exit
 listener 12001
  type netflow
  maxconnect 10
  acl 2
  exit
 listener 11000
  type bundle
  maxconnect 10
  acl 4
  exit
 listener 11001
  type tci
  maxconnect 10
  acl 1
  exit
 listener 12003
  type peer
  maxconnect 10
  acl 2
  exit
! listener 12004
!  type snmp
!  maxconnect 10
!  acl 2
!  exit

 ! address pool
 pool 1
  addr 10.1.1.1
  addr 10.1.1.2
  addr 10.1.1.3
  exit

 ! databases
 database etc/xml-db.xml@localhost:0
  cryptokey 'dbkey'
  login tacacs
  password Tacppd
  module xml-db.so
  shutdown no
  exit
! database billing@192.168.0.3:5432
!  cryptokey 'dbkey'
!  login tacppduser
!  password tacppdpwd
!  module pgsql.so
!  shutdown no
!  exit

 ! network device
 device 192.168.0.1
  description 'voip'
  tacacskey 'VoiP'
  snmpcommunity 'common'
  loginstring 'login++:'
  pwdstring 'pwd++:'
  defauthorization no
  module cisco.so
  polldelay 5
  inttrfcount no
  snmppolling yes
  icmppolling no
  shutdown no
  exit

 ! tacppd bundle
 bundle 239.1.1.1:11000
  cryptokey none
  priority 1
  ttl 1
  exit

 ! billing modules
 billing 0
  description 'void'
  module none.so
  exit
! billing 1
!  description 'per-time'
!  module sc-per-time.so
!  exit

 ! peers
! peer www1.kht.ru:8001
!  description 'none'
!  cryptokey key1
!  mask ^Pppwww1.*|^4212.*
!  mask ^ppp.*
!  exit
! peer www2.kht.ru:8002
!  description 'none'
!  cryptokey key2
!  mask ^Pppwww2.*|^4213.*
!  mask ^www.*
!  exit

! exit word must be here
exit
! that is all
