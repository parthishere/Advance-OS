#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "debug_print.h"

void rand_char(char *str, int size)
{
    char new[size];
    for (int i = 0; i < size; i++)
    {
        new[i] = 'A' + (rand() % 26);
    }
    new[size] = '\0';
    strncpy(str, new, size);
    return;
}

/**
 * Setup virtual ethernet pair for container
 */
int setup_veth_pair(network_config_t *conf)
{
    char buffer[1024];

    printf("\n\r");
    
    snprintf(buffer, sizeof(buffer), 
    "bash src/scripts/create_pair.sh %s %s %s %s %s", conf->veth_bridge_pb_end, conf->veth_pc_pb_end, conf->bridge_ip, conf->bridge_name, conf->bridge_mac);
    system(buffer);

    system("sudo ip link list");

    return 0;
}

/**
 * Setup virtual ethernet pair for container and bridge
 */
int setup_veth_pair_ns(network_config_t *conf, bool is_host)
{
    char buffer[1024];

    printf("\n\r");
    
    snprintf(buffer, sizeof(buffer), 
    "bash src/scripts/create_pair_ns.sh %s %s %s %s %d %s %d", conf->veth_container_cb_end, conf->veth_bridge_cb_end, conf->container_ip, conf->bridge_name, conf->pid, conf->container_mac, is_host);
    system(buffer);

    system("ip link list");

    return 0;
}

/**
 * Setup bridge for container networking // will be called on host
 */
int setup_bridge(network_config_t *conf)
{
    char buffer[2024];

    DEBUG_PRINT("Setting up bridge %s", conf->bridge_name);

    snprintf(buffer, sizeof(buffer), 
    "bash src/scripts/create_bridge.sh %s", conf->bridge_name);
    system(buffer);

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
    system(
    "sudo sysctl -w net.ipv4.ip_forward=1 && \
    iptables -P FORWARD ACCEPT && \
    iptables -t nat -A POSTROUTING -s 172.17.0.0/16 -j MASQUERADE"
    );

    return 0;
}


void cleanup_networking(network_config_t *config)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "ip link set dev %s down", config->bridge_name);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link delete %s type bridge", config->bridge_name);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link delete %s type veth", config->veth_container_cb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link delete %s type veth", config->veth_bridge_pb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    
}

int initialize_networking_in_host(network_config_t *config)
{
    printf("========================");
    setup_bridge(config);
    printf("veth pair\n\r");
    setup_veth_pair(config);
    printf("veth pair ns\n\r");
    setup_veth_pair_ns(config, true);
    printf("nat \n\r");
    setup_nat();
    printf("========================");
}

int initialize_networking_in_container(network_config_t *config)
{
    printf("========================");
    setup_veth_pair_ns(config, false);
    printf("========================");
}


#endif