#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug_print.h"

/**
 * Setup virtual ethernet pair for container
 */
int setup_veth_pair(const char *veth_host, const char *veth_container)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "ip link add %s type veth peer name %s", veth_host, veth_container);
    system(buffer);

    snprintf(buffer, sizeof(buffer), "ip link set %s up", veth_host);
    system(buffer);

    return 0;
}

/**
 * Setup bridge for container networking
 */
int setup_bridge(const char *bridge_name, const char *bridge_ip)
{
    char buffer[1024];

    DEBUG_PRINT("Setting up bridge br0");
    
    // Create bridge if it doesn't exist
    if (system("ip link show br0 2>/dev/null") != 0) {
        if (system("ip link add br0 type bridge") != 0) {
            ERROR_PRINT("Failed to create bridge");
            return -1;
        }
    }
    
    // Enable bridge
    if (system("ip link set br0 up") != 0) {
        ERROR_PRINT("Failed to enable bridge");
        return -1;
    }

    // Set bridge IP if not already set
    if (system("ip addr show br0 | grep '192.168.0.3' 2>/dev/null") != 0) {
        if (system("ip address add 192.168.0.3/24 dev br0") != 0) {
            ERROR_PRINT("Failed to set bridge IP");
            return -1;
        }
    }

    INFO_PRINT("Bridge setup completed successfully");

    return 0;
}

/**
 * Setup NAT for internet access
 * call before container (in host)
 */
int setup_nat()
{
    char buffer[1024];
    system("echo 1 > /proc/sys/net/ipv4/ip_forward");

    snprintf(buffer, sizeof(buffer), "iptables -t nat -A POSTROUTING -s 172.17.0.0/16 -j MASQUERADE");
    return 0;
}

/**
 * Configure container network
 */
int setup_container_network(const char *veth_container,
                            const char *container_ip,
                            const char *container_netmask)
{
    char buffer[1024];
    // snprintf(buffer, sizeof(buffer), "ip link set %s up", veth_container);

    snprintf(buffer, sizeof(buffer), "ip addr add %s/%s dev %s", container_ip, container_netmask, veth_container);
}

int initialize_networking(network_config_t * config)
{
    setup_veth_pair(config->veth_host, config->veth_container);
    setup_bridge(config->bridge_name, config->bridge_ip);
    setup_nat();
}


#endif