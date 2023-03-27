
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




typedef struct lettres_i{

  char * contenu;
  struct lettres_i * lettres_suiv ;

}liste_lettres;

typedef struct bal_i{

  int num_bal;
  liste_lettres * lettres;
  struct bal_i * bal_suiv ;
  
}Liste_bal;





void construire_message (char *message, char motif, int lg,int compt) {
  int i;
  char num_msg[6]= "";
  
  sprintf(num_msg, "%5d",compt);
  memcpy(message ,num_msg,5);
  
  for (i=5; i<=lg; i++) message[i] = motif; 
}
void afficher_donnees(int lg , int nbmsg , int s){
  if (s == 1){
    printf("SOURCE : lg_message=%d, nb message=%d, ",lg,nbmsg);
  }
  else{
    if(nbmsg==-1){
      printf("PUITS : lg_message=%d, nb message=infini ",lg);
    }else {
      printf("PUITS : lg_message=%d, nb message=%d ",lg,nbmsg);
    }
  }

}

// Fonction afficher message pour la partie 1 
void afficher_message1 (char *message, int lg,int s,int compt) {
  int i;
  if(s==1){
    printf("SOURCE : Envoi n°%5d (%5d) ",compt,lg);
  }
  else{
    printf("PUITS: Reception n°%5d (%5d)",compt,lg);
  }
  printf("[");
  for (i=0 ; i<=lg ; i++) printf("%c", message[i]) ;
  printf("]");
  printf("\n");
}

// Fonction afficher message pour la partie 2
void afficher_message2 (char *message, int lg,int s,int compt,int idr) {
  int i;
  // Cas pour afficher dans l'emetteur
  if(s==1){
    printf("SOURCE : Envoi lettre n°%2d à destination du récepteur %2d ",compt,idr);
  }
  // Cas pour afficher dans le recepteur
  else if (s==0){
    printf("RECEPTION: Récuperation lettre par le recepteur %2d",idr);
  }
  // Cas pour afficher dans la BAL
  else if (s==-1)
    printf("PUITS: Reception et stockage lettre n°%2d pour le recepteur n°%2d",compt,idr);
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
  int lg_message = 30; //Sert pour le -l
  int udp = 0;
  int B = -1 ;
  int E = -1 ;
  int id_r ;
  
  while ((c = getopt(argc, argv, "be:r:pn:sul:")) != -1) {
    switch (c) {
    case 'p':
      if (source == 1) {
  printf("usage: cmd [-p|-s][-n ##]\n");
  exit(1);
      }
      source = 0;
      break;

    case 's':
      if (source == 0) {
  printf("usage: cmd [-p|-s][-n ##]\n");
  exit(1) ;
      }
      source = 1;
      break;

    case 'n':
      nb_message = atoi(optarg);
      break;

    case 'u':
      udp=1;
      break;

    case'l':
      lg_message=atoi(optarg);
      break;

    case 'b' :
      B=1 ;
      break ;
      
    case 'e' :
      E=1 ;
      id_r = atoi(optarg);
      break ;
      
    case 'r' :
      E=0 ;
      id_r = atoi(optarg);
      break ;
      

      
    default:
      printf("usage: cmd [-p|-s][-n ##]\n");
      break;
    }
  }

  
  /*=============================================================================*/



  if (nb_message != -1) { //Si -n spécifiée
    
    if (source == 1 || E==1) // Si on envoie
      afficher_donnees(lg_message , nb_message , 1);
      
    
    else if (E==-1) // Si on reçoit
      afficher_donnees(lg_message , nb_message , source);
    
  }
  else { //Si -n  nn spécifiée
    
    if (source == 1 || E==1 ) { // Si on envoie
      nb_message = 10 ;
      afficher_donnees(lg_message , nb_message , 1);
    }
    else if (E ==-1) // Si on reçoit
      afficher_donnees(lg_message , nb_message , source);
    
  }


  if (source == -1 && E==-1 && B==-1) {
    printf("usage: cmd [-p|-s][-n ##]\n");
    exit(1) ;
  }

  /*=============================================================================*/
  //Partie 1
  
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
  motif= motif+1;
  send(sock,msg,lg_message,0);
      }

      //Fermeture connexion
      shutdown(sock,1) ;
      
      
    }
  }
  else if (source==0 && E==-1 && B==-1)  {                                   /* PUIT */
    
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
      
      if (sockbis==-1){
  printf("PUITS : Echec de la connexion\n");
      }
      else{
  printf("PUITS : Connexion accepté\n");
      }
      
      
      // Reception + affichage msg
    
      while(i!=nb_message){
  lg_eff=recv(sockbis,adr_msg,lg_message,0);
  if (lg_eff !=0){
    afficher_message1(adr_msg,lg_eff,source,i+1);
  }
  else{  //On ne reçoit un message que s'il n'est pas vide
    exit(1);
  }
  i++;
      }


  
    }
  }

  /*=============================================================================*/
  
  //Partie 2
  if (E == 1){          //On souhaite envoyer à une BAL
    
    struct sockaddr_in adr_distant;
    struct hostent * hp;
    int sock;
    char * msg;
    char * msg_bal ;
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

    //Informer serveur BAL : convention : l'en-tête fait toujours 4 octets
    msg_bal=malloc(sizeof("E")) ; //On envoie une première lettre "E"
    msg_bal="E" ;
    send(sock,msg_bal,strlen(msg_bal),0);


    if(nb_message<10){
      msg_bal=malloc(nb_message) ; //On envoie une deuxième lettre avec le nombre de lettre à envoyer
      sprintf(msg_bal,"-%d",nb_message) ; //à voir
      send(sock,msg_bal,strlen(msg_bal),0);

    }else{
      msg_bal=malloc(nb_message) ; //On envoie une deuxième lettre avec le nombre de lettre à envoyer
      sprintf(msg_bal,"%d",nb_message) ; //à voir
      send(sock,msg_bal,strlen(msg_bal),0);
    }
  
    if(id_r<10){
      msg_bal=malloc(id_r) ; //On envoie une troisième lettre avec l'identifiant du destinataire
      sprintf(msg_bal,"-%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);

    }else{
      msg_bal=malloc(id_r) ; //On envoie une troisième lettre avec l'identifiant du destinataire
      sprintf(msg_bal,"%d",id_r) ;
      send(sock,msg_bal,strlen(msg_bal),0);

    }

      
    // Emission message
    msg=malloc(nb_message*sizeof(lg_message));
    for (i=0;i<nb_message;i++){
      if (motif=='{'){
  motif='a';
      }
      construire_message(msg,motif,lg_message,id_r);
      afficher_message2(msg,lg_message,1,i+1,id_r);
      motif= motif+1;
      send(sock,msg,lg_message,0);
    }

    //Fermeture connexion
    shutdown(sock,1) ;
    
  }
  else if (B==1){  //Cas où on crée une BAL
    
    struct sockaddr_in adr_local;
    int sock;
    int sockbis ;
    struct sockaddr_in adr_distant;
    char * adr_msg;
    adr_msg=malloc(sizeof(lg_message));
    unsigned int adr_lg=sizeof(struct sockaddr_in);
    int i=0;
    Liste_bal * ListedeBal = malloc(sizeof(Liste_bal));
    ListedeBal = NULL;
      
    // Création socket distant
    sock=socket(AF_INET,SOCK_STREAM,0);
    adr_local.sin_port=htons(atoi(argv[argc-1]));
    adr_local.sin_addr.s_addr = INADDR_ANY ;
    adr_local.sin_family=AF_INET;

    // Association @->socket
    
    if (bind(sock,(struct sockaddr *)&adr_local,sizeof(adr_local)) ==-1){

      printf("erreur bind");
      exit (1);
    }
    printf("Port=%s, proto=TCP, \n",argv[argc-1]);
    
    //Bufferisation des requêtes
    listen(sock, 5);

    while(1){
      // Mise en état d'acceptation
      sockbis=accept(sock,(struct sockaddr *)&adr_distant,&adr_lg); 
      
      if (sockbis==-1){
	printf("PUITS : Echec de la connexion\n");
	exit(1);
      }
      else{
	printf("PUITS : Connexion accepté\n");
      }
      
      
      // Reception + affichage msg
      //On rappelle que l'en tête fait 4 octets par convention

      //Traitement premier octets 'E ou R'
      recv(sockbis,adr_msg,1,0); 
     


      //Cas où requête emettrice
      if (adr_msg[0]=='E'){ 
      recv(sockbis,adr_msg,2,0);


      //Traitement 2ème et 3ème octets
      if(adr_msg[0]=='-'){
        adr_msg++;
      }    
      nb_message = atoi(adr_msg) ;

      //Traitement 4ème et 5ème octets
      recv(sockbis,adr_msg,2,0);
      if(adr_msg[0]=='-'){
        adr_msg++;
      }
      id_r = atoi(adr_msg) ;
  


      //Reception et stockage des n messages
      Liste_bal * bal_courante = ListedeBal ;
      Liste_bal * tmp ;
      
      while(bal_courante != NULL && bal_courante -> num_bal != id_r  ){
              bal_courante = bal_courante -> bal_suiv;
          }
      if (bal_courante == NULL){
	 
	bal_courante = malloc(sizeof(Liste_bal));
	bal_courante->num_bal = id_r;
	bal_courante->bal_suiv = NULL;
	if(ListedeBal==NULL){
	  printf("je créée bal = n%d\n", bal_courante->num_bal);
	  ListedeBal=bal_courante;
	}
	else{
	  printf("je créée bal == n%d\n", bal_courante->num_bal);
	  printf("%d",ListedeBal ->num_bal);
	  tmp=ListedeBal;
	  while(tmp->bal_suiv != NULL){
	    printf("%d",tmp ->num_bal);
	    tmp=tmp->bal_suiv;
	    printf("%d",tmp ->num_bal);
	  }
	      
	  tmp -> bal_suiv = bal_courante;
	}
      }


      
      for (i=0; i<nb_message;i++){
        read(sockbis,adr_msg,lg_message);
        afficher_message2(adr_msg,lg_message,-1,i+1,id_r);
        liste_lettres * new_lettres =malloc(sizeof(liste_lettres));
	new_lettres -> contenu = malloc(lg_message);
	strncpy(new_lettres -> contenu,adr_msg,lg_message);
        new_lettres->lettres_suiv = NULL;

	
        //Cas où aucune BAL est presente
	while(bal_courante -> num_bal != id_r){
	  bal_courante = bal_courante -> bal_suiv;
	}
	if(bal_courante -> lettres == NULL){
	       bal_courante -> lettres = new_lettres;
	}
	else {
	 liste_lettres * derniere_lettres = malloc(sizeof(liste_lettres));
         derniere_lettres = bal_courante -> lettres;
            while(derniere_lettres -> lettres_suiv !=NULL){
              derniere_lettres = derniere_lettres-> lettres_suiv;
            }
            derniere_lettres ->lettres_suiv = new_lettres;
 
	}
          
         
          //Si la bal_i n'existe pas : on la crée et on y insère les lettres
         
          //Si elle existe : on insère à la fin de la liste les nouvelles lettres
     

           
          
	  
      }
      
    } 
      //Cas où requête Receptrice
      //Convention l'en-tete fait toujours 3 octets 
      
      else if (adr_msg[0]=='R') 
      {
        Liste_bal * bal_courante = ListedeBal ;
	//Liste_bal * bal_premiere = ListedeBal;
        //Traitement 2ème et 3ème octets
        recv(sockbis,adr_msg,2,0);
	printf("%d\n",adr_msg[0]);
	printf("%d\n",adr_msg[1]);

            
        if(adr_msg[0]=='-'){
          adr_msg++;
        }
	printf("%d",adr_msg[0]);
        id_r = atoi(adr_msg) ;
	printf("id_r %d\n",id_r);

          
        //Envoie des n messages au recepteur
	printf("==%d\n",bal_courante ->num_bal);
        while(bal_courante ->num_bal !=id_r){
	  printf("====%d\n",bal_courante ->num_bal);
          bal_courante=bal_courante -> bal_suiv;
	  
        }
	Liste_bal * bal_i = bal_courante; 
        liste_lettres * lettres_i=bal_i -> lettres;
        while(lettres_i!=NULL){
	  send(sockbis,lettres_i->contenu,lg_message,0);
	  afficher_message2(lettres_i->contenu,lg_message,0,i+1,id_r);
	  lettres_i = lettres_i->lettres_suiv;
                
        }
	//free(bal_i);
	//bal_courante=bal_premiere ;
	printf("%d\n",ListedeBal -> num_bal);
      }
    }
  }
  else if (E==0){    //Cas ou on souhaite recevoir des messages de la BAL

    
    struct sockaddr_in adr_distant;
    struct hostent * hp;
    int sock;
    char * msg_bal ;
    unsigned int adr_lg=sizeof(struct sockaddr_in);
    int i ;
    int succes ;
    int lg_eff;
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

    //Informer serveur BAL : convention : l'en-tête fait toujours 3 octets
    //On envoie une première lettre "R"
    msg_bal=malloc(sizeof("R")) ; 
    msg_bal="R" ;
    send(sock,msg_bal,strlen(msg_bal),0);


    //On envoie une deuxième lettre avec l'identifiant du destinataire
    if(id_r<10){
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"-%d",id_r) ;
      printf("%s\n",msg_bal);
      send(sock,msg_bal,strlen(msg_bal),0);
      
    }else{
      msg_bal=malloc(id_r) ; 
      sprintf(msg_bal,"%d",id_r) ;
      
      send(sock,msg_bal,strlen(msg_bal),0);
    }

      
    // reception des messages
  
    while(i!=nb_message){
      msg_bal=malloc(lg_message);
      lg_eff=recv(sock,msg_bal,lg_message,0);
      if (lg_eff !=0){
	afficher_message2(msg_bal,lg_eff,0,i+1,id_r);
      }
      else{  //On ne reçoit un message que s'il n'est pas vide
	shutdown(sock,1) ;
	exit(1);
      }
      i++;
    }

    //Fermeture connexion
    shutdown(sock,1) ;
      






  }

}



  
