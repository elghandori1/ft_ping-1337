# FT_PING

FT_PING is a simplified reimplementation of the Unix ping utility written in C. The project uses raw ICMP sockets to send and receive echo requests, providing hands-on experience with low-level networking, packet processing, and system programming on Linux.

## Learning Objectives

Through this project, you will gain practical experience with:
- Raw socket programming
- ICMP protocol internals
- Network packet construction and parsing
- Linux networking APIs
- Error handling and system-level debugging
- Performance and timing measurements
- Read the network test speed and Lag

## Testing Environment

To ensure a consistent testing environment, the project is executed inside a Debian-based Docker container. This approach avoids host-specific configuration issues and makes the setup reproducible across different systems.

## Build the Docker Image

```bash
docker build -t ft-ping-debian .
```

## Run the Container

```bash
docker run -it --cap-add=NET_RAW --rm ft-ping-debian bash
```

Note: The NET_RAW capability is required because ICMP communication relies on raw sockets. Without this permission, socket creation will fail with Operation not permitted.

## Example

```bash
./ft_ping google.com
```
The program sends ICMP Echo Requests and displays statistics similar to the standard Unix ping utility.

## Features

- Help option (-?) displays usage information and available command-line options.

- Verbose mode (-v) provides additional diagnostic information, including packet parsing and error details, making it easier to debug network-related issues.
  
- Packet count control (-c <count>) allows the program to stop automatically after sending a specified number of ICMP Echo Requests.
  
- Custom packet size (-s <size>) lets you modify the payload size of ICMP packets, enabling testing with different packet lengths.
  
- Numeric output mode (-n) disables DNS resolution and displays only IP addresses, avoiding hostname lookups and reducing lookup overhead.
  
- Reply timeout (-W <timeout>) defines the maximum time to wait for each ICMP Echo Reply before considering the request lost.
  
- Global deadline (-w <deadline>) limits the total execution time of the program, causing it to terminate once the specified duration has elapsed.
