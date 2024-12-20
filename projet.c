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

#define LARGEUR 15
#define HAUTEUR 35

// Code flèche gauche 91 68
// Code Flèche droite 91 67
// Code flèche haut   91 65
// Code Flèche bas    91 66

// 1 > Objet    5 > Méga Objet
// 2 > Bombe    6 > Méga Bombe
// 3 > Bonus    7 > Méga Bonus
// 4 > Malus    8 > Méga Malus

struct jeu {
  int grille[LARGEUR][HAUTEUR];
  int score;
  int taille;
  int position_radeau;
};

struct jeu init_jeu(){
  struct jeu p;
  p.score=-49;
  p.taille=5;
  p.position_radeau=LARGEUR/2-p.taille/2-1;
  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++)
      p.grille[x][y]=0;

  for(int x=0; x<p.taille;x++){
    p.grille[LARGEUR/2-p.taille/2-1+x][HAUTEUR-1]=30;
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
  struct jeu mise_a_jour_objets(struct jeu, int[4], int);
  struct jeu verifier_collision(struct jeu, int*);
  void sauvegarde_partie(struct jeu, int);
  struct jeu charge_partie(int);
  void config_terminal();
  void affiche_menu_sauvegardes(int,int);
  void affiche_menu_difficulté(int);
  
  config_terminal();

  int mode=7; // Mode affiché (Jeu, Menu, Pause, Sélection Sauvegarde...)
  int gameOver=0;
  int difficulté;
  int nb_radeaux;
  int fréquence_Apparition[4]={2,4,6,6};
  int fréquence_Vitesse=5;
  int sequence_touche[2]={0,0};
  int selection=0;
  int temp_difficulté=0;
  int i=0;
  char touche;
  while(p.score>-50 && gameOver!=1){
  
    switch (mode){
    case 0: // En mode jeu
      
      if(read(STDIN_FILENO,&touche,1) == 1){
        
        p=déplacer(touche,p);

        if(touche=='m'){ // Change le mode au menu

          mode=2;
          i=0;
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
      if(i==difficulté){
        
        p=mise_a_jour_objets(p,fréquence_Apparition,fréquence_Vitesse);
        p=verifier_collision(p, &gameOver);
        i=0;

        // Termine le jeu si le radeau est détruit
        nb_radeaux=0;
        for(int x=0;x<LARGEUR;x++){
          if(p.grille[x][HAUTEUR-1]>=30)

            nb_radeaux++;
        }

        if(nb_radeaux==0){
          gameOver=1;
          break;
        }
        
        i=0;
      }

      // Augmente la difficulté avec le temps
      if(temp_difficulté==60000){

        if(difficulté>5)
          difficulté--;
        
        temp_difficulté=0;
      }

      affiche_jeu(p,mode);
      break;
    
    case 1: // En mode Pause

      if(read(STDIN_FILENO,&touche,1) == 1){
    
        if(touche=='m'){
          
          mode=2;
          i=0;
          continue;
        }

        if(touche=='a' || touche=='d' || sequence_touche[0]==91 && sequence_touche[1]==68 || sequence_touche[0]==91 && sequence_touche[1]==67){
          
          p=déplacer(touche,p);
          mode=0;
          i=0;
          temp_difficulté=0;
          continue;
        }
      }

      affiche_jeu(p,mode);
      break;

    case 2: // Menu
      
      if(read(STDIN_FILENO,&touche,1) == 1){
      
        if(touche=='s'){
          mode=3;
          i=0;
        }
        
        if(touche=='c'){
          mode=4;
          i=0;
        }

        if(touche=='m'){
          mode=1;
          i=0;
        }
        
        if(touche=='q')
          gameOver=1;
        
        if(touche=='r'){
          p=init_jeu();
          mode=7;
          i=0;
          temp_difficulté=0;
        }

        if(touche=='j'){
        	mode=5;
          i=0;
        }

        if(touche=='d'){
          mode=6;
          i=0;
        }
      }

      if(i==5){
        
        affiche_menu(p);
        i=0;
      }

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
          i=0;
        }

        if(touche=='m' || touche=='s')
          mode=2;
          i=0;
      }

      if(i==5){
        
        affiche_menu_sauvegardes(selection,1);
        i=0;
      }

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
          i=0;
        }

        if(touche=='m' || touche=='c')
          mode=2;
          i=0;
        }

      if(i==5){

        affiche_menu_sauvegardes(selection,0);
        i=0;
      }

      break;
      
    case 5: // Crédits
      system("clear");
      printf("Bienvenue dans le meilleur jeu que cette UE vous proposera, Le principe est simple, récolter le plus d'objets possible à l'aide de votre radeau (1 objet récolté= +1 point, 1 objet perdu = -1 point). Mais gare aux bombes (Q) ! En toucher une fera exploser une partie de votre radeau, créant un trou béant... Des malus et des bonus seront aussi attrapables. Un + recolté aggrandira la taille de votre radeau alors qu'un - le rétrécira. Attention à ne pas atteindre un score de -50 ou ce sera GAME OVER. Bon jeu à toi jeune pirate. De nouvelles mises à jour seront prochainement disposibles ! n'hésites pas à aider au bon développement du jeu en faisant un don à l'adresse Paypal suivante : benarclem@gmail.com ; Merci !");
      break;
    
    case 6: // Sélection de la difficulté

      // Montée et Descente
      if(sequence_touche[0]==91 && sequence_touche[1]==65 && selection>=0){
        
        if(selection>0)
          selection--;
        
        else
          selection=4;
      }
      
      if(sequence_touche[0]==91 && sequence_touche[1]==66 && selection<=4){
        
        if(selection<4)
          selection++;
        
        else
          selection=0;
      }

      // Sélectionne la difficulté
      if(read(STDIN_FILENO,&touche,1)==1){

        if(touche=='\n' || touche=='f'){

          // Difficulté Facile
          if(selection==0){
            
            difficulté=25; // Rapidité de la descente au début

            fréquence_Apparition[0]=3; // Chance d'apparition d'un Objet
            fréquence_Apparition[1]=24; // Chance d'apparition d'une Bombe
            fréquence_Apparition[2]=48; // Chance d'apparition d'un Bonus
            fréquence_Apparition[3]=48; // Chance d'apparition d'un Malus

            fréquence_Vitesse=15; // Chance qu'un Objet apparaisse avec une vitesse horizontalle
          }

          // Difficulté Normale
          if(selection==1){
            
            difficulté=20;

            fréquence_Apparition[0]=3;
            fréquence_Apparition[1]=24;
            fréquence_Apparition[2]=48;
            fréquence_Apparition[3]=48;

            fréquence_Vitesse=10;
          }

          // Difficulté Difficile
          if(selection==2){
            
            difficulté=20;

            fréquence_Apparition[0]=6;
            fréquence_Apparition[1]=12;
            fréquence_Apparition[2]=48;
            fréquence_Apparition[3]=24;

            fréquence_Vitesse=7;
          }

          // Simulateur de pluie
          if(selection==3){
            
            difficulté=5;

            fréquence_Apparition[0]=3;
            fréquence_Apparition[1]=224;
            fréquence_Apparition[2]=248;
            fréquence_Apparition[3]=248;

            fréquence_Vitesse=116;
          }

          // Flipper
          if(selection==4){
            
            difficulté=1;

            fréquence_Apparition[0]=3;
            fréquence_Apparition[1]=224;
            fréquence_Apparition[2]=248;
            fréquence_Apparition[3]=248;

            fréquence_Vitesse=3;
            p.score=500;
          }

          mode=0;
          i=0;
          temp_difficulté=0;
        }
        
        if(touche=='m'){
          mode=2;
          i=0;
        }
      }

      if(i==5){

        affiche_menu_difficulté(selection);
        i=0;
      }

      break;
    
    case 7: // Sélection difficulté début de partie (Identique à la Sélection de difficulté sans le retour au menu)

      // Montée et Descente
      if(sequence_touche[0]==91 && sequence_touche[1]==65 && selection>=0){
        
        if(selection>0)
          selection--;
        
        else
          selection=4;
      }
      
      if(sequence_touche[0]==91 && sequence_touche[1]==66 && selection<=4){
        
        if(selection<4)
          selection++;
        
        else
          selection=0;
      }

      // Sélectionne la difficulté
      if(read(STDIN_FILENO,&touche,1)==1){

        if(touche=='\n' || touche=='f'){

          // Difficulté Facile
          if(selection==0){
            
            difficulté=25; // Rapidité de la descente au début

            fréquence_Apparition[0]=3; // Chance d'apparition d'un Objet
            fréquence_Apparition[1]=24; // Chance d'apparition d'une Bombe
            fréquence_Apparition[2]=48; // Chance d'apparition d'un Bonus
            fréquence_Apparition[3]=48; // Chance d'apparition d'un Malus

            fréquence_Vitesse=15; // Chance qu'un Objet apparaisse avec une vitesse horizontalle
          }

          // Difficulté Normale
          if(selection==1){
            
            difficulté=20;

            fréquence_Apparition[0]=3;
            fréquence_Apparition[1]=24;
            fréquence_Apparition[2]=48;
            fréquence_Apparition[3]=48;

            fréquence_Vitesse=10;
          }

          // Difficulté Difficile
          if(selection==2){
            
            difficulté=20;

            fréquence_Apparition[0]=6;
            fréquence_Apparition[1]=12;
            fréquence_Apparition[2]=48;
            fréquence_Apparition[3]=24;

            fréquence_Vitesse=7;
          }

          // Simulateur de pluie
          if(selection==3){
            
            difficulté=1;

            fréquence_Apparition[0]=2;
            fréquence_Apparition[1]=224;
            fréquence_Apparition[2]=248;
            fréquence_Apparition[3]=248;

            fréquence_Vitesse=116;
            p.score=500;
          }

                    // Flipper
          if(selection==4){
            
            difficulté=1;

            fréquence_Apparition[0]=3;
            fréquence_Apparition[1]=224;
            fréquence_Apparition[2]=248;
            fréquence_Apparition[3]=248;

            fréquence_Vitesse=3;
            p.score=500;
          }

          mode=0;
          i=0;
          temp_difficulté=0;
        }
      }

      if(i==5){

        affiche_menu_difficulté(selection);
        i=0;
      }

      break;
    }

  sequence_touche[0]=sequence_touche[1];
  sequence_touche[1]=getchar();
  usleep(1e4);
  i++;
  temp_difficulté++;
  }
  
  system("clear");
  printf("\n\n\n\n\n\n\n\n\n\t\tPerdu\n\n");
} 


void affiche_jeu(struct jeu j, int jeu_en_pause){

  system("clear");

	printf("\n\n\t\t");

  // Affiche le haut de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf(" * ");
    
  printf("\n");
  for(int y=0;y<HAUTEUR;y++){
  
    if(jeu_en_pause==1 && y==(int)(HAUTEUR/2))

      printf("\n\t\tAppuyez sur A, D ou les flèches directionnelles pour reprendre la partie\n\n");

    // Affiche le bord gauche de la grille
    printf("\t\t * ");

    // Affiche une ligne de la grille
    for(int x=0;x<LARGEUR;x++){

      if ((int)j.grille[x][y]/10==3){ // Radeau
        printf("---");
        continue;
      }

      if (j.grille[x][y]%10==0) // Vide
        printf("   ");

      if (j.grille[x][y]%10==1) // Objet
        printf(" 0 ");
       
      if (j.grille[x][y]%10==2) // Bombe
      	printf(" Q ");
      	     	
      if (j.grille[x][y]%10==3) // Bonus
      	printf(" + ");
      	
      if (j.grille[x][y]%10==4) // Malus
      	printf(" _ ");

      if (j.grille[x][y]%10==5) // Méga Objet
        printf("000");
      
      if (j.grille[x][y]%10==6) // Méga Bombe
        printf("QQQ");
      
      if (j.grille[x][y]%10==7) // Méga Bonus
        printf("+++");
      
      if (j.grille[x][y]%10==8) // Méga Malus
        printf("___");
      }
      
    // Affiche le bord droit de la grille
    printf(" * \n");
    }
  
  printf("\t\t");
  // Affiche le bas de la grille
  for(int i=0;i<LARGEUR+2;i++)
    printf(" * ");

  printf("\n");

  printf("   ");
  printf("\n\t\t");

  printf("Score : %d",j.score);

  return;
}

struct jeu déplacer(char direction, struct jeu j){

  if(direction=='a' && j.position_radeau>0){

    for(int x=j.position_radeau;x<j.taille+j.position_radeau;x++){
      
      if((int)(j.grille[x][HAUTEUR-1]/10)==3){ // Un radeau est inscrit comme un objet dont la dizaine est 3
        
        j.grille[x-1][HAUTEUR-1]+=30;
        j.grille[x][HAUTEUR-1]-=30;
      }
    }
    
    j.position_radeau--;
    return j;
  }

  if(direction=='d' && j.position_radeau<LARGEUR-j.taille){
  
    for(int x=j.taille+j.position_radeau;x>=j.position_radeau;x--){
      
      if((int)(j.grille[x][HAUTEUR-1]/10)==3){
        
        j.grille[x+1][HAUTEUR-1]+=30;
        j.grille[x][HAUTEUR-1]-=30;
      }
    }

    j.position_radeau++;
    return j;
  }
}

struct jeu mise_a_jour_objets(struct jeu j, int fréquence_Apparition[4],int fréquence_Vitesse){

  int cases[3];

  //----------------------------------------
  // Descente des objets
  //----------------------------------------

  int nouvel_Objet=0;

  for(int y=HAUTEUR-1;y>0;y--)
    for(int x=0;x<LARGEUR;x++){

      cases[0]=0;cases[1]=0;cases[2]=0;
    
      // Ajoute la case directement au dessus sur la grille au tableau si sa vitesse est correcte (verticale) et qu'elle contient un objet
      if((int)(j.grille[x][y-1]/10)==0 && j.grille[x][y-1]!=0){
        cases[0]=j.grille[x][y-1];
        j.grille[x][y-1]=0;
      }

      // Ajoute la case au dessus à gauche si sa vitesse est correcte (vers la droite) et qu'elle contient un objet
      if(x>0 && (int)(j.grille[x-1][y-1]/10)==2 && j.grille[x-1][y-1]!=0){
        cases[1]=j.grille[x-1][y-1];
        j.grille[x-1][y-1]=0;
      }

      // Ajoute la case au dessus à droite si sa vitesse est correcte (vers la gauche) et qu'elle contient un objet
      if(x<LARGEUR-1 && (int)(j.grille[x+1][y-1]/10)==1 && j.grille[x+1][y-1]!=0){
        cases[2]=j.grille[x+1][y-1];
        j.grille[x+1][y-1]=0;
      }

      // Vitesse

      nouvel_Objet=0;
      if(y<HAUTEUR-1){

        // Seul un des objets dans les cases supérieures à une vitesse Horizontalle => l'Objet créé garde cette vitesse
        // Si les deux vitesses s'annulent, la vitesse finale est nulle
        if(((int)(cases[1]/10)!=0 && cases[2]==0) || ((int)(cases[2]/10)!=0 && cases[1]==0))

          nouvel_Objet+=((int)(cases[1]/10)+(int)(cases[2]/10))*10;
        
        // Sur la bordure gauche + Vitesse horizontalle vers la gauche => Rebond
        if(x==0 && cases[2]!=0)
          nouvel_Objet+=10;
        
        // Sur la bordure droite + Vitesse horizontalle vers la droite => Rebond
        if(x==LARGEUR-1 && cases[1]!=0)
          nouvel_Objet-=10;
      }
      
      // Les trois casses sont vides => Rien à faire descendre
      if(cases[0]==0 && cases[1]==0 && cases[2]==0){

        nouvel_Objet=0;
        j.grille[x][y]+=nouvel_Objet;
        continue;
      }
      
      // Les trois cases sont pleines
      if(cases[0]!=0 && cases[1]!=0 && cases[2]!=0){

        // Les trois cases sont différentes => Annulation de la descente
        if(cases[0]%10!=cases[1]%10 && cases[0]%10!=cases[2]%10 && cases[1]%10!=cases[2]%10){
          
          nouvel_Objet=0;
          j.grille[x][y]+=nouvel_Objet;
          continue;
        }
        
        // 2 ou 3 cases sont égales => Création Méga Objet
        if(cases[0]%10==cases[1]%10 || cases[0]%10==cases[2]%10){
          
          // Créé le Méga Objet sauf si deux Méga Objets se rencontrent
          if(cases[0]%10<5){
            
            nouvel_Objet+=cases[0]%10+4;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
          else{

            nouvel_Objet+=cases[0]%10;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
        }
        if(cases[1]%10==cases[2]%10){

          if(cases[1]%10<5){
            
            nouvel_Objet+=cases[1]%10+4;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
          else{

            nouvel_Objet+=cases[1]%10;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
        }
      }

      // Deux cases sont pleines
      if((cases[0]!=0 && cases[1]!=0) || (cases[0]!=0 && cases[2]!=0) || (cases[1]!=0 && cases[2]!=0)){
        
        // Les deux cases pleines sont différentes => Annulation de la descente
        if((cases[0]==0 && cases[1]%10!=cases[2]%10) || (cases[1]==0 && cases[0]%10!=cases[2]%10) || (cases[2]==0 && cases[0]%10!=cases[1]%10)){

          nouvel_Objet=0;
          j.grille[x][y]+=nouvel_Objet;
          continue;
        }

        // Les deux cases pleines sont égales => Création Méga Objet
        if((cases[1]==0 && cases[0]%10==cases[2]%10) || (cases[2]==0 && cases[0]%10==cases[1]%10)){

          if(cases[0]%10<5){
            
            nouvel_Objet+=cases[0]%10+4;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
          else{

            nouvel_Objet+=cases[0]%10;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
        }
        if(cases[0]==0 && cases[1]%10==cases[2]%10){

          if(cases[1]%10<5){
            
            nouvel_Objet+=cases[1]%10+4;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
          else{

            nouvel_Objet+=cases[1]%10;
            j.grille[x][y]+=nouvel_Objet;
            continue;
          }
        }
      }
      
      // Seule une case est pleine => Fait descendre cette case
      if((cases[0]==0 && cases[1]==0) || (cases[0]==0 && cases[2]==0) || (cases[1]==0 && cases[2]==0)){

        if(cases[0]!=0){
         
          nouvel_Objet+=cases[0]%10;
          j.grille[x][y]+=nouvel_Objet;
          continue;
        }

        if(cases[1]!=0){
         
          nouvel_Objet+=cases[1]%10;
          j.grille[x][y]+=nouvel_Objet;
          continue;
        }

        if(cases[2]!=0){
         
          nouvel_Objet+=cases[2]%10;
          j.grille[x][y]+=nouvel_Objet;
          continue;
        }
      }
    }
    

  //----------------------------------------
  // Destruction des objets en bas de grille
  //----------------------------------------

  for(int x=0;x<LARGEUR;x++){

    int obj=j.grille[x][HAUTEUR-1];

    if(obj==1){ // Objet perdu

      j.grille[x][HAUTEUR-1]=0;
      j.score-=1;
    }

    if(obj==5){ // Méga Objet perdu

      j.grille[x][HAUTEUR-1]=0;
      j.score-=10;
    }
    
    // L'objet n'est ni un radeau, ni un objet, ni un Méga objet => Destruction sans toucher aux points
    if(obj<10 && obj!=1 && obj!=5)

      j.grille[x][HAUTEUR-1]=0;
  }
  
  //----------------------------------------
  // Création des nouveaux objets
  //----------------------------------------

  int vitesse_aléatoire=0;
  int random_x=0;

  int objet_aleatoire=rand()%fréquence_Apparition[0];
  
  if(objet_aleatoire==1){
    
    // Choisis une position aléatoire
    random_x=rand()%LARGEUR;

    // Donne une vitesse aléatoire à l'objet (1 ou 2 => Gauche ou droite; le reste => Pas de vitesse)
    vitesse_aléatoire=rand()%fréquence_Vitesse;

    if(vitesse_aléatoire>2)
      vitesse_aléatoire=0;
    
    if((random_x==0 && vitesse_aléatoire==1) || (random_x==LARGEUR-1 && vitesse_aléatoire==2))
      vitesse_aléatoire=0;
    
    j.grille[random_x][0]=1+vitesse_aléatoire*10;
  }
  
  int bombe=rand()%fréquence_Apparition[1];
  
  if(bombe==1){

    random_x=rand()%LARGEUR;
    
    vitesse_aléatoire=rand()%fréquence_Vitesse;

    if(vitesse_aléatoire>2)
      vitesse_aléatoire=0;
    
    if((random_x==0 && vitesse_aléatoire==1) || (random_x==LARGEUR-1 && vitesse_aléatoire==2))
      vitesse_aléatoire=0;

    j.grille[random_x][0]=2+vitesse_aléatoire*10;
  }
  
  int bonus=rand()%fréquence_Apparition[2];
  
  if(bonus==1){

    random_x=rand()%LARGEUR;

    vitesse_aléatoire=rand()%fréquence_Vitesse;

    if(vitesse_aléatoire>2)
      vitesse_aléatoire=0;
    
    if((random_x==0 && vitesse_aléatoire==1) || (random_x==LARGEUR-1 && vitesse_aléatoire==2))
      vitesse_aléatoire=0;

  	j.grille[random_x][0]=3+vitesse_aléatoire*10;
  }
  	
  int malus=rand()%fréquence_Apparition[3];
  
  if(malus==1){

    random_x=rand()%LARGEUR;

    vitesse_aléatoire=rand()%fréquence_Vitesse;

    if(vitesse_aléatoire>2)
      vitesse_aléatoire=0;
    
    if((random_x==0 && vitesse_aléatoire==1) || (random_x==LARGEUR-1 && vitesse_aléatoire==2))
      vitesse_aléatoire=0;

  	j.grille[random_x][0]=4+vitesse_aléatoire*10;
  }

  return j;
  
}

struct jeu verifier_collision(struct jeu j, int* gameOver){

  int nb_radeau_ajoutés;
  int décalage;
  int temp_taille=j.taille;

  for(int x=0;x<LARGEUR;x++) {

    if(j.grille[x][HAUTEUR-1]==31){ // Objet

      j.score++;
      j.grille[x][HAUTEUR-1]=30;
    }
    

    if(j.grille[x][HAUTEUR-1]==32){ // Bombe

	    j.grille[x][HAUTEUR-1]=0; // Suprime une partie du radeau au contact d'une bombe

      // Détermine la nouvelle position du radeau
      for(int x=0;x<LARGEUR;x++)
        if(j.grille[x][HAUTEUR-1]>=30){
          
          j.position_radeau=x;
          x=LARGEUR;
        }
      
      // Détermine la nouvelle taille du radeau
      for(int x=LARGEUR-1;x>=0;x--)
        if(j.grille[x][HAUTEUR-1]>=30){
          
          j.taille=x-j.position_radeau+1;
          x=0;
        }
	  }
	

    if(j.grille[x][HAUTEUR-1]==33){ // Bonus

      j.grille[x][HAUTEUR-1]=30;

      // Comble le radeau si il a des trous
      nb_radeau_ajoutés=0;
      for(int i=j.position_radeau;i<j.position_radeau+j.taille;i++)
        if(j.grille[i][HAUTEUR-1]<30 && nb_radeau_ajoutés<2){

          j.grille[i][HAUTEUR-1]=30;
          nb_radeau_ajoutés++;
        }
      
      // Détermine la nouvelle position du radeau
      for(int x=0;x<LARGEUR;x++)
        if(j.grille[x][HAUTEUR-1]>=30){
          
          j.position_radeau=x;
          x=LARGEUR;
        }
      

      // Aggrandis le radeau s'il n'as pas été comblé et qu'il n'est pas trop grand
      if(LARGEUR-j.taille>6){

        décalage=1;
        temp_taille=j.taille;
        while(nb_radeau_ajoutés<2 && décalage<LARGEUR){

          if(j.position_radeau-décalage>=0 && nb_radeau_ajoutés<2){
            
            j.grille[j.position_radeau-décalage][HAUTEUR-1]=30;
            nb_radeau_ajoutés++;
            temp_taille++;
          }

          if(j.position_radeau+j.taille+décalage-1<=LARGEUR-1 && nb_radeau_ajoutés<2){
            
            j.grille[j.position_radeau+j.taille+décalage-1][HAUTEUR-1]=30;
            nb_radeau_ajoutés++;
            temp_taille++;
          }
          décalage++;
        }
        
        // Détermine la nouvelle position et la nouvelle taille du radeau
        j.taille=temp_taille;

        for(int x=0;x<LARGEUR;x++)
          if(j.grille[x][HAUTEUR-1]>=30){

            j.position_radeau=x;
            x=LARGEUR;
          }
      }
    }


    if(j.grille[x][HAUTEUR-1]==34){ // Malus

      j.grille[x][HAUTEUR-1]=30;

      // Rétrécis le radeau si il est assez grand
      if(j.taille>3){

        temp_taille=j.taille;

        // Rétrécis le radeau
        nb_radeau_ajoutés=0;
        décalage=0;
        while(nb_radeau_ajoutés<2 && décalage<LARGEUR){
          
         if(j.position_radeau+décalage>=0 && j.grille[j.position_radeau+décalage][HAUTEUR-1]>=30 && nb_radeau_ajoutés<2){
            
            j.grille[j.position_radeau+décalage][HAUTEUR-1]=0;
            nb_radeau_ajoutés++;
            temp_taille--;
          }

          if(j.position_radeau+j.taille-décalage-1<=LARGEUR-1 && j.grille[j.position_radeau+j.taille-décalage-1][HAUTEUR-1]>=30 && nb_radeau_ajoutés<2){
            
            j.grille[j.position_radeau+j.taille-décalage-1][HAUTEUR-1]=0;
            nb_radeau_ajoutés++;
            temp_taille--;
          }

          décalage++;
        }
        
        // Détermine les nouvelles position et taille du radeau
        j.taille=temp_taille;

        for(int x=0;x<LARGEUR;x++)
          if(j.grille[x][HAUTEUR-1]>=30){

            j.position_radeau=x;
            x=LARGEUR;
          }
      }

      // Termine le jeu si le radeau est trop petit
      else{

        * gameOver=1;
      }
    }


    if(j.grille[x][HAUTEUR-1]==35){ // Méga Objet (Analogue à l'objet)

      j.score+=10;
      j.grille[x][HAUTEUR-1]=30;
    }

    
    if(j.grille[x][HAUTEUR-1]==36){ // Méga Bombe (Termine le jeu)

      j.grille[x][HAUTEUR-1]=30;
      * gameOver=1;
    }

  
    if(j.grille[x][HAUTEUR-1]==37){ // Méga Bonus (Analogue du bonus)

      j.grille[x][HAUTEUR-1]=30;

      // Comble le radeau si il a des trous
      nb_radeau_ajoutés=0;
      for(int i=j.position_radeau;i<j.position_radeau+j.taille;i++)
        if(j.grille[i][HAUTEUR-1]<30 && nb_radeau_ajoutés<4){

          j.grille[i][HAUTEUR-1]=30;
          nb_radeau_ajoutés++;
        }
      
      // Détermine la nouvelle position du radeau
      for(int x=0;x<LARGEUR;x++)
        if(j.grille[x][HAUTEUR-1]>=30){
          
          j.position_radeau=x;
          x=LARGEUR;
        }
      

      // Aggrandis le radeau s'il n'as pas été comblé et qu'il n'est pas trop grand
      if(LARGEUR-j.taille>6){

        décalage=1;
        temp_taille=j.taille;
        while(nb_radeau_ajoutés<4 && décalage<LARGEUR){

          if(j.position_radeau-décalage>=0 && nb_radeau_ajoutés<4){
            
            j.grille[j.position_radeau-décalage][HAUTEUR-1]=30;
            nb_radeau_ajoutés++;
            temp_taille++;
          }

          if(j.position_radeau+j.taille+décalage-1<=LARGEUR-1 && nb_radeau_ajoutés<4){
            
            j.grille[j.position_radeau+j.taille+décalage-1][HAUTEUR-1]=30;
            nb_radeau_ajoutés++;
            temp_taille++;
          }
          décalage++;
        }
        
        // Détermine la nouvelle position et la nouvelle taille du radeau
        j.taille=temp_taille;

        for(int x=0;x<LARGEUR;x++)
          if(j.grille[x][HAUTEUR-1]>=30){

            j.position_radeau=x;
            x=LARGEUR;
          }
      }
    }


    if(j.grille[x][HAUTEUR-1]==38){ // Méga Malus (Analogue au bonus)
      
      j.grille[x][HAUTEUR-1]=30;

      // Rétrécis le radeau si il est assez grand
      if(j.taille>5){

        temp_taille=j.taille;

        // Rétrécis le radeau
        nb_radeau_ajoutés=0;
        décalage=0;
        while(nb_radeau_ajoutés<4 && décalage<LARGEUR){
          
          if(j.position_radeau+décalage>=0 && j.grille[j.position_radeau+décalage][HAUTEUR-1]>=30 && nb_radeau_ajoutés<4){
            
            j.grille[j.position_radeau+décalage][HAUTEUR-1]=0;
            nb_radeau_ajoutés++;
            temp_taille--;
          }

          if(j.position_radeau+j.taille-décalage-1<=LARGEUR-1 && j.grille[j.position_radeau+j.taille-décalage-1][HAUTEUR-1]>=30 && nb_radeau_ajoutés<4){
            
            j.grille[j.position_radeau+j.taille-décalage-1][HAUTEUR-1]=0;
            nb_radeau_ajoutés++;
            temp_taille--;
          }

          décalage++;
        }
        
        // Détermine les nouvelles position et taille du radeau
        j.taille=temp_taille;

        for(int x=0;x<LARGEUR;x++)
          if(j.grille[x][HAUTEUR-1]>=30){

            j.position_radeau=x;
            x=LARGEUR;
          }
      }

      // Termine le jeu si le radeau est trop petit
      else{

        * gameOver=1;
      }
    }
  }

  return j;
}

void sauvegarde_partie(struct jeu j, int numero_sauvegarde){

  int taille_ligne=LARGEUR*HAUTEUR*3+7+3+4;  // Nombre de char de la grille + Nombre de char du score + Nb de char de la taille du radeau + Position Radeau

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");

  fseek(fichier_sauvegarde,0,SEEK_END); // Va à la fin du fichier pour connaitre le nombre de charactères dans le fichier
  int nb_lignes_fichier=ftell(fichier_sauvegarde)/taille_ligne+1; // Nombre de Charactères / Nombre de charactères dans une ligne +1 (Ligne vide)= Nombre de lignes
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
    
    int position_fichier=0;
    char temp_buffer[7];

    // Ecrit la grille dans le buffer
    for(int y=0;y<HAUTEUR;y++)
      for(int x=0;x<LARGEUR;x++){

        // Formate l'objet (00)
        snprintf(temp_buffer,3,"%02d",j.grille[x][y]);

        // Ecrit l'objet formatté dans le buffer
        buffer[position_fichier]=temp_buffer[0];
        buffer[position_fichier+1]=temp_buffer[1];
        buffer[position_fichier+2]='\t';
        position_fichier+=3;      
      }

    // Formate le score (000000)
    snprintf(temp_buffer,7,"%06d",j.score);

    // Ecrit le score dans le buffer
    for(int i=0;i<6;i++)  
      buffer[position_fichier+i]=temp_buffer[i];

    position_fichier+=6;
    
    buffer[position_fichier]='\t';
    position_fichier++;

    // Formate la taille du radeau (00)
    snprintf(temp_buffer,3,"%02d",j.taille);

    // Ecrit la taille du radeau dans le buffer
    for(int i=0;i<2;i++)
      buffer[position_fichier+i]=temp_buffer[i];
    
    position_fichier+=2;

    buffer[position_fichier]='\t';
    position_fichier++;

    // Formate la position du radeau
    snprintf(temp_buffer,3,"%02d",j.position_radeau);

    for(int i=0;i<2;i++)
      buffer[position_fichier+i]=temp_buffer[i];
    
    position_fichier+=2;

    // Copie le buffer dans le fichier enregistré

    for(int i=0;i<taille_ligne+1;i++)
      contenu_fichier[numero_sauvegarde][i]=buffer[i];
  }


  // Réécrit le fichier modifié dans le fichier sauvegarde

  fclose(fichier_sauvegarde);
  fichier_sauvegarde=fopen("fichier_sauvegarde.txt","w");

  for(int i=0;i<nb_lignes_fichier;i++){

    fputs(contenu_fichier[i],fichier_sauvegarde);
  }
  fclose(fichier_sauvegarde);
}

struct jeu charge_partie(int numero_sauvegarde){

  FILE *fichier_sauvegarde=fopen("fichier_sauvegarde.txt","r");
  struct jeu j;
  int taille_ligne=LARGEUR*HAUTEUR*3+7+3+3;  // Nombre de char de la grille + Nombre de char du score + Nb de char de la taille du radeau
  char buffer[16];

  // Se déplace dans le fichier à la ligne de la sauvegarde à charger

  fseek(fichier_sauvegarde,taille_ligne*numero_sauvegarde,SEEK_SET);

  // Charge la grille

  for(int y=0;y<HAUTEUR;y++)
    for(int x=0;x<LARGEUR;x++){
      
      fgets(buffer,3,fichier_sauvegarde);

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

  // Charge la position du radeau

  fseek(fichier_sauvegarde,1,SEEK_CUR);
  fgets(buffer,3,fichier_sauvegarde);
  sscanf(buffer,"%d",&j.position_radeau);

  fclose(fichier_sauvegarde);

  return j;
}

// affichage du menu

void affiche_menu(struct jeu j){
	
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

	printf("\n\n");
  printf("\t\tMEILLEUR SCORE : (%d)\n",meilleur_score);
	printf("\t\tscore : (%d)\n\n\n",j.score);
	printf("\t\tm: reprendre la partie\n\n");
	printf("\t\ts: sauvegarder la partie\n\n");
	printf("\t\tc: charger la partie\n\n");
	printf("\t\tr: recommencer une partie\n\n");
	printf("\t\tq: quitter le jeu\n\n");
	printf("\t\tj: règle\n\n");
  printf("\t\td: choix difficulté\n");
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

  printf("\n\n");

  for(int i=0;i<nb_lignes_fichier;i++){
    
    printf("\t\t");
    // Indique la sauvegarde sélectionnée
    if(i==numero_sauvegarde)
      printf("*  ");
    
    printf("Sauvegarde %d",i+1);

    if(i==numero_sauvegarde)
      printf("  *");
    
    printf("\n");
  }

  // Affiche la nouvelle sauvegarde en dernière
  printf("\t\t");
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
    printf("\n\n\t\tScore: %d\n\tTaille du radeau: %d",sauvegarde.score,sauvegarde.taille);
  }
}

void affiche_menu_difficulté(int numero_difficulté){

  system("clear");

  printf("\n\n");


  // Difficulté Facile
  printf("\t\t");
  if(numero_difficulté==0)
    printf("*  ");
  
  printf("Facile");

  if(numero_difficulté==0)
    printf("  *");
  printf("\n");


  // Difficumté Normale
  printf("\t\t");
  if(numero_difficulté==1)
    printf("*  ");
  
  printf("Normale");

  if(numero_difficulté==1)
    printf("  *");
  printf("\n");

  // Difficulté Difficile
  printf("\t\t");
  if(numero_difficulté==2)
    printf("*  ");
  
  printf("Difficile");

  if(numero_difficulté==2)
    printf("  *");
  printf("\n\n\n\n");

  // Difficulté Simulateur de Pluie
  printf("\t\t(Injouable)\n\t\t");
  if(numero_difficulté==3)
    printf("*  ");
  
  printf("Simulateur de pluie");

  if(numero_difficulté==3)
    printf("  *");
  printf("\n");

  // Difficulté Flipper
  printf("\t\t");
  if(numero_difficulté==4)
    printf("*  ");
  
  printf("Flipper");

  if(numero_difficulté==4)
    printf("  *");
  printf("\n");
}

struct termios ancien_param;

void restaurer_terminal(){
    tcsetattr(STDIN_FILENO, TCSANOW, &ancien_param);
}

// Configuration du terminal pour lire les entrées sans appuyer sur Entrée
void config_terminal(){
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


