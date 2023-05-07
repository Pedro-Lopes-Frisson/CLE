import matplotlib.pyplot as plt
import numpy as np

# Read the file
filename = "dados.csv"  # Replace with your file name
data = np.loadtxt(filename, delimiter=",")

number_of_processes = np.unique(data[:,0])

avg_execution_times_plot = []
std_deviations_plot = []
for p in number_of_processes:
    # Extract the execution time values
    execution_times = data[data[:, 0] == p]


    # Calculate average execution time
    avg_execution_time = np.mean(execution_times)
    avg_execution_times_plot.append(np.mean(execution_times))

    # Calculate standard deviation
    std_deviation = np.std(execution_times)
    std_deviations_plot.append(np.mean(execution_times))

    # Calculate maximum error
    max_error = np.max(np.abs(execution_times - avg_execution_time))

    # Print the results
    print("Number of process ", p)
    print("Average Execution Time:", avg_execution_time)
    print("Standard Deviation:", std_deviation)
    print("Maximum Error:", max_error)



# Extract the execution time values
execution_times = data[:, 1]

# Generate x-axis values (number of processes)
n_processes = data[:, 0]

# Calculate average execution time
avg_execution_time = np.mean(execution_times)

# Calculate standard deviation
std_deviation = np.std(execution_times)

# Plot the mean execution time with error bars representing the standard deviation
plt.errorbar(number_of_processes - 1, avg_execution_times_plot, yerr=std_deviations_plot, marker='o', linestyle='-', label='Execution Time')

# Set plot labels
plt.xlabel("Number of Workers")
plt.ylabel("Execution Time(ns)")

# Set plot title
plt.title("Mean Execution Time with Standard Deviation")

# Display the legend
plt.legend()

# Display the plot
plt.show()
