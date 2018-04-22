#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <getopt.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#define TRUE 1
#define NBRECARACTERE 512
#define TAILLEPSEUDO 50
#define TAILLENOMENTREPRISE 50
#define TAILLENOM 5
#define TAILLECOLOR 6

/* Tableau comprenant les noms des entreprises */
/* du groupe */
char ENTREPRISE[6][12]= {"Centrale", "Nantes", "Cholet", "Brest", "Angers", "Rennes"};

/* Definition de la structure l'utilisateur (client)*/
struct Utilisateur {
    char * pseudo;
};

char* findColor(char* data, int taille_data, int tailleColor);
char* couleur = "blanc";

/* Affichage de l'interface chat*/
void affichage(){
  	printf("--------------------------------------------------------------------\n");
	printf("|                                                                  |\n");
	printf("|                                                                  |\n");
  	printf("|          Système U : Le commerce qui profite à tous !            |\n");
  	printf("|                                                                  |\n");
	printf("|               Bienvenue au Chat du groupement                    |\n");
 	printf("|                                                                  |\n");
	printf("--------------------------------------------------------------------\n");
}

/* fonction envoi de message */
/* prend en paramètre le socket du serveur, un taille et la taille du message et retourne 0 si le message est envoié */

//modification de couleur
void afficher_couleur(char *couleur)
{
  if (strcmp(couleur,"noir") == 0){
	  printf("\033[30m");
	  } else if (strcmp(couleur,"rouge") == 0) {
	  printf("\033[31m");
	  } else if (strcmp(couleur,"vert") == 0) {
	  printf("\033[32m");
	  } else if (strcmp(couleur,"jaune") == 0) {
	  printf("\033[33m");
	  } else if (strcmp(couleur,"bleu") == 0) {
	  printf("\033[34m");
	  } else if (strcmp(couleur,"magenta") == 0) {
	  printf("\033[35m");
	  } else if (strcmp(couleur,"cyan") == 0) {
	  printf("\033[36m");
	  } else {
	  printf("\033[37m");
	  }
}

//fonction qui cherche dans le envoyer_message du client la nouvelle couleur
char* findColor(char* data, int taille_data, int tailleColor) {
  data[taille_data - 1] = '\0';
  taille_data--;
  char* color = (char*) malloc((taille_data - tailleColor) * sizeof (char));
  int i = 0;

  while ((i < taille_data - tailleColor - 1)&&(data[i + tailleColor + 1] != ' ')) {
  *(color + i) = data[i + tailleColor + 1];
  i++;
  }
  *(color + i) = '\0';
  return color;
}


int envoyer_message(int sock_serv, char data[], int tailleData) {
    char *couleur;
    if (strstr(data, "/name")) {
        data[tailleData - 1] = '\0';
        tailleData--;
    } else if (strstr(data, "/quit")) {
        write(sock_serv, data, tailleData);
        close(sock_serv);
        exit(EXIT_SUCCESS);
    }
    //cas d'un changement de couleur d'interface
    else if(strstr(data, "/color" )){
      couleur = findColor(data, tailleData,TAILLECOLOR);
      afficher_couleur(couleur);
      if ((strcmp(couleur,"noir") == 0) || (strcmp(couleur,"rouge") == 0) ||
      (strcmp(couleur,"vert") == 0) || (strcmp(couleur,"jaune") == 0) || (strcmp(couleur,"bleu") == 0) ||
      (strcmp(couleur,"magenta") == 0) || (strcmp(couleur,"cyan") == 0) || (strcmp(couleur,"blanc") == 0)) {
      printf("Couleur d'interface changée ! \n");
      } else {
      printf("*********************************************************************\n");
      printf("\n");
      printf("cette couleur n'existe pas! \n");
      printf("couleur changée en blanc \n");
      }
    }

    write(sock_serv, data, tailleData);
    return EXIT_SUCCESS;
}

/* Defiition de la structure du client ou de l'utilisateur */
/* chaque utilisateur est utilise son pseudo , le numero du socket du serveur et le nom de son entreprise pour connnecter */


struct Utilisateur connexion_client(char * pseudo, int sock_serv,  char * entreprise) {
    struct Utilisateur user;
    char request[TAILLEPSEUDO + TAILLENOM + 1] = "/name ";
    char name[TAILLEPSEUDO] = "";

	strcat(pseudo, "@");
	strcat(pseudo, entreprise);

    strncat(request, pseudo, TAILLEPSEUDO);


    if (write(sock_serv, request, strlen(request)) != strlen(request))
        perror("L'écriture du nom à échouer");

    user.pseudo = strncat(name, pseudo, TAILLEPSEUDO);

    if (strlen(pseudo) > TAILLEPSEUDO) {
        fprintf(stderr, "Votre pseudo est trop long, veuillez rentrer un nouveau pseudo : %s\n", user.pseudo);
    }

    return user;
}


int gestion_client(int sock_serv) {
    fd_set readfds;
    int resSelect;
    int resReadClient = 0;
    int resReadServ = 0;
    char dataServ[NBRECARACTERE];
    char dataClie[NBRECARACTERE];


    while (TRUE) {

        FD_ZERO(&readfds);
        FD_SET(1, &readfds);
        FD_SET(sock_serv, &readfds);

        resSelect = select(sock_serv + 1, &readfds, NULL, NULL, NULL);

        switch (resSelect) {
            case 0:
                break;
            case -1:
                perror(" La sélection a échoué ");
                break;
            default:
                if (FD_ISSET(1, &readfds)) {

                    resReadClient = read(1, dataClie, NBRECARACTERE);

                    if (resReadClient >= 2)
                        envoyer_message(sock_serv, dataClie, resReadClient);
                }


                if (FD_ISSET(sock_serv, &readfds)) {


                    if ((resReadServ = read(sock_serv, dataServ, NBRECARACTERE)) < 0) {
                        perror("read failed in client");
                        break;
                    }
                    if(resReadServ == 0){
                        printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n");
                        fprintf(stderr,"le serveur a quitté...\n fin du programme client\n");
                        close(sock_serv);
                        exit(0);
                    }
                    write(1, dataServ, resReadServ);
                }
        }
    }
}

/* Connexion au serveur */
/* la fonction prend en paramètre le port et l'adresse ip du serveur*/

int connect_serv(int port, char* ip) {
    int sock_client;
    struct hostent* hp;
    struct sockaddr_in adresse_serv;

    // on teste si l'adresse Ip existe
    if ((hp = gethostbyname(ip)) == NULL) {
        perror("hostname");
        exit(1);
    }
    // on place l'adresse de la machine dans la structure de la socket du serveur
    bcopy(hp->h_addr, &adresse_serv.sin_addr, hp->h_length);
    adresse_serv.sin_family = hp->h_addrtype;

    adresse_serv.sin_port = htons(port);

    // on teste si le socket existe
    if ((sock_client = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    // on teste si l'adresse du serveur existe
    if (connect(sock_client, (struct sockaddr *) &adresse_serv, sizeof (adresse_serv)) < 0) {
        perror("connect");
        close(sock_client);
        exit(EXIT_FAILURE);
    }
    printf("-------------------------------------------------------------------------\n");
    fprintf(stderr, "Vous etes maintenant connecté au serveur \n");
    fprintf(stderr, " \n");
    printf("-------------------------------------------------------------------------\n");
    fprintf(stderr, "Bienvenue dans le réseau du groupement U \n");

    return sock_client;
}



int main(int argc, char* argv[]) {
    int c;
    char* ip = "127.0.0.1"; // adresse Ip par default
    int port = 9999;
    char* name = "Anonyme";
    char* entreprise="Centrale";
    int sock;
    int trouve=0;
    struct Utilisateur params;

    while ((c = getopt(argc, argv, "s:p:r:u:")) != -1)
        switch (c) {
            case 's':
                ip = optarg;
                break;

            case 'p':
                port = atoi(optarg);
                break;

            case 'r':
                name = optarg;
                break;

            case 'u':
                entreprise = optarg;
                int i;
                for(i=0; i<6; i++){
             // On verifie si l'entreprise fait partie du groupe
                if (strcmp(entreprise,ENTREPRISE[i]) == 0)
                {
                  trouve=1;
                }
                }
                if(trouve != 1){
                  printf("L'enseigne saisie ne fait pas partie du groupement !!\n");
                  return 0;
                }
                break;

            default:
                printf("./serveur [-pa]\n");
                printf("-p : Le numero de port est      :\n");
                printf("-s : L'adresse ip du serveur    :\n");
                printf("-r : Le nom de l'utilisateur    :\n");
                printf("-u : Le nom de son Enseigne est :\n");
                break;
        }
    printf("                             \n");
    printf("                             \n");
    printf("-------------------------------------------------------------------------\n");
    printf("                             \n");

    printf("L'Adresse IP du serveur est : %s \n", ip);
    printf("-------------------------------------------------------------------------\n");
    printf("                             \n");

    printf("Le numero de port est       : %d \n", port);
    printf("-------------------------------------------------------------------------\n");
    printf("                             \n");

    printf("Votre Nom                   : %s \n", name);
    printf("-------------------------------------------------------------------------\n");
    printf("                             \n");

    printf("Le nom de l'entreprise est  : %s \n", entreprise);
    printf("-------------------------------------------------------------------------\n");
    printf("                             \n");

    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    printf("                             \n");
    affichage();

    sock = connect_serv(port, ip);
    params = connexion_client(name, sock, entreprise);
    gestion_client(sock);
    return 1;
}
