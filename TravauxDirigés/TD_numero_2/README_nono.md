# TP2 de MOREL et MSSELLATI

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`

## Exercice 2.1
(reference à la slides 28 du cours 1)
1/ Imaginons un premier scenario. Le process de rang 0 envoi à 2, celui de rang 1 n'as pas encore eu le temps d'envoyer, donc celui de rang 2 recoi bien cle message de 1. Ensuite 2 envoi à 0 et attends le message de 2, qu'il recoi. 

2/ Dans un second scenraio ou l'on pourrait avoir interblocage dans le cas ou les tag ne corresponderait pas. En effet dans le cas ou le tag du send de rang 1 correponderais au tag du premier receive de rang 2. Et que le tag de de receive de rang 0 et send de rang 2 ne correspondent pas, on se retrouve dans une situation ou 0 attends le message de 2 et 2 attends le message de 1, on est donc face à un interblocage à plus de 2 processus. 

Nous pensons que probabilité pour que ce genre d'erreurs de code arrivent realtivement souvent.

## Exercice 2.1
 En utilisant la loi d'Amdahl pour n>>1, on que que le speed up peut etre au maximum egale à 1/(10%) = 10
 Pour n=2 

## Mandelbrot 

*Expliquer votre stratégie pour faire une partition équitable des lignes de l'image entre chaque processus*

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |              
1          |              
2          |              
3          |              
4          |              
8          |              


*Discuter sur ce qu'on observe, la logique qui s'y cache.*

*Expliquer votre stratégie pour faire une partition dynamique des lignes de l'image entre chaque processus*

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |              
1          |              
2          |              
3          |              
4          |              
8          |              



## Produit matrice-vecteur



*Expliquer la façon dont vous avez calculé la dimension locale sur chaque processus, en particulier quand le nombre de processus ne divise pas la dimension de la matrice.*
