# 🖥️ System Statistics Monitor

A C-based real-time system monitor that displays vital system statistics such as CPU usage, memory consumption, network interface activity, and disk I/O performance. Data is retrieved by reading and parsing Linux's `/proc` filesystem.

## 📌 Features

- **CPU Utilization**
  - Calculates CPU usage percentage by parsing `/proc/stat`.

- **Memory Usage**
  - Displays percentage of memory used via `/proc/meminfo`.

- **Network Statistics**
  - Monitors bytes sent and received on a specified network interface (e.g., `eth0`) using `/proc/net/dev`.

- **Disk I/O Statistics**
  - Reports read and write times (in milliseconds) for a specified disk (e.g., `sda`) via `/proc/diskstats`.


