set ns [new Simulator]

$ns color 1 Blue
$ns color 2 Yellow

set nf [open 2.nam w]
$ns namtrace-all $nf

proc sim {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam 2.nam
  exit 0;
}

set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]
set n5 [$ns node]
$ns duplex-link $n0 $n2 1Mb 10ms SFQ
$ns duplex-link $n1 $n2 1Mb 10ms SFQ
$ns duplex-link $n3 $n2 1Mb 10ms SFQ
$ns duplex-link $n2 $n5 1Mb 10ms SFQ
$ns duplex-link $n4 $n5 1Mb 10ms SFQ

$ns duplex-link-op $n0 $n2 orient right-down
$ns duplex-link-op $n1 $n2 orient left-down
$ns duplex-link-op $n3 $n2 orient left-up
$ns duplex-link-op $n2 $n5 orient down
$ns duplex-link-op $n4 $n5 orient up


#Cria um agente UDP e liga-o ao nó n0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

#Cria uma fonte de tráfego CBR e liga-a ao udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

#Cria um agente UDP e liga-o ao nó n1
set udp1 [new Agent/UDP]
$ns attach-agent $n1 $udp1

#Cria uma fonte de tráfego CBR e liga-a ao udp1
set cbr1 [new Application/Traffic/CBR]
$cbr1 set packetSize_ 500
$cbr1 set interval_ 0.005
$cbr1 attach-agent $udp1

#Cria um agente Null e liga-o ao nó n3
set null0 [new Agent/Null]
$ns attach-agent $n3 $null0

#Cria um agente Null e liga-o ao nó n4
set null1 [new Agent/Null]
$ns attach-agent $n4 $null1

$ns connect $udp0 $null0
$ns connect $udp1 $null1

$ns at 0.5 "$cbr0 start"
$ns at 1.0 "$cbr1 start"
$ns at 4.0 "$cbr1 stop"
$ns at 4.5 "$cbr0 stop"

$udp0 set class_ 1
$udp1 set class_ 2

$ns at 5.0 "sim"

$ns run
