
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
  données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>



//Liste chainé de lettres 
typedef struct lettres_i{

  char * contenu;
  struct lettres_i * lettres_suiv ;

}liste_lettres;

//Liste chainé de boites aux lettres
typedef struct bal_i{

  int num_bal;
  liste_lettres * lettres;
  struct bal_i * bal_suiv ;
  
}Liste_bal;




//Constuit un message avec une entête de 5 octets reservée pour le numéro du message dans l'utilisation normale ou le numéro du destinataire dans le cas de l'utilisation en BAL
void construire_message (char *message, char motif, int lg,int compt) {
  int i;
  char num_msg[6]= "";
  sprintf(num_msg, "%5d",compt);
  memcpy(message ,num_msg,5);
  
  for (i=5; i<=lg; i++) message[i] = motif; 
}

// Permet d'afficher les données: s=1 affiche en 
void afficher_donnees(int lg , int nb , int s){
  if (s == 1){                                                //Si on est en utilisation classique en source ou en emetteur de BAL
    printf("SOURCE : lg_message=%d, nb message=%d, ",lg,nb);
  }
  else if (s==0){                                            //Si on est en utilisation classique en puits ou en serveur BAL
    if(nb==-1){                                              //nb message non defini
      printf("PUITS : lg_message=%d, nb message=infini ",lg);
    }else {                                                  //nb message non defini
      printf("PUITS : lg_message=%d, nb message=%d ",lg,nb);
    }
  }
  else{                                                     //Si on est en recepteur de BAL
    printf("RECEPTION : Demande de récupération de ses lettres par le recepteur %d ",nb);
  }

}

// Fonction afficher message pour la partie 1 
void afficher_message1 (char *message, int lg,int s,int compt) {
  int i;
  if(s==1){                                       //Si on est en utilisation classique en source 
    printf("SOURCE : Envoi n°%5d (%5d) ",compt,lg);
  }
  else{                                          //Si on est en utilisation classique en puits
    printf("PUITS: Reception n°%5d (%5d)",compt,lg);
  }
  printf("[");
  for (i=0 ; i<=lg ; i++) 
    printf("%c", message[i]) ;
  printf("]");
  printf("\n");
}

// Fonction afficher message pour la partie 2
void afficher_message2 (char *message, int lg,int s,int compt,int idr) {
  int i;
  // Cas pour afficher dans l'emetteur
  if(s==1){
    printf("SOURCE : Envoi lettre n°%2d à destination du récepteur %2d (%5d) ",compt,idr,lg);
  }
  // Cas pour afficher dans le recepteur
  else if (s==0){
    printf("RECEPTION: Récuperation lettre par le recepteur %2d (%5d)",idr,lg);
  }
  // Cas pour afficher dans la BAL si on reçoit des lettres d'un emetteur
  else if (s==-1)
    printf("PUITS: Reception et stockage lettre n°%2d pour le recepteur n°%2d",compt,idr);
  // Cas pour afficher dans la BAL si on envoie des lettres d'un recepteur
  else if (s==-2)
    printf("PUITS: Recuperation lettre n°%2d par le recepteur n°%2d",compt,idr);

  printf("[");
  for (i=0 ; i<lg ; i++) printf("%c", message[i]) ;
    printf("]");
  printf("\n");
  
}



int main(int argc, char **argv)
{
  int c;
  extern char *optarg;
  extern int optind;
  int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
  int source = -1 ; /* 0=puits, 1=source */
  int lg_message = 30; //Sert pour le -l de base a 30 octets
  int udp = 0;  //udp = 0 => proto=TCP   udp = 1 => proto=UDP
  int B = -1 ;  //B=1 => mode BAL
  int E = -1 ;  //E=1 => mode emetteur   E=0 => mode recepteur
  int id_r ;  
  
  while ((c = getopt(argc, argv, "be:r:pn:sul:")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
        printf("Attention ! On ne peut pas être en mode source et puits en même temps Veuillez choisir -p ou -s\n");
        exit(1);
      }
      source = 0;
      break;

    case 's':
      if (source == 0) {
        printf("Attention ! On ne peut pas être en mode source et puits en même temps Veuillez choisir -p ou -s\n");
        exit(1) ;
      }
      source = 1;
      break;

    case 'n':
      nb_message = atoi(optarg);
      if(nb_message>100000){
        printf("Attention ! Trop de messages\n");
      }
      break;

    case 'u':
      if(B==1){
        printf("Attention ! L'usage de la BAL se fait en mode TCP Veuillez retirer -u\n");
      }
      udp=1;
      break;

    case'l':
      lg_message=atoi(optarg);
      if (lg_message<5){
        printf("Attention ! Vous devez avoir une taille de message au minimum de 6 octets\n");
        exit(1);
      }
      break;

    case 'b' :
      if(source!=-1){
        printf("Attention ! On ne peut pas utiliser le mode classique avec le mode BAL Veuillez choisir -b ou -s|-p\n");
        exit(1);
      }
      B=1 ;
      break ;
      
    case 'e' :      
    if(source!=-1){
        printf("Attention ! On ne peut pas utiliser l'usage classique avec le mode BAL Veuillez choisir -b ou -s|-p\n");
        exit(1);
      }   
      else if(E==0) {
         printf("Attention ! On ne peut pas être emetteur et recepteur Veuillez choisir -e ou -r\n");
         exit(1);
      }
      E=1 ;
      id_r = atoi(optarg);
      break ;
      
    case 'r' :
      if(source!=-1){
        printf("Attention ! On ne peut pas utiliser le mode classique avec le mode BAL Veuillez choisir -b ou -s|-p\n");
        exit(1);
      }
      else if(E==1) {
         printf("Attention ! On ne peut pas être emetteur et recepteur Veuillez choisir -e ou -r\n");
         exit(1);
      }
      E=0 ;
      id_r = atoi(optarg);
      break ;
      

      
    default:
      printf("usage: cmd [-p|-s][-n ##]\n");
      break;
    }
  }

  
  /*================================================================================================================================================================================================================*/
  // Interface Graphique


  if (nb_message != -1) {             //Si -n spécifiée
    
    if (source == 1 || E==1)          // Si on envoie en source ou en emetteur
      afficher_donnees(lg_message , nb_message , 1);
      
    else if (E==-1 && B==-1){        // Si on reçoit en puits classique
      afficher_donnees(lg_message , nb_message , 0);
    }
    else if(E==0) {                  // Si on reçoit en recepteur de BAL
      afficher_donnees(lg_message , id_r , -1);
    } 
    
  }
  else {                            //Si -n  non spécifiée
    
    if (source == 1 || E==1 ) {     // Si on envoie
      nb_message = 10 ;
      afficher_donnees(lg_message , nb_message , 1);
    }
    else if (E ==-1 && B==-1){      // Si on reçoit en puits classique
      afficher_donnees(lg_message , nb_message , 0);
    }
    else if (E==0 ){                // Si on reçoit en recepteur de BAL
      afficher_donnees(lg_message , id_r , -1);
    } 
  }


  if (source == -1 && E==-1 && B==-1) {
    printf("usage: cmd [-p|-s][-n ##]\n");
    exit(1) ;
  }

  /*=================================================================================================PARTIE 1=========================================================================================================*/

  
  if (source == 1 && E==-1 && B==-1){                        /*Source*/   

    if (udp==1){                          //Si on est en mode UDP
      
      struct sockaddr_in adr_distant;
      struct hostent * hp;
      int sock;
      char * msg;
      char motif = 'a';
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i=0 ;


  
      /* Création socket */
      sock=socket(AF_INET,SOCK_DGRAM,0);
      adr_distant.sin_port=htons(atoi(argv[argc-1]));
      hp = gethostbyname(argv[argc-2]);
      memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
      adr_distant.sin_family=AF_INET;
      
      // Création + emission msg
      msg=malloc(nb_message*sizeof(lg_message));
      printf("Port=%s, proto=UDP, dest -> %s \n",argv[argc-1],argv[argc-2]);
      for (i=0;i<nb_message;i++){
        if (motif=='{'){
          motif='a';
        }
        construire_message(msg,motif,lg_message,i+1);
        afficher_message1(msg,lg_message,source,i+1);
        motif += 1;
        sendto(sock,msg,lg_message,0,(struct sockaddr *)&adr_distant,adr_lg);
      }
              
    }
    else{                                //Si on est en mode TCP

      struct sockaddr_in adr_distant;
      struct hostent * hp;
      int sock;
      char * msg;
      char motif = 'a';
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i ;
      int succes ;

      /* Création socket */
      sock=socket(AF_INET,SOCK_STREAM,0);
      adr_distant.sin_port=htons(atoi(argv[argc-1]));
      hp = gethostbyname(argv[argc-2]);
      memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
      adr_distant.sin_family=AF_INET;

      //Requete connexion
      succes = connect(sock,(struct sockaddr *)&adr_distant,adr_lg) ;
      printf("Port=%s, proto=TCP, dest -> %s \n",argv[argc-1],argv[argc-2]);
      if (succes == 0){
        printf("SOURCE : Connexion etablie \n");
      }
      else {
        printf("SOURCE : Connexion refusée \n");
        exit(1) ;
      }
      
      // Emission message
      
      msg=malloc(nb_message*sizeof(lg_message));
      for (i=0;i<nb_message;i++){
        if (motif=='{'){
          motif='a';
        }
        construire_message(msg,motif,lg_message,i+1);
        afficher_message1(msg,lg_message,source,i+1);
        motif += 1;
        send(sock,msg,lg_message,0);
      }

      //Fermeture connexion
      shutdown(sock,1) ;
      
      
    }
  }
  else if (source==0 && E==-1 && B==-1)  {                        /* PUIT */
    
    if (udp==1){  // Pour le mode UDP
      
      struct sockaddr_in adr_local;
      int sock;
      struct sockaddr_in adr_distant;
      char * adr_msg;
      adr_msg=malloc(sizeof(lg_message));
      int lg_eff;
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i=0;
      
      // Création socket distant
      sock=socket(AF_INET,SOCK_DGRAM,0);
      adr_local.sin_port=htons(atoi(argv[argc-1]));
      adr_local.sin_addr.s_addr = INADDR_ANY ;
      adr_local.sin_family=AF_INET;
      
      // Association @->socket
      bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local));
      
      // Reception + affichage msg
      printf("Port=%s, proto=UDP, \n",argv[argc-1]);
      while(i!=nb_message){
        lg_eff=recvfrom(sock,adr_msg,lg_message,0,(struct sockaddr *)&adr_distant,&adr_lg);
        afficher_message1(adr_msg,lg_eff,source,i+1);
        i++;
      }
    }
    else{                                    //Si on est en mode TCP
        
      struct sockaddr_in adr_local;
      int sock;
      int sockbis ;
      struct sockaddr_in adr_distant;
      char * adr_msg;
      adr_msg=malloc(sizeof(lg_message));
      int lg_eff;
      unsigned int adr_lg=sizeof(struct sockaddr_in);
      int i=0;
      
      // Création socket distant
      sock=socket(AF_INET,SOCK_STREAM,0);
      adr_local.sin_port=htons(atoi(argv[argc-1]));
      adr_local.sin_addr.s_addr = INADDR_ANY ;
      adr_local.sin_family=AF_INET;

      // Association @->socket
      bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local));
      printf("Port=%s, proto=UDP, \n",argv[argc-1]);

      //Bufferisation des requêtes
      listen(sock, 5);
      
      // Mise en état d'acceptation
      sockbis=accept(sock,(struct sockaddr *)&adr_distant,&adr_lg); 
      if (sockbis==-1)
        printf("PUITS : Echec de la connexion\n");
      else
        printf("PUITS : Connexion acceptée\n");
      
      
      // Reception + affichage msg
    
      while(i!=nb_message){
        lg_eff=recv(sockbis,adr_msg,lg_message,0);
        if (lg_eff !=0){  //On affiche un message que s'il n'est pas vide
          afficher_message1(adr_msg,lg_eff,source,i+1);
        }
        else{  //On ne reçoit un message que s'il n'est pas vide
          exit(1);
        }
        i++;
      }
    }
  }

  /*=================================================================================================PARTIE 2========================================================================================================*/
  
 
  if (E == 1){          //On souhaite envoyer à une BAL = mode Emetteur -e
    
    struct sockaddr_in adr_distant;
    struct hostent * hp;
    int sock;
    char * msg;
    char * msg_bal ;
    char motif = 'a';
    unsigned int adr_lg=sizeof(struct sockaddr_in);
    int i ;

    /* Création socket */
    sock=socket(AF_INET,SOCK_STREAM,0);
    adr_distant.sin_port=htons(atoi(argv[argc-1]));
    hp = gethostbyname(argv[argc-2]);
    memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
    adr_distant.sin_family=AF_INET;

    //Requete connexion

    printf("Port=%s, proto=TCP, dest -> %s \n",argv[argc-1],argv[argc-2]);
    if (connect(sock,(struct sockaddr *)&adr_distant,adr_lg) != 0){
      printf("Erreur de connect, Veuillez réessayer");
      exit(1) ;
    }

    //Informer serveur BAL : convention : l'en-tête fait toujours 3 octets
    //On envoie un premier message pour dire qu'on est un emetteur
    msg_bal=malloc(sizeof("E")) ; //On envoie une première lettre "E"
    msg_bal="E" ;
    send(sock,msg_bal,strlen(msg_bal),0);


    //On formate le message differement si on a un nb_message <10 et un nb_message compris entre 10 et 99
    //On envoie une deuxième lettre avec le nombre de lettre à envoyer
    if(nb_message<10){
      msg_bal=malloc(nb_message) ; 
      sprintf(msg_bal,"-%d",nb_message) ; //si le nb_message est <10 on envoie le chiffre precedé d'un - pour pouvoir le traiter en reception en BAL
      send(sock,msg_bal,strlen(msg_bal),0);

    }
    else if(nb_message<99){
      msg_bal=malloc(nb_message) ; 
      sprintf(msg_bal,"%d",nb_message) ; 
      send(sock,msg_bal,strlen(msg_bal),0);
    }
    else{
      printf("Le programme ne sait pas gerer des requête superieure à 99 messages Veuillez saisir un nombre de lettre plus petit");
      exit(1);
    }
  

    //On formate le message differement si on a un id recepteur <10 et un id recepteur compris entre 10 et 99
    //On envoie une troisème lettre avec le numero du recepteur

    if(id_r<10){
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"-%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);

    }
    else if (id_r<99){
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);
    }
    else{
      printf("Le programme ne sait pas gerer des requête superieure à 99 messages Veuillez saisir un nombre de lettre plus petit");
      exit(1);
    }

      
    // Emission  des lettres à la BAL
    msg=malloc(nb_message*sizeof(lg_message));
    for (i=0;i<nb_message;i++){
      if (motif=='{'){
        motif='a';
      }
      construire_message(msg,motif,lg_message,id_r);
      afficher_message2(msg,lg_message,1,i+1,id_r);  //l'argument '1' permet a la fonction afficher message d'afficher l'interface graphique de l'emetteur 
      motif= motif+1;
      send(sock,msg,lg_message,0);
    }

    //Fermeture connexion
    printf("SOURCE : FIN\n");
    shutdown(sock,1) ;
    
  }
  else if (B==1){  //Cas où on crée un serveur BAL
    
    struct sockaddr_in adr_local;
    int sock;
    int sockbis ;
    struct sockaddr_in adr_distant;
    char * adr_msg;
    char * info;          //Message contenant le nombre de lettres et le numero du recepteur
    info = malloc(sizeof(int)*2);
    adr_msg=malloc(sizeof(lg_message));
    unsigned int adr_lg=sizeof(struct sockaddr_in);
    int i=0;
    

    Liste_bal * ListedeBal = malloc(sizeof(Liste_bal));  
    ListedeBal = NULL;  //Pointeur general sur la liste des BAL
      
    // Création socket distant
    sock=socket(AF_INET,SOCK_STREAM,0);
    adr_local.sin_port=htons(atoi(argv[argc-1]));
    adr_local.sin_addr.s_addr = INADDR_ANY ;
    adr_local.sin_family=AF_INET;

    // Association @->socket
    if (bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local)) ==-1){
      printf("Attention ! Erreur bind Veuillez tester un autre port\n");
      exit (1);
    }
    printf("PUITS : Port=%s, proto=TCP, \n",argv[argc-1]);
    
    //Bufferisation des requêtes
    listen(sock, 5);

    while(1){
      // Mise en état d'acceptation
    
      if ((sockbis=accept(sock,(struct sockaddr *)&adr_distant,&adr_lg))==-1){
	      printf("PUITS : Echec de la connexion\n");
	      exit(1);
      }
    
      
      //Reception des premières lettres de l'emetteur ou du recepteur 
      //Première lettres indique si c'est un emetteur ou un recepteur
      recv(sockbis,adr_msg,1,0); 

      //Cas où requête emettrice
      if (adr_msg[0]=='E'){ 

        //On recupère la 2ème lettre contenant le nombre de lettres;
        recv(sockbis,info,2,0);

        //Si il y'a moins de 10 lettres, le premier octets est un - donc on decale de 1 pour recuperer le nombre de lettres
        if(info[0]=='-'){
          info++;
        }    
        nb_message = atoi(info) ;

        //On recupère la 3ème lettre contenant l'id du recepteur
        recv(sockbis,info,2,0);

        //Si il y'a moins de 10 recepteur, le premier octets est un - donc on decale de 1 pour recuperer l'id du recepteur
        if(info[0]=='-'){
          info++;
        }
        id_r = atoi(info) ;

        Liste_bal * bal_courante = ListedeBal ; // Pointeur de bal qui repointe sur la liste general à chaque appel de recuperation de lettres
        Liste_bal * tmp ; //Pointeur de bal temporaire

        //Création de BAL si elle n'existe pas deja 

        //On cherche si la BAL est existante
        while(bal_courante != NULL && bal_courante -> num_bal != id_r  ){
          bal_courante = bal_courante -> bal_suiv;
        }

        //Si elle n'existe pas on créée une nouvelle BAL
        if (bal_courante == NULL){
	 
          bal_courante = malloc(sizeof(Liste_bal));
          bal_courante->num_bal = id_r;
          bal_courante->bal_suiv = NULL;

          if(ListedeBal==NULL){       //Si il n'y avait aucune BAL dans la liste on ajoute celle qui vient d'être créée
            ListedeBal=bal_courante;
          }
          else{                       //Si la liste n'était pas vide on rajoute la nouvelle BAL en fin de liste
            tmp=ListedeBal;
            while(tmp->bal_suiv != NULL){
              tmp=tmp->bal_suiv;
            }   
            tmp -> bal_suiv = bal_courante;
          }
        }

        //On ajoute ensuite les lettres correspondant à la bonne BAL

        for (i=0; i<nb_message;i++){
          read(sockbis,adr_msg,lg_message);
          afficher_message2(adr_msg,lg_message,-1,i+1,id_r);
          liste_lettres * new_lettres =malloc(sizeof(liste_lettres));
          new_lettres -> contenu = malloc(lg_message);
          strncpy(new_lettres -> contenu,adr_msg,lg_message);
          new_lettres->lettres_suiv = NULL;

    
          //On cherche la bonne BAL
          while(bal_courante -> num_bal != id_r){
            bal_courante = bal_courante -> bal_suiv;
          }
          if(bal_courante -> lettres == NULL){      //Si il n'y a aucune lettres dans la BAL on rajoute simplement la nouvelle
                bal_courante -> lettres = new_lettres;
          }
          else {                                   //Sinon on cherche la dernière lettres pour l'ajouter à la fin
          liste_lettres * derniere_lettres = malloc(sizeof(liste_lettres));
                derniere_lettres = bal_courante -> lettres;
                    while(derniere_lettres -> lettres_suiv !=NULL){
                      derniere_lettres = derniere_lettres-> lettres_suiv;
                    }
                    derniere_lettres ->lettres_suiv = new_lettres;
          }
        } 
      } 


      //Cas où requête Receptrice
      
      else if (adr_msg[0]=='R') {

        Liste_bal * bal_courante = ListedeBal ;
        i=0;
        char *info = malloc(sizeof(int)*2);

        //On reçoit la lettre contenant le numero du recepteur
        recv(sockbis,info,2,0);      

        //Si il y'a moins de 10 recepteur, le premier octets est un - donc on decale de 1 pour recuperer l'id du recepteur
        if(info[0]=='-'){
          info++;
        }
        id_r = atoi(info) ;
            
        //On cherche la BAL associé au recepteur
        while(bal_courante ->num_bal !=id_r){
          bal_courante=bal_courante -> bal_suiv;
        }

        Liste_bal * bal_i = bal_courante;             // Pointeur temporaire sur la bonne BAL
        liste_lettres * lettres_i=bal_i -> lettres;   // Pointeur temporaire sur les lettres de la BAL

        while(lettres_i!=NULL){
          send(sockbis,lettres_i->contenu,lg_message,0);
          afficher_message2(lettres_i->contenu,lg_message,-2,i+1,id_r);
          lettres_i = lettres_i->lettres_suiv;     
          i++;   
        }
        free(bal_courante);                   // On libère la boite aux lettres après l'avoir vidé 
      }
    }
  }
  else if (E==0){    //Cas ou on souhaite recevoir des messages de la BAL = mode Recepteur -r

    
    struct sockaddr_in adr_distant;
    struct hostent * hp;
    int sock;
    char * msg_bal ;
    unsigned int adr_lg=sizeof(struct sockaddr_in);
    int i ;
    int lg_eff;


    /* Création socket */
    sock=socket(AF_INET,SOCK_STREAM,0);
    adr_distant.sin_port=htons(atoi(argv[argc-1]));
    hp = gethostbyname(argv[argc-2]);
    memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
    adr_distant.sin_family=AF_INET;

    //Requete connexion

    printf("Port=%s, proto=TCP, dest -> %s \n",argv[argc-1],argv[argc-2]);
    if (connect(sock,(struct sockaddr *)&adr_distant,adr_lg) != 0){
      printf("Erreur de connect, Veuillez réessayer");
      exit(1) ;
    }

    //Informer serveur BAL qu'on est un Recepteur et du numero du recepteur

    //Premère lettre pour informer qu'on est un recepteur
    msg_bal=malloc(sizeof("R")) ; 
    msg_bal="R" ;
    send(sock,msg_bal,strlen(msg_bal),0);


    //On envoie une deuxième lettre avec l'identifiant du destinataire

    //Formatage different si l'id_r est <10 et s'il est compris entre 10 et 99
    if(id_r<10){
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"-%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);
      
    }else if (id_r<99){
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);
    }
    else{
      printf("Attention ! Le programme ne peut pas gerer les numéro de recepteur supérieur à 99 Veuillez changer de numéro\n");
      exit(1);
    }

      
    //Reception des lettres

    while(i!=nb_message){
      msg_bal=malloc(lg_message);
      lg_eff=recv(sock,msg_bal,lg_message,0);
      if (lg_eff !=0){      //On affiche un message que s'il n'est pas vide
	      afficher_message2(msg_bal,lg_eff,0,i+1,id_r);
      }
      else{  
        printf("RECEPTION : Fin\n");
	      shutdown(sock,1) ;
	      exit(1);
      }
      i++;
    }

    //Fermeture connexion
    printf("RECEPTION : Fin\n");
    shutdown(sock,1) ;
  }
}



  
