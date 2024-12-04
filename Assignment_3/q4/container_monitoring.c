// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>

#include <string.h>
#include <stdlib.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include "container_monitoring.skel.h"




static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}


// // Callback function to handle events from the ring buffer
// static int handle_event(void *ctx, void *data, size_t data_sz){
	

// 	return 0;
// }

int main(int argc, char **argv)
{
	struct container_monitoring_bpf *skel;
    int err;
    struct stat sb;

    if(argc != 2){
        printf("you messed up, command should be ./container_monitoring <continer-host:pid> <continer-child:pid> \n");
        return 0;
    }
    else{
        printf("%s\n", argv[1]);
    }
    

	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Open BPF application */
	skel = container_monitoring_bpf__open();
	if (!skel)
	{
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}

    
    

	// /* ensure BPF program only handles write() syscalls from our process */
	skel->bss->dev = sb.st_dev;
	skel->bss->ino = sb.st_ino;

	/* Load & verify BPF programs */
	err = container_monitoring_bpf__load(skel);
	if (err)
	{
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* Attach tracepoint handler */
	err = container_monitoring_bpf__attach(skel);
	if (err)
	{
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}


	

	// rb = ring_buffer__new(bpf_map__fd(skel->maps.data_map), handle_event, NULL, NULL);
	// if (!rb)
	// {
	// 	fprintf(stderr, "Failed to create ring buffer\n");
	// 	err = -1;
	// 	goto cleanup;
	// }

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
		   "to see output of the BPF programs.\n");

	for (;;)
	{
		
		/* trigger our BPF program */
		// fprintf(stderr, ".");
		// sleep(1);
	}

cleanup:
    container_monitoring_bpf__detach(skel);
	container_monitoring_bpf__destroy(skel);
	return -err;
}
