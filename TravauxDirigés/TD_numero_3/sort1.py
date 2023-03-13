import numpy as np
from mpi4py import MPI
from time import time
import matplotlib.pyplot as plt

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

if rank == 0:
    #generate random realizations of a GAuss process with float with a sigma of 100
    data = np.random.uniform(0, 100, size=100000)

    deb = time()
    
    max_elem = np.max(data)
    min_elem = np.min(data)
    delta = (max_elem - min_elem)/size
    tranches = [min_elem + i*delta for i in range(1,size)]

    data_sp = []
    d = time()
    data_sp.append([x for x in data if x < tranches[0]])
    for k in range(len(tranches)-1):
        data_sp.append([x for x in data if x < tranches[k+1] and x >= tranches[k]])
    data_sp.append([x for x in data if x >= tranches[-1]])
    #print("time dispatch: ", time()-d)

else : 
    data_sp = None

bucket = comm.scatter(data_sp, root=0)

d = time()
bucket = np.sort(bucket)
#print("sort bucket ", rank, " : ", time()-d)


bucket = comm.gather(bucket, root=0)

if rank == 0:
    data_sorted = np.concatenate(bucket)

    print("time bucket sort : ", time()-deb)
    print("data_sorted : ", len(data_sorted))
    print("data : ", len(data))

    
    deb = time()
    data_sorted2 = np.sort(data)
    print("time classical sort : ", time()-deb)

    print("same ? : ", np.all(data_sorted == data_sorted2))
    
MPI.Finalize()