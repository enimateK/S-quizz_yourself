#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

static void * ecoute (void * socket_descriptor){
    int longueur;
    int* socket = (int *) socket_descriptor;
    char buffer[256];
    while(1){
        if((longueur = read(*socket, buffer, (int)sizeof(buffer)))<=0)
            exit(1);
        buffer[longueur]='\0';
        printf("%s \n", buffer);
    }
}


int main(int argc, char **argv) {
  
    int     		socket_descriptor, 	
		    		longueur; 		    
    sockaddr_in 	adresse_locale; 	
    hostent *		ptr_host; 		    
    servent *		ptr_service; 		
    char 			buffer[256];
    char *			prog; 			   
    char *			host; 			    
    char  			msg[256]; 			

    char			pseudo[50];
    pthread_t       thread_ecoute;     
    
    if (argc != 2) {
		perror("usage : client <adresse-serveur>");
		exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    
    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);
    
    if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse.");
		exit(1);
    }
    
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; 

    adresse_locale.sin_port = htons(5000);
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port)); 
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
    }   
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
    }   
    printf("connexion etablie avec le serveur. \n\n\n");

    
	printf("Bienvenue sur le chat 		serveur: %s\n", host);
    
    printf("Veuillez entrer votre nom: \n");
    fgets(pseudo, sizeof pseudo, stdin);
    pseudo[strcspn(pseudo, "\n")] = '\0'; 
    if ((write(socket_descriptor, pseudo, strlen(pseudo))) < 0) {
            perror("erreur : impossible d'ecrire le message destine au serveur.");
            exit(1);
    }
    printf("__________________________\n");
    printf("                          \n");
    printf("/quit - Quitte le serveur\n");
    printf("/list - Affiche les utilisateurs\n");
    printf("/start - Démarre le quizz\n");
    printf("/end - Stop le jeu\n");
    printf("/help - Affiche l'aide\n");
    printf("__________________________\n\n");
    printf("Bienvenue %s !\n", pseudo);

    pthread_create(&thread_ecoute, NULL, ecoute, &socket_descriptor);

	while(strcmp(msg,"/quit")!=0){
        
        fgets(msg, sizeof(msg), stdin);
        msg[strcspn(msg, "\n")] = '\0';
          if ((write(socket_descriptor, msg, strlen(msg))) < 0) {
                perror("erreur : impossible d'ecrire le message destine au serveur.");
                exit(1);
            }
	}
	printf("Vous quittez le chat.\n");

    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    

    exit(0);
    
}

