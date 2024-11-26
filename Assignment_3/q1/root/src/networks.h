#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug_print.h"


void rand_char(char *str,int size)
{
	char new[size];
	for(int i=0;i<size;i++){
		new[i] = 'A' + (rand() % 26);
	}
	new[size] = '\0';
	strncpy(str,new,size);
	return;
}


/**     
 * Setup virtual ethernet pair for container
 */
int setup_veth_pair(network_config_t * conf)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "ip link add %s type veth peer name %s", conf->veth_bridge_end, conf->veth_container_end);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer),"ip link set %s netns %d", conf->veth_container_end, conf->pid);
    ok(system, buffer);

    return 0;
}

/**
 * Setup bridge for container networking // will be called on host
 */
int setup_bridge(network_config_t * conf)
{
    char buffer[1024];

    DEBUG_PRINT("Setting up bridge %s", conf->bridge_name);
    
    // Create bridge if it doesn't exist
    snprintf(buffer, sizeof(buffer), "ip link show %s 2>/dev/null", conf->bridge_name);
    if (system(buffer) != 0) {
        snprintf(buffer, sizeof(buffer), "ip link add %s type bridge", conf->bridge_name);
        ok(system, buffer);
    }
    
    // Enable bridge
    snprintf(buffer, sizeof(buffer), "ip link set %s up", conf->bridge_name);
    ok(system, buffer);

    
    snprintf(buffer, sizeof(buffer), "ip address add %s/24 dev %s", conf->bridge_ip, conf->bridge_name);
    ok(system, buffer);
    
    // snprintf(buffer, sizeof(buffer), "sudo ip link set %s master %s", conf->veth_bridge_end, conf->bridge_name);
    // ok(system, buffer);

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
    ok(system, buffer);
    return 0;
}

/**
 * Configure container network // called in network
 */
int setup_container_network(network_config_t * conf)
{
    char buffer[1024];
    // snprintf(buffer, sizeof(buffer), "ip link set %s up", veth_container_end);
    snprintf(buffer, sizeof(buffer), "ip link set %s up", conf->veth_container_end);
    ok(system, buffer);
    snprintf(buffer, sizeof(buffer), "ip addr add %s/24 dev %s", conf->container_ip, conf->veth_container_end);
    ok(system, buffer);
    snprintf(buffer, sizeof(buffer), "ip route add default via %s", conf->bridge_ip);
    ok(system, buffer);

    ok(system, "ip link set lo up");
}

int initialize_networking(network_config_t * config)
{
    setup_bridge(config);
    setup_veth_pair(config);
    setup_nat();
}


#endif