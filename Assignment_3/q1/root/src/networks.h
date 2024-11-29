#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    snprintf(buffer, sizeof(buffer), "ip link show %s &> /dev/null;", conf->veth_bridge_pb_end);

    if (system(buffer) == 0)
    {
        DEBUG_PRINT("Veth pair Bridge and pc already exists");
        snprintf(buffer, sizeof(buffer), "ip link delete %s type veth", conf->veth_bridge_pb_end);
        printf("%s\n\r", buffer);
        ok(system, buffer);
    }

    snprintf(buffer, sizeof(buffer), "ip link add name %s type veth peer name %s", conf->veth_bridge_pb_end, conf->veth_pc_pb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set %s address %s", conf->veth_bridge_pb_end, conf->bridge_mac);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip addr add %s brd + dev %s", conf->bridge_ip, conf->veth_bridge_pb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set %s master %s", conf->veth_pc_pb_end, conf->bridge_name);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set dev %s up", conf->veth_bridge_pb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set dev %s up", conf->veth_pc_pb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    return 0;
}

/**
 * Setup virtual ethernet pair for container
 */
int setup_veth_pair_ns(network_config_t *conf)
{
    char buffer[1024];
    printf("\n\r");
    snprintf(buffer, sizeof(buffer), "ip link show %s &> /dev/null;", conf->veth_container_cb_end);
    printf("%s\n\r", buffer);
    if (system(buffer) == 0)
    {
        DEBUG_PRINT("Veth pair already exists");
        snprintf(buffer, sizeof(buffer), "ip link delete %s type veth", conf->veth_container_cb_end);
        printf("%s\n\r", buffer);
        ok(system, buffer);
    }

    snprintf(buffer, sizeof(buffer), "ip link add name %s type veth peer name %s", conf->veth_container_cb_end, conf->veth_bridge_cb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set %s master %s", conf->veth_bridge_cb_end, conf->bridge_name);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set dev %s up", conf->veth_bridge_cb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip link set %s netns %d", conf->veth_container_cb_end, conf->pid);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip netns exec %d ip addr add %s brd + dev %s", conf->pid, conf->container_ip, conf->veth_container_cb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip netns exec %d ip link set %s address %s", conf->pid, conf->veth_container_cb_end, conf->container_mac);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip netns exec %d ip link set dev %s up", conf->pid, conf->veth_container_cb_end);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "ip netns exec %d ip link set lo up", conf->pid);
    printf("%s\n\r", buffer);
    ok(system, buffer);

    return 0;
}

/**
 * Setup bridge for container networking // will be called on host
 */
int setup_bridge(network_config_t *conf)
{
    char buffer[1024];

    DEBUG_PRINT("Setting up bridge %s", conf->bridge_name);

    // Create bridge if it doesn't exist
    snprintf(buffer, sizeof(buffer), "ip link show %s 2>/dev/null", conf->bridge_name);
    if (system(buffer) == 0)
    {
        DEBUG_PRINT("Bridge already exists");
        snprintf(buffer, sizeof(buffer), "ip link set dev %s down", conf->bridge_name);
        printf("%s\n\r", buffer);
        ok(system, buffer);

        snprintf(buffer, sizeof(buffer), "ip link delete %s type bridge", conf->bridge_name);
        printf("%s\n\r", buffer);
        ok(system, buffer);
    }

    snprintf(buffer, sizeof(buffer), "ip link add name %s type bridge", conf->bridge_name);
    ok(system, buffer);

    // Enable bridge
    snprintf(buffer, sizeof(buffer), "ip link set dev %s up", conf->bridge_name);
    ok(system, buffer);

    // snprintf(buffer, sizeof(buffer), "ip address add %s dev %s", conf->bridge_ip, conf->bridge_name);
    // ok(system, buffer);

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
    system("sudo sysctl -w net.ipv4.ip_forward=1");

    snprintf(buffer, sizeof(buffer), "sudo iptables -P FORWARD ACCEPT");
    printf("%s\n\r", buffer);
    ok(system, buffer);

    snprintf(buffer, sizeof(buffer), "iptables -t nat -A POSTROUTING -s 172.17.0.0/16 -j MASQUERADE");
    printf("%s\n\r", buffer);
    ok(system, buffer);
    return 0;
}

int check_connectivity(int pid, char *ip)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "sudo ip netns exec %d ping -c 3 %s", pid, ip);
    printf("%s\n\r", buffer);
    system(buffer);
}

void check_ping(network_config_t *config)
{
    check_connectivity(config->pid, config->container_ip);
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

int initialize_networking(network_config_t *config)
{
    printf("========================");
    setup_bridge(config);
    printf("veth pair\n\r");
    setup_veth_pair(config);
    printf("veth pair ns\n\r");
    setup_veth_pair_ns(config);
    printf("nat \n\r");
    setup_nat();
    // printf("\n\r");
    // check_ping(config);
    printf("========================");
}

#endif