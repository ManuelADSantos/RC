set ns [new Simulator]

set nf [open 3.nam w]
$ns namtrace-all $nf

proc sim {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam 3.nam
  exit 0;
}

set n0 [$ns node]
set n1 [$ns node]

$ns duplex-link $n0 $n1 4Mb 12.048ms SFQ

#Cria um agente UDP e liga-o ao nó n0
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0

#Cria uma fonte de tráfego CBR e liga-a ao udp0
set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 1024
$cbr0 set interval_ 0.005
$cbr0 set maxpkts_ 1
$cbr0 attach-agent $udp0

#Cria um agente Null e liga-o ao nó n3
set null0 [new Agent/Null]
$ns attach-agent $n1 $null0

$ns connect $udp0 $null0

$ns at 0 "$cbr0 start"

$ns at 2.0 "sim"

$ns run
