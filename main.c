/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include <signal.h>
#include "system.h"

/**
 * Needs:
 *   signal()
 */

static volatile int done;

static void
_signal_(int signum) {
    assert(SIGINT == signum);

    done = 1;
}

double
cpu_util(const char *s) {
    static unsigned sum_, vector_[7];
    unsigned sum, vector[7];
    const char *p;
    double util;
    uint64_t i;

    /*
      user
      nice
      system
      idle
      iowait
      irq
      softirq
    */

    if (!(p = strstr(s, " ")) ||
        (7 != sscanf(p,
                     "%u %u %u %u %u %u %u",
                     &vector[0],
                     &vector[1],
                     &vector[2],
                     &vector[3],
                     &vector[4],
                     &vector[5],
                     &vector[6]))) {
        return 0;
    }
    sum = 0.0;
    for (i = 0; i < ARRAY_SIZE(vector); ++i) {
        sum += vector[i];
    }
    util = (1.0 - (vector[3] - vector_[3]) / (double) (sum - sum_)) * 100.0;
    sum_ = sum;
    for (i = 0; i < ARRAY_SIZE(vector); ++i) {
        vector_[i] = vector[i];
    }
    return util;
}

/**
 * @brief This function retrieves and prints the percentage of memory used 
 *        on the system by parsing data from the "/proc/meminfo" file.
 */
void memory_stats(void) {
    /* Define the path to the /proc/meminfo file as a constant string. */
    const char *const MEMINFO_PATH = "/proc/meminfo";
    FILE *file_pointer;      /* File pointer for accessing /proc/meminfo. */
    char buffer[1024];       /* Buffer for reading lines from the file. */
    unsigned long total_memory = 0, free_memory = 0; /* Variables to store total and free memory. */

    /* Attempt to open the /proc/meminfo file for reading. */
    if (!(file_pointer = fopen(MEMINFO_PATH, "r"))) {
        /* Log an error if the file cannot be opened and return. */
        TRACE("fopen() - Memory");
        return;
    }

    /* Read the file line by line. */
    while (fgets(buffer, sizeof(buffer), file_pointer)) {
        /* Check if the line contains "MemTotal:" and extract the value. */
        if (strncmp(buffer, "MemTotal:", 9) == 0) {
            sscanf(buffer + 9, "%lu", &total_memory); /* Parse the total memory value. */
        }
        /* Check if the line contains "MemFree:" and extract the value. */
        else if (strncmp(buffer, "MemFree:", 8) == 0) {
            sscanf(buffer + 8, "%lu", &free_memory); /* Parse the free memory value. */
        }
    }

    /* Close the file after processing to release resources. */
    fclose(file_pointer);

    /* 
     * Calculate the percentage of memory used and print it. 
     * The formula used is: ((total_memory - free_memory) / total_memory) * 100.
     */
    printf(" | Memory Used Percentage: %5.1f%%", 
           (double)(total_memory - free_memory) / (double)total_memory * 100);
}

/**
 * @brief Reads network statistics for a specified network interface from /proc/net/dev
 *
 * This function retrieves and displays the number of bytes received and sent
 * for a specified network interface by parsing the `/proc/net/dev` file.
 *
 * @param target_interface Pointer to a string representing the target network interface (e.g., "eth0").
 */
void network_stats(char *target_interface) {
    /* Path to the /proc/net/dev file containing network statistics */
    const char *const PROC_NET_DEV = "/proc/net/dev";
    
    /* File pointer to access the network device statistics */
    FILE *network_file;

    /* Buffer to store each line read from the /proc/net/dev file */
    char file_line_buffer[1024];
    
    /* Loop counter used to skip header lines */
    int loop_index;

    /* Attempt to open the /proc/net/dev file in read mode */
    if (!(network_file = fopen(PROC_NET_DEV, "r"))) {
        /* Log an error if the file cannot be opened */
        TRACE("fopen() - Failed to open /proc/net/dev");
        return;  /* Exit the function if the file could not be opened */
    }

    /* Skip the first two lines of the file (header information) */
    for (loop_index = 0; loop_index < 2; ++loop_index) {
        /* Read and discard the line; log an error if reading fails */
        if (!fgets(file_line_buffer, sizeof(file_line_buffer), network_file)) {
            TRACE("fgets() - Failed to read /proc/net/dev header");
            fclose(network_file);  /* Ensure the file is closed before exiting */
            return;
        }
    }

    /* Read the remaining lines of the file */
    while (fgets(file_line_buffer, sizeof(file_line_buffer), network_file)) {
        /* Buffer to store the current network interface name */
        char current_interface[32];

        /* Variables to store bytes received and sent */
        unsigned long bytes_received, bytes_sent;

        /**
         * Parse the current line using sscanf to extract:
         * - Interface name (string)
         * - Bytes received (unsigned long)
         * - Bytes sent (unsigned long)
         * 
         * The format string skips other fields in the line using `%*`.
         */
        if (sscanf(file_line_buffer, "%s %lu %*u %*u %*u %*u %*u %*u %*u %lu",
                   current_interface, &bytes_received, &bytes_sent) == 3) {
            /**
             * Compare the extracted interface name with the target interface.
             * If they match, print the bytes received and sent for the interface.
             */
            if (strcmp(current_interface, target_interface) == 0) {
                printf(" | %s Receive: %lu bytes | %s Send: %lu bytes", 
                       target_interface, bytes_received, target_interface, bytes_sent);
                break;  /* Exit the loop once the desired interface is found */
            }
        }
    }

    /* Close the /proc/net/dev file after processing */
    fclose(network_file);
}



/**
 * @brief Reads disk I/O statistics for a specified disk from /proc/diskstats
 *
 * This function reads the contents of the `/proc/diskstats` file to gather 
 * information about read and write operations for a specified disk.
 * It outputs the read and write times in milliseconds for the specified disk.
 *
 * @param disk_name Pointer to a string representing the disk name (e.g., "sda").
 */
void disk_stats(char *disk_name) {
    /* Path to the /proc/diskstats file containing disk statistics */
    const char *const DISK_STATS_PATH = "/proc/diskstats";

    /* File pointer to read /proc/diskstats */
    FILE *disk_stats_file;

    /* Buffer to store each line read from /proc/diskstats */
    char stats_line[1024];

    /* Attempt to open the /proc/diskstats file in read mode */
    if (!(disk_stats_file = fopen(DISK_STATS_PATH, "r"))) {
        TRACE("fopen() - Failed to open /proc/diskstats"); /* Log error if the file cannot be opened */
        return; /* Exit the function if the file could not be opened */
    }

    /* Loop through each line in the /proc/diskstats file */
    while (fgets(stats_line, sizeof(stats_line), disk_stats_file)) {
        /* Buffer to store the device name parsed from the current line */
        char device_name[32];

        /* Variables to store the number of I/O operations for read and write */
        unsigned long read_operations, write_operations;

        /**
         * Parse the line using sscanf to extract:
         * - Device name (string)
         * - Read operations (unsigned long)
         * - Write operations (unsigned long)
         * 
         * The format string skips other fields in the line using `%*`.
         */
        if (sscanf(stats_line, "%*u %*u %s %*u %*u %*u %lu %*u %*u %*u %lu", 
                   device_name, &read_operations, &write_operations) != 3) {
            continue; /* Skip to the next line if parsing fails */
        }

        /**
         * Compare the parsed device name with the specified disk name.
         * If they match, print the read and write statistics and exit the loop.
         */
        if (strcmp(device_name, disk_name) == 0) {
            printf(" | %s Read: %lu ms | %s Write: %lu ms", 
                   disk_name, read_operations, disk_name, write_operations);
            break;
        }
    }

    /* Close the /proc/diskstats file */
    fclose(disk_stats_file);
}


int main(int argc, char *argv[]) {
    const char *const PROC_STAT = "/proc/stat";

    char line[1024];
    FILE *file;

    UNUSED(argc);
    UNUSED(argv);

    if (SIG_ERR == signal(SIGINT, _signal_)) {
        TRACE("signal()");
        return -1;
    }

    while (!done) {
        if (!(file = fopen(PROC_STAT, "r"))) {
            TRACE("fopen() - CPU");
            return -1;
        }
        if (fgets(line, sizeof(line), file)) {
            printf("\rCPU Utilization: %5.1f%%", cpu_util(line));
            fflush(stdout);
        }
        fclose(file);

        memory_stats();
        network_stats("eth0:");
        disk_stats("sda");

        us_sleep(500000);
    }

    printf("\rDone!                                                                                            \n");
    return 0;
}