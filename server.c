
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	
#include <sys/socket.h>
#include <netdb.h> 		
#include <string.h> 		
#include <pthread.h> 
#include <stdlib.h>         
#include <unistd.h>


#define TAILLE_MAX_NOM      256
#define NB_CLIENTS_MAX      10


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

typedef struct{
    int sock;              
    char pseudo[50];      
    pthread_t thread;    
    int connected;        
} Client;

typedef struct
{
    char * question;    
    char * reponse;     
	char * indice;
}quest_rep;


Client arrClient[NB_CLIENTS_MAX];         
int nb_client = 0;                        


int in_game;                         
Client * maitre_jeu;                      

void envoyer_message(Client * client, char buffer[]){
    
    char *answer = malloc (sizeof (*answer) * 256);
    strcpy(answer, (*client).pseudo);
    strcat(answer,": ");
    strcat(answer,buffer);
    printf("%s\n", answer);
    int i;
    for (i=0;i<nb_client;i++){
        if(strcmp((*client).pseudo,arrClient[i].pseudo)!=0){
            if((write(arrClient[i].sock,answer,strlen(answer)+1)) < 0){
                perror("erreur : impossible d'ecrire le message destine au serveur.");
                exit(1);
            } 
        }     
    }    
}




void message_serv(char buffer[]){
    char *answer = malloc (sizeof (*answer) * 256);
    strcat(answer,buffer);
    printf("%s\n", answer);
    int i;
    for (i=0;i<nb_client;i++){
            write(arrClient[i].sock,answer,strlen(answer)+1); 
    }    
}


void colorer(char* answer, char choix){

    char originale[10]  = "\x1B[0m" ;
    char rouge[10]      = "\x1B[31m";
    char vert[10]       = "\x1B[32m";
    char jaune[10]      = "\x1B[33m";
    char bleu[10]       = "\x1B[34m";
    char magenta[10]    = "\x1B[35m";
    char cyan[10] 		= "\x1B[36m";

    char couleur[10];

    switch(choix) {
        case 'o' :
            strcpy(couleur, originale);
            break;
		case 'c' :
            strcpy(couleur, cyan);
            break;  
        case 'v' :
            strcpy(couleur, vert);
            break;
        case 'j' :
            strcpy(couleur, jaune);
            break;
        case 'b' :
            strcpy(couleur, bleu);
            break;
		case 'r' :
            strcpy(couleur, rouge);
            break;
        case 'm' :
            strcpy(couleur, magenta);
            break;                                                
        default :
            printf("Mauvais choix de couleur\n" );
    }


    char *res = malloc (sizeof (*res) * 256);
    strcpy(res, couleur);
    strcat(res, answer);
    strcat(res, originale);

    strcpy(answer, res);
}


static void * jeu (Client * c){
	char *phrase = malloc (sizeof (*phrase) * 256);
	char * quest;
	char buffer[256];
	quest_rep Question_jeu; 
    char * rep; 
	int longueur;
	int questionOk = 0;
	int repOk = 0;
	int indOk = 0;
	maitre_jeu = c;
	
        printf("%d\n",(*maitre_jeu).sock);
        printf("%s\n",buffer);
        printf("%d\n",(int)sizeof(buffer));
        while(questionOk == 0){
			printf("avant phrase");
             phrase = "Inscrivez la question :";
             printf("%s\n", phrase);
							printf("après phrase");
			 write((*c).sock,phrase,strlen(phrase)+1);
				printf("après write");
			buffer[longueur]='\0';
			 longueur = read((*maitre_jeu).sock, buffer, sizeof(buffer));
						printf("apres read");

             buffer[longueur]='\0';
						printf("après NULL");

        	printf("%s \n", buffer);
             Question_jeu.question = (char*) malloc (256*sizeof(longueur));	
			printf("après maloc");
             strcpy(Question_jeu.question,"Question");
			printf("avant if");
             if (Question_jeu.question != '\0') {
                 printf("dans if");
				 questionOk = 1;
             }
			printf("après if");
		}
}


char* listeClient(){
    char* res = malloc(nb_client*50*sizeof(char));
    strcpy(res,"Les utilisateurs connectés :");
    int i = 0;
    for(i; i<nb_client; i++){
        strcat(res, "\n - ");
        strcat(res, arrClient[i].pseudo);
    }
    printf("\n");

    return res;
}

void supprimerUtilisateur(Client *client_supprime){
    Client copyArray[NB_CLIENTS_MAX];
    int i,j = 0;
    int tmp_nb_client = nb_client;
    for(i; i<tmp_nb_client; i++){
        if(arrClient[i].sock != (*client_supprime).sock){
            copyArray[j] = arrClient[i];
            j++;
        }
        else{
            close(arrClient[i].sock);
            nb_client--;
        }
    }

    memcpy(arrClient, copyArray, sizeof(arrClient));
}

static void * commande (void * c){
    Client * client = (Client *) c;
	char buffer[256];
	char *answer = malloc (sizeof (*answer) * 256);
	int longueur;
    pthread_t thread_game;         

    in_game=0;

    while(strlen((*client).pseudo)<=1){
        longueur = read((*client).sock, buffer, sizeof(buffer));
        sleep(3);
        buffer[longueur]='\0'; 
        strcpy((*client).pseudo, buffer);
        write(1,buffer,longueur);
    }
	
	
    while(1){
    	longueur = read((*client).sock, buffer, sizeof(buffer));

        buffer[longueur]='\0';    

    	sleep(3);
    	if(strcmp(buffer,"/quit")==0){
    		printf("%s a lancé la commande /quit\n", (*client).pseudo);
            strcpy(answer, (*client).pseudo);
            strcat(answer," vient de se déconnecter.\n");
           	colorer(answer, 'j');
           	envoyer_message(client, answer);
           	supprimerUtilisateur(client);
            pthread_exit(NULL);
    	}
    	else if(strcmp(buffer,"/list")==0){
    		printf("%s a lancé la commande /list\n", (*client).pseudo);
    		strcpy(answer, listeClient());
    		colorer(answer, 'b');
    		write((*client).sock,answer,strlen(answer)+1); 
    	}
        else if(strcmp(buffer,"/help")==0){
            printf("%s a lancé la commande /help\n", (*client).pseudo);
            strcpy(answer, "__________________________\n                          \n/quit - Quitte le serveur\n/list - Affiche la liste des utilisateurs\n/start - Commence le jeu\n/end - Stop le jeu\n/help - Affiche l'aide\n__________________________\n\n");
            colorer(answer, 'b');
            write((*client).sock,answer,strlen(answer)+1);  
        }
        else if (strcmp(buffer,"/start")==0){
				
        if (in_game==0){
            pthread_create(&thread_game, NULL, jeu(client), &arrClient[nb_client]);
            in_game=1;
            
        }
			
        }
        else if (strcmp(buffer,"/end")==0) {
            if (in_game==1){
                in_game=0;}
        }          
        else if(longueur > 0){
            envoyer_message(client, buffer);	
    	}



    }


}




static void * commandeServeur (void * socket_serveur){
    int* socket = (int *) socket_serveur;
    char* cmd = malloc(16*sizeof(char));
    char* mesg = malloc(2048*sizeof(char));
    char* help = malloc(1024*sizeof(char));
    strcat(help, "/quit - Quitte le serveur\n");
    strcat(help, "/list - Liste les informations des clients\n");
    strcat(help, "/help - Affiche l'aide\n");

    while(1){
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strcspn(cmd, "\n")] = '\0'; 

        if(strcmp(cmd,"/quit")==0){
            printf("/quit entrée\n");
            strcpy(mesg, "Arrêt du serveur, connection interrompue.");
            colorer(mesg, 'b');
            int i = 0;

            for(i; i<nb_client; i++){
                if((write(arrClient[i].sock,mesg,strlen(mesg)+1)) < 0){
                        perror("erreur : impossible d'ecrire le message destine au serveur.");
                        exit(1);
                }
                close(arrClient[i].sock); 
            } 
            sleep(3);
            close(*socket) ; 
            printf("Arrêt du serveur\n");
            exit(0);          
        }
        else if(strcmp(cmd,"/list")==0){
            printf("/list entrée\n");
            strcpy(mesg, listeClient());
            colorer(mesg, 'r');
            printf("%s\n", mesg);
        }
        else if(strcmp(cmd,"/help")==0){
            printf("/help entrée\n");
            colorer(help, 'r');
            printf("%s\n", help);
        }
    }

}

int main(int argc, char **argv) {
  
    int             socket_descriptor, 			
	               	nouv_socket_descriptor, 	
			     	longueur_adresse_courante; 	
    sockaddr_in 	adresse_locale, 			
			        adresse_client_courant; 	
    hostent*		ptr_hote; 				
    servent*		ptr_service; 				
    char 			machine[TAILLE_MAX_NOM+1]; 

    pthread_t       thread_cmd;                  
    

    gethostname(machine,TAILLE_MAX_NOM);		
    
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 		


    adresse_locale.sin_port = htons(5000);
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port));
    
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }
    
    listen(socket_descriptor,5);

    pthread_create(&thread_cmd, NULL, commandeServeur, &socket_descriptor);

    while(1) {
    
		longueur_adresse_courante = sizeof(adresse_client_courant);
		
		if (nb_client >= NB_CLIENTS_MAX) {
					perror("erreur : le serveur est saturé");
					exit(1);
		}
        else{
            if(arrClient[nb_client].connected == 0){
                if ((nouv_socket_descriptor = accept(socket_descriptor,(sockaddr*)(&adresse_client_courant),&longueur_adresse_courante))< 0) {
                    perror("erreur : impossible d'accepter la connexion avec le client.");
                    exit(1);
                }
                else{
                    arrClient[nb_client].connected = 1;
                    arrClient[nb_client].pseudo[0] = '\0';
                    arrClient[nb_client].sock = nouv_socket_descriptor;
                    pthread_create(&arrClient[nb_client].thread, NULL, commande, &arrClient[nb_client]);
                    nb_client++;    
                }                
            }
        }



    }

    return 0;  
}