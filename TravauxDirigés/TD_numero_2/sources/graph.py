import numpy as np
import matplotlib.pyplot as plt

nbp1 = [1,2,3,4,6,8,10,12,14,16,18,20,22,24,26]
nbp2 = [2,3,4,6,8,10,12,14,16,18,20,22,24,26]

tps1 = [7.35,3.64,2.59,1.99,1.31,1.10,0.89,0.86,1.24,1.13,1.21,0.87,0.81,0.77, 0.72]

tps2 = [7.07, 3.65,2.50,1.61,1.19,0.92,0.8,1.05,1.1,1.12,1.04,1.01,0.85,0.71]




fig, ax = plt.subplots()
ax.plot(nbp1, tps1, label='stratégie scatter-gather')
ax.plot(nbp2, tps2, label='stratégie maître-esclave')
ax.legend()
plt.title("temps de calcul des points de la figure de mandelbrot 1024 x 1024")
ax.set_xlabel("nbp")
ax.set_ylabel("temps de calcul (en s)")
plt.show()
