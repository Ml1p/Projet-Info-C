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
  int mode=0; // 0 Jeu en cours, 1 Menu, 2 Jeu en Pause
  int gameOver=0;

  void affiche_jeu(struct jeu,int);
  void affiche_menu(struct jeu);
  struct jeu déplacer(char direction, struct jeu);
  struct jeu mise_a_jour_objets(struct jeu);
  struct jeu verifier_collision(struct jeu);
  void sauvegarde_partie(struct jeu);
  struct jeu charge_partie();
  void config_terminal();
  
  config_terminal();
  

  int i=0;
  char touche;
  while(p.score>-50 && gameOver!=1) {
  
    switch (mode){
    case 0: // En mode jeu
      
      if(read(STDIN_FILENO, &touche,1) == 1){
        
        p=déplacer(touche,p);

        if(touche=='m'){ // Change le mode au menu

          mode=2;
          continue;
        }
      }

      // Fait descendre les objets toutes les 10 images
      // Permet de séparer le nombre d'images par secondes et la difficulté
      if(i==10){ 
        
        p=mise_a_jour_objets(p);
        p=verifier_collision(p);
        i=0;
      }

      affiche_jeu(p,mode);
      printf("i %d",i);
      break;
    
    case 1: // En mode Pause

      if(read(STDIN_FILENO, &touche,1) == 1){
    
        if(touche=='m'){
          
          mode=2;
          continue;
        }

        if(touche=='a' || touche=='d'){
          
          p=déplacer(touche,p);
          mode=0;
          i=0;
          continue;
        }
      }

      affiche_jeu(p,mode);
      break;

    case 2: // En mode Menu
      
      if(read(STDIN_FILENO, &touche,1) == 1){
      
        if(touche=='s')
          sauvegarde_partie(p);
        
        if(touche=='c')
          p=charge_partie();

        if(touche=='m')
          mode=1;
        
        if(touche=='q')
          gameOver=1;
        
        if(touche=='r')
          init_jeu(p);
      }

      affiche_menu(p);
      break;

    }

  usleep(1e4);
  i++;
  }
} 


void affiche_jeu(struct jeu j,int jeu_en_pause){

  system("clear");

  if(jeu_en_pause==1)
    printf("Appuyez sur a ou d pour Continuer\n\n\n");

  // Affiche le haut de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf("*  ");
    
  printf("\n");
  for(int y=0;y<HAUTEUR;y++) {
  
    // Affiche le bord gauche de la grille
    printf("*  ");

    // Affiche une ligne de la grille
    for(int x=0;x<LARGEUR;x++){

      if (j.grille[x][y] == 0)
        printf("   ");

      if (j.grille[x][y] == 1)
        printf("0  ");

      if (j.grille[x][y] == 2)
        printf("-  ");
      }
      
    // Affiche le bord droit de la grille
    printf("*  \n");
    }
    
  // Affiche le bas de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf("*  ");

  printf("\n");

  printf("Score : %d\n\n\n\n\n\n\n\n",j.score);

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
  int taille_ligne=LARGEUR*HAUTEUR*2+7+3;  // Nombre de char de la grille + Nombre de char du score + Nb de char de la taille du radeau
  char buffer[16];

  // Se déplace dans le fichier à la ligne de la sauvegarde à charger

  fseek(fichier_sauvegarde,0,SEEK_SET);

  // Charge la grille

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++){
      
      fgets(buffer,2,fichier_sauvegarde);

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

  fclose(fichier_sauvegarde);

  return j;
}

// affichage du menu

void affiche_menu(struct jeu j) {
	system("clear");
  //printf("MEILLEUR SCORE :("%d\n\n\n") :)",p.meilleur_score);
	printf("jeu en pause !\n\n\n");
	printf("score : (%d)\n",j.score);
	printf("m: reprendre la partie\n");
	printf("s: sauvegarder la partie\n");
	printf("c: charger la partie\n");
	printf("r: recommencer une partie\n");
	printf("q: quitter le jeu\n");
	
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