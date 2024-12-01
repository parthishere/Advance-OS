#!/bin/bash


# Helper function for error exit on ping failure
function ping_or_fail() {
  if ! sudo ip netns exec $1 ping -c 3 $2; then
    echo "Ping from $1 to $2 failed!"
    exit 1
  fi
}

# Ping test with failure checks
function check_connectivity() {
  echo "Testing connectivity between namespaces and Load Balancer..."

  # Ping from h2 to h3 and h3 to h2
  ping_or_fail h2 10.0.0.3
  ping_or_fail h3 10.0.0.2

  # Ping from h2 to Load Balancer and h3 to Load Balancer
  ping_or_fail h2 10.0.0.10
  ping_or_fail h3 10.0.0.10

  # Ping from Load Balancer to h2 and h3
  ping_or_fail lb 10.0.0.2
  ping_or_fail lb 10.0.0.3

  # Ping from Local Machine to Load Balancer
  ping -c 3 10.0.0.10 || { echo "Ping from Local Machine to Load Balancer failed!"; exit 1; }

  echo "All ping tests passed!"
}

# Debugging helper functions

# Check if all interfaces are up and running
check_interfaces () {
  for ns in h2 h3 lb; do
    echo "Checking interfaces in namespace $ns..."
    sudo ip netns exec $ns ip addr show
    sudo ip netns exec $ns ip link show
  done

  echo "Checking bridge br0..."
  ip addr show br0
  ip link show br0
}

# Check IP forwarding settings
check_ip_forwarding () {
  echo "Checking IP forwarding status on the host..."
  sudo sysctl net.ipv4.ip_forward

  echo "Checking IP forwarding status in namespace $ns..."
  sudo ip netns exec $ns sysctl net.ipv4.ip_forward
}

# Check ARP table
check_arp_table () {
  echo "Checking ARP table on the host..."
  arp -n

  for ns in h2 h3 lb; do
    echo "Checking ARP table in namespace $ns..."
    sudo ip netns exec $ns ip neigh show
  done
}

# Check routing tables
check_routing_table () {
  echo "Checking routing table on the host..."
  ip route show

  for ns in h2 h3 lb; do
    echo "Checking routing table in namespace $ns..."
    sudo ip netns exec $ns ip route show
  done
}

# Check if firewall rules are blocking traffic
check_firewall_rules () {
  echo "Checking firewall rules on the host..."
  sudo iptables -L
}

# Run checks to verify the network
check_interfaces
check_ip_forwarding
check_arp_table
check_routing_table
check_firewall_rules
check_connectivity

echo "Setup and checks completed!"