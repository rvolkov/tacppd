#!/bin/sh

#
# !!! uncomment appropriate method !!!
#

#
# you need set root directory for your tacppd installation
#
TACPPD_DIR=/home/cisco/tacppd

# if you use tacppd with Perl support, you can have need in next string,
# if you use shared libperl
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`perl -MConfig -e 'print $Config{archlib}'`/CORE
export LD_LIBRARY_PATH

# if you use Oracle database, set ORACLE_BASE/ORACLE_HOME variables
#ORACLE_BASE=/export/home/oracle
#export ORACLE_BASE
#ORACLE_HOME=$ORACLE_BASE/product/9.2.0
#export ORACLE_HOME
#LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$ORACLE_HOME/lib"
#export LD_LIBRARY_PATH

# 1. via environment variable in daemon mode
#export TACPPD_DIR
#${TACPPD_DIR}/tacppd

# 2. via environment variable in foreground mode
#export TACPPD_DIR
#${TACPPD_DIR}/tacppd -f

# 3. via command line in daemon mode
${TACPPD_DIR}/build/tacppd -d ${TACPPD_DIR}

# 4. via command line in foreground mode
#${TACPPD_DIR}/tacppd -f -d ${TACPPD_DIR}

# 5. via command line in foreground mode with valgrind memory debugger
#valgrind ${TACPPD_DIR}/tacppd -f -d ${TACPPD_DIR}
