import matplotlib.pyplot as plt

threads = [1, 10, 15, 20, 40, 60, 79, 120, 500, 750, 1000, 1250, 1500, 1750, 2000, 2500, 2750, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]

real_times = [0.023, 0.011, 0.011, 0.012, 0.015, 0.018, 0.029, 0.006, 0.119, 0.140, 0.142, 0.166, 0.191, 0.248, 0.230, 0.281, 0.318, 0.349, 0.409, 0.597, 0.617, 0.882, 0.920, 0.998, 1.159]
user_times = [0.011, 0.015, 0.008, 0.008, 0.016, 0.010, 0.000, 0.000, 0.023, 0.009, 0.015, 0.025, 0.025, 0.018, 0.056, 0.018, 0.016, 0.032, 0.032, 0.025, 0.057, 0.053, 0.071, 0.105, 0.116]
sys_times = [0.001, 0.001, 0.001, 0.002, 0.008, 0.010, 0.027, 0.002, 0.096, 0.134, 0.134, 0.146, 0.174, 0.238, 0.182, 0.283, 0.310, 0.341, 0.400, 0.575, 0.583, 0.909, 0.916, 0.967, 1.125]

plt.figure(figsize=(10, 6))

plt.plot(threads, real_times, marker='o', linestyle='-', color='b', label="Real Time")
plt.plot(threads, user_times, marker='o', linestyle='-', color='g', label="User Time")
plt.plot(threads, sys_times, marker='o', linestyle='-', color='r', label="Sys Time")

plt.xlabel('Number of Threads')
plt.ylabel('Time (seconds)')
plt.title('Impact of Threading on Execution Time (Real, User, Sys)')
plt.grid(True)

plt.legend()

plt.show()
