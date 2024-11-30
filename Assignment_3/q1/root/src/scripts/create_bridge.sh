#!/bin/bash



create_bridge () {
  ip link set lo up
  if ! ip link show $1 &> /dev/null; then
    ip link add name $1 type bridge
    ip link set dev $1 up
  else
    echo "Bridge $1 already exists."
  fi
}

create_bridge $1  # br0

ip link show $1