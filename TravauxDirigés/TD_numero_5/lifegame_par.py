"""
Le jeu de la vie
################
Le jeu de la vie est un automate cellulaire inventé par Conway se basant normalement sur une grille infinie
de cellules en deux dimensions. Ces cellules peuvent prendre deux états :
    - un état vivant
    - un état mort
A l'initialisation, certaines cellules sont vivantes, d'autres mortes.
Le principe du jeu est alors d'itérer de telle sorte qu'à chaque itération, une cellule va devoir interagir avec
les huit cellules voisines (gauche, droite, bas, haut et les quatre en diagonales.) L'interaction se fait selon les
règles suivantes pour calculer l'irération suivante :
    - Une cellule vivante avec moins de deux cellules voisines vivantes meurt ( sous-population )
    - Une cellule vivante avec deux ou trois cellules voisines vivantes reste vivante
    - Une cellule vivante avec plus de trois cellules voisines vivantes meurt ( sur-population )
    - Une cellule morte avec exactement trois cellules voisines vivantes devient vivante ( reproduction )

Pour ce projet, on change légèrement les règles en transformant la grille infinie en un tore contenant un
nombre fini de cellules. Les cellules les plus à gauche ont pour voisines les cellules les plus à droite
et inversement, et de même les cellules les plus en haut ont pour voisines les cellules les plus en bas
et inversement.

On itère ensuite pour étudier la façon dont évolue la population des cellules sur la grille.
"""

import numpy   as np
import time
from mpi4py import MPI
import sys

class grille:
    """
    Grille torique décrivant l'automate cellulaire.
    En entrée lors de la création de la grille :
        - dimensions est un tuple contenant le nombre de cellules dans les deux directions (nombre lignes, nombre colonnes)
        - init_pattern est une liste de cellules initialement vivantes sur cette grille (les autres sont considérées comme mortes)
        - color_life est la couleur dans laquelle on affiche une cellule vivante
        - color_dead est la couleur dans laquelle on affiche une cellule morte
    Si aucun pattern n'est donné, on tire au hasard quels sont les cellules vivantes et les cellules mortes
    Exemple :
       grid = grille( (10,10), init_pattern=[(2,2),(0,2),(4,2),(2,0),(2,4)], color_life="red", color_dead="black")
    """
    def __init__( self, dim, init_pattern = None, color_life = "black", color_dead = "white" ):
        import random
        self.dimensions = dim
        if init_pattern is not None:
            self.cells = np.zeros(self.dimensions, dtype=np.uint8)
            indices_i = [v[0] for v in init_pattern]
            indices_j = [v[1] for v in init_pattern]
            self.cells[indices_i,indices_j] = 1
        else:
            self.cells = np.random.randint(2, size=dim, dtype=np.uint8)
        self.col_life = color_life
        self.col_dead = color_dead

    def compute_next_iteration(self):
        """
        Calcule la prochaine génération de cellules en suivant les règles du jeu de la vie
        """
        # Remarque 1: on pourrait optimiser en faisant du vectoriel, mais pour plus de clarté, on utilise les boucles
        # Remarque 2: on voit la grille plus comme une matrice qu'une grille géométrique. L'indice (0,0) est donc en haut
        #             à gauche de la grille !
        ny = self.dimensions[0]
        nx = self.dimensions[1]
        next_cells = np.empty(self.dimensions, dtype=np.uint8)
        diff_cells = []
        for i in range(ny):
            i_above = (i+ny-1)%ny
            i_below = (i+1)%ny
            for j in range(nx):
                j_left = (j-1+nx)%nx
                j_right= (j+1)%nx
                voisins_i = [i_above,i_above,i_above, i     , i      , i_below, i_below, i_below]
                voisins_j = [j_left ,j      ,j_right, j_left, j_right, j_left , j      , j_right]
                voisines = np.array(self.cells[voisins_i,voisins_j])
                nb_voisines_vivantes = np.sum(voisines)
                if self.cells[i,j] == 1: # Si la cellule est vivante
                    if (nb_voisines_vivantes < 2) or (nb_voisines_vivantes > 3):
                        next_cells[i,j] = 0 # Cas de sous ou sur population, la cellule meurt
                        diff_cells.append(i*nx+j)
                    else:
                        next_cells[i,j] = 1 # Sinon elle reste vivante
                elif nb_voisines_vivantes == 3: # Cas où cellule morte mais entourée exactement de trois vivantes
                    next_cells[i,j] = 1         # Naissance de la cellule
                    diff_cells.append(i*nx+j)
                else:
                    next_cells[i,j] = 0         # Morte, elle reste morte.
        self.cells = next_cells
        return diff_cells
class App:
    """
    Cette classe décrit la fenêtre affichant la grille à l'écran
        - geometry est un tuple de deux entiers donnant le nombre de pixels verticaux et horizontaux (dans cet ordre)
        - grid est la grille décrivant l'automate cellulaire (voir plus haut)
    """
    def __init__(self, geometry, grid):
        self.grid = grid
        # Calcul de la taille d'une cellule par rapport à la taille de la fenêtre et de la grille à afficher :
        self.size_x = geometry[1]//grid.dimensions[1]
        self.size_y = geometry[0]//grid.dimensions[0]
        if self.size_x > 4 and self.size_y > 4 :
            self.draw_color='black'
        else:
            self.draw_color=""
        # Ajustement de la taille de la fenêtre pour bien fitter la dimension de la grille
        self.width = grid.dimensions[1] * self.size_x
        self.height= grid.dimensions[0] * self.size_y
        # Création de la fenêtre à l'aide de tkinter
        self.root = tk.Tk()
        # Création de l'objet d'affichage
        self.canvas = tk.Canvas(self.root, height = self.height, width = self.width)
        self.canvas.pack()
        #
        self.canvas_cells = []

    def compute_rectange(self, i : int, j : int):
        """
        Calcul la géométrie du rectangle correspondant à la cellule (i,j)
        """
        return (self.size_x*j,self.height - self.size_y*i - 1, self.size_x*j+self.size_x-1, self.height - self.size_y*(i+1) )

    def compute_color(self,i : int,j : int):
        if self.grid.cells[i,j] == 0:
            return self.grid.col_dead
        else:
            return self.grid.col_life
        
    def upgrade_grid_from_diff(self, diff):
        nx = grid.dimensions[1]
        ny = grid.dimensions[0]
        for ind in diff:
            self.grid.cells[ind//nx,ind%nx] = 1 - self.grid.cells[ind//nx,ind%nx]

    def draw(self, diff):
        if len(self.canvas_cells) == 0:
            # Création la première fois des cellules en tant qu'entité graphique :
            self.canvas_cells = [self.canvas.create_rectangle(*self.compute_rectange(i,j), fill=self.compute_color(i,j),outline=self.draw_color) for i in range(self.grid.dimensions[0]) for j in range(self.grid.dimensions[1])]
        else:
            nx = grid.dimensions[1]
            ny = grid.dimensions[0]
            [self.canvas.itemconfig(self.canvas_cells[ind], fill=self.compute_color(ind//nx,ind%nx),outline=self.draw_color) for ind in diff]
        self.root.update_idletasks()
        self.root.update()

if __name__ == '__main__':
    import tkinter as tk
    comm = MPI.COMM_WORLD
    size = comm.Get_size()
    rank = comm.Get_rank()

    dico_patterns = { # Dimension et pattern dans un tuple
        'blinker' : ((5,5),[(2,1),(2,2),(2,3)]),
        'toad'    : ((6,6),[(2,2),(2,3),(2,4),(3,3),(3,4),(3,5)]),
        "acorn"   : ((100,100), [(51,52),(52,54),(53,51),(53,52),(53,55),(53,56),(53,57)]),
        "beacon"  : ((6,6), [(1,3),(1,4),(2,3),(2,4),(3,1),(3,2),(4,1),(4,2)]),
        "boat" : ((5,5),[(1,1),(1,2),(2,1),(2,3),(3,2)]),
        "glider": ((100,90),[(1,1),(2,2),(2,3),(3,1),(3,2)]),
        "glider_gun": ((200,100),[(51,76),(52,74),(52,76),(53,64),(53,65),(53,72),(53,73),(53,86),(53,87),(54,63),(54,67),(54,72),(54,73),(54,86),(54,87),(55,52),(55,53),(55,62),(55,68),(55,72),(55,73),(56,52),(56,53),(56,62),(56,66),(56,68),(56,69),(56,74),(56,76),(57,62),(57,68),(57,76),(58,63),(58,67),(59,64),(59,65)]),
        "space_ship": ((25,25),[(11,13),(11,14),(12,11),(12,12),(12,14),(12,15),(13,11),(13,12),(13,13),(13,14),(14,12),(14,13)]),
        "die_hard" : ((100,100), [(51,57),(52,51),(52,52),(53,52),(53,56),(53,57),(53,58)]),
        "pulsar": ((17,17),[(2,4),(2,5),(2,6),(7,4),(7,5),(7,6),(9,4),(9,5),(9,6),(14,4),(14,5),(14,6),(2,10),(2,11),(2,12),(7,10),(7,11),(7,12),(9,10),(9,11),(9,12),(14,10),(14,11),(14,12),(4,2),(5,2),(6,2),(4,7),(5,7),(6,7),(4,9),(5,9),(6,9),(4,14),(5,14),(6,14),(10,2),(11,2),(12,2),(10,7),(11,7),(12,7),(10,9),(11,9),(12,9),(10,14),(11,14),(12,14)]),
        "floraison" : ((40,40), [(19,18),(19,19),(19,20),(20,17),(20,19),(20,21),(21,18),(21,19),(21,20)]),
        "block_switch_engine" : ((400,400), [(201,202),(201,203),(202,202),(202,203),(211,203),(212,204),(212,202),(214,204),(214,201),(215,201),(215,202),(216,201)]),
        "u" : ((200,200), [(101,101),(102,102),(103,102),(103,101),(104,103),(105,103),(105,102),(105,101),(105,105),(103,105),(102,105),(101,105),(101,104)]),
        "flat" : ((200,400), [(80,200),(81,200),(82,200),(83,200),(84,200),(85,200),(86,200),(87,200), (89,200),(90,200),(91,200),(92,200),(93,200),(97,200),(98,200),(99,200),(106,200),(107,200),(108,200),(109,200),(110,200),(111,200),(112,200),(114,200),(115,200),(116,200),(117,200),(118,200)])
    }
    choice = 'beacon'
    if len(sys.argv) > 1 :
        choice = sys.argv[1]
    resx = 800
    resy = 800
    if len(sys.argv) > 3 :
        resx = int(sys.argv[2])
        resy = int(sys.argv[3])
    print(f"Pattern initial choisi : {choice}")
    print(f"resolution ecran : {resx,resy}")
    init_pattern = dico_patterns[choice]
    

    grid = grille(*init_pattern)
    appli = App((resx,resy),grid)
       

    ny, nx = dico_patterns[choice][0]


    if rank == 0:
        partial_grid = grille(dim = (ny//3+1,nx))
        partial_grid.cells = grid.cells[0:grid.dimensions[0]//3+1,:]

        diff = partial_grid.compute_next_iteration()

        diff_last_line = [x for x in diff if x//grid.dimensions[1] == grid.dimensions[0]-1]
        diff = [x for x in diff if x not in diff_last_line]

        comm.send(diff_last_line, dest=1)
        comm.send(diff, dest=3)
    
    elif rank == 1:
        partial_grid = grille(dim = (ny//3+2,nx))
        partial_grid.cells = grid.cells[grid.dimensions[0]//3-1:2*grid.dimensions[0]//3+1,:]

        diff = partial_grid.compute_next_iteration()

        diff_first_line = [x for x in diff if x//grid.dimensions[1] == 0]
        diff_last_line = [x for x in diff if x//grid.dimensions[1] == grid.dimensions[0]-1]
        diff = [x for x in diff if x not in diff_first_line and x not in diff_last_line]

        comm.send(diff_first_line, dest=0)
        comm.send(diff_last_line, dest=2)
        comm.send(diff, dest=3)

    elif rank == 2:
        partial_grid = grille(dim = (ny//3+1,nx))
        partial_grid.cells = grid.cells[2*grid.cells.shape[0]//3-1:,:]

        diff = partial_grid.compute_next_iteration()

        diff_first_line = [x for x in diff if x//grid.dimensions[1] == 0]
        diff = [x for x in diff if x not in diff_first_line]

        comm.send(diff_first_line, dest=1)
        comm.send(diff, dest=3)

    elif rank == 3:

        diff0 = comm.recv(source=0)
        diff1 = comm.recv(source=1)
        diff2 = comm.recv(source=2)

        diff1 = [x + grid.dimensions[0]//3 for x in diff1]
        diff2 = [x + 2*grid.dimensions[0]//3 for x in diff2]

        diff = diff0 + diff1 + diff2

        appli.upgrade_grid_from_diff(diff)
        appli.draw(diff)

    
    while(True):
        time.sleep(0.2) # A régler ou commenter pour vitesse maxi
        
        if rank == 0:
            print('rank0 receive from 1')
            diff_last_line = comm.recv(source=1)
            print('rank0 receive from 1 done')
            for point in diff_last_line:
                partial_grid.cells[-1,point%grid.dimensions[1]] = 1 - partial_grid.cells[-1,point%grid.dimensions[1]]
            
            diff = partial_grid.compute_next_iteration()
            print("rank 0 is calculating...")

            diff_first_line = [x for x in diff if x//grid.dimensions[1] == 0]
            diff_last_line = [x for x in diff if x//grid.dimensions[1] == grid.dimensions[0]-1]
            diff = [x for x in diff if x not in diff_first_line and x not in diff_last_line]

            comm.send(diff_first_line, dest=0)
            comm.send(diff_last_line, dest=2)
            comm.send(diff, dest=3)
        
        
        elif rank == 1:
            print('rank1 receive from 0')
            diff_first_line = comm.recv(source=0)
            print('rank1 receive from 0 done')
            for point in diff_first_line:
                partial_grid.cells[0,point%grid.dimensions[1]] = 1 - partial_grid.cells[0,point%grid.dimensions[1]]

            print('rank1 receive from 2')
            diff_last_line = comm.recv(source=2)
            print('rank1 receive from 2 done')
            for point in diff_last_line:
                partial_grid.cells[-1,point%grid.dimensions[1]] = 1 - partial_grid.cells[-1,point%grid.dimensions[1]]


            diff = partial_grid.compute_next_iteration()
            print("rank 1 is calculating...")
            
            diff_first_line = [x for x in diff if x[0] == 0]
            diff_last_line = [x for x in diff if x[0] == partial_grid.dim[-1]-1]
            diff = [x for x in diff if x not in diff_first_line and x not in diff_last_line]

            comm.send(diff_first_line, dest=0)
            comm.send(diff_last_line, dest=2)
            comm.send(diff, dest=3, tag=1)
        
        elif rank == 2:
            print('rank2 receive from 1')
            diff_first_line = comm.recv(source=1)
            print('rank2 receive from 1 done')
            for point in diff_first_line:
                partial_grid.cells[0,point%grid.dimensions[1]] = 1 - partial_grid.cells[0,point%grid.dimensions[1]]

            diff = partial_grid.compute_next_iteration()
            print("rank 2 is calculating...")
            
            diff_first_line = [x for x in diff if x[0] == 0]
            diff = [x for x in diff if x not in diff_first_line and x not in diff_last_line]

            comm.send(diff_first_line, dest=1)
            comm.send(diff, dest=3, tag=2)

        elif rank == 3:
            # Affichage


            diff0 = comm.recv(source=0)
            diff1 = comm.recv(source=1)
            diff2 = comm.recv(source=2)

            print("rank 3 is displaying...")

            diff1 = [x + grid.dimensions[0]//3 for x in diff1]
            diff2 = [x + 2*grid.dimensions[0]//3 for x in diff2]

            diff = diff0 + diff1 + diff2

            appli.upgrade_grid_from_diff(diff)
            appli.draw(diff)

MPI.finalize()

        
           

            


