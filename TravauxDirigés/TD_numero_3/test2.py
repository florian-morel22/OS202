from mpi4py import MPI
import numpy as np
from time import time
import matplotlib.pyplot as plt

def bucket_sort(arr):
    # Define the number of buckets
    n = len(arr)
    num_buckets = MPI.COMM_WORLD.Get_size()
    bucket_size = n // num_buckets

    # Divide the array into buckets
    buckets = np.array_split(arr, num_buckets)

    # Sort the elements in each bucket locally
    for i in range(num_buckets):
        buckets[i] = np.sort(buckets[i])

    # Gather the sorted buckets from all processes
    sorted_buckets = MPI.COMM_WORLD.allgather(buckets)

    # Flatten the sorted buckets into a single array
    sorted_arr = np.concatenate(sorted_buckets)

    return sorted_arr

# Generate random data for testing
arr = np.random.normal(0, 100, size=1000)

# Sort the data using parallel bucket sort
deb = time()
sorted_arr = bucket_sort(arr)
end = time()
print(sorted_arr)
print(" bucket_sort time : ", end-deb)

deb = time()
a = np.sort(arr)
print(" classic sort time : ", time()-deb)


print("same ? : ", np.array_equal(sorted_arr, a))


# Print the sorted data
