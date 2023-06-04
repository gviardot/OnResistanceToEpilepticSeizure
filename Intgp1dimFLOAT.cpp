/**************************************************************************************
   * Nom du programme :INTGP1dim.CPP										*
   *Auteurs : VIARDOT Geoffrey											*
   * Derniére modification : 22 mai 2009										*
   * Ce programme reprend, en partie, les noms de variables du programme INTGP1.CPP			*
   * Auteurs : MAURICE Damien 											*
   * Derniére modification : 26 juin 1998  										*
   * !!! Cependant la méthode utilisée est très différente !!!							*
   *																*
   * But :	Ce programme calcule l'integrale de corrélation selon l'algorithme de	                   	*
   *		Grassberger et Procaccia modifié par Theiller.					                   	*
   *		  													        	*
   * Syntaxe : Intgp.exe <nom_fichier_entree> <dimension> <tau> <param_theiller> <nb_blocs_5s>         *
   *																*
   * Paramètre indispensable : <nom_fichier_entree> nom du fichier contenant les données EEG	* 
   *					Les données sont sur 16 bits et echantillonnées à 256 Hz		*
   * 																*
   * 1er Paramètre facultatif : <dimension> nombre entier définissant la dimension de l'espace		*
   *					valeur par défaut : 15								*
   * 																*
   * 2ème Paramètre facultatif : <tau> nombre entier ; nombre d'échantillons entre deux dimensions	*
   *					valeur par défaut : 16  (=62.5ms)						*
   * 																*
   * 3ème Paramètre facultatif : <param_theiller> nombre entier définissant la correction pour 	*
   *					l'artefact "brin unique"								*
   *					valeur par défaut : 10								*
   * 																*
     * 4ème Paramètre debuguage : <nb_blocs_5s> nombre entier à supprimer à la fin			*
   * 																*
   *************************************************************************************/

#include <time.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define nb_rayons 136
#define longueur_bloc_5sec 5*256
#define longueur_decalage_bloc 1*256
 

void main(int argc,char *argv[])
{

	unsigned long **Density,nb_donnees,compteur,offset_sur_bloc_5s,nb_blocs_5s,numero_bloc,fin_du_bloc,nbpoints_bloc_5s,numero_rayon;
	/* char debugmode=1; */
	
	float *distances,tmp;
  
	/* Données EEG issues du fichier d'entrée */
	register short *dataShort;
	register float *data;
	
	/* Paramètres de l'algo */
	unsigned long param_theiller,dimension,tau;
	
	/* Variable pour les fichiers en entrée et en sortie */
	char nom_fich_entree[256], nom_fich_sortie[256];
	FILE *fich_entree,*fich_sortie;
	
	/* Liste des rayons à partir des quels il faut calculer la densité de Lerner */
	float rayons[]={1,2,3,4,5,6,7,8,9,10,11,13,14,16,18,20,22,25,28,32,35,40,45,
    50,56,63,71,79,89,100,112,126,141,158,178,200,224,251,282,316,355,398,447,501,
    562,631,708,794,891,1000,1122,1259,1413,1585,1778,1995,2239,2512,2818,3162,
    3548,3981,4467,5012,5623,6310,7079,7943,8913,10000,11220,12589,14125,15849,
    17783,19953,22387,25119,28184,31623,35481,39811,44668,50119,56234,63096,70795,
    79433,89125,100000,112202,125893,141254,158489,177828,199526,223872,251189,
    281838,316228,354813,398107,446684,501187,562341,630957,707946,794328,891251,
    1000000,1122018,1258925,1412538,1584893,1778279,1995262,2238721,2511886,2818382,
    3162278,3548134,3981072,4466836,5011872,5623413,6309573,7079458,7943282,8912509,
    10000000,11220184,12589254,14125375,15848931,17782794,19952622};
	
	/* Variables muettes pour les boucles courtes */
	register unsigned long i,j,k;
	
	clock_t start, finish;
	
	start = clock();
	dimension=15;
	tau=16;
	param_theiller=10;
	printf("%s\n",argv[1]);
	
	strcpy(nom_fich_entree,argv[1]);
	strcpy(nom_fich_sortie,argv[1]);
	/* le fichier de sortie a le même nom que celui d'entrée mais avec un 'A' ajouté à la fin */
	nom_fich_sortie[strlen(nom_fich_sortie)]=65;
	nom_fich_sortie[strlen(nom_fich_entree)+1]=0;

	/* Recuperation des paramètres d'entrée */
	if (argc<1) /* Si il n'y a aucun paramètre on arrete tout immédiatement car le nom du fichier d'entrée est indispensable */
	{
		fprintf(stderr,"Relancer en indiquant le nom du FICHIER de données\n\n");
		getch();
		exit(-1);
	}
	if (argc>1) /* Si il a un second paramètre c'est la dimension de l'espace des phases */
	{
		dimension=atol(argv[2]);
	}
	if (argc>2) /* Si il a un troisième paramètre c'est Tau */
	{
		tau=atol(argv[3]);
	}
	if (argc>3) /* Si il a un quatrième paramètre c'est celui de reduction de l'artefact brin unique */
	{
		param_theiller=atol(argv[4]);
	} 
	
	/* Ouverture du fichier contenant les données EEG */
	if (( fich_entree = fopen(nom_fich_entree,"rb")) == NULL)
	{
		fprintf(stderr,"FICHIER INEXISTANT %s \n\n",argv[1]);
		getch();
		exit(-1);
	}

	/* On ouvre immédiatement le fichier de sortie : inutile de lancer le calcul si on ne peut pas sauver les resultats à la fin */
	fich_sortie = fopen(nom_fich_sortie,"w");
	if ( fich_sortie == NULL)
	{
		fprintf(stderr,"Impossible de créer le fichier de sortie \n");
		getch();
		exit(-1);
	}
	/* Sauvegarde des paramètres utilisés pour les calculs... 
	rien de plus inutile que des resultats pour lesquels on ne connait pas 
	les données sources et les paramètres utilisés*/
	fprintf(fich_sortie,"données=%s\n",nom_fich_entree);
	fprintf(fich_sortie,"dimension=%d\n",dimension);
	fprintf(fich_sortie,"tau=%d\n",tau);
	fprintf(fich_sortie,"param_theiller=%d\n",param_theiller);
	
	printf("dimension=%d\n",dimension);
	printf("tau=%d\n",tau);
	printf("param_theiller=%d\n",param_theiller);
	
	
    /* estimation du la quantité de données à charger en se placant à la fin du fichier */
	fseek(fich_entree,0L,SEEK_END);
	nb_donnees=(ftell(fich_entree)/sizeof(short));
	printf("nb ech a charger : %ld\n",nb_donnees);
	/* écriture de l'info dans le fichier de sortie */
	fprintf(fich_sortie,"echantillons_dispo=%d\n",nb_donnees);
	/* reservation de l'espace mémoire pour les données */
	dataShort=(short*)(malloc(nb_donnees*sizeof(short)));// PENSEZ au FREE
	
	/* chargement des données après s'être replacé en début de fichier */
	fseek(fich_entree,0,SEEK_SET);
	// for (i=0;i<nb_donnees;i++)
	// fread(&data[i],sizeof(short),1,fich_entree);
	fread(&dataShort[0],sizeof(short),nb_donnees,fich_entree); 
    /* fermeture du fichier de données... toutes les données sont en mémoire */
	fclose(fich_entree);
	
	data=(float*)(malloc(nb_donnees*sizeof(float)));// PENSEZ au FREE
	for (i=0;i<nb_donnees;i++)
		data[i]=(float)dataShort[i];
	free(dataShort);

    /* on estime la densité de Lerner sur des périodes de 5 secondes 
	On calcule le nombre de périodes de 5 secondes à partir du nombre de données EEG dispo et de l'overlap */
    printf("nb ech charge : %d  \n",nb_donnees);
	printf("nb ech charge - longueur bloc : %d  \n",(nb_donnees-longueur_bloc_5sec-dimension*tau+1));
	printf("nb longueur decalage bloc : %d  \n",longueur_decalage_bloc);
	nb_blocs_5s=(floor((nb_donnees-longueur_bloc_5sec-dimension*tau+1)>>8)+1);//  div par 256 (1sec)
	printf("nb blocs 5 sec :%ld\n",nb_blocs_5s);
	
	if (argc>6)
	{
		return;
	}
	
	// on va travailler avec les carrés de rayon 
	for (j=0;j<nb_rayons;j++)
	{
		printf("%f ->",rayons[j]);
		rayons[j]=rayons[j]*rayons[j];
		printf("%f\n",rayons[j]);
	}
	
	
    /* on prepare l'endroit ou stocker les densités ( ATTENTION : il ne s'agit pas d'une densité (probabilité) mais d'un nombre d'occurences 
	il s'agit de la matrice dont les données seront sauvées */
	Density=(unsigned long**)(malloc( nb_blocs_5s * sizeof(unsigned long*)));// PENSEZ au FREE
	for (i=0;i<nb_blocs_5s;i++)
	{
		Density[i]=(unsigned long*)(malloc( (nb_rayons+1) * sizeof(unsigned long)));// PENSEZ au FREE
		//printf("{%d=%d}",i,Density[i]);
		for (j=0;j<nb_rayons+1;j++)
			Density[i][j]=0;
	}
    printf("reservation Density : OK\n");
	printf("argc : %d\n",argc);
	if (argc>5) /* Si il a un cinquième paramètre c'est que l'on ne souhaite pas calculer sur tout le signal  */
	{
		nb_blocs_5s=nb_blocs_5s-atol(argv[5]);
	} 
	
	
	/* Boucle principale sur les blocs de 5 secondes avec un recouvrement de 4 secondes*/
	
	/* j'alloue une seule fois la table pour les distance */
	nbpoints_bloc_5s=longueur_bloc_5sec+dimension*tau;
	distances=(float*)(malloc((nbpoints_bloc_5s-param_theiller)*(nbpoints_bloc_5s-param_theiller+1)/2*sizeof(float)));
	/* verification faite ce n'est pas plus rapide qu'en allouant/desallouant pour chaque bloc ??? */
	
	for (numero_bloc=0;numero_bloc<nb_blocs_5s;numero_bloc++)
	{
		printf("[%d/%d:",numero_bloc,nb_blocs_5s);
		offset_sur_bloc_5s=numero_bloc*longueur_decalage_bloc;
		fin_du_bloc=offset_sur_bloc_5s+longueur_bloc_5sec+dimension*tau-1;
		//printf("%d-%d",offset_sur_bloc_5s,fin_du_bloc);
		if (fin_du_bloc>=nb_donnees)
			fin_du_bloc=nb_donnees-1;
		nbpoints_bloc_5s=fin_du_bloc-offset_sur_bloc_5s+1;
		printf("=%ld]",nbpoints_bloc_5s);
		
		//printf("reservation mémoire pour 'distances' : %d",(nbpoints_bloc_5s-param_theiller)*(nbpoints_bloc_5s-param_theiller+1)/2);
		compteur=0;
		for (i=0;i<nbpoints_bloc_5s-param_theiller;i++)
		{
			for (j=i+param_theiller;j<nbpoints_bloc_5s;j++)
			{
				distances[compteur]=0;
				for (k=0;k<dimension;k++)
				{	/* tmp = distance algébrique entre les points i et j selon la dimension k de l'espace des phases */
					tmp=(data[i+k*tau+offset_sur_bloc_5s]-data[j+k*tau+offset_sur_bloc_5s]);
					distances[compteur]=distances[compteur]+tmp*tmp;
				}
				//printf("~%f",distances[compteur]);
				compteur++;
				/* en sortie de cette boucle j'ai le carré de la distance */
			}	
		}
		
		for (j=0;j<compteur;j++)
		{
			//distances[j]=sqrt(distances[j]);
			/* initialement  j'ai le carré de la distance */
			/* je ne peux pas la comparer aux carrés des rayons car je travaille avec les entiers sur 32 bits */
			/* le carré des grands rayons depasse les 32 bits */
			/* la racine carrée accroît grandement le temps de calcul mais bon... maintenant j'ai la distance */
			for (numero_rayon=0;numero_rayon<nb_rayons;numero_rayon++)
				if (distances[j]<=rayons[numero_rayon])
					Density[numero_bloc][numero_rayon]++;	
			/* nombre total de points */			
			Density[numero_bloc][nb_rayons]=compteur;
		}						
				
		
		//printf("]");
	}
	printf("\nCalculs OK\n");
	free(distances);
	printf("Sauvegarde : ");
	for (numero_bloc=0;numero_bloc<nb_blocs_5s;numero_bloc++)
	{
		printf("#");
		for (numero_rayon=0;numero_rayon<nb_rayons;numero_rayon++)
			fprintf(fich_sortie,"%d;",Density[numero_bloc][numero_rayon]);
		fprintf(fich_sortie,"%d\n",Density[numero_bloc][nb_rayons]);	
	}
	fclose(fich_sortie);
	printf("\nSauvegarde terminée\n");
	
	free(data);
	printf("Liberation mémoire de 'data'\n");
	
	//for (i=nb_blocs_5s-1;i>-1;i--) // pas de liberation de Density[0] !!!!! sinon bug... mais pourquoi ?
	for (i=0;i<nb_blocs_5s;i++)
	{
		//printf("{%d=%d}",i,Density[i]);
		free(Density[i]);
	}
	free(Density);		
	
	printf("Liberation mémoire terminée\n");
	finish = clock();
	printf( "Temps de calcul : %2.1f secondes\n", (double)(finish - start) / CLOCKS_PER_SEC );
}
