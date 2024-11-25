#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#define NS_1 "red"
#define NS_2 "blue"

void create_container(){
    system("sudo ip netns add container red");
    system("sudo ip netns add container blue");
    system("sudo ip link add br0 type bridge");
    system("sudo ip link set br0 up");
}

#endif 