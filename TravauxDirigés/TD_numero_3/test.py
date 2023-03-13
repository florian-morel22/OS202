from mpi4py import MPI

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

# Génération de données locales sur chaque processus
local_data = [rank] * (rank + 1)

# Récupération de toutes les données sur le processus 0
if rank == 0:
    data = [None] * size
else:
    data = None

recvcounts = comm.gather(len(local_data), root=0)

offset = 0
if rank == 0:
    for i in range(size):
        offset += recvcounts[i-1] if i > 0 else 0
        print(offset, recvcounts[i])

comm.Gatherv(local_data,[] ,root=0)

if rank == 0:
    print(data)
    