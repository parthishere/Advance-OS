#!/bin/bash

if [ $1 ]; then
    echo "add namespace name in arg 1"
fi

sudo ip netns add container $1 # do not need this 
sudo ip netns add container $2  # do not need this
sudo ip link add br0 type bridge
sudo ip link set br0 up


sudo ip link add veth-ns1 type veth peer name veth-ns1-br
sudo ip link add veth-ns2 type veth peer name veth-ns2-br

sudo ip link set veth-ns1 netns $1
sudo ip link set veth-ns2 netns $2


sudo ip link set veth-ns1-br master br0


# Up the loopback interfaces
sudo ip netns exec $1 ip link set lo up
sudo ip netns exec $2 ip link set lo up

sudo ip netns exec $1 ip link set veth-ns1 up
sudo ip netns exec $2 ip link set veth-ns2 up

sudo ip link set veth-ns1-br up
sudo ip link set veth-ns2-br up


# Assign ip address
sudo ip netns exec $1 ip addr add 192.168.0.1/24 dev veth-ns1
sudo ip netns exec $2 ip addr add 192.168.0.2/24 dev veth-ns2

# Assign an ip to the bridge
sudo ip address add 192.168.0.3/24 dev br0


# Set default routes
sudo ip netns exec $1 ip route add default via 192.168.0.3
sudo ip netns exec $2 ip route add default via 192.168.0.3


# Create firewall rules
sudo iptables --append FORWARD --in-interface v-net-0 --jump ACCEPT
sudo iptables --append FORWARD --out-interface v-net-0 --jump ACCEPT

