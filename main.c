/********************************************************************************
*********************************************************************************

Programme de d�monstration de MinMax avec �lagage alpha/b�ta pour le jeu d'�checs
TP2 CRP - 2CSSID / ESI 2022/2023

Programme princial : main.c

Le programme comporte une interface textuelle permettant 2 types de jeu:
  1- utilisateur contre ordinateur
  2- ordinateur contre lui m�me

Cette version inclus :
  - Traitement de "l'effet horizon" en adaptant la profondeur d'exploration en
    fonction de la stabilit� des configuration
  - Maximiser les coupes alpha/beta � l'aide du "Move Ordering"
  - Influence des fonctions d'estimation

*********************************************************************************
********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>          // pour INT_MAX
#include "jeu.h"


/************************/
/* Variables Globales : */
/************************/

// Tableau de config pour garder la trace des conf d�j� visit�es
struct config Partie[MAXPARTIE];

// Fichier pour sauvegarder l'historique des parties
FILE *f;

// compteur de coups effectu�s
int num_coup = 0;
int avg_depth =0;
// profondeur initiale d'exploration pr�liminaire avant le tri des alternatives
int h0 = 0;

// tableau de pointeurs de fonctions (les fonctions d'estimations disponibles)
int (*Est[10])(struct config *);

// nb de fonctions d'estimation dans le tableau pr�c�dent
int nbEst;

int duree ;

// pour statistques sur le nombre de coupes effectu�es
int nbAlpha = 0;
int nbBeta = 0;




/*******************************************/
/*********** Programme principal  **********/
/*******************************************/

int main( int argc, char *argv[] )
{

   int n, i, j, score, stop, cout, hauteur, largeur, tour, estMin, estMax, nbp;


   int sx, dx, cout2, legal;
   int cmin, cmax;
   int typeExec, refaire;

   char coup[20] = "";
   char nomf[20];  // nom du fichier de sauvegarde
   char ch[100];
   char sy, dy;

   struct config T[100], conf, conf1;

// initialiser le tableau des fonctions d'estimation
   Est[0] = estim1;
   Est[1] = estim2;
   Est[2] = estim3;
   Est[3] = estim4;
   Est[4] = estim5;
   Est[5] = estim6;
   Est[6] = estim7;
   // Nombre de fonctions d'estimation disponibles
   nbEst = 7;

   // Choix du type d'ex�cution (pc-contre-pc ou user-contre-pc) ...
   printf("Type de parties (B:Blancs  N:Noirs) :\n");
   printf("1- PC(B)   contre PC(N)\n");
   printf("2- USER(N) contre PC(B)\n");
   printf("3- USER(B) contre PC(N)\n");
   printf("\tChoix : ");
   scanf(" %d", &typeExec);
   if (typeExec != 2 && typeExec != 3) typeExec = 1;

   // Choix des fonctions d'estimation � utiliser ...
   do {
      printf("\nLes fonctions d'estimations disponibles sont:\n");
      printf("1- bas�e uniquement sur le nombre de pi�ces\n");
      printf("2- bas�e sur le nb de pieces, l'occupation, la d�fense du roi et les roques\n");
      printf("3- bas�e sur le nb de pi�ces et une perturbation al�atoire\n");
      printf("4- bas�e sur le nb de pieces et les menaces\n");
      printf("5- bas�e sur le nb de pieces et l'occupation\n");
      printf("6- bas�e sur une combinaisant de 3 estimations: (2 -> 5 -> 4)\n");
      printf("7- bas�e sur le nb de pieces et leur positions \n\n");
      if (typeExec != 3) {
         printf("Donnez la fonction d'estimation utilis�e par le PC pour le joueur B : ");
         scanf(" %d", &estMax);
      }
      else
         estMax = 7;
      if (typeExec != 2) {
         printf("Donnez la fonction d'estimation utilis�e par le PC pour le joueur N : ");
         scanf(" %d", &estMin);
      }
      else
         estMin = 7;
   }
   while ( estMax < 1 || estMax > nbEst || estMin < 1 || estMin > nbEst );

   estMax--;
   estMin--;

   printf("\n--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );

   printf("\n'B' : joueur maximisant et 'N' : joueur minimisant\n");
   if (typeExec == 1)
      printf("\nPC 'B' contre PC 'N'\n\n");
   if (typeExec == 2)
      printf("\nUSER 'N' contre PC 'B'\n\n");
   if (typeExec == 3)
      printf("\nUSER 'B' contre PC 'N'\n\n");

   printf("Param�tres de MinMax\n");

   printf("Le temps sp�cifi� a chaque joueur (s) : ");
   scanf("%d", &duree);

   printf("Nombre d'alternatives maximum � consid�rer dans chaque coup (0 pour +infini) : ");
   scanf(" %d", &largeur);
   if ( largeur == 0 ) largeur = +INFINI;

   // Initialise la configuration de d�part
   init( &conf );
   for (i=0; i<MAXPARTIE; i++)
              copier( &conf, &Partie[i] );

   num_coup = 0;

   // initialise le g�n�rateur de nombre al�atoire pour la fonction estim3(...) si elle est utilis�e
   srand( time(NULL) );

   // sauter la fin de ligne des lectures pr�c�dentes
   while ((i = getchar()) != '\n' && i != EOF) { }

   printf("\nNom du fichier texte o� sera sauvegarder la partie : ");
   fgets(ch, 20, stdin);
   sscanf(ch," %s", nomf);
   f = fopen(nomf, "w");

   fprintf(f, "--- Estimation_pour_Blancs = %d \t Estimation_pour_Noirs = %d ---\n", \
           (typeExec != 3 ? estMax+1 : 0), (typeExec != 2 ? estMin+1 : 0) );

   printf("\n---------------------------------------------\n\n");

   // Boucle principale du d�roulement d'une partie ...
   stop = 0;
   tour = MAX;            // le joueur MAX commence en premier
   nbAlpha = nbBeta = 0;  // les compteurs de coupes alpha et beta

   while ( !stop ) {

      affich( &conf, coup, num_coup );
      copier( &conf, &Partie[num_coup % MAXPARTIE] );        // rajouter conf au tableau 'Partie'
      sauvConf( &conf );
      refaire = 0;  // indicateur de coup illegal, pour refaire le mouvement

      if ( tour == MAX ) {   // au tour de MAX ...

         if (typeExec == 3) {   // c-a-d MAX ===> USER
            printf("Au tour du joueur maximisant USER 'B'\n");
            // r�cup�rer le coup du joueur ...
            do {
               printf("Donnez srcY srcX destY destX (par exemple d2d3) : ");
               fgets(ch, 100, stdin);
               i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
               if (i != 4)
                  printf("Lecture incorrecte, recommencer ...\n");
            }
            while ( i != 4 );

            copier(&conf, &conf1);

            // Traitement du coup du joueur ...

            if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy + 2) { // petit roque ...
               conf1.mat[0][4] = 0;
               conf1.mat[0][7] = 0;
               conf1.mat[0][6] = 'r';
               conf1.xrB = 0; conf1.yrB = 6;
               conf1.mat[0][5] = 't';
               conf1.roqueB = 'e';
            }
            else
               if (sx == conf.xrB+1 && sy-'a' == conf.yrB && dy == sy - 2) { // grand roque ...
                  conf1.mat[0][4] = 0;
                  conf1.mat[0][0] = 0;
                  conf1.mat[0][2] = 'r';
                  conf1.xrB = 0; conf1.yrB = 2;
                  conf1.mat[0][3] = 't';
                  conf1.roqueB = 'e';
               }
               else  {  // deplacement normal (les autres coups) ...
                  conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
                  conf1.mat[sx-1][sy-'a'] = 0;
                  // v�rifier possibilit� de transformation d'un pion arriv� en fin d'�chiquier ...
                  if (dx == 8 && conf1.mat[dx-1][dy-'a'] == 'p') {
                     printf("Pion arriv� en ligne 8, transformer en (p/c/f/t/n) : ");
                     scanf(" %s", ch);
                     switch (ch[0]) {
                        case 'c' : conf1.mat[dx-1][dy-'a'] = 'c'; break;
                        case 'f' : conf1.mat[dx-1][dy-'a'] = 'f'; break;
                        case 't' : conf1.mat[dx-1][dy-'a'] = 't'; break;
                        case 'p' : conf1.mat[dx-1][dy-'a'] = 'p'; break;
                        default  : conf1.mat[dx-1][dy-'a'] = 'n';
                     }
                  }
               }

            // v�rifier si victoire (le roi N n'existe plus) ...
            if ( conf1.xrN == dx-1 && conf1.yrN == dy-'a' ) {
               conf1.xrN = -1;
               conf1.yrN = -1;
            }

            // v�rification de la l�galit� du coup effectu� par le joueur ...
            generer_succ(  &conf, MAX, T, &n );

            legal = 0;
            for (i=0; i<n && !legal; i++)
                if ( egal(T[i].mat, conf1.mat) )  legal = 1;

            if ( legal && !feuille(&conf1,&cout) ) {
               printf("OK\n\n");
               i--;
               formuler_coup( &conf, &T[i], coup );
               copier( &T[i], &conf );
            }
            else
               if ( !legal && n > 0 ) {
                  printf("Coup ill�gal (%c%d%c%d) -- r�essayer\n", sy,sx,dy,dx);
                  refaire = 1; // pour forcer la prochaine it�ration � rester MAX
               }
               else
                  stop = 1;
         } // if (typeExec == 3)

         else { // MAX ===> PC

            printf("Au tour du joueur maximisant PC 'B' "); fflush(stdout);

            generer_succ(  &conf, MAX, T, &n );
            nbp = npieces( &conf );
            printf("  hauteur = %d    nb alternatives = %d : \n", hauteur, n); fflush(stdout);
            avg_depth +=hauteur;
            // Iterative Deepening ...
            // On effectue un tri sur les alternatives selon l'estimation de leur qualit�
            // Le but est d'explorer les alternatives les plus prometteuses d'abord
            // pour maximiser les coupes lors des �valuation minmax avec alpha-b�ta

            // 1- on commence donc par une petite exploration de profondeur h0
            //    pour r�cup�rer des estimations plus pr�cises sur chaque coups:
            for (i=0; i<n; i++)
                T[i].val = minmax_ab( &T[i], MIN, h0, -INFINI, +INFINI, largeur, estMax, nbp, +INFINI );

            // 2- on r�alise le tri des alternatives T suivant les estimations r�cup�r�es:
            qsort(T, n, sizeof(struct config), confcmp321);   // en ordre d�croissant des �valuations
             //printf("n = %d\n", n);

            if ( largeur < n ) n = largeur;

            // 3- on lance l'exploration des alternatives tri�es avec la profondeur voulue:
            score = -INFINI;
            j = -1;


            int hcc = 1;
            int now = clock()/CLOCKS_PER_SEC;

           int cc = now+duree;
            bool stop_time = false;
            while (!stop_time&&n!=0){
            for (i=0;i<n;i++){

                cout = minmax_ab( &T[i], MIN, hcc, score, +INFINI, largeur, estMax, nbp, cc );
                T[i].val = cout;
                if ( cout > score ) {  // Choisir le meilleur coup (c-a-d le plus grand score)
                           score = cout;
                           j = i;
                    }
                if ( cout == 100 ) {
                           printf("v"); fflush(stdout);
                           stop_time=true;
                           break;
                            }
              if (clock()/CLOCKS_PER_SEC>now+duree) {stop_time = true ; break;}

            }


            /*
            Avant de passer d�un niveau � un autre, on essaie d�ordonner le tableau des configuration selon
        leur score obtenu au niveau pr�c�dent et donc elles soient ordonn�s du plus prometteur au moins prometteur
            */
            qsort(T, n, sizeof(struct config), confcmp321);
            hcc++;
            }

            hauteur = hcc;



            if ( j != -1 ) { // jouer le coup et aller � la prochaine it�ration ...
               printf(" choix=%d (score:%d)\n", j+1, score);
               printf("\n");
               formuler_coup( &conf, &T[j], coup );
               copier( &T[j], &conf );
               conf.val = score;
            }
            else   // S'il n'y a pas de successeur possible, le joueur MAX � perdu
               stop = 1;
         } // fin else // MAX ===> PC

         if (stop) {
            printf("\n *** le joueur maximisant 'B' a perdu ***\n");
            fprintf(f, "Victoire de 'N'\n");
         }

      }  // if ( tour == MAX )

      else {  // donc tour == MIN

         if (typeExec == 2) {   // c-a-d MIN ===> USER
            printf("Au tour du joueur minimisant USER 'N'\n");
            // r�cup�rer le coup du joueur ...
            do {
               printf("Donnez srcY srcX destY destX (par exemple d7d5) : ");
               fgets(ch, 100, stdin);
               i = sscanf(ch, " %c %d %c %d", &sy, &sx, &dy, &dx );
               if (i != 4)
                  printf("Lecture incorrecte, recommencer ...\n");
            }
            while ( i != 4 );

            copier(&conf, &conf1);

            // Traitement du coup du joueur ...

            if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy + 2) { // petit roque ...
               conf1.mat[7][4] = 0;
               conf1.mat[7][7] = 0;
               conf1.mat[7][6] = -'r';
               conf1.xrN = 7; conf1.yrN = 6;
               conf1.mat[7][5] = -'t';
               conf1.roqueN = 'e';
            }
            else
               if (sx == conf.xrN+1 && sy-'a' == conf.yrN && dy == sy - 2) { // grand roque ...
                  conf1.mat[7][4] = 0;
                  conf1.mat[7][0] = 0;
                  conf1.mat[7][2] = -'r';
                  conf1.xrN = 7; conf1.yrN = 2;
                  conf1.mat[7][3] = -'t';
                  conf1.roqueN = 'e';
               }
               else  {  // deplacement normal (les autres coups) ...
                  conf1.mat[dx-1][dy-'a'] = conf1.mat[sx-1][sy-'a'];
                  conf1.mat[sx-1][sy-'a'] = 0;
                  // v�rifier possibilit� de transformation d'un pion arriv� en fin d'�chiquier ...
                  if (dx == 1 && conf1.mat[dx-1][dy-'a'] == -'p') {
                     printf("Pion arriv� en ligne 8, transformer en (p/c/f/t/n) : ");
                     scanf(" %s", ch);
                     switch (ch[0]) {
                       case 'c' : conf1.mat[dx-1][dy-'a'] = -'c'; break;
                       case 'f' : conf1.mat[dx-1][dy-'a'] = -'f'; break;
                       case 't' : conf1.mat[dx-1][dy-'a'] = -'t'; break;
                       case 'p' : conf1.mat[dx-1][dy-'a'] = -'p'; break;
                       default  : conf1.mat[dx-1][dy-'a'] = -'n';
                     }
                  }
               }

            // v�rifier si victoire (le roi B n'existe plus) ...
            if ( conf1.xrB == dx-1 && conf1.yrB == dy-'a' ) {
               conf1.xrB = -1;
               conf1.yrB = -1;
            }

            // v�rification de la l�galit� du coup effectu� par le joueur ...
            generer_succ(  &conf, MIN, T, &n );

            legal = 0;
            for (i=0; i<n && !legal; i++)
                if ( egal(T[i].mat, conf1.mat) )  legal = 1;

            if ( legal && !feuille(&conf1,&cout) ) {
               printf("OK\n\n");
               i--;
               formuler_coup( &conf, &T[i], coup );
               copier( &T[i], &conf );
            }
            else
               if ( !legal && n > 0 ) {
                  printf("Coup ill�gal (%c%d%c%d) -- r�essayer\n", sy,sx,dy,dx);
                  refaire = 1; // pour forcer la prochaine it�ration � rester MIN
               }
               else
                  stop = 1;
         } // if (typeExec == 2)

         else { // MIN ===> PC

            printf("Au tour du joueur minimisant PC 'N' "); fflush(stdout);

            // G�n�rer tous les coups possibles pour le joueur N dans le tableau T
            generer_succ(  &conf, MIN, T, &n );
            nbp = npieces( &conf);
            printf("  hauteur = %d    nb alternatives = %d : \n", hauteur, n); fflush(stdout);
            avg_depth +=hauteur;
            // Iterative Deepening ...
            // On effectue un tri sur les alternatives selon l'estimation de leur qualit�
            // Le but est d'explorer les alternatives les plus prometteuses d'abord
            // pour maximiser les coupes lors des �valuation minmax avec alpha-b�ta

            // 1- on commence donc par une petite exploration de profondeur h0
            //    pour r�cup�rer des estimations plus pr�cises sur chaque coups:
            for (i=0; i<n; i++)
                T[i].val = minmax_ab( &T[i], MAX, h0, -INFINI, +INFINI, largeur, estMin, nbp,+INFINI );

            // 2- on r�alise le tri des alternatives T suivant les estimations r�cup�r�es:
            qsort(T, n, sizeof(struct config), confcmp123);   // en ordre croissant des �valuations
            if ( largeur < n ) n = largeur;

            // 3- on lance l'exploration des alternatives tri�es avec la profondeur voulue:
            score = +INFINI;
            j = -1;

              int hcc = 1;
            int now = clock()/CLOCKS_PER_SEC;
            int cc = now+duree;

            bool stop_time = false;
            while (!stop_time&&n!=0){
            for (i=0;i<n;i++){

                cout = minmax_ab( &T[i], MAX, hcc, -INFINI, score, largeur, estMin, nbp,cc );
                T[i].val = cout;
                 if ( cout < score ) {  // Choisir le meilleur coup (c-a-d le plus petit score)
                   score = cout;
                   j = i;
                }
                if ( cout == -100 ) {
                   printf("v"); fflush(stdout);
                   stop_time=true;
                   break;
                }
              if (clock()/CLOCKS_PER_SEC>now+duree) {stop_time = true ; break;}

            }
            /*
            Avant de passer d�un niveau � un autre, on essaie d�ordonner le tableau des configuration selon
        leur score obtenu au niveau pr�c�dent et donc elles soient ordonn�s du plus prometteur au moins prometteur
            */
            qsort(T, n, sizeof(struct config), confcmp123);
            hcc++;
            }
            hauteur = hcc;

            if ( j != -1 ) { // jouer le coup et aller � la prochaine it�ration ...
               printf(" choix=%d (score:%d)\n", j+1, score);
               printf("\n");
               formuler_coup( &conf, &T[j], coup );
               copier( &T[j], &conf );
               conf.val = score;
            }
            else  // S'il n'y a pas de successeur possible, le joueur MIN � perdu
               stop = 1;

         } // fin else // MIN ===> PC

         if (stop) {
            printf("\n *** le joueur minimisant 'N' a perdu ***\n");
            fprintf(f, "Victoire de 'B'\n");
         }

      } // fin else // tour == MIN

      if ( !refaire ) {
         num_coup++;
         tour = ( tour == MIN ? MAX : MIN );
      }

   } // fin while ( !stop )

   printf("\nNb de coupes (alpha:%d + beta:%d)  = %d", nbAlpha, nbBeta,nbAlpha+nbBeta);
    float moy = avg_depth/num_coup;
   printf("\n L'hauteur moyenne = %f", moy);
   printf("\nFin de partie\n");

   return 0;

} // fin de main

