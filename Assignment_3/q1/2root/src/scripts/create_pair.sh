#!/bin/bash


# create_pair veth0 veth1 "10.0.0.1/24" br0 01
create_pair () {
    echo "create_pair "$1 $2 $3 $4 $5 $6
  if ! ip link show $1 &> /dev/null; then
    ip link add name $1 type veth peer name $2
    ip link set $1 address "$5"
    ip addr add $3 brd + dev $1
    ip link set $2 master $4
    ip link set dev $1 up
    ip link set dev $2 up
  else
    echo "Veth pair $1 <--> $2 already exists."
  fi
}



# Create veth pairs and assign IPs
# $1 : veth pair to keep in container
# $2 : veth pair to keep in bridge end
# $3 : ip of container 
# $4 : bridge_name
# $5 : MAC
create_pair $1 $2 $3 $4 $5