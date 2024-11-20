#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#define N 10
#define EPS 1.e-8

#define LARGEUR 15
#define HAUTEUR 20

struct jeu {
  int grille[LARGEUR][HAUTEUR];
  int score;
  int taille;
};

struct jeu init_jeu(){
  struct jeu p;
  p.score=0;
  p.taille=2;
  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++)
      p.grille[x][y]=0;

  p.grille[2][4]=1;
  p.grille [3][5]=1;
  p.grille[0][10]=1;

  for(int x=0; x<p.taille*2+1;x++){
    p.grille[LARGEUR/2-p.taille+x][HAUTEUR-1]=2;
  }
  return p;
}

int main(void) {
      
  srand(time(NULL));
  struct jeu init_jeu();
  struct jeu p = init_jeu();

  void affiche_jeu(struct jeu);
  struct jeu déplacer(char direction, struct jeu);
  struct jeu mise_a_jour_objets(struct jeu);
  struct jeu verifier_collision(struct jeu);
  void sauvegarde_partie(struct jeu, int);
  struct jeu charge_partie(int);

  struct jeu p2;
  p2=charge_partie(0);

  int i=0;
  while(i<0) {
      
      int r=rand()%2;
      if(r==0)
          p=déplacer('a',p);
      if(r==1)
          p=déplacer('d',p);

      p=mise_a_jour_objets(p);

      p=verifier_collision(p);
      
      affiche_jeu(p);
      i++;
      printf("Score : %d\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",p.score);
      usleep(1e4);
      printf("\n\n\n\n\n\n\n\n\n\n");
  }
} 


void affiche_jeu(struct jeu j){

  printf("\t");
  for(int i=0;i<LARGEUR+2;i++)
    printf("%d\t",i);
  printf("\n");

  // Affiche le haut de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf("*\t");
    
  printf("\n");
  for(int y=0;y<HAUTEUR;y++) {
  
    // Affiche le bord gauche de la grille
    printf("*\t");

    // Affiche une ligne de la grille
    for(int x=0;x<LARGEUR;x++){

      if (j.grille[x][y] == 0)
        printf(" \t");

      if (j.grille[x][y] == 1)
        printf("0\t");

      if (j.grille[x][y] == 2)
        printf("-\t");
      }
      
    // Affiche le bord droit de la grille
    printf("*\t%d\n",y);
    }
    
  // Affiche le bas de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf("*\t");

  printf("\n");
  return;
}

struct jeu déplacer(char direction, struct jeu j){

  if(direction=='a'){
    
    // Parcours la grille en cherchant la limite gauche du radeau
    for (int x=0; x<LARGEUR;x++){
        
        if(j.grille[x][HAUTEUR-1]==2){

          // Stoppe la recherche si la limite gauche du radeau ne peut être déplacée
          if(x<1)
            break;

          // Déplace chaque position du radeau sur la gauche
          for(int xi=x;xi<x+j.taille*2+1;xi++){

            j.grille[xi][HAUTEUR-1]-=2;
            j.grille[xi-1][HAUTEUR-1]+=2;
          }

          // Finis la recherche une fois le déplacement effectué
          break;
        }
    }
  }

  if(direction=='d'){
    
    // Parcours la grille (de gauche à droite)
    // et déplace les positions avec des radeaux sur la droite
    for (int x=0; x<LARGEUR;x++){
        
        if(j.grille[x][HAUTEUR-1]==2){

          // Stoppe la recherche si la limite droite du radeau ne peut être déplacée
          if(x>LARGEUR-j.taille*2-2)
            break;

          //Déplace chaque position du radeau sur la droite
          for(int xi=x;xi<x+j.taille*2+1;xi++){

            j.grille[xi][HAUTEUR-1]-=2;
            j.grille[xi+1][HAUTEUR-1]+=2;
          }

          // Finis la recherche une fois le déplacement effectué
          break;
        }
    }
  }

  return j;
}

struct jeu mise_a_jour_objets(struct jeu j) {
   
  for(int y=HAUTEUR-2;y>=0;y--) {

    for(int x=0;x<LARGEUR;x++) {

      if(j.grille[x][y]==1) {
          
          j.grille[x][y]-=1; //la grille actuelle perd un objet
          j.grille[x][y+1]+=1; // la grille actuelle+1 gagne un objet
        }
      }   
    }

  for(int x=0;x<LARGEUR;x++) {

    if(j.grille[x][HAUTEUR-1]==1){

      j.grille[x][HAUTEUR-1]=0; //objets atteignants la fin de la grille disparaissent
      j.score-=1;
    }
  }

  int objet_aleatoire=rand()%3;
  
  if(objet_aleatoire==1)

    j.grille[rand()%LARGEUR][0]=1;


  return j;      
  
}

struct jeu verifier_collision(struct jeu j) {

  for(int x=0;x<LARGEUR;x++) {

    if(j.grille[x][HAUTEUR-1]==3) {

      j.score++;
      j.grille[x][HAUTEUR-1]=2;
    }
  }

  return j;
}


/*
void sauvegarde_partie(struct jeu j){

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","w");

  // Enregistre la grille sur la première ligne

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++)
      fprintf(fichier_sauvegarde,"%d\t",j.grille[x][y]);

  // Enregistre le score à la suite sur la même ligne

  fprintf(fichier_sauvegarde,"%d\t",j.score);

  // Enregistre la taille du radeau après le score

  fprintf(fichier_sauvegarde,"%d",j.taille);

  fclose(fichier_sauvegarde);
}

struct jeu charge_partie(){

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
  struct jeu j;

  // Charge la grille

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++){
      
      fscanf(fichier_sauvegarde,"%d",&j.grille[x][y]);

    }

  // Charge le score
  
  fscanf(fichier_sauvegarde,"%d",&j.score);

  // Charge la taille du radeau

  fscanf(fichier_sauvegarde,"%d",&j.taille);

  fclose(fichier_sauvegarde);

  return j;
}
*/


void sauvegarde_partie(struct jeu j, int numero_sauvegarde){

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");

  // Enregistre la grille sur la première ligne

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++)
      fprintf(fichier_sauvegarde,"%d\t",j.grille[x][y]);

  // Enregistre le score à la suite sur la même ligne

  fprintf(fichier_sauvegarde,"%d\t",j.score);

  // Enregistre la taille du radeau après le score

  fprintf(fichier_sauvegarde,"%d",j.taille);

  fclose(fichier_sauvegarde);
}

struct jeu charge_partie(int numero_sauvegarde){

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
  struct jeu j;
  int taille_ligne=LARGEUR*HAUTEUR*2+7+3;  // Nombre de char de la grille + Nombre de char du score + Nb de char de la taille du radeau
  char buffer[16];
  
  // Se déplace dans le fichier à la ligne de la sauvegarde à charger

  fseek(fichier_sauvegarde,taille_ligne*numero_sauvegarde,SEEK_SET);
  
  // Charge la grille

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++){
      
      fgets(buffer,2,fichier_sauvegarde);
      //printf("Char n° : %d\tValeur : %s\n",ftell(fichier_sauvegarde),buffer);

      sscanf(buffer,"%d",&j.grille[x][y]);
      fseek(fichier_sauvegarde,1,SEEK_CUR); // Décale la lecture sur la ligne
    }

  // Charge le score

  fgets(buffer,7,fichier_sauvegarde);
  sscanf(buffer,"%d",&j.score);

  // Charge la taille du radeau

  fseek(fichier_sauvegarde,1,SEEK_CUR);
  fgets(buffer,3,fichier_sauvegarde);
  sscanf(buffer,"%d",&j.taille);

  fseek(fichier_sauvegarde,1,SEEK_CUR);
  printf("%d - %d\t%s\n",taille_ligne,ftell(fichier_sauvegarde),buffer);
  fgets(buffer, 16,fichier_sauvegarde);

  fclose(fichier_sauvegarde);

  return j;
}

