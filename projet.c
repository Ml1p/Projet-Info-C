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

// Code flèche gauche 91 68
// Code Flèche droite 91 67
// Code flèche haut   91 65
// Code Flèche bas    91 66

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

  for(int x=0; x<p.taille*2+1;x++){
    p.grille[LARGEUR/2-p.taille+x][HAUTEUR-1]=2;
  }
  return p;
}

int main(void) {
      
  srand(time(NULL));
  struct jeu init_jeu();
  struct jeu p = init_jeu();

  void affiche_jeu(struct jeu,int);
  void affiche_menu(struct jeu);
  struct jeu déplacer(char direction, struct jeu);
  struct jeu mise_a_jour_objets(struct jeu);
  struct jeu verifier_collision(struct jeu);
  void sauvegarde_partie(struct jeu, int);
  struct jeu charge_partie(int);
  void config_terminal();
  void affiche_menu_sauvegardes(int,int);
  
  config_terminal();

  int mode=0; // Mode affiché (Jeu, Menu, Pause, Sélection Sauvegarde...)
  int gameOver=0;
  int sequence_touche[2]={0,0};
  int selection=0;
  int i=0;
  char touche;
  while(p.score>-50 && gameOver!=1) {
  
    switch (mode){
    case 0: // En mode jeu
      
      if(read(STDIN_FILENO,&touche,1) == 1){
        
        p=déplacer(touche,p);

        if(touche=='m'){ // Change le mode au menu

          mode=2;
          continue;
        }
      }

      // Permet de déplacer le radeau avec les flèches directionnelles

      if(sequence_touche[0]==91 && sequence_touche[1]==68)
        p=déplacer('a',p);
      
      if(sequence_touche[0]==91 && sequence_touche[1]==67)
        p=déplacer('d',p);


      // Fait descendre les objets toutes les 10 images
      // Permet de séparer le nombre d'images par secondes et la difficulté
      if(i==10){ 
        
        p=mise_a_jour_objets(p);
        p=verifier_collision(p);
        i=0;
      }

      affiche_jeu(p,mode);
      break;
    
    case 1: // En mode Pause

      if(read(STDIN_FILENO,&touche,1) == 1){
    
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

    case 2: // Menu
      
      if(read(STDIN_FILENO,&touche,1) == 1){
      
        if(touche=='s')
          mode=3;
        
        if(touche=='c')
          mode=4;

        if(touche=='m')
          mode=1;
        
        if(touche=='q')
          gameOver=1;
        
        if(touche=='r'){
          p=init_jeu();
          mode=0;
          i=0;
        }
      }

      affiche_menu(p);
      break;
    
    case 3: // Sélection Sauvegarde

      // Détermine le nombre de lignes dans le fichier sauvegarde

      FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
      int taille_ligne=LARGEUR*HAUTEUR*2+7+2+1;

      fseek(fichier_sauvegarde,0,SEEK_END);
      int nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne;
      fseek(fichier_sauvegarde,0,SEEK_SET);
  
      fclose(fichier_sauvegarde);

      // Fait monter le curseur de sélection de sauvegarde et le rammène en haut si on est sur la dernière sauvegarde
      if(sequence_touche[0]==91 && sequence_touche[1]==65 && selection>=0){
        
        if(selection>0)
          selection--;
        
        else
          selection=nb_lignes_fichier;
      }
      
      // Analogue pour la montée
      if(sequence_touche[0]==91 && sequence_touche[1]==66 && selection<=nb_lignes_fichier){
        
        if(selection<nb_lignes_fichier)
          selection++;
        
        else
          selection=0;
      }
        
      
      // Sauvegarde le jeu à l'emplacement sélectionné
      if(read(STDIN_FILENO,&touche,1)==1){
        if(touche=='\n' || touche=='f'){
          
          sauvegarde_partie(p,selection);
          mode=2;
        }

        if(touche=='m' || touche=='s')
          mode=2;
      }

      affiche_menu_sauvegardes(selection,1);
      break;


    case 4: // Sélection Chargement (Analogue à la sélection de la sauvegarde)

      fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
      taille_ligne=LARGEUR*HAUTEUR*2+7+2+1;

      fseek(fichier_sauvegarde,0,SEEK_END);
      nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne;
      fseek(fichier_sauvegarde,0,SEEK_SET);
  
      fclose(fichier_sauvegarde);

      // Montée et Descente
      if(sequence_touche[0]==91 && sequence_touche[1]==65 && selection>=0){
        
        if(selection>0)
          selection--;
        
        else
          selection=nb_lignes_fichier-1;
      }
      
      if(sequence_touche[0]==91 && sequence_touche[1]==66 && selection<=nb_lignes_fichier-1){
        
        if(selection<nb_lignes_fichier-1)
          selection++;
        
        else
          selection=0;
      }
        
      
      // Charge le jeu à l'emplacement sélectionné
      if(read(STDIN_FILENO,&touche,1)==1){
        if(touche=='\n' || touche=='f'){
          
          p=charge_partie(selection);
          mode=2;
        }

        if(touche=='m' || touche=='c')
          mode=2;
      }

      affiche_menu_sauvegardes(selection,0);
      break;
    }

  sequence_touche[0]=sequence_touche[1];
  sequence_touche[1]=getchar();
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

void sauvegarde_partie(struct jeu j, int numero_sauvegarde){

  int taille_ligne=LARGEUR*HAUTEUR*2+7+3;  // Nombre de char de la grille + Nombre de char du score + Nb de char de la taille du radeau

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");

  fseek(fichier_sauvegarde,0,SEEK_END); // Va à la fin du fichier pour connaitre le nombre de charactères dans le fichier
  int nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne; // Nombre de Charactères / Nombre de charactères dans une ligne = Nombre de lignes
  fseek(fichier_sauvegarde,0,SEEK_SET); // Retourne au début du fichier


  // Stoppe la sauvegarde si le numéro de sauvegarde est trop grand

  if(numero_sauvegarde>nb_lignes_fichier+1)
    return;


  // Enregistre l'entièreté du fichier

  char buffer[taille_ligne+1];
  char contenu_fichier[nb_lignes_fichier][taille_ligne+1];

  for(int i=0; i<nb_lignes_fichier;i++){
    
    fgets(buffer,taille_ligne+1,fichier_sauvegarde);
    fseek(fichier_sauvegarde,0,SEEK_CUR);

    for(int j=0;j<taille_ligne+1;j++)
      contenu_fichier[i][j]=buffer[j];
  }


  // Modifie la ligne de la sauvegarde

  if(numero_sauvegarde<=nb_lignes_fichier){

    // Ecrit la grille dans le buffer
    for(int y=0;y<HAUTEUR;y++)
      for(int x=0;x<LARGEUR;x++){

        buffer[x*2+y*LARGEUR*2]=j.grille[x][y]+'0';
        buffer[x*2+y*LARGEUR*2+1]='\t';
      }
    

    // Formate le score (000000)
    char temp_buffer[7];
    snprintf(temp_buffer,7,"%06d",j.score);

    // Ecrit le score dans le buffer
    for(int i=0;i<6;i++){
      
      buffer[HAUTEUR*LARGEUR*2+i]=temp_buffer[i];
    }
    buffer[HAUTEUR*LARGEUR*2+6]='\t';

    // Formate la taille du radeau (00)
    snprintf(temp_buffer,3,"%02d",j.taille);

    // Ecrit la taille du radeau dans le buffer
    for(int i=0;i<2;i++)
      buffer[HAUTEUR*LARGEUR*2+7+i]=temp_buffer[i];
    
    buffer[HAUTEUR*LARGEUR*2+9]='\n';


    // Copie le buffer dans le fichier enregistré

    for(int i=0;i<taille_ligne+1;i++)
      contenu_fichier[numero_sauvegarde][i]=buffer[i];
  }


  // Réécrit le fichier modifié dans le fichier sauvegarde

  fclose(fichier_sauvegarde);
  fichier_sauvegarde=fopen("fichier_sauvegarde.txt","w");

  for(int i=0;i<nb_lignes_fichier+1;i++){

    fputs(contenu_fichier[i],fichier_sauvegarde);
  }
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


  // Détermine le nombre de sauvegardes

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
  int taille_ligne=LARGEUR*HAUTEUR*2+7+2+1;

  fseek(fichier_sauvegarde,0,SEEK_END);
  int nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne;
  fseek(fichier_sauvegarde,0,SEEK_SET);
  
  fclose(fichier_sauvegarde);

  int meilleur_score=0;
  for(int i=0;i<nb_lignes_fichier;i++)

    if(charge_partie(i).score>meilleur_score)

      meilleur_score=charge_partie(i).score;

	printf("jeu en pause !\n\n\n");
  printf("MEILLEUR SCORE : (%d)\n\n",meilleur_score);
	printf("score : (%d)\n",j.score);
	printf("m: reprendre la partie\n");
	printf("s: sauvegarder la partie\n");
	printf("c: charger la partie\n");
	printf("r: recommencer une partie\n");
	printf("q: quitter le jeu\n");
	
}

void affiche_menu_sauvegardes(int numero_sauvegarde, int mode_sauvegarde){

  system("clear");

  // Détermine le nombre de sauvegardes

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
  int taille_ligne=LARGEUR*HAUTEUR*2+7+2+1;

  fseek(fichier_sauvegarde,0,SEEK_END);
  int nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne;
  fseek(fichier_sauvegarde,0,SEEK_SET);
  
  fclose(fichier_sauvegarde);


  for(int i=0;i<nb_lignes_fichier;i++){
    
    // Indique la sauvegarde sélectionnée
    if(i==numero_sauvegarde)
      printf("*  ");
    
    printf("Sauvegarde %d",i+1);

    if(i==numero_sauvegarde)
      printf("  *");
    
    printf("\n");
  }

  // Affiche la nouvelle sauvegarde en dernière
  if(mode_sauvegarde==1){
    if(numero_sauvegarde==nb_lignes_fichier)
      printf("*  ");
    
    printf("Nouvelle Sauvegarde");

    if(numero_sauvegarde==nb_lignes_fichier)
      printf("  *");
  }


  // Affiche le score et la taille du radeau de la sauvegarde séléctionnée

  if(numero_sauvegarde<nb_lignes_fichier){
    struct jeu sauvegarde=charge_partie(numero_sauvegarde);
    printf("\n\nScore: %d\nTaille du radeau: %d",numero_sauvegarde,sauvegarde.score,sauvegarde.taille);
  }
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