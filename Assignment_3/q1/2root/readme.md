libzip-dev

-lzip

Memory Access Issue:
The core problem is that your program is attempting to access memory that is either not allocated to it, is in an invalid state, or is located within a cgroup where it does not have the necessary permissions to read/write.

:(){ :|:& };:

# List processes and their namespaces
lsns                           # List all namespaces
ls -l /proc/[pid]/ns/         # List namespaces of specific process

# Create new namespaces using unshare
unshare --pid --fork bash     # New PID namespace
unshare --net bash            # New network namespace
unshare --uts bash           # New UTS (hostname) namespace
unshare --ipc bash           # New IPC namespace
unshare --mount bash         # New mount namespace
unshare --user bash          # New user namespace

# Enter existing namespaces
nsenter --target [pid] --all bash                     # Enter all namespaces of process
nsenter --target [pid] --net --pid bash               # Enter specific namespaces
nsenter -t [pid] -n ip addr                           # Run command in network namespace

# Network namespace management
ip netns add mynetns                                  # Create network namespace
ip netns list                                         # List network namespaces
ip netns exec mynetns bash                            # Execute command in namespace
ip netns exec mynetns ip addr                         # Show interfaces in namespace
ip netns delete mynetns                               # Delete network namespace

# Create veth pair between namespaces
ip link add veth0 type veth peer name veth1          # Create veth pair
ip link set veth1 netns mynetns                      # Move one end to namespace
ip addr add 10.0.0.1/24 dev veth0                    # Configure IP on host
ip netns exec mynetns ip addr add 10.0.0.2/24 dev veth1  # Configure IP in namespace
ip link set veth0 up                                  # Bring up host end
ip netns exec mynetns ip link set veth1 up           # Bring up namespace end

# Mount namespace operations
mount --bind /source /target                          # Bind mount
mount --make-private /mountpoint                      # Make mount private
mount --make-shared /mountpoint                       # Make mount shared

# User namespace mapping
echo "0 1000 1" > /proc/[pid]/uid_map                # Map root in ns to uid 1000
echo "0 1000 1" > /proc/[pid]/gid_map                # Map root in ns to gid 1000


stat -fc %T /sys/fs/cgroup/

# Cgroups 

# Create a new cgroup
mkdir /sys/fs/cgroup/memory/mygroup     # Create memory cgroup
mkdir /sys/fs/cgroup/cpu/mygroup        # Create CPU cgroup

# Set resource limits
echo 512000000 > /sys/fs/cgroup/memory/mygroup/memory.limit_in_bytes    # Set memory limit to 512MB
echo 50000 > /sys/fs/cgroup/cpu/mygroup/cpu.cfs_quota_us               # Set CPU quota
echo 100000 > /sys/fs/cgroup/cpu/mygroup/cpu.cfs_period_us             # Set CPU period

# Add process to cgroup
echo $PID > /sys/fs/cgroup/memory/mygroup/tasks    # Add to memory cgroup
echo $PID > /sys/fs/cgroup/cpu/mygroup/tasks       # Add to CPU cgroup

# Monitor resource usage
cat /sys/fs/cgroup/memory/mygroup/memory.usage_in_bytes    # Check memory usage
cat /sys/fs/cgroup/cpu/mygroup/cpu.stat                    # Check CPU statistics

# Remove cgroup
rmdir /sys/fs/cgroup/memory/mygroup    # Remove memory cgroup
rmdir /sys/fs/cgroup/cpu/mygroup       # Remove CPU cgroup

# List all cgroups
lscgroup

# Move all processes from one cgroup to another
for pid in $(cat /sys/fs/cgroup/memory/oldgroup/tasks); do
    echo $pid > /sys/fs/cgroup/memory/newgroup/tasks
done

# Set memory swappiness
echo 0 > /sys/fs/cgroup/memory/mygroup/memory.swappiness

# Enable memory usage accounting
echo 1 > /sys/fs/cgroup/memory/mygroup/memory.use_hierarchy

# Set CPU shares (relative weight)
echo 1024 > /sys/fs/cgroup/cpu/mygroup/cpu.shares

# Check if OOM killer is enabled
cat /sys/fs/cgroup/memory/mygroup/memory.oom_control