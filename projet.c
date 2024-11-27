// Bénard Clément
// Abancourt-Bevilacqua Titouan

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#define N 10
#define EPS 1.e-8

#define LARGEUR 10
#define HAUTEUR 30

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
  void sauvegarde_partie(struct jeu);
  struct jeu charge_partie();
  void config_terminal();
  
  config_terminal();
  

  int i=0;
  char touche;
  while(p.score>-50) {
  
	if(read(STDIN_FILENO, &touche,1) == 1) 
		p=déplacer(touche,p);
  	
  	
      	if(i==10){ // Fait descendre les objets toutes les 10 images
      		// Permet de séparer le nombre d'images par secondes et la difficulté
      	
		p=mise_a_jour_objets(p);
		p=verifier_collision(p);
		i=0;
	
	}

	affiche_jeu(p);
	printf("Score : %d\n\n\n\n\n\n\n\n",p.score);
	usleep(1e4);
	i++;
  }
} 


void affiche_jeu(struct jeu j){

  system("clear");

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
    printf("*\t\n");
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
    return j;
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
    return j;
  }
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




struct termios ancien_param;

void restaurer_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &ancien_param);
}

// Configuration du terminal pour lire les entrées sans appuyer sur Entrée
void config_terminal() {
    struct termios nouveau_param;
    
    // Sauvegarde de la configuration actuelle du terminal
    tcgetattr(STDIN_FILENO, &ancien_param);
    nouveau_param = ancien_param;
    nouveau_param.c_lflag &= ~(ICANON | ECHO); // Désactiver le mode canonique et l'affichage
    
    tcsetattr(STDIN_FILENO, TCSANOW, &nouveau_param);
    
    // Rendre l'entrée non bloquante
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    
    // Rétablissement automatique de la configuration à la sortie
    atexit(restaurer_terminal);
}
