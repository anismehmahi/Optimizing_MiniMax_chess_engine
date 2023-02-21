
/********************************************************************************
*********************************************************************************

Programme de d�monstration de MinMax avec �lagage alpha/b�ta pour le jeu d'�checs
TP2 CRP - 2CSSID / ESI 2022/2023

Impl�mentation du module : jeu.c

Les fonctions sont regroup�es en 4 parties :
      - Partie : Minmax avec Alpha/Beta
      - Partie : Evaluations et Estimations
      - Partie : G�n�ration des alternatives
      - Partie : Fonctions utilitaires

*********************************************************************************
*********************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>          // pour INT_MAX
#include "jeu.h"



/*******************************************************/
/*** Les variables globales d�finies dans "main.c"   ***/
/*******************************************************/

// Tableau de config pour garder la trace des conf d�j� visit�es
extern struct config Partie[MAXPARTIE];

// Fichier pour sauvegarder l'historique des parties
extern FILE *f;

// compteur de coups effectu�s
extern int num_coup;

// profondeur initiale d'exploration pr�liminaire avant le tri des alternatives
extern int h0;

// tableau de pointeurs de fonctions (les fonctions d'estimations disponibles)
extern int (*Est[10])(struct config *);

// nb de fonctions d'estimation dans le tableau pr�c�dent
extern int nbEst;

// pour statistques sur le nombre de coupes effectu�es
extern int nbAlpha;
extern int nbBeta;


/*********************************************************/
/*** Les variables internes (static) du module "jeu.c" ***/
/*********************************************************/

// vecteurs pour g�n�rer les diff�rents d�placements par type de pi�ce ...
//    cavalier :
static int dC[8][2] = { {-2,+1} , {-1,+2} , {+1,+2} , {+2,+1} , {+2,-1} , {+1,-2} , {-1,-2} , {-2,-1} };
//    fou (indices impairs), tour (indices pairs), reine et roi (indices pairs et impairs):
static int D[8][2] = { {+1,0} , {+1,+1} , {0,+1} , {-1,+1} , {-1,0} , {-1,-1} , {0,-1} , {+1,-1} };


// mirror of table (player N )
const int mirror [7][64] = {
{
// Pawns - early/mid
0, 0, 0, 0, 0, 0, 0, 0,
5, 10, 10, -20, -20, 10, 10, 5,
5, -5, -10, 0, 0, -10, -5, 5,
0, 0, 0, 20, 20, 0, 0, 0,
5, 5, 10, 25, 25, 10, 5, 5,
10, 10, 20, 30, 30, 20, 10, 10,
50, 50, 50, 50, 50, 50, 50, 50,
0, 0, 0, 0, 0, 0, 0, 0
},{
// Knights - early/mid
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  5,  5,  0,-20,-40,
-30,  5, 10, 15, 15, 10,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 10, 15, 15, 10,  0,-30,
-40,-20,  0,  0,  0,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50,
},{
// Bishops - early/mid
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  5,  0,  0,  0,  0,  5,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  0,  0,  0,  0,  0,  0,-10,
-20,-10,-10,-10,-10,-10,-10,-20,

},{
// Rooks - early/mid
  0,  0,  0,  5,  5,  0,  0,  0,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  5, 10, 10, 10, 10, 10, 10,  5,
  0,  0,  0,  0,  0,  0,  0,  0,
},{
// Queens - early/mid
},
{
// Kings - mid
 20, 30, 10,  0,  0, 10, 30, 20,
 20, 20,  0,  0,  0,  0, 20, 20,
-10,-20,-20,-20,-20,-20,-20,-10,
-20,-30,-30,-40,-40,-30,-30,-20,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,

},
{
    // king end game
-50,-30,-30,-30,-30,-30,-30,-50,
-30,-30,  0,  0,  0,  0,-30,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-20,-10,  0,  0,-10,-20,-30,
-50,-40,-30,-20,-20,-30,-40,-50,

}

};




// evaluation table by position (player B)
const int piece_square_table [7][64] = {
{
// Pawns - early/mid
 0,  0,  0,  0,  0,  0,  0,  0,
50, 50, 50, 50, 50, 50, 50, 50,
10, 10, 20, 30, 30, 20, 10, 10,
 5,  5, 10, 25, 25, 10,  5,  5,
 0,  0,  0, 20, 20,  0,  0,  0,
 5, -5,-10,  0,  0,-10, -5,  5,
 5, 10, 10,-20,-20, 10, 10,  5,
 0,  0,  0,  0,  0,  0,  0,  0
},{
// Knights - early/mid
-50,-40,-30,-30,-30,-30,-40,-50,
-40,-20,  0,  0,  0,  0,-20,-40,
-30,  0, 10, 15, 15, 10,  0,-30,
-30,  5, 15, 20, 20, 15,  5,-30,
-30,  0, 15, 20, 20, 15,  0,-30,
-30,  5, 10, 15, 15, 10,  5,-30,
-40,-20,  0,  5,  5,  0,-20,-40,
-50,-40,-30,-30,-30,-30,-40,-50,
},{
// Bishops - early/mid
-20,-10,-10,-10,-10,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5, 10, 10,  5,  0,-10,
-10,  5,  5, 10, 10,  5,  5,-10,
-10,  0, 10, 10, 10, 10,  0,-10,
-10, 10, 10, 10, 10, 10, 10,-10,
-10,  5,  0,  0,  0,  0,  5,-10,
-20,-10,-10,-10,-10,-10,-10,-20,

},{
// Rooks - early/mid
  0,  0,  0,  0,  0,  0,  0,  0,
  5, 10, 10, 10, 10, 10, 10,  5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
 -5,  0,  0,  0,  0,  0,  0, -5,
  0,  0,  0,  5,  5,  0,  0,  0
},{
// Queens - early/mid
-20,-10,-10, -5, -5,-10,-10,-20,
-10,  0,  0,  0,  0,  0,  0,-10,
-10,  0,  5,  5,  5,  5,  0,-10,
 -5,  0,  5,  5,  5,  5,  0, -5,
  0,  0,  5,  5,  5,  5,  0, -5,
-10,  5,  5,  5,  5,  5,  0,-10,
-10,  0,  5,  0,  0,  0,  0,-10,
-20,-10,-10, -5, -5,-10,-10,-20
},
{
// Kings - mid
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20

},
{
    // king end game
-50,-40,-30,-20,-20,-30,-40,-50,
-30,-20,-10,  0,  0,-10,-20,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-30,  0,  0,  0,  0,-30,-30,
-50,-30,-30,-30,-30,-30,-30,-50

}

};



/***********************************************************************************/


// ******************************
// Partie:  MinMax avec AlphaBeta
// ******************************


/* MinMax avec �lagage alpha-beta :
 Evalue la configuration 'conf' du joueur 'mode' en descendant de 'niv' niveaux.
 Le param�tre 'niv' est decr�ment� � chaque niveau (appel r�cursif).
 'alpha' et 'beta' repr�sentent les bornes initiales de l'intervalle d'int�r�t
 (pour pouvoir effectuer les coupes alpha et b�ta).
 'largeur' repr�sente le nombre max d'alternatives � explorer en profondeur � chaque niveau.
 Si 'largeur == +INFINI' toutes les alternatives seront prises en compte
 (c'est le comportement par d�faut).
 'numFctEst' est le num�ro de la fonction d'estimation � utiliser lorsqu'on arrive � la
 fronti�re d'exploration (c-a-d 'niv' atteint 0)
 npp : le nombre de pieces dans la conf parente (pour verifier si conf est stable ou non)
*/
int minmax_ab( struct config *conf, int mode, int niv, int alpha, int beta, int largeur, int numFctEst, int npp , int after)
{



int now = clock()/CLOCKS_PER_SEC;
    if (now > after)
    {
    //printf("now  = %d  , after = %d \n", now,after );
         return Est[numFctEst]( conf );	// evaluer par estimation de conf
    }


   int n, i, score, score2, npc;
   struct config T[100];

   // npc: le nombre de pieces dans conf
   // npp: le nombre de pieces dans le parent de conf
   npc = npieces(conf);

   if ( feuille(conf, &score) )
      return score;

   if ( niv == 0 ) {	// si le niveau souhait� est atteint
      			// traitement partiel (et simpliste) de l'effet horizon :
      if ( npp == npc  || npp == 0) {	// si conf est stable ou alors on a deja ajout� un niveau
         return Est[numFctEst]( conf );	// evaluer par estimation de conf
      }
      else {				// sinon
         niv = 1;			// ajouter un niveau supplementaire dans l'exploration
         npc = 0;			// et ne plus considerer la stabilit� de la prochaine conf
      }

   }
//if (clock()/CLOCKS_PER_SEC<after){
   if ( mode == MAX ) {

      generer_succ( conf, MAX, T, &n );

      if ( largeur != +INFINI ) {
         for (i=0; i<n; i++)
             T[i].val = Est[numFctEst]( &T[i] );

	 // trier en ordre decroissant les successeurs selon leurs estimations
         qsort(T, n, sizeof(struct config), confcmp321);
         if ( largeur < n ) n = largeur;   // pour limiter la largeur d'exploration
      }

      score = alpha;
      for ( i=0; i<n; i++ ) {
              score2 = minmax_ab( &T[i], MIN, niv-1, score, beta, largeur, numFctEst, npc,after);
          if (score2 > score) score = score2;
             if (score >= beta) {
                // Coupe Beta
                nbBeta++;       // compteur de coupes beta
                return score;   // evaluation tronquee ***
             }
          }
   }

   else  { // mode == MIN

      generer_succ( conf, MIN, T, &n );

      if ( largeur != +INFINI ) {
         for (i=0; i<n; i++)
             T[i].val = Est[numFctEst]( &T[i] );

	 // trier en ordre croissant les successeurs selon leurs estimations
         qsort(T, n, sizeof(struct config), confcmp123);
         if ( largeur < n ) n = largeur;   // pour limiter la largeur d'exploration
      }

      score = beta;
      for ( i=0; i<n; i++ ) {
          score2 = minmax_ab( &T[i], MAX, niv-1, alpha, score, largeur, numFctEst, npc,after );
          if (score2 < score) score = score2;
             if (score <= alpha) {
                // Coupe Alpha
                nbAlpha++;      // compteur de coupes alpha
                return score;   // evaluation tronquee ***
             }
          }
      }

      if ( score == +INFINI ) score = +100;
      if ( score == -INFINI ) score = -100;

      return score;
   //  }
} // fin de minmax_ab

/***********************************************************************************/



// ***********************************
// Partie:  Evaluations et Estimations
// ***********************************


/* Teste si conf repr�sente une fin de partie et retourne dans 'cout' la valeur associ�e */
int feuille( struct config *conf, int *cout )
{

   *cout = 0;

   // Si victoire pour les Noirs cout = -100
   if ( conf->xrB == -1 ) {
      *cout = -100;
      return 1;
   }

   // Si victoire pour les Blancs cout = +100
   if ( conf->xrN == -1 ) {
      *cout = +100;
      return 1;
   }

   // Si Match nul cout = 0
   if (  conf->xrB != -1 &&  conf->xrN != -1 && AucunCoupPossible( conf ) )
      return 1;

   // Sinon ce n'est pas une config feuille
   return 0;

}  // fin de feuille


/* Quelques exemples de fonctions d'estimation simples (estim1, estim2, ...) */
/* Voir fonction estim plus bas pour le choix l'estimation � utiliser */

/* cette estimation est bas�e uniquement sur le nombre de pi�ces */
int estim1( struct config *conf )
{

        int i, j, ScrQte;
        int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

        // parties : nombre de pi�ces
        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                switch (conf->mat[i][j]) {
                   case 'p' : pionB++;   break;
                   case 'c' :
                   case 'f' : cfB++;  break;
                   case 't' : tB++; break;
                   case 'n' : nB++;  break;

                   case -'p' : pionN++;  break;
                   case -'c' :
                   case -'f' : cfN++;  break;
                   case -'t' : tN++; break;
                   case -'n' : nN++;  break;
                }
           }

        // Somme pond�r�e de pi�ces de chaque joueur.
        // Les poids sont fix�s comme suit: pion:2  cavalier/fou:6  tour:8  et  reine:20
        // Le facteur 100/76 pour ne pas sortir de l'intervalle ]-100 , +100[
        ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) ) * 100.0/76;

        if (ScrQte > 95) ScrQte = 95;                // pour r�tr�cir l'intervalle �
        if (ScrQte < -95) ScrQte = -95;                // ]-95 , +95[ car ce n'est qu'une estimation

        return ScrQte;

} // fin de estim1


// estimation bas�e sur le nb de pieces, l'occupation, la d�fense du roi et les roques
int estim2(  struct config *conf )
{
        int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, Score;
        int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
        int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;

        // parties : nombre de pi�ces et occupation du centre
        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
                if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
                if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
                switch (conf->mat[i][j]) {
                   case 'p' : pionB++; occCentreB += bns;  break;
                   case 'c' :
                   case 'f' : cfB++; occCentreB += 4*bns; break;
                   case 't' : tB++; break;
                   case 'n' : nB++; occCentreB += 4*bns; break;

                   case -'p' : pionN++; occCentreN += bns; break;
                   case -'c' :
                   case -'f' : cfN++; occCentreN += 4*bns; break;
                   case -'t' : tN++; break;
                   case -'n' : nN++; occCentreN += 4*bns; break;
                }
           }

        ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );
        // donc ScrQteMax ==> 76

        ScrDisp = occCentreB - occCentreN;
        // donc ScrDispMax ==> 42

        // partie : d�fense du roi B ...
        for (i=0; i<8; i += 1) {
           // traitement des 8 directions paires et impaires
           stop = 0;
           a = conf->xrB + D[i][0];
           b = conf->yrB + D[i][1];
           while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                if ( conf->mat[a][b] != 0 )  stop = 1;
                else {
                    a = a + D[i][0];
                    b = b + D[i][1];
                }
           if ( stop )
                if ( conf->mat[a][b] > 0 ) protectRB++;
        } // for

        // partie : d�fense du roi N ...
        for (i=0; i<8; i += 1) {
           // traitement des 8 directions paires et impaires
           stop = 0;
           a = conf->xrN + D[i][0];
           b = conf->yrN + D[i][1];
           while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                if ( conf->mat[a][b] != 0 )  stop = 1;
                else {
                    a = a + D[i][0];
                    b = b + D[i][1];
                }
           if ( stop )
                if ( conf->mat[a][b] < 0 ) protectRN++;
        } // for

        ScrDfs = protectRB - protectRN;
        // donc ScrDfsMax ==> 8

        // Partie : autres consid�rations ...
        if ( conf->roqueB == 'e' ) divB = 24;        // favoriser les roques B
        if ( conf->roqueB == 'r' ) divB = 12;
        if ( conf->roqueB == 'p' || conf->roqueB == 'g' ) divB = 10;

        if ( conf->roqueN == 'e' ) divN = 24;        // favoriser les roques N
        if ( conf->roqueN == 'r' ) divN = 12;
        if ( conf->roqueN == 'p' || conf->roqueN == 'g' ) divN = 10;

        ScrDivers = divB - divN;
        // donc ScrDiversMax ==> 24

        Score = (4*ScrQte + ScrDisp + ScrDfs + ScrDivers) * 100.0/(4*76+42+8+24);
        // pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

        return Score;

} // fin de estim2


/* prise en compte du nombre de pi�ces avec petite perturbation al�atoire */
int estim3( struct config *conf )
{

        int i, j, ScrQte, Score;
        int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

        // parties : nombre de pi�ces
        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                switch (conf->mat[i][j]) {
                   case 'p' : pionB++; break;
                   case 'c' :
                   case 'f' : cfB++; break;
                   case 't' : tB++; break;
                   case 'n' : nB++; break;

                   case -'p' : pionN++; break;
                   case -'c' :
                   case -'f' : cfN++; break;
                   case -'t' : tN++; break;
                   case -'n' : nN++; break;
                }
           }

        ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );
        // donc ScrQteMax ==> 76

        Score = (10*ScrQte + rand()%10) * 100.0 / (10*76+10);
        // pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

        return Score;

} // fin de estim3


// estimation bas�e sur le nb de pieces et les menaces
int estim4( struct config *conf )
{

        int i, j, Score;
        int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
        int npmB = 0, npmN = 0;

        // parties : nombre de pi�ces
        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                switch (conf->mat[i][j]) {
                   case 'p' : pionB++;   break;
                   case 'c' :
                   case 'f' : cfB++;  break;
                   case 't' : tB++; break;
                   case 'n' : nB++;  break;

                   case -'p' : pionN++;  break;
                   case -'c' :
                   case -'f' : cfN++;  break;
                   case -'t' : tN++; break;
                   case -'n' : nN++;  break;
                }
           }

        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                if ( conf->mat[i][j] < 0 && caseMenaceePar(MAX, i, j, conf) ) {
                   npmB++;
                   if ( conf->mat[i][j] == -'c' || conf->mat[i][j] == -'f' )
                           npmB++;
                   if ( conf->mat[i][j] == -'t' || conf->mat[i][j] == -'n' )
                           npmB += 2;
                   if ( conf->mat[i][j] == -'r' )
                           npmB += 5;
                }
                if ( conf->mat[i][j] > 0 && caseMenaceePar(MIN, i, j, conf) ) {
                   npmN++;
                   if ( conf->mat[i][j] == 'c' || conf->mat[i][j] == 'f' )
                           npmN++;
                   if ( conf->mat[i][j] == 't' || conf->mat[i][j] == 'n' )
                           npmN += 2;
                   if ( conf->mat[i][j] == 'r' )
                           npmN += 5;
                }
           }

        Score = ( 4*((pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20)) + \
                  (npmB - npmN) ) * 100.0/(4*76+31);

        // pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 95) Score = 95;                // pour r�tr�cir l'intervalle �
        if (Score < -95) Score = -95;                // ]-95 , +95[ car ce n'est qu'une estimation

        return Score;

} // fin de estim4


// estimation bas�e sur le nb de pieces et l'occupation
int estim5(  struct config *conf )
{
        int i, j, a, b, stop, bns, ScrQte, ScrDisp, ScrDfs, ScrDivers, Score;
        int pionB = 0, pionN = 0, cfB = 0, cfN = 0, tB = 0, tN = 0, nB = 0, nN = 0;
        int occCentreB = 0, occCentreN = 0, protectRB = 0, protectRN = 0, divB = 0, divN = 0;

        // parties : nombre de pi�ces et occupation du centre
        for (i=0; i<8; i++)
           for (j=0; j<8; j++) {
                bns = 0;  // bonus pour l'occupation du centre de l'�chiquier
                if (i>1 && i<6 && j>=0 && j<=7 ) bns = 1;
                if (i>2 && i<5 && j>=2 && j<=5 ) bns = 2;
                switch (conf->mat[i][j]) {
                   case 'p' : pionB++; occCentreB += bns;  break;
                   case 'c' :
                   case 'f' : cfB++; occCentreB += 4*bns; break;
                   case 't' : tB++; break;
                   case 'n' : nB++; occCentreB += 4*bns; break;

                   case -'p' : pionN++; occCentreN += bns; break;
                   case -'c' :
                   case -'f' : cfN++; occCentreN += 4*bns; break;
                   case -'t' : tN++; break;
                   case -'n' : nN++; occCentreN += 4*bns; break;
                }
           }

        ScrQte = ( (pionB*2 + cfB*6 + tB*8 + nB*20) - (pionN*2 + cfN*6 + tN*8 + nN*20) );

        ScrDisp = occCentreB - occCentreN;

        Score = (4*ScrQte + ScrDisp) * 100.0/(4*76+42);
        // pour les poids des pi�ces et le facteur multiplicatif voir commentaire dans estim1

        if (Score > 98 ) Score = 98;
        if (Score < -98 ) Score = -98;

        return Score;

} // fin de estim5


/* on peut aussi combiner entre plusieurs estimation comme cet exemple: */
int estim6( struct config *conf )
{
        // Combinaison de 3 fonctions d'estimation:
        // -au d�but on utilise estim2 pour favoriser l'occupation du centre et la defense du roi
        // -au milieu on utilise estim5 pour continuer � bien se positionner
        // -dans la derni�re phase on utilise estim4 pour favoriser l'attaque
        if ( num_coup < 25 )                        // 1ere phase
           return estim2(conf);
        if ( num_coup >= 25 && num_coup < 35 )        // 2e phase
           return estim5(conf);
        if ( num_coup >= 35 )                        // 3e phase
           return estim4(conf);

} // fin de estim6


/* Une fonction d'estimation vide */
int estim7( struct config *conf )
{
	int i, j=0;
	float ScrQte=0;
	int pionB = 0, pionN = 0, cB = 0, cN = 0, fB = 0, fN = 0, tB = 0, tN = 0, nB = 0, nN = 0,rB=0,rN=0;


	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {

		switch (conf->mat[i][j]) {
           case 'p' : pionB++;
                ScrQte += piece_square_table[0][i*8+j];
                break;
		   case 'c' : cB++;
                ScrQte += piece_square_table[1][i*8+j];
                break;
		   case 'f' : fB++;
                ScrQte += piece_square_table[2][i*8+j];
                break;
		   case 't' : tB++;
                ScrQte += piece_square_table[3][i*8+j];
                break;
		   case 'n' : nB++;
                ScrQte += piece_square_table[4][i*8+j];
                break;
           case 'r' : rB++;
                ScrQte += piece_square_table[5][i*8+j];
                if (nN+nB==0){
                    ScrQte += piece_square_table[6][conf->xrB*8+conf->yrB];
	   }
	   else{ // mid - early king
       ScrQte += piece_square_table[5][conf->xrB*8+conf->yrB];
	   }
                break;
		   case -'p' : pionN++;
                ScrQte -= mirror[0][i*8+j];
                break;
		   case -'c' : cN++;
                ScrQte -= mirror[1][i*8+j];
                break;
		   case -'f' : fN++;
                ScrQte -= mirror[2][i*8+j];
                break;
		   case -'t' : tN++;
                ScrQte -= mirror[3][i*8+j];
                break;
		   case -'n' : nN++;
                ScrQte -= mirror[4][i*8+j];

                break;
           case -'r' : rN++;
               ScrQte -= mirror[5][i*8+j];
               	   // end game king
	   if (nN+nB==0){
            ScrQte -= mirror[6][conf->xrN*8+conf->yrN];
	   }else{ // mid - early king

    ScrQte -= mirror[5][conf->xrN*8+conf->yrN];
	   }
                break;

		}
	   }


	// Somme pond�r�e de pi�ces de chaque joueur.
	ScrQte += ( (pionB*100 + cB*320 + fB*330 + tB*500 + nB*900+ 20000*rB) - (pionN*100 + cN*320 + fN*330 + tN*500 + nN*900+ 20000*rN) );

	// Le facteur 1/100 pour ne pas sortir de l'intervalle ]-100 , +100[
	return ScrQte/100;


} // fin de estim7

/***********************************************************************************/


// ************************************
// Partie:  G�n�ration des alternatives
// ************************************


/* G�n�re, pour le joueur 'mode', les successeurs de la configuration 'conf' dans le tableau 'T',
   retourne aussi dans 'n' le nombre de configurations filles g�n�r�es */
void generer_succ( struct config *conf, int mode, struct config T[], int *n )
{
        int i, j, k, stop;

        *n = 0;

        if ( mode == MAX ) {                // mode == MAX
           for (i=0; i<8; i++)
              for (j=0; j<8; j++)
                 if ( conf->mat[i][j] > 0 )
                    deplacementsB(conf, i, j, T, n );

           // v�rifier si le roi est en echec, auquel cas on ne garde que les succ �vitants l'�chec
           // ou alors si une conf est d�j� visit�e dans Partie, auquel cas on l'enl�ve aussi ...
           for (k=0; k < *n; k++) {
               i = T[k].xrB; j = T[k].yrB;  // pos du roi B dans T[k]
               // v�rifier si roi menac� dans la config T[k] ou alors T[k] est dej� visit�e ...
               if ( caseMenaceePar( MIN, i, j, &T[k] ) || dejaVisitee( &T[k] ) ) {
                   T[k] = T[(*n)-1];        // alors supprimer T[k] de la liste des succ...
                   (*n)--;
                   k--;
               }
           } // for k
        }

        else {                                 // mode == MIN
           for (i=0; i<8; i++)
              for (j=0; j<8; j++)
                 if ( conf->mat[i][j] < 0 )
                    deplacementsN(conf, i, j, T, n );

           // v�rifier si le roi est en echec, auquel cas on ne garde que les succ �vitants l'�chec
           // ou alors si une conf est d�j� visit�e dans Partie, auquel cas on l'enl�ve aussi ...
           for (k=0; k < *n; k++) {
               i = T[k].xrN; j = T[k].yrN;
               // v�rifier si roi menac� dans la config T[k] ou alors T[k] est dej� visit�e ...
               if ( caseMenaceePar( MAX, i, j, &T[k] ) || dejaVisitee( &T[k] ) ) {
                  T[k] = T[(*n)-1];        // alors supprimer T[k] de la liste des succ...
                  (*n)--;
                  k--;
               }
           } // for k
        } // if (mode == MAX) ... else ...

} // fin de generer_succ


/* G�n�re dans T les configurations obtenues � partir de conf lorsqu'un pion (a,b) va atteindre
   la limite de l'�chiquier: pos (x,y)  */
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n )
{
        int signe = +1;
        if (conf->mat[a][b] < 0 ) signe = -1;
        copier(conf, &T[*n]);
        T[*n].mat[a][b] = 0;
        T[*n].mat[x][y] = signe * 'n';        // transformation en Reine
        (*n)++;
        copier(conf, &T[*n]);
        T[*n].mat[a][b] = 0;
        T[*n].mat[x][y] = signe * 'c';        // transformation en Cavalier
        (*n)++;
        copier(conf, &T[*n]);
        T[*n].mat[a][b] = 0;
        T[*n].mat[x][y] = signe * 'f';        // transformation en Fou
        (*n)++;
        copier(conf, &T[*n]);
        T[*n].mat[a][b] = 0;
        T[*n].mat[x][y] = signe * 't';        // transformation en Tour
        (*n)++;

} // fin de transformPion


/* G�nere dans T tous les coups possibles de la pi�ce (de couleur N) se trouvant � la pos x,y */
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n )
{
        int i, j, a, b, stop;

        switch(conf->mat[x][y]) {
        // mvmt PION ...
        case -'p' :
                if ( x > 0 && conf->mat[x-1][y] == 0 ) {
                        // avance d'une case
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x-1][y] = -'p';
                        (*n)++;
                        if ( x == 1 ) transformPion( conf, x, y, x-1, y, T, n );
                }
                if ( x == 6 && conf->mat[5][y] == 0 && conf->mat[4][y] == 0) {
                        // avance de 2 cases
                        copier(conf, &T[*n]);
                        T[*n].mat[6][y] = 0;
                        T[*n].mat[4][y] = -'p';
                        (*n)++;
                }
                if ( x > 0 && y >0 && conf->mat[x-1][y-1] > 0 ) {
                        // attaque � droite (en descendant)
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x-1][y-1] = -'p';
                        // cas o� le roi adverse est pris...
                        if (T[*n].xrB == x-1 && T[*n].yrB == y-1) {
                                T[*n].xrB = -1; T[*n].yrB = -1;
                        }

                        (*n)++;
                        if ( x == 1 ) transformPion( conf, x, y, x-1, y-1, T, n );
                }
                if ( x > 0 && y < 7 && conf->mat[x-1][y+1] > 0 ) {
                        // attaque � gauche (en descendant)
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x-1][y+1] = -'p';
                        // cas o� le roi adverse est pris...
                        if (T[*n].xrB == x-1 && T[*n].yrB == y+1) {
                                T[*n].xrB = -1; T[*n].yrB = -1;
                        }

                        (*n)++;
                        if ( x == 1 ) transformPion( conf, x, y, x-1, y+1, T, n );
                }
                break;

        // mvmt CAVALIER ...
        case -'c' :
                for (i=0; i<8; i++)
                   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
                        if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] >= 0 )  {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = -'c';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrB == x+dC[i][0] && T[*n].yrB == y+dC[i][1]) {
                                T[*n].xrB = -1; T[*n].yrB = -1;
                           }

                           (*n)++;
                        }
                break;

        // mvmt FOU ...
        case -'f' :
                for (i=1; i<8; i += 2) {
                   // traitement des directions impaires (1, 3, 5 et 7)
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] > 0 ) stop = 1;
                           T[*n].mat[a][b] = -'f';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt TOUR ...
        case -'t' :
                for (i=0; i<8; i += 2) {
                   // traitement des directions paires (0, 2, 4 et 6)
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] > 0 ) stop = 1;
                           T[*n].mat[a][b] = -'t';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

                           if ( conf->roqueN != 'e' && conf->roqueN != 'n' ) {
                              if ( x == 7 && y == 0 && conf->roqueN != 'p')
                                 // le grand roque ne sera plus possible
                                    T[*n].roqueN = 'g';
                              else if ( x == 7 && y == 0 )
                                      // ni le grand roque ni le petit roque ne seront possibles
                                         T[*n].roqueN = 'n';

                              if ( x == 7 && y == 7 && conf->roqueN != 'g' )
                                 // le petit roque ne sera plus possible
                                    T[*n].roqueN = 'p';
                              else if ( x == 7 && y == 7 )
                                      // ni le grand roque ni le petit roque ne seront possibles
                                         T[*n].roqueN = 'n';
                           }

                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt REINE ...
        case -'n' :
                for (i=0; i<8; i += 1) {
                   // traitement des 8 directions paires et impaires
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] < 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] > 0 ) stop = 1;
                           T[*n].mat[a][b] = -'n';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt ROI ...
        case -'r' :
                // v�rifier possibilit� de faire un roque ...
                if ( conf->roqueN != 'n' && conf->roqueN != 'e' ) {
                   if ( conf->roqueN != 'g' && conf->mat[7][1] == 0 \
                        && conf->mat[7][2] == 0 && conf->mat[7][3] == 0 )
                      if ( !caseMenaceePar( MAX, 7, 1, conf ) && !caseMenaceePar( MAX, 7, 2, conf ) && \
                           !caseMenaceePar( MAX, 7, 3, conf ) && !caseMenaceePar( MAX, 7, 4, conf ) )  {
                        // Faire un grand roque ...
                        copier(conf, &T[*n]);
                        T[*n].mat[7][4] = 0;
                        T[*n].mat[7][0] = 0;
                        T[*n].mat[7][2] = -'r'; T[*n].xrN = 7; T[*n].yrN = 2;
                        T[*n].mat[7][3] = -'t';
                        // aucun roque ne sera plus possible � partir de cette config
                        T[*n].roqueN = 'e';
                        (*n)++;
                      }
                   if ( conf->roqueN != 'p' && conf->mat[7][5] == 0 && conf->mat[7][6] == 0 )
                      if ( !caseMenaceePar( MAX, 7, 4, conf ) && !caseMenaceePar( MAX, 7, 5, conf ) && \
                           !caseMenaceePar( MAX, 7, 6, conf ) )  {
                        // Faire un petit roque ...
                        copier(conf, &T[*n]);
                        T[*n].mat[7][4] = 0;
                        T[*n].mat[7][7] = 0;
                        T[*n].mat[7][6] = -'r'; T[*n].xrN = 7; T[*n].yrN = 6;
                        T[*n].mat[7][5] = -'t';
                        // aucun roque ne sera plus possible � partir de cette config
                        T[*n].roqueN = 'e';
                        (*n)++;
                      }
                }

                // v�rifier les autres mouvements du roi ...
                for (i=0; i<8; i += 1) {
                   // traitement des 8 directions paires et impaires
                   a = x + D[i][0];
                   b = y + D[i][1];
                   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                        if ( conf->mat[a][b] >= 0 ) {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           T[*n].mat[a][b] = -'r'; T[*n].xrN = a; T[*n].yrN = b;
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrB == a && T[*n].yrB == b) {
                              T[*n].xrB = -1;
                              T[*n].yrB = -1;
                           }
                           // aucun roque ne sera plus possible � partir de cette config
                           T[*n].roqueN = 'n';
                           (*n)++;
                        }
                } // for
                break;

        }

} // fin de deplacementsN


/* G�nere dans T tous les coups possibles de la pi�ce (de couleur B) se trouvant � la pos x,y */
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n )
{
        int i, j, a, b, stop;

        switch(conf->mat[x][y]) {
        // mvmt PION ...
        case 'p' :
                if ( x <7 && conf->mat[x+1][y] == 0 ) {
                        // avance d'une case
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x+1][y] = 'p';
                        (*n)++;
                        if ( x == 6 ) transformPion( conf, x, y, x+1, y, T, n );
                }
                if ( x == 1 && conf->mat[2][y] == 0 && conf->mat[3][y] == 0) {
                        // avance de 2 cases
                        copier(conf, &T[*n]);
                        T[*n].mat[1][y] = 0;
                        T[*n].mat[3][y] = 'p';
                        (*n)++;
                }
                if ( x < 7 && y > 0 && conf->mat[x+1][y-1] < 0 ) {
                        // attaque � gauche (en montant)
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x+1][y-1] = 'p';
                        // cas o� le roi adverse est pris...
                        if (T[*n].xrN == x+1 && T[*n].yrN == y-1) {
                                T[*n].xrN = -1; T[*n].yrN = -1;
                        }

                        (*n)++;
                        if ( x == 6 ) transformPion( conf, x, y, x+1, y-1, T, n );
                }
                if ( x < 7 && y < 7 && conf->mat[x+1][y+1] < 0 ) {
                        // attaque � droite (en montant)
                        copier(conf, &T[*n]);
                        T[*n].mat[x][y] = 0;
                        T[*n].mat[x+1][y+1] = 'p';
                        // cas o� le roi adverse est pris...
                        if (T[*n].xrN == x+1 && T[*n].yrN == y+1) {
                                T[*n].xrN = -1; T[*n].yrN = -1;
                        }

                        (*n)++;
                        if ( x == 6 ) transformPion( conf, x, y, x+1, y+1, T, n );
                }
                break;

        // mvmt CAVALIER ...
        case 'c' :
                for (i=0; i<8; i++)
                   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
                        if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] <= 0 )  {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = 'c';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrN == x+dC[i][0] && T[*n].yrN == y+dC[i][1]) {
                                T[*n].xrN = -1;
                                T[*n].yrN = -1;
                           }

                           (*n)++;
                        }
                break;

        // mvmt FOU ...
        case 'f' :
                for (i=1; i<8; i += 2) {
                   // traitement des directions impaires (1, 3, 5 et 7)
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] < 0 ) stop = 1;
                           T[*n].mat[a][b] = 'f';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrN == a && T[*n].yrN == b) {
                                T[*n].xrN = -1;
                                T[*n].yrN = -1;
                           }
                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt TOUR ...
        case 't' :
                for (i=0; i<8; i += 2) {
                   // traitement des directions paires (0, 2, 4 et 6)
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] < 0 ) stop = 1;
                           T[*n].mat[a][b] = 't';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrN == a && T[*n].yrN == b) {
                                T[*n].xrN = -1;
                                T[*n].yrN = -1;
                           }
                           if ( conf->roqueB != 'e' && conf->roqueB != 'n' ) {
                             if ( x == 0 && y == 0 && conf->roqueB != 'p')
                                // le grand roque ne sera plus possible
                                   T[*n].roqueB = 'g';
                             else if ( x == 0 && y == 0 )
                                     // ni le grand roque ni le petit roque ne seront possibles
                                        T[*n].roqueB = 'n';
                             if ( x == 0 && y == 7 && conf->roqueB != 'g' )
                                // le petit roque ne sera plus possible
                                   T[*n].roqueB = 'p';
                             else if ( x == 0 && y == 7 )
                                     // ni le grand roque ni le petit roque ne seront possibles
                                        T[*n].roqueB = 'n';
                           }

                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt REINE ...
        case 'n' :
                for (i=0; i<8; i += 1) {
                   // traitement des 8 directions paires et impaires
                   stop = 0;
                   a = x + D[i][0];
                   b = y + D[i][1];
                   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
                        if ( conf->mat[ a ] [ b ] > 0 )  stop = 1;
                        else {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           if ( T[*n].mat[a][b] < 0 ) stop = 1;
                           T[*n].mat[a][b] = 'n';
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrN == a && T[*n].yrN == b) {
                              T[*n].xrN = -1;
                              T[*n].yrN = -1;
                           }
                           (*n)++;
                              a = a + D[i][0];
                              b = b + D[i][1];
                        }
                   } // while
                } // for
                break;

        // mvmt ROI ...
        case 'r' :
                // v�rifier possibilit� de faire un roque ...
                if ( conf->roqueB != 'n' && conf->roqueB != 'e' ) {
                   if ( conf->roqueB != 'g' && conf->mat[0][1] == 0 && \
                        conf->mat[0][2] == 0 && conf->mat[0][3] == 0 )
                      if ( !caseMenaceePar( MIN, 0, 1, conf ) && !caseMenaceePar( MIN, 0, 2, conf ) && \
                           !caseMenaceePar( MIN, 0, 3, conf ) && !caseMenaceePar( MIN, 0, 4, conf ) )  {
                        // Faire un grand roque ...
                        copier(conf, &T[*n]);
                        T[*n].mat[0][4] = 0;
                        T[*n].mat[0][0] = 0;
                        T[*n].mat[0][2] = 'r'; T[*n].xrB = 0; T[*n].yrB = 2;
                        T[*n].mat[0][3] = 't';
                        // aucun roque ne sera plus possible � partir de cette config
                        T[*n].roqueB = 'e';
                        (*n)++;
                      }
                   if ( conf->roqueB != 'p' && conf->mat[0][5] == 0 && conf->mat[0][6] == 0 )
                      if ( !caseMenaceePar( MIN, 0, 4, conf ) && !caseMenaceePar( MIN, 0, 5, conf ) && \
                           !caseMenaceePar( MIN, 0, 6, conf ) )  {
                        // Faire un petit roque ...
                        copier(conf, &T[*n]);
                        T[*n].mat[0][4] = 0;
                        T[*n].mat[0][7] = 0;
                        T[*n].mat[0][6] = 'r'; T[*n].xrB = 0; T[*n].yrB = 6;
                        T[*n].mat[0][5] = 't';
                        // aucun roque ne sera plus possible � partir de cette config
                        T[*n].roqueB = 'e';
                        (*n)++;
                      }
                }

                // v�rifier les autres mouvements du roi ...
                for (i=0; i<8; i += 1) {
                   // traitement des 8 directions paires et impaires
                   a = x + D[i][0];
                   b = y + D[i][1];
                   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                        if ( conf->mat[a][b] <= 0 ) {
                           copier(conf, &T[*n]);
                           T[*n].mat[x][y] = 0;
                           T[*n].mat[a][b] = 'r'; T[*n].xrB = a; T[*n].yrB = b;
                           // cas o� le roi adverse est pris...
                           if (T[*n].xrN == a && T[*n].yrN == b) {
                              T[*n].xrN = -1;
                              T[*n].yrN = -1;
                           }
                           // aucun roque ne sera plus possible � partir de cette config
                           T[*n].roqueB = 'n';
                           (*n)++;
                        }
                } // for
                break;

        }

} // fin de deplacementsB

/***********************************************************************************/


// *****************************
// Partie: Fonctions utilitaires
// *****************************


// V�rifie si la case (x,y) est menac�e par une des pi�ces du joueur 'mode'
int caseMenaceePar( int mode, int x, int y, struct config *conf )
{
        int i, j, a, b, stop;

        // menace par le roi ...
        for (i=0; i<8; i += 1) {
           // traitement des 8 directions paires et impaires
           a = x + D[i][0];
           b = y + D[i][1];
           if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                if ( conf->mat[a][b]*mode == 'r' ) return 1;
        } // for

        // menace par cavalier ...
        for (i=0; i<8; i++)
           if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
                if ( conf->mat[ x+dC[i][0] ] [ y+dC[i][1] ] * mode == 'c' )
                   return 1;

        // menace par pion ...
        if ( (x-mode) >= 0 && (x-mode) <= 7 && y > 0 && conf->mat[x-mode][y-1]*mode == 'p' )
           return 1;
        if ( (x-mode) >= 0 && (x-mode) <= 7 && y < 7 && conf->mat[x-mode][y+1]*mode == 'p' )
           return 1;

        // menace par fou, tour ou reine ...
        for (i=0; i<8; i += 1) {
           // traitement des 8 directions paires et impaires
           stop = 0;
           a = x + D[i][0];
           b = y + D[i][1];
           while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 )
                if ( conf->mat[a][b] != 0 )  stop = 1;
                else {
                    a = a + D[i][0];
                    b = b + D[i][1];
                }
           if ( stop )  {
                if ( conf->mat[a][b]*mode == 'f' && i % 2 != 0 ) return 1;
                if ( conf->mat[a][b]*mode == 't' && i % 2 == 0 ) return 1;
                if ( conf->mat[a][b]*mode == 'n' ) return 1;
           }
        } // for

        // sinon, aucune menace ...
        return 0;

} // fin de caseMenaceePar


/* Fonctions de comparaison utilis�es avec qsort     */
// 'a' et 'b' sont des configurations
int confcmp123(const void *a, const void *b)        // pour l'ordre croissant
{
    int x = ((struct config *)a)->val, y = ((struct config *)b)->val;
    if ( x < y )
        return -1;
    if ( x == y )
        return 0;
    return 1;
}  // fin confcmp123

int confcmp321(const void *a, const void *b)        // pour l'ordre decroissant
{
    int x = ((struct config *)a)->val, y = ((struct config *)b)->val;
    if ( x < y )
        return 1;
    if ( x == y )
        return 0;
    return -1;
}  // fin confcmp321


/* Le nombre de pi�ces de N et B dans l'echiquier 'conf' */
int npieces( struct config *conf )
{
    int i,j;
    int nb = 0;
    for (i=0; i<8; i++)
        for (j=0; j<8; j++)
            if ( conf->mat[i][j] != 0 ) nb++;
    return nb;
}


/* Sauvegarder conf dans le fichier f (pour l'historique) */
void sauvConf( struct config *conf )
{

   char buf[72] = "";
   int i, j;
   for (i=7; i>=0; i--) {
        for (j=0; j<8; j++)
           if (conf->mat[i][j] == 0)  strcat(buf," ");
           else if (conf->mat[i][j] < 0) {
                   strcat(buf, "-");
                   buf[strlen(buf)-1] = -conf->mat[i][j];
                }
                else {
                   strcat(buf, "+");
                   buf[strlen(buf)-1] = conf->mat[i][j] - 32;
                }
        strcat(buf, "\n");
   }
   fprintf(f, "--- coup N� %d ---\n%s\n",num_coup, buf);
   fflush(f);

} // fin sauvConf


/* Intialise la disposition des pieces dans la configuration initiale conf */
void init( struct config *conf )
{
           int i, j;

            for (i=0; i<8; i++)
                for (j=0; j<8; j++)
                        conf->mat[i][j] = 0;        // Les cases vides sont initialis�es avec 0

        conf->mat[0][0]='t'; conf->mat[0][1]='c'; conf->mat[0][2]='f'; conf->mat[0][3]='n';
        conf->mat[0][4]='r'; conf->mat[0][5]='f'; conf->mat[0][6]='c'; conf->mat[0][7]='t';

        for (j=0; j<8; j++) {
                conf->mat[1][j] = 'p';
                 conf->mat[6][j] = -'p';
                conf->mat[7][j] = -conf->mat[0][j];
        }

        conf->xrB = 0; conf->yrB = 4;
        conf->xrN = 7; conf->yrN = 4;

        conf->roqueB = 'r';
        conf->roqueN = 'r';

        conf->val = 0;

} // fin de init


/* g�n�re un texte d�crivant le dernier coup effectu� (pour l'affichage) */
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup )
{
        int i,j;
        char piece[20];

        // verifier si roqueB effectu� ...
        if ( newconf->roqueB == 'e' && oldconf->roqueB != 'e' ) {
           if ( newconf->yrB == 2 ) sprintf(coup, "g_roqueB" );
           else  sprintf(coup, "p_roqueB" );
           return;
        }

        // verifier si roqueN effectu� ...
        if ( newconf->roqueN == 'e' && oldconf->roqueN != 'e' ) {
           if ( newconf->yrN == 2 ) sprintf(coup, "g_roqueN" );
           else  sprintf(coup, "p_roqueN" );
           return;
        }

        // Autres mouvements de pi�ces
        for(i=0; i<8; i++)
           for (j=0; j<8; j++)
                if ( oldconf->mat[i][j] != newconf->mat[i][j] )
                   if ( newconf->mat[i][j] != 0 ) {
                        switch (newconf->mat[i][j]) {
                           case -'p' : sprintf(piece,"pionN"); break;
                           case 'p' : sprintf(piece,"pionB"); break;
                           case -'c' : sprintf(piece,"cavalierN"); break;
                           case 'c' : sprintf(piece,"cavalierB"); break;
                           case -'f' : sprintf(piece,"fouN"); break;
                           case 'f' : sprintf(piece,"fouB"); break;
                           case -'t' : sprintf(piece,"tourN"); break;
                           case 't' : sprintf(piece,"tourB"); break;
                           case -'n' : sprintf(piece,"reineN"); break;
                           case 'n' : sprintf(piece,"reineB"); break;
                           case -'r' : sprintf(piece,"roiN"); break;
                           case 'r' : sprintf(piece,"roiB"); break;
                        }
                        sprintf(coup, "%s en %c%d", piece, 'a'+j, i+1);
                        return;
                   }
} // fin de formuler_coup


/* Affiche la configuration conf */
void affich( struct config *conf, char *coup, int num )
{
        int i, j, k;
        int pB = 0, pN = 0, cB = 0, cN = 0, fB = 0, fN = 0, tB = 0, tN = 0, nB = 0, nN = 0;

             printf("Coup num:%3d : %s\n", num, coup);
             printf("\n");
        for (i=0;  i<8; i++)
                printf("\t   %c", i+'a');
           printf("\n");

        for (i=0;  i<8; i++)
                printf("\t-------");
           printf("\n");

        for(i=8; i>0; i--)  {
                printf("    %d", i);
                for (j=0; j<8; j++) {
                        if ( conf->mat[i-1][j] < 0 ) printf("\t  %cN", -conf->mat[i-1][j]);
                        else if ( conf->mat[i-1][j] > 0 ) printf("\t  %cB", conf->mat[i-1][j]);
                                  else printf("\t   ");
                        switch (conf->mat[i-1][j]) {
                            case -'p' : pN++; break;
                            case 'p'  : pB++; break;
                            case -'c' : cN++; break;
                            case 'c'  : cB++; break;
                            case -'f' : fN++; break;
                            case 'f'  : fB++; break;
                            case -'t' : tN++; break;
                            case 't'  : tB++; break;
                            case -'n' : nN++; break;
                            case 'n'  : nB++; break;
                        }
                }
                printf("\n");

                for (k=0;  k<8; k++)
                        printf("\t-------");
                   printf("\n");

        }
        printf("\n\tB : p(%d) c(%d) f(%d) t(%d) n(%d) \t N : p(%d) c(%d) f(%d) t(%d) n(%d)\n\n",
                pB, cB, fB, tB, nB, pN, cN, fN, tN, nN);
        printf("\n");

} // fin de  affich


/* Copie la configuration c1 dans c2  */
void copier( struct config *c1, struct config *c2 )
{
        int i, j;

        for (i=0; i<8; i++)
                for (j=0; j<8; j++)
                        c2->mat[i][j] = c1->mat[i][j];

        c2->val = c1->val;
        c2->xrB = c1->xrB;
        c2->yrB = c1->yrB;
        c2->xrN = c1->xrN;
        c2->yrN = c1->yrN;

        c2->roqueB = c1->roqueB;
        c2->roqueN = c1->roqueN;
} // fin de copier


/* Teste si les �chiquiers c1 et c2 sont �gaux */
int egal(char c1[8][8], char c2[8][8] )
{
        int i, j;

        for (i=0; i<8; i++)
                for (j=0; j<8; j++)
                        if (c1[i][j] != c2[i][j]) return 0;
        return 1;
} // fin de egal


/* Teste si conf a d�j� �t� jou�e (dans le tableau partie) */
int dejaVisitee( struct config *conf )
{
   int i = 0;
   int trouv = 0;
   while ( i < MAXPARTIE && trouv == 0 )
        if ( egal( conf->mat, Partie[i].mat ) ) trouv = i+1;
        else i++;
   return trouv;
}

/* verifie si le mat�riel restant est suffisant pour pouvoir continuer la partie  */
int verrifMateriel(struct config*conf)
{

	int match_null=0;
	 int  cfB = 0, cfN = 0;
	 int i,j;
	//partie null pour  mat�riel insuffisant
	// parties : nombre de pi�ces restantes de tous types
	for (i=0; i<8; i++)
	   for (j=0; j<8; j++) {
		switch (conf->mat[i][j]) {
		   case 'p' : return 0;
		   case 'c' :
		   case 'f' : cfB++;
		    if (cfB+cfN>=2) return 0;
		     else break;
		   case 't' : return 0;
		   case 'n' : return 0;
		   case -'p' : return 0;
		   case -'c' : //cavalier_N++;
		   case -'f' : cfN++;
		   if (cfB+cfN>=2) return 0;
		  else  break;
		   case -'t' : return 0;
		   case -'n' : return 0;
		   default : break;
		}}
		return 1;
}

/* Teste s'il n'y a aucun coup possible dans la configuration conf */
int AucunCoupPossible( struct config *conf )
{
        // ... A completer pour les matchs nuls
        // ... v�rifier que generer_succ retourne 0 configurations filles ...
        // ... ou qu'une m�me configuration a �t� g�n�r�e plusieurs fois ...
struct config T[100];
int n=0;

	if(num_coup%2==0 && caseMenaceePar(MIN,conf->xrB,conf->yrB,conf)) return 0;
	 if(num_coup%2==1 && caseMenaceePar(MAX,conf->xrN,conf->yrN,conf)) return 0;


	if(num_coup%2==0) generer_succ(conf, MAX, T, &n);
	else generer_succ(conf,MIN,T,&n);
	// partie null (un des deux joueurs ne peut plus se deplacer et il n'ai pas en echec)
     if(n==0) return 1;


	 int match_null=0;
	 match_null=verrifMateriel(conf);
	//partie null pour  mat�riel insuffisant
	if (match_null==1) return 1;

    int nbr = dejaVisitee(conf);
    if (nbr >=3) return 1;
	// partie null configuration deja visit�e 3 fois
    return match_null;

} // fin de AucunCoupPossible

/***********************************************************************************/

