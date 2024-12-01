#!/bin/bash


create_pair_ns () {
  echo "create pair ns "$1 $2 $3 $4 $5 $6 $7 $8
  if ! ip link show $2 &> /dev/null; then
    if [ "$7" = "1" ]; then
      echo "host :)"
      ip link add name $1 type veth peer name $2
      ip link set $2 master $4
      ip link set dev $2 up
    
      ip link set $1 netns $5
    else
      echo "container :("
      ip addr add $3 brd + dev $1
      ip link set $1 address $6
      ip link set dev $1 up
      ip link set lo up  # Bring up loopback interface
      ip route add default via ${8%/*}
    fi
    
  else
    echo "Veth pair $1 <--> $2 already exists in namespace $5."
  fi
}


# Create veth pairs in namespaces h2, h3, and lb
# $1 : veth pair to keep in pc end
# $2 : veth pair to keep in bridge end
# $3 : ip of container 
# $4 : bridge_name
# $5 : pid (ns)
# $6 : MAC last two bits
# $7 : is host
# $8 : ip of bridge
create_pair_ns $1 $2 $3 $4 $5 $6 $7 $8
# create_pair_ns veth2 veth3 "10.0.0.2/24" br0 h2 "MAC" 1