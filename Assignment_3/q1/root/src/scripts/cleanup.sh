#!/bin/bash

set -xe

rm_bridge () {
  if ip link show $1 &> /dev/null; then
    ip link set dev $1 down
    ip link delete $1 type bridge
  fi
}

rm_pair () {
  if ip link show $1 &> /dev/null; then
    ip link delete $1 type veth
  fi
}

rm_ns () {
  if ip netns list | grep -w "$1" &> /dev/null; then
    ip netns delete $1
  fi
}


# $1 bridge_name
# $2 veth bridge <-> pc pair in bridge end 
# $3 veth contianer <-> bridge pair in container end 
# $4 pid
sudo sysctl -w net.ipv4.ip_forward=0

# Remove bridge br0
rm_bridge $1

# Remove veth pairs
rm_pair $2
rm_pair $3

# Remove namespaces
rm_ns $4
