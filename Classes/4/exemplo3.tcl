set ns [new Simulator]

$ns rtproto DV

set nf [open out.nam w]
$ns namtrace-all $nf

proc fim {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam out.nam
  exit 0;
}


for {set i 0} {$i<7} {incr i} {
  set n($i) [$ns node]
}

for {set i 0} {$i<7} {incr i} {
  $ns duplex-link $n($i) $n([expr ($i+1)%7]) 1Mb 10ms DropTail
}

set udp0 [new Agent/UDP]
$ns attach-agent $n(0) $udp0

set cbr0 [new Application/Traffic/CBR]
$cbr0 set packetSize_ 500
$cbr0 set interval_ 0.005
$cbr0 attach-agent $udp0

set null0 [new Agent/Null]
$ns attach-agent $n(3) $null0

$ns connect $udp0 $null0

$ns at 0.5 "$cbr0 start"
$ns at 4.5 "$cbr0 stop"


$ns rtmodel-at 1.0 down $n(1) $n(2)
$ns rtmodel-at 2.0 up $n(1) $n(2)

$ns at 5.0 "fim"

$ns run