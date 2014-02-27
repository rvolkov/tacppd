# Script Locked by: rv
# Script Version: 0.0.2
# Script Name: debitcard
# Script Lock Date: Fri Aug  2 16:34:50 2002
#         

proc do_get_account {} {
    global state
    global account

    set prompt(url) tftp://tftpserver/VoIP/enter_account.au
    set prompt(interrupt) true
    set prompt(abortKey) *
#    set prompt(terminationKey) #
#    set patterns(account) .+
    set patterns(account) ...
    set event [promptAndCollect prompt info patterns ]

    if {$event == "collect success"} {
        set state get_pin
        set account $info(digits)
        return 0
    }     
    if {$event == "collect aborted"} {
        set state get_account
        return 0
    }     
    if {$event == "collect fail"} {
        set state get_account
        return 0    
    }
    if {$event == "collect timeout"} {
        set state get_account
        return 0
    }     
    set state end
    return 0
}         

proc do_get_pin {} {
    global state
    global pin

    set prompt(url) tftp://tftpserver/VoIP/enter_pin.au
    set prompt(interrupt) true
    set prompt(abortKey) *
#    set prompt(terminationKey) #
#    set patterns(account) .+
    set patterns(account) ....
    set event [promptAndCollect prompt info patterns ]

    if {$event == "collect success"} {
        set state get_dest
#        set state authenticate
        set pin $info(digits)
        return 0
    }     

    if {$event == "collect aborted"} {
        set state get_account
        return 0
    }     

    if {$event == "collect fail" || $event == "collect timeout"} {
        # timeout
        if {$info(code) == 102} {
            set state get_pin
            return 0
        } 

        # invalid number
        if {$info(code) == 28} {
            set state get_pin
            return 0
        } 
    }     

    set state end
    return 0
}

#
# authenticate throught AAA engine
#
proc do_authenticate {} {
    global state
    global pin
    global account

    set event [authenticate $account $pin info]
    puts "AUTHENTICATION RESULT: event=$event acc=$account pin=$pin"

    if { $event == "authenticated" } {
        set state authorize
        return 0
    }

#    if { [info exists info(roam)] && ![string compare $info(roam) "roam"]} {
#        set state authen_pass
#        return 0
#    }

    if {$event == "authentication failed"} {
        set state authen_fail
        return 0
    }
    set state end
    return 0
}

#
# collect destination number
#
proc do_get_dest {} {
    global state
    global destination

    set dnislen [string len [dnis]]

    set prompt(url) tftp://tftpserver/VoIP/enter_destination.au
    set prompt(interrupt) true
    set prompt(abortKey) *
    set prompt(terminationKey) #
    set prompt(dialPlan) true

    set event [promptAndCollect prompt info]

    puts "POINT-2A event=$event"

    if {$event == "collect success"} {
        set destination $info(digits)
	puts "POINT-2A dest=$destination"
#	set state place_call
        set state authorize
#        set state authenticate
        return 0
    }
    if {$event == "collect aborted"} {
        set state get_dest
        return 0
    }
    if {$event == "collect fail"} {
        set state get_dest
        return 0
    }    
    if {$event == "collect timeout"} {
        set state get_dest
        return 0
    }
    set state end
    return 0
}

#
# authorize this connection by AAA engine
#
proc do_authorize {} {
    global state
    global pin
    global account
    global destination
    global credittime

    #set event [authorize $account $pin $destination [ani] info ]

    #set av-send(h323-ivr-out,0) "dest:$destination"
    #set av-send(h323-ivr-out,1) "proto:h323"
    set event [authorize $account $pin $destination [ani] av-send info]

    puts "TCL-AUTHOR: evt=$event act=$account pin=$pin dst=$destination"

    if {$event == "authorized"} {
	#set num [getVariable aaa h323-credit-amount CreditAmount]
	#set credittime [getVariable aaa h323-credit-time CreditTime]
	set credittime [getVariable aaa h323-credit-time info]
	set event [creditTimeLeft]
	puts "TCL-AUTHOR: CreditTimeLeft = $event sec, credittime = $credittime sec"

	if {$event == 0 || $event == "uninitialized"} {
	    set state out_of_time
	    return 0
	}
	set state place_call
	return 0
    }
    set state authen_fail
    return 0
}

#proc do_authen_pass {} {
#    global state
#    global destination
#
#    set dnislen [string len [dnis]]
#
#    if { [did] && $dnislen } {
#        set destination [dnis]
#        set state place_call
#    } else {
#        set state get_dest
#    }     
#    return 0
#}

proc do_place_call {} {
    global state
    global destination

    set event [placeCall $destination callInfo info ]

    puts "do_place_call event=$event"

    if {$event == "active"} {
        set state active
        return 0
    }
    if {$event == "call fail"} {
        set state place_fail
        return 0
    }
    set state end
    return 0
}

proc do_active_notimer {} {
    global state

    set event [waitEvent]
    while { $event == "digit" } {
        set event [waitEvent]
    }
    set state end
    return 0
}

proc do_active_last_timer {} {
    global state

    # beep if only one minute has
    insertMessage tftp://tftpserver/VoIP/beep.au retInfo

    set event [startTimer [creditTimeLeft] info ]
    while { $event == "digit" } {
        set event [startTimer $info(timeLeft) info ]
    }
    if { $event == "timeout" } {
        clearOutgoingLeg retInfo
        set state out_of_time
    } else {
        set state end
    }
    return 0
}

proc do_active_timer {} {
    global state

    if { [creditTimeLeft] < 60 } {
	do_active_last_timer
        return 0
    }
    set delay [expr [creditTimeLeft] - 60]
    set event [startTimer $delay info]
    while { $event == "digit" } {
    	set event [startTimer $info(timeLeft) info ]
    }
    if { $event == "timeout" } {
    	do_active_last_timer
    } else {
    	set state end
    }
    return 0
}

proc do_active {} {
    global state

    if { ( [creditTimeLeft] == "unlimited") || 
         ([creditTimeLeft] == "uninitialized") } {
             do_active_notimer
    } else {
             do_active_timer
    }     
    return 0
}

proc do_out_of_time {} {
    global state

    set prompt(url) tftp://tftpserver/VoIP/out_of_time.au
    set prompt(playComplete) true
    set event [promptAndCollect prompt info ]
    set state end
    return 0
}

proc do_authen_fail {} {
    global state

    set prompt(url) tftp://tftpserver/VoIP/auth_failed.au
    set prompt(interrupt) true
#    set prompt(playComplete) true
#    set event [promptAndCollect prompt info ]
    set state end
    return 0
}

proc do_place_fail {} {
    global state

    set prompt(url) tftp://tftpserver/VoIP/dest_failed.au
    set prompt(interrupt) true
#    set prompt(playComplete) true
#    set event [promptAndCollect prompt info ]
#    playFailureTone 5 retInfo
    set state end
    return 0
}


#---------------------------------------
# And here is the main loop
#         

acceptCall

#set event [authenticate [ani] [dnis] retInfo]

#if {$event != "authenticated"} {
#  if { [info exists retInfo(roam)] && ![string compare $retInfo(roam) "roam"]} {
#        set state authen_pass
#    } else {
#        set state get_account
#    }
#} else {  
#    set state authen_pass
#}

set state get_account

while {$state != "end"} {
    puts "cid([callID]) running state $state"
    if {$state == "get_account"} {
        do_get_account
    } elseif {$state == "get_pin"} {
        do_get_pin
    } elseif {$state == "authenticate"} {
        do_authenticate
    } elseif {$state == "get_dest"} {
        do_get_dest
    } elseif {$state == "authorize"} {
	do_authorize
    } elseif {$state == "place_call"} {
        do_place_call
    } elseif {$state == "active"} {
        do_active
    } elseif {$state == "authen_fail" } {
        do_authen_fail
#    } elseif {$state == "authen_pass" } {
#        do_authen_pass
    } elseif {$state == "place_fail"} {
        do_place_fail
    } elseif {$state == "out_of_time"} {
        do_out_of_time
    } else {
        break
    }
}
# Script Approval Signature: C/616b
