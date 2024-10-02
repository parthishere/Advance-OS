#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_LINE 1000
#define MAX_DISKS 256

#define DEFAULT_INTERVAL 1

typedef struct {
    char version[100];
    char build_info[200];
    char distro[50];
    char build_date[100];
} KernelInfo;

typedef struct {
    char manufacturer[50];
    float cpu_mhz;
    int cache_size;
    int num_cpus;
} ProcessorInfo;

typedef struct {
    long long user;
    long long nice;
    long long system;
    long long idle;
    long long iowait;
    long long irq;
    long long softirq;
    long long steal;
    long long guest;
    long long guest_nice;
} CPUStats;

typedef struct {
    int irq;
    int context_switches;
    int boot_time;
    int processes_created;
} SystemStats;

typedef struct {
    int major;
    int minor;
    char device_name[64];
    unsigned long long reads_completed;
    unsigned long long sectors_read;
    unsigned long long writes_completed;
    unsigned long long sectors_written;
    unsigned long long io_in_progress;
    unsigned long long time_io;
} DiskStats;

typedef struct {
    long total;
    long free;
    long available;
} MemInfo;


KernelInfo kernel_info;
ProcessorInfo processor_info;
CPUStats cpu_stats;
SystemStats sys_stats;
DiskStats disk_stats[MAX_DISKS];
int num_disks;
MemInfo mem_info;

void trim(char *str) {
    char *end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

void decode_kernel_info(KernelInfo *info) {
    FILE *kernel_version_file = fopen("/proc/version", "r");
    char line[MAX_LINE];
    fgets(line, sizeof(line), kernel_version_file);
    fclose(kernel_version_file);

    char *token = strtok(line, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");
    strncpy(info->version, token, sizeof(info->version) - 1);

    token = strtok(NULL, "(");
    token = strtok(NULL, ")");
    strncpy(info->build_info, token, sizeof(info->build_info) - 1);

    token = strtok(NULL, " ");
    token = strtok(NULL, " ");
    strncpy(info->distro, token, sizeof(info->distro) - 1);

    token = strtok(NULL, "\n");
    strncpy(info->build_date, token, sizeof(info->build_date) - 1);
}

void decode_processor_info(ProcessorInfo *info) {
    FILE *processor_info_file = fopen("/proc/cpuinfo", "r");
    char line[MAX_LINE];
    char *token;
    int line_no = 1;

    while(fgets(line, sizeof(line), processor_info_file)) {
        if(*line == '\n') break;
        token = strtok(line, ":");
        token = strtok(NULL, " ");
        trim(token);
        
        switch(line_no) {
            case 2:
                strncpy(info->manufacturer, token, sizeof(info->manufacturer) - 1);
                break;
            case 8:
                info->cpu_mhz = atof(token);
                break;
            case 9:
                info->cache_size = atoi(token);
                break;
            case 13:
                info->num_cpus = 2 * atoi(token);
                break;
        }
        line_no++;
    }
    fclose(processor_info_file);
}

void decode_stat(CPUStats *cpu_stats, SystemStats *sys_stats) {
    FILE *stat_file = fopen("/proc/stat", "r");
    char line[MAX_LINE];
    char *token;
    int line_no = 1;

    while(fgets(line, sizeof(line), stat_file)) {
        token = strtok(line, " ");
        if(strcmp(token, "cpu") == 0) {
            sscanf(line, "cpu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
                   &cpu_stats->user, &cpu_stats->nice, &cpu_stats->system,
                   &cpu_stats->idle, &cpu_stats->iowait, &cpu_stats->irq,
                   &cpu_stats->softirq, &cpu_stats->steal, &cpu_stats->guest,
                   &cpu_stats->guest_nice);
        } else if(strcmp(token, "intr") == 0) {
            token = strtok(NULL, " ");
            sys_stats->irq = atoi(token);
        } else if(strcmp(token, "ctxt") == 0) {
            token = strtok(NULL, " ");
            sys_stats->context_switches = atoi(token);
        } else if(strcmp(token, "btime") == 0) {
            token = strtok(NULL, " ");
            sys_stats->boot_time = atoi(token);
        } else if(strcmp(token, "processes") == 0) {
            token = strtok(NULL, " ");
            sys_stats->processes_created = atoi(token);
        }
        line_no++;
    }
    fclose(stat_file);
}

void decode_disk(DiskStats *disk_stats, int *num_disks) {
    FILE *fp = fopen("/proc/diskstats", "r");
    char line[MAX_LINE];
    *num_disks = 0;

    while (fgets(line, sizeof(line), fp) && *num_disks < MAX_DISKS) {
        sscanf(line, "%d %d %s %llu %*d %llu %*d %llu %*d %llu %*d %llu %llu",
               &disk_stats[*num_disks].major, &disk_stats[*num_disks].minor,
               disk_stats[*num_disks].device_name,
               &disk_stats[*num_disks].reads_completed,
               &disk_stats[*num_disks].sectors_read,
               &disk_stats[*num_disks].writes_completed,
               &disk_stats[*num_disks].sectors_written,
               &disk_stats[*num_disks].io_in_progress,
               &disk_stats[*num_disks].time_io);
        (*num_disks)++;
    }
    fclose(fp);
}

void decode_meminfo(MemInfo *mem_info) {
    FILE *meminfo_file = fopen("/proc/meminfo", "r");
    char line[MAX_LINE];
    char *token;

    while(fgets(line, sizeof(line), meminfo_file)) {
        token = strtok(line, ":");
        if(strcmp(token, "MemTotal") == 0) {
            token = strtok(NULL, " kB");
            mem_info->total = atol(token);
        } else if(strcmp(token, "MemFree") == 0) {
            token = strtok(NULL, " kB");
            mem_info->free = atol(token);
        } else if(strcmp(token, "MemAvailable") == 0) {
            token = strtok(NULL, " kB");
            mem_info->available = atol(token);
            break;
        }
    }
    fclose(meminfo_file);
}

// /proc/kallsyms displays virtual addresses of kernel symbols, whereas /proc/iomem shows physical addresses of memory regions.
void decode_iomem(){
    FILE *iomem = fopen("/proc/iomem", "r");
    if (iomem) {
        char line[1000];
        unsigned long start, end;
        char type[16];
        char sub_type[16];
        while (fgets(line, sizeof(line), iomem)) {
            if (strstr(line, "System RAM") || strstr(line, "Kernel code") || 
                strstr(line, "Kernel data") || strstr(line, "Kernel bss") ||
                strstr(line, "Kernel rodata")) {
                sscanf(line, "%lx-%lx : %16s %16s", &start, &end, type, sub_type);
                printf("start: 0x%lX end: 0x%lX | Type : %s %s \n", start, end, type, sub_type);
            }
        }
        fclose(iomem);
    }

    // FILE * kallsyms = fopen("/proc/kallsyms", "rd");
    
    
}


void print_static_info(){
    
    decode_stat(&cpu_stats, &sys_stats);
    decode_processor_info(&processor_info);
    decode_kernel_info(&kernel_info);
    decode_meminfo(&mem_info);

    printf("Processor Manufacturer: %s\n", processor_info.manufacturer);
    printf("CPU MHz: %.2f\n", processor_info.cpu_mhz);
    printf("Cache Size: %d KB\n", processor_info.cache_size);
    printf("Number of CPUs: %d\n\n", processor_info.num_cpus);


    printf("Kernel Version: %s\n", kernel_info.version);
    printf("Build Info: %s\n", kernel_info.build_info);
    printf("Distro: %s\n", kernel_info.distro);
    printf("Build Date: %s\n\n", kernel_info.build_date);


    printf("Memory Info:\n");
    printf("Total: %ld KB\n", mem_info.total);
    printf("Free: %ld KB\n", mem_info.free);
    printf("Available RAM for new processes: %ld kB (%ld MB)\n", 
           mem_info.available, mem_info.available / 1024);

    decode_iomem();
    decode_stat(&cpu_stats, &sys_stats);
    printf("Boot Time: %d second\n", sys_stats.boot_time);
    
}



void print_dynamic_info(int read_rate, int print_rate) {
    CPUStats cpu_stats, prev_cpu_stats;
    SystemStats sys_stats, prev_sys_stats;
    DiskStats disk_stats[MAX_DISKS], prev_disk_stats[MAX_DISKS];
    MemInfo mem_info;
    int num_disks;
    int samples = 0;
    unsigned long long total_user = 0, total_system = 0, total_idle = 0;
    unsigned long long total_free_mem = 0, total_context_switches = 0, total_processes = 0;
    unsigned long long total_read_sectors = 0, total_write_sectors = 0;

    decode_stat(&prev_cpu_stats, &prev_sys_stats);
    decode_disk(prev_disk_stats, &num_disks);

    while (1) {
        sleep(read_rate);
        samples++;

        decode_stat(&cpu_stats, &sys_stats);
        decode_disk(disk_stats, &num_disks);
        decode_meminfo(&mem_info);

        unsigned long long cpu_user = cpu_stats.user - prev_cpu_stats.user;
        unsigned long long cpu_system = cpu_stats.system - prev_cpu_stats.system;
        unsigned long long cpu_idle = cpu_stats.idle - prev_cpu_stats.idle;
        unsigned long long cpu_total = cpu_user + cpu_system + cpu_idle;

        total_user += cpu_user;
        total_system += cpu_system;
        total_idle += cpu_idle;
        total_free_mem += mem_info.free;
        total_context_switches += sys_stats.context_switches - prev_sys_stats.context_switches;
        total_processes += sys_stats.processes_created - prev_sys_stats.processes_created;

        for (int i = 0; i < num_disks; i++) {
            total_read_sectors += disk_stats[i].sectors_read - prev_disk_stats[i].sectors_read;
            total_write_sectors += disk_stats[i].sectors_written - prev_disk_stats[i].sectors_written;
        }

        if (samples * read_rate >= print_rate) {
            double avg_user = (double)total_user / (total_user + total_system + total_idle) * 100;
            double avg_system = (double)total_system / (total_user + total_system + total_idle) * 100;
            double avg_idle = (double)total_idle / (total_user + total_system + total_idle) * 100;
            double avg_free_mem = (double)total_free_mem / samples;
            double avg_free_mem_percent = (double)avg_free_mem / mem_info.total * 100;
            double avg_read_rate = (double)total_read_sectors / print_rate;
            double avg_write_rate = (double)total_write_sectors / print_rate;
            double avg_context_switches = (double)total_context_switches / print_rate;
            double avg_processes = (double)total_processes / print_rate;

            printf("CPU usage: %.2f%% user, %.2f%% system, %.2f%% idle\n", avg_user, avg_system, avg_idle);
            printf("Memory: %.2f kB (%.2f%%) free\n", avg_free_mem, avg_free_mem_percent);
            printf("Disk I/O: %.2f sectors/s read, %.2f sectors/s written\n", avg_read_rate, avg_write_rate);
            printf("Context switches: %.2f/s\n", avg_context_switches);
            printf("Process creations: %.2f/s\n\n", avg_processes);

            samples = 0;
            total_user = total_system = total_idle = 0;
            total_free_mem = total_context_switches = total_processes = 0;
            total_read_sectors = total_write_sectors = 0;
        }

        memcpy(&prev_cpu_stats, &cpu_stats, sizeof(CPUStats));
        memcpy(&prev_sys_stats, &sys_stats, sizeof(SystemStats));
        memcpy(prev_disk_stats, disk_stats, sizeof(DiskStats) * num_disks);
    }
}



int main(int argc, char * argv[]) {

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        int read_rate = DEFAULT_INTERVAL;
        int print_rate = DEFAULT_INTERVAL*10;
        if (argc > 3) {
            read_rate = atoi(argv[2]);
            print_rate = atoi(argv[3]);
        }
        print_dynamic_info(read_rate, print_rate);
    } else {
        print_static_info();
    }

    return 0;
}