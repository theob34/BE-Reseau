#include <mictcp.h>
#include <api/mictcp_core.h>

//Variables servant pour numéroter les paquets
int num_seq = 0;
int num_ack = 0;

//Adresse locale et distante
mic_tcp_sock SOCKET_LOCAL ;
mic_tcp_sock_addr ADRESSE_DISTANTE ;


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   
   int result = -1;
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(0);

   return result;
}


/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   
   //Je crée mon socket local en lui attribuant une adresse et un numéro de socket
   SOCKET_LOCAL.fd = socket ;
   SOCKET_LOCAL.state = IDLE;
   SOCKET_LOCAL.addr = addr;

   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr * addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    SOCKET_LOCAL.state = ESTABLISHED ; 
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */

int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    
    //Je passe la connexion en mode établi et définit l'adresse avec laquelle je vais communiquer (i.e. le puits)
    SOCKET_LOCAL.state = ESTABLISHED ; 
    ADRESSE_DISTANTE = addr;

    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    //printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    
    //Je crée le PDU contenant mon message à envoyer
    int sent_size ;
    mic_tcp_pdu pdu;

    //Je remplis mon PDU
        //Header
    pdu.header.source_port = SOCKET_LOCAL.addr.port;
    pdu.header.dest_port = ADRESSE_DISTANTE.port;
    pdu.header.seq_num = num_seq;
    pdu.header.ack_num = num_ack;
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;
        //Playload
    pdu.payload.data = mesg ;
    pdu.payload.size = mesg_size ;

    //Je vérifie que la connexion est bien établie entre ma source et mon puits
    if (SOCKET_LOCAL.state != ESTABLISHED) {
        printf("Le socket n'est pas en état connecté");
        return -1;
    }

    //J'incrémente le numéro de paquet (qui sera celui du prochain paquet à envoyer)
    num_seq = (num_seq + 1)%2 ; 

    //J'envoie mon paquet
    if((sent_size = (IP_send(pdu, SOCKET_LOCAL.addr)))==-1) {
        printf("[MIC-TCP] Erreur dans l'envoi du paquet");
        exit(0);
    }

    return sent_size ;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    //printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    //Je construis le payload qui va recevoir le message
    mic_tcp_payload payload;
    payload.data = mesg;
    payload.size = max_mesg_size;

    int read_size;

    //Je recupère le message depuis le buffer et je retourne le nombre d'octet(s) lu(s)
    read_size = app_buffer_get(payload);

    return read_size;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 * PAS BESOIN DE CODER CETTE ANNEE
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    //printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    //Je stocke le paquet reçu dans le buffer
    app_buffer_put(pdu.payload);

}




