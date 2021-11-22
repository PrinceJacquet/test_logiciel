#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/* Fichier de test pour le sujet de TP 4
   14/10/11
	COMPILATION
   gcc -g -o TP3bis_correction TP3bis_correction.c -lm
	EXECUTION
   ./TP3bis_correction
*/

double ** creation(int n);
void suppression(double ** M, int n);
void affichage (double ** M, int n);
void elimination (int i, int j, int n, double ** M, double ** Melim);
double determinant(double ** M, int n);
void inverse(double ** M, double ** iM, int n);



int main()
{
	int n=10;
	int lig,col;
	double ** Matrice;
	double ** iMatrice;
	double detMatrice;
	
	// Allocation de mémoire pour Matrice
	Matrice = creation(n);


	// Allocation de mémoire pour iMatrice
	iMatrice = creation(n);

	// Remplissage de Matrice de van der Monde
	for (lig=0;lig<n;lig++)
	{
		for (col=0;col<n;col++)
		{
			Matrice[lig][col] = pow(lig,col);
		}
	}

	// Affichage de la matrice M
	printf("Matrice M \n");
	affichage(Matrice,n);

	// Calcul du déterminants
	detMatrice = determinant(Matrice,n);
	printf("Déterminant de M : %f \n",detMatrice);

	// calcul de l'inverse de Matrice
	inverse(Matrice, iMatrice, n);
	printf("Inverse Matrice M \n");
	affichage(iMatrice,n);


	// Libération mémoire
	suppression(Matrice,n);
	suppression(iMatrice,n);
	return 0;
}

double ** creation(int n)
{
	int i;
	double ** M;
	M = (double **) malloc(n*sizeof(double *));
	for (i=0;i<n;i++)
		M[i]=(double *) malloc(n*sizeof(double));
	return M;
}

void suppression(double ** M, int n)
{
	int i;
	for (i=0;i<n;i++)
		free(M[i]);
	free(M);
}

void elimination (int i, int j, int n, double ** M, double ** Melim)
{
	int col,colcor;
	int lig,ligcor;
	
	for(lig=0;lig<n;lig++)
	{
		if(lig != i)
		{
			if(lig<i)
				ligcor=lig;
			else
				ligcor=lig-1;
			for(col=0;col<n;col++)
			{
				if(col != j)
				{
					if(col<j)
						colcor=col;
					else
						colcor=col-1;
					Melim[ligcor][colcor]=M[lig][col];
				}
			}
		}
	}
}

void affichage (double ** M, int n)
{
	int col,lig;
	for(lig=0;lig<n;lig++)
	{
		for(col=0;col<n;col++)
		{
			printf("%f\t",M[lig][col]);
		}
		printf("\n");
	}
}

double determinant(double ** M, int n) // récursif, complexité en n factorielle
{
	double det=0;
	double ** M_elim;
	int i,k,signe;
	if (n==1)
		det=M[0][0];
	else
	{
		// Allocation de mémoire pour M_elim
		M_elim = creation(n-1);
		for(k=0;k<n;k++)
		{
			// Affectation des valeurs de M_elim
			elimination(0,k,n,M,M_elim);
			det=det + M[0][k] * determinant(M_elim,n-1) * (1-(k-(k/2)*2)*2);
				// La gestion du signe est un peu sale ici, mais elle fonctionne. Il serait peut être plus simple d'utiliser une variable supplémentaire valant -1 ou 1
		}
		// Liberation de l'espace mémoire de M_elim
		suppression(M_elim,n-1);
	}
	return det;
}

void inverse(double ** M, double ** iM, int n)
{
	double det,codet;
	int lig,col;
	int i;
	double ** M_elim;

	det = determinant(M,n);
	if(det==0)
	{
		printf("Inversion impossible, déterminant nul \n");
	}
	else
	{
		// Calcul de la matrice inverse : 1/det * transposée de la comatrice
		// Allocation de mémoire pour M_elim
		M_elim = creation(n-1);

		for(lig=0;lig<n;lig++)
		{
			for(col=0;col<n;col++)
			{
				elimination(lig,col,n, M, M_elim);
				codet = determinant(M_elim,n-1) * (1-((lig+col)-((lig+col)/2)*2)*2); // idem
				iM[col][lig] = 1/det * codet; // ici transposé en inversant col et lig
			}
		}
		suppression(M_elim,n-1);
	}
}
