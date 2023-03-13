import numpy as np
from mpi4py import MPI
from time import time

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

if rank == 0:
    #generate random realizations of a GAuss process with float with a sigma of 100
    data = np.random.normal(0, 100, size=1000)

    deb = time()
    data_sp = np.array_split(data, size)
    
    # sort my data in ascending order
    print("data : ", data_sp)
    
else: 
    data_sp = None
    
bucket = comm.scatter(data_sp, root=0)

quartiles = []
for k in range(size-1):
    quartiles.append(np.percentile(bucket, (k+1)/size * 100))


bucket = comm.gather(bucket, root=0)

if rank == 0:
    
    global_quartiles = []
    for k in range(size-1):
        global_quartiles.append(np.percentile(bucket, (k+1)/size * 100))
    global_quartiles.reverse()

    data_split1 = []
    data_split2 = []

    for single_data in data:
        for i in range(size):
            if i == size-1:
                data_split2.append(single_data)
            elif single_data < global_quartiles[i]:
                data_split1.append(single_data)
                break
            

    print("global_quartiles : ", global_quartiles)
    print("data_split[0] : ", len(data_split1))
    print("data_split[1] : ", len(data_split2))

    data_split = [data_split1, data_split2]
    
          

else:
    data_split = None


bucket_split = comm.scatter(data_split, root=0)


bucket_split = np.sort(bucket_split)
print("rank : ", rank, ", bucket_split : ", bucket_split)

bucket_split = comm.gather(bucket_split,root=0)

if rank == 0:
    data_sorted = np.concatenate(tuple([bucket_split[i] for i in range(size)]), axis=0)
    print(data_sorted)

    end = time()
    print(f"Temps du calcul du buckerSort : {end-deb}")

    deb = time()
    data_sorted = np.sort(data)
    end = time()
    print(f"Temps du calcul du classicSort : {end-deb}")



MPI.Finalize()