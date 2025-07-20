# ========================================================================
#				CENÁRIO 2
# ========================================================================

set ns [new Simulator]

$ns color 1 Blue
$ns color 2 Red

$ns rtproto DV

set nf [open ex2_2.nam w]
$ns namtrace-all $nf

proc end {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam ex2_2.nam
  exit 0;
}


# ==================== Set computer nodes ====================
set nC1 [$ns node]
set nC2 [$ns node]
set nC3 [$ns node]
set nC4 [$ns node]


# ==================== Set router nodes ====================
set nR1 [$ns node]
set nR2 [$ns node]
set nR3 [$ns node]
set nR4 [$ns node]
set nR5 [$ns node]


# ==================== Set connections ====================
# Computador 1 ---> Router 1
$ns duplex-link $nC1 $nR1 80Mb 10ms DropTail
$ns duplex-link-op $nC1 $nR1 orient right-down
# Computador 2 ---> Router 1
$ns duplex-link $nC2 $nR1 0.4Gb 10ms DropTail
$ns duplex-link-op $nC2 $nR1 orient right-up
# Router 1 ---> Router 2
$ns duplex-link $nR1 $nR2 200Mb 10ms DropTail
$ns duplex-link-op $nR1 $nR2 orient left-right
# Router 2 ---> Router 3
$ns duplex-link $nR2 $nR3 1Gb 10ms DropTail
$ns duplex-link-op $nR2 $nR3 orient left-right
# Router 1 ---> Router 4
$ns duplex-link $nR1 $nR4 300Mb 10ms DropTail
$ns duplex-link-op $nR1 $nR4 orient right-down
# Router 4 ---> Router 5
$ns duplex-link $nR4 $nR5 400Mb 10ms DropTail
$ns duplex-link-op $nR4 $nR5 orient left-right
# Router 5 ---> Router 3
$ns duplex-link $nR5 $nR3 200Mb 10ms DropTail
$ns duplex-link-op $nR5 $nR3 orient right-up
# Router 3 ---> Computer 3
$ns duplex-link $nR3 $nC3 80Mb 3ms DropTail
$ns duplex-link-op $nR3 $nC3 orient right-up
# Router 3 ---> Computer 4
$ns duplex-link $nR3 $nC4 40Mb 10ms DropTail
$ns duplex-link-op $nR3 $nC4 orient right-down


# ==================== Set connections ====================
# Send 200KB from Computer 1 ---> Computer 4 using TCP socket 
# Set tcp user in Computer 1
set tcp1 [new Agent/TCP]
$ns attach-agent $nC1 $tcp1
$tcp1 set window_ 20
$tcp1 set class_ 1
# Set tcp sink in Computer 4
set sink4 [new Agent/TCPSink]
$ns attach-agent $nC4 $sink4
# Set CBR traffic from Computer 1 ---> Computer 4
set cbr14 [new Application/Traffic/CBR]
$cbr14 set packetSize_ 204800
$cbr14 set interval_ 0.005
$cbr14 set maxpkts_ 1
$cbr14 attach-agent $tcp1
# Connect cbr to sink
$ns connect $tcp1 $sink4

# Send 3Mb/s from Computer 2 ---> Computer 3 using UDP socket 
# Set udp user in Computer 2
set udp2 [new Agent/UDP]
$ns attach-agent $nC2 $udp2
$udp2 set class_ 2
# Set udp null in Computer 3
set null3 [new Agent/Null]
$ns attach-agent $nC3 $null3
# Set CBR traffic from Computer 2 ---> Computer 3
set cbr23 [new Application/Traffic/CBR]
#$cbr23 set packetSize_ 500
#$cbr23 set interval_ 0.005 
$cbr23 set rate_ 3Mb
$cbr23 attach-agent $udp2
# Connect cbr to null
$ns connect $udp2 $null3


$ns queue-limit $nR1 $nR4 800
$ns queue-limit $nR1 $nR2 800

# ==================== Set schedule ====================
$ns at 0.5 "$cbr23 start"
$ns at 0.5 "$cbr14 start"

$ns rtmodel-at 0.6 down $nR1 $nR2
$ns rtmodel-at 0.7 up $nR1 $nR2

$ns at 1.5 "end"


$ns run
