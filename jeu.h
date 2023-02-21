/********************************************************************************
*********************************************************************************

Programme de d�monstration de MinMax avec �lagage alpha/b�ta pour le jeu d'�checs
TP2 CRP - 2CSSID / ESI 2022/2023

Header du module : jeu.h

*********************************************************************************
*********************************************************************************/



#ifndef JEU_H
#define JEU_H

#define MAX +1          // Joueur Maximisant
#define MIN -1          // Joueur Minimisant

#define INFINI INT_MAX
#define MAXPARTIE 50    // Taille max du tableau Partie
                        // qui sert � v�rifier si une conf a d�j� �t� g�n�r�e
                        // pour ne pas le re-consid�rer une 2e fois.
                        // On se limitera donc aux MAXPARTIE derniers coups


// Le type d'une configuration
struct config {
   char mat[8][8];            // Echiquier
   int val;                   // Estimation de la config
   char xrN, yrN, xrB, yrB;   // Positions des rois Noir et Blanc
   char roqueN, roqueB;       // Indicateurs de roque pour N et B :
                                   // 'g' grand roque non r�alisable
                                   // 'p' petit roque non r�alisable
                                   // 'n' non r�alisable des 2 cot�s
                                   // 'r' r�alisable (valeur initiale)
                                   // 'e' effectu�
};



/**************************/
/* Ent�te des fonctions : */
/**************************/

/* ----------------------------------
 MinMax avec �lagage alpha-beta :
 Evalue la configuration 'conf' du joueur 'mode' en descendant de 'niv' niveaux.
 Le param�tre 'niv' est decr�ment� � chaque niveau (appel r�cursif).
 'alpha' et 'beta' repr�sentent les bornes initiales de l'intervalle d'int�r�t
 (pour pouvoir effectuer les coupes alpha et b�ta).
 'largeur' repr�sente le nombre max d'alternatives � explorer en profondeur � chaque niveau.
 Si 'largeur == +INFINI' toutes les alternatives seront prises en compte
 (c'est le comportement par d�faut).
 'numFctEst' est le num�ro de la fonction d'estimation � utiliser lorsqu'on arrive � la
 fronti�re d'exploration (c-a-d lorsque 'niv' atteint 0)
 'npp' repr�sente le nombre de pi�ces dans la configuration parente � 'conf'.
 Cela permet de v�rifier, par exemple, si le nombre de pi�ces dans conf a chang� ou non
 (donc avoir une indication sur la stabilit� de la configuration courante: conf)
*/
int minmax_ab( struct config *conf, int mode, int niv, int min, int max, int largeur, int numFctEst, int npp, int after );


/* ----------------------------------
  La fonction d'estimation � utiliser, retourne une valeur dans ]-100, +100[
  quelques fonctions d'estimation disponibles (comme exemples).
  Le param�tre 'conf' repr�sente la configuration � estimer
*/
int estim1( struct config *conf );
int estim2( struct config *conf );
int estim3( struct config *conf );
int estim4( struct config *conf );
int estim5( struct config *conf );
int estim6( struct config *conf );
/*
   Pour tester votre propre fonction d'estimation: changer le code de estim7
   et utilisez la pour un des 2 joueurs N ou B (voir la fonction main)
*/
int estim7( struct config *conf );


/* ----------------------------------
   G�n�re, pour le joueur 'mode', les successeurs de la configuration 'conf' dans le tableau 'T',
   retourne aussi dans 'n' le nombre de configurations filles g�n�r�es
*/
void generer_succ( struct config *conf, int mode, struct config T[], int *n );


/* ----------------------------------
  G�n�re dans 'T' les configurations obtenues � partir de 'conf' lorsqu'un pion � la pos (a,b)
  va atteindre la limite de l'�chiquier (x,y) et se transformer en une autre pi�ce.
  Le nombre de possibilit�s sera retourn� dans 'n'
*/
void transformPion( struct config *conf, int a, int b, int x, int y, struct config T[], int *n );


/* ----------------------------------
  G�nere dans 'T' tous les coups possibles de la pi�ce (de couleur N ou B)
  se trouvant � la pos 'x','y'. Le nombre de possibilit�s sera retourn� dans 'n'
*/
void deplacementsN(struct config *conf, int x, int y, struct config T[], int *n );
void deplacementsB(struct config *conf, int x, int y, struct config T[], int *n );


/* ----------------------------------
  V�rifie si la case (x,y) est menac�e par une des pi�ces du joueur 'mode'
*/
int caseMenaceePar( int mode, int x, int y, struct config *conf );


/* ----------------------------------
  Intialise la disposition des pieces dans la configuration initiale 'conf'
*/
void init( struct config *conf );


/* ----------------------------------
  Compte le nombre pieces (blancs et noirs) dans l'echiquier
*/
int npieces( struct config *conf );


/* ----------------------------------
  Affiche la configuration 'conf'
*/
void affich( struct config *conf, char *coup, int num );


/* ----------------------------------
  Teste si 'conf' repr�sente une configuration d�j� jou�e (dans le tableau Partie)
  auquel cas elle retourne le num du coup + 1
*/
int dejaVisitee( struct config *conf );


/* ----------------------------------
  Savegarde la config 'conf' dans le fichier f (global)
*/
void sauvConf( struct config *conf );


/* ----------------------------------
  Copie la configuration 'c1' dans 'c2'
*/
void copier( struct config *c1, struct config *c2 );


/* ----------------------------------
  Teste si les 2 grilles (�chiquiers) 'c1' et 'c2' sont �gales
*/
int egal(char c1[8][8], char c2[8][8] );


/* ----------------------------------
  Teste s'il n'y a aucun coup possible � partir de la configuration 'conf'
*/
int AucunCoupPossible( struct config *conf );


/* ----------------------------------
  Teste si 'conf' repr�sente une fin de partie
  et retourne dans 'cout' son score -100, 0 ou +100
*/
int feuille( struct config *conf, int *cout );


/* ----------------------------------
  Comparaisons des configurations 'a' et 'b' pour le compte de qsort
  pour l'ordre croissant (confcmp123) et d�croissant (confcmp321)
*/
int confcmp123(const void *a, const void *b);
int confcmp321(const void *a, const void *b);


/* ----------------------------------
  G�n�re dans 'coup' un texte d�crivant le dernier coup effectu� (pour l'affichage)
  en comparant deux configurations: l'ancienne 'oldconf' et la nouvelle 'newconf'
*/
void formuler_coup( struct config *oldconf, struct config *newconf, char *coup );



#endif

