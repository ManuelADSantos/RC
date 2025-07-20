set ns [new Simulator]

$ns color 1 Blue
$ns color 2 Yellow
$ns color 3 Green

set nf [open ex1.nam w]
$ns namtrace-all $nf

proc sim {} {
  global ns nf
  $ns flush-trace
  close $nf
  exec nam ex1.nam
  exit 0;
}

set n0 [$ns node]
set nA [$ns node]
set nB [$ns node]
set nC [$ns node]

$ns duplex-link $n0 $nA 1.2Gb 3ms SFQ
$ns duplex-link $n0 $nB 332Mb 0.2ms SFQ
$ns duplex-link $n0 $nC 1Gb 133ms SFQ


$ns duplex-link-op $n0 $nA orient right-up
$ns duplex-link-op $n0 $nB orient right
$ns duplex-link-op $n0 $nC orient right-down


#Cria um agente UDP e liga-o ao nó n0 (Máquina A)
set udpA [new Agent/UDP]
$ns attach-agent $n0 $udpA
#Cria uma fonte de tráfego CBR e liga-a ao udpA
set cbrA [new Application/Traffic/CBR]
$cbrA set packetSize_ 10240
$cbrA set interval_ 0.005
$cbrA set maxpkts_ 1
$cbrA attach-agent $udpA
#Cria um agente Null e liga-o ao nó n3
set nullA [new Agent/Null]
$ns attach-agent $nA $nullA


#Cria um agente UDP e liga-o ao nó n0 (Máquina B)
set udpB [new Agent/UDP]
$ns attach-agent $n0 $udpB
#Cria uma fonte de tráfego CBR e liga-a ao udpB
set cbrB [new Application/Traffic/CBR]
$cbrB set packetSize_ 10240
$cbrB set interval_ 0.005
$cbrB set maxpkts_ 1
$cbrB attach-agent $udpB
#Cria um agente Null e liga-o ao nó n3
set nullB [new Agent/Null]
$ns attach-agent $nB $nullB


#Cria um agente UDP e liga-o ao nó n0 (Máquina C)
set udpC [new Agent/UDP]
$ns attach-agent $n0 $udpC
#Cria uma fonte de tráfego CBR e liga-a ao udpC
set cbrC [new Application/Traffic/CBR]
$cbrC set packetSize_ 10240
$cbrC set interval_ 0.005
$cbrC set maxpkts_ 1
$cbrC attach-agent $udpC
#Cria um agente Null e liga-o ao nó n3
set nullC [new Agent/Null]
$ns attach-agent $nC $nullC


$ns connect $udpA $nullA
$ns connect $udpB $nullB
$ns connect $udpC $nullC


$ns at 0.5 "$cbrA start"
$ns at 0.5 "$cbrB start"
$ns at 0.5 "$cbrC start"

#$ns at 4.5 "$cbr0 stop"

$udpA set class_ 1
$udpB set class_ 2
$udpC set class_ 3

$ns at 3.0 "sim"

$ns run
