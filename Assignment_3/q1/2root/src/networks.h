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
    char buffer[2024];

    printf("\n\r");
    system("ls");

    snprintf(buffer, sizeof(buffer),
    "bash src/scripts/create_pair_ns.sh %s %s %s %s %d %s %d %s", conf->veth_container_cb_end, conf->veth_bridge_cb_end, conf->container_ip, conf->bridge_name, conf->pid, conf->container_mac, is_host, conf->bridge_ip);
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
    system("sudo sysctl -w net.ipv4.ip_forward=1 && sudo iptables -P FORWARD ACCEPT");

    // iptables -t nat -A POSTROUTING -s 192.168.0.0/255.255.255.0 -o ens5 -j MASQUERADE
    
    // Add specific NAT rules without disturbing existing ones
    snprintf(buffer, sizeof(buffer), "sudo iptables -t nat -A POSTROUTING -s 10.0.0.2/24 -o wlp2s0 -j MASQUERADE");
    system(buffer);

    snprintf(buffer, sizeof(buffer), "iptables -A FORWARD -i wlp2s0 -o veth0 -j ACCEPT && iptables -A FORWARD -o wlp2s0 -i veth0 -j ACCEPT");
    system(buffer);


    return 0;
}

void cleanup_networking(network_config_t *conf)
{
    char buffer[2024];

    DEBUG_PRINT("Clanup");

    snprintf(buffer, sizeof(buffer),
             "bash src/scripts/cleanup.sh %s %s %s %d", conf->bridge_name, conf->veth_bridge_pb_end, conf->veth_container_cb_end, conf->pid);
    system(buffer);

    INFO_PRINT("Bridge setup completed successfully");
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

int initialize_networking_in_container(network_config_t *conf)
{

    // setup_veth_pair_ns(conf, true);
    printf("========================");
    char buffer[4096];

    setup_veth_pair_ns(conf, false);

    printf("====container : buffer %s==================\n\r", buffer);
}

#endif