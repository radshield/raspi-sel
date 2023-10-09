cpupower frequency-set -f 600000
sleep 3
numactl -C 0 stress-ng --matrix 1 -t 3s
numactl -C 0,1 stress-ng --matrix 2 -t 3s
numactl -C 0,1,2 stress-ng --matrix 3 -t 3s
numactl -C 0,1,2,3 stress-ng --matrix 4 -t 3s
cpupower frequency-set -f 700000
sleep 3
numactl -C 0 stress-ng --matrix 1 -t 3s
numactl -C 0,1 stress-ng --matrix 2 -t 3s
numactl -C 0,1,2 stress-ng --matrix 3 -t 3s
numactl -C 0,1,2,3 stress-ng --matrix 4 -t 3s
cpupower frequency-set -f 800000
sleep 3
numactl -C 0 stress-ng --matrix 1 -t 3s
numactl -C 0,1 stress-ng --matrix 2 -t 3s
numactl -C 0,1,2 stress-ng --matrix 3 -t 3s
numactl -C 0,1,2,3 stress-ng --matrix 4 -t 3s
cpupower frequency-set -f 900000
sleep 3
numactl -C 0 stress-ng --matrix 1 -t 3s
numactl -C 0,1 stress-ng --matrix 2 -t 3s
numactl -C 0,1,2 stress-ng --matrix 3 -t 3s
numactl -C 0,1,2,3 stress-ng --matrix 4 -t 3s
cpupower frequency-set -f 1000000
sleep 3
numactl -C 0 stress-ng --matrix 1 -t 3s
numactl -C 0,1 stress-ng --matrix 2 -t 3s
numactl -C 0,1,2 stress-ng --matrix 3 -t 3s
numactl -C 0,1,2,3 stress-ng --matrix 4 -t 3s

