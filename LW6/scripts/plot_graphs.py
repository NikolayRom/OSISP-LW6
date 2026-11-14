import matplotlib.pyplot as plt

batches = ['10 Events', '50 Events', '100 Events']
avg_times = [1.90, 9.98, 16.39]
eps = [5263, 5012, 6099]

plt.figure(figsize=(8, 5))
bars1 = plt.bar(batches, avg_times, color=['#377eb8', '#4daf4a', '#e41a1c'], width=0.45)
plt.title('Graph 1. Event Log Writing Time vs Batch Size (Windows Event Log)', fontsize=11, fontweight='bold')
plt.xlabel('Batch Size (Events)', fontsize=10)
plt.ylabel('Average Execution Time (ms)', fontsize=10)
plt.ylim(0, 20)
plt.grid(axis='y', linestyle='--', alpha=0.6)

for bar in bars1:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width() / 2.0, yval + 0.3, f'{yval:.2f} ms', ha='center', va='bottom', fontsize=9, fontweight='bold')

plt.tight_layout()
plt.savefig('graph1_time.png', dpi=300)
print('Graph 1 saved: graph1_time.png')

plt.figure(figsize=(8, 5))
bars2 = plt.bar(batches, eps, color=['#4c72b0', '#55a868', '#8172b2'], width=0.45)
plt.title('Graph 2. Event Log Writing Throughput (Events / sec)', fontsize=11, fontweight='bold')
plt.xlabel('Batch Size (Events)', fontsize=10)
plt.ylabel('Throughput (Events / sec)', fontsize=10)
plt.ylim(0, 7500)
plt.grid(axis='y', linestyle='--', alpha=0.6)

for bar in bars2:
    yval = bar.get_height()
    plt.text(bar.get_x() + bar.get_width() / 2.0, yval + 100, f'{int(yval)} eps', ha='center', va='bottom', fontsize=9, fontweight='bold')

plt.tight_layout()
plt.savefig('graph2_throughput.png', dpi=300)
print('Graph 2 saved: graph2_throughput.png')

plt.show()