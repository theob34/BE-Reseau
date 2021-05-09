#include <mictcp.h>
#include <api/mictcp_core.h>

//Variables servant pour numéroter les paquets
int seq_num = 0;
int ack_num = 0;

//Adresses locale et distante
mic_tcp_sock SOCKET_LOCAL ;     //Correspond à la source
mic_tcp_sock_addr ADRESSE_DISTANTE ;    //Correspond au puits


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

   int result = -1;
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(50);

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
    pdu.header.seq_num = seq_num;
    pdu.header.ack_num = 0;
    pdu.header.syn = 0;
    pdu.header.ack = 0;
    pdu.header.fin = 0;
        //Playload
    pdu.payload.data = mesg ;
    pdu.payload.size = mesg_size ;

    //Je vérifie que la connexion est bien établie entre ma source et mon puits
    if (SOCKET_LOCAL.state != ESTABLISHED) {
        printf("Le socket n'est pas en état connecté\n");
        return -1;
    }

    //J'incrémente le numéro de paquet modulo 2 (qui sera celui du prochain paquet à envoyer)
    seq_num = (seq_num + 1)%2;

    //Je crée les variables me permettant de recevoir le PDU contenant l'ACK
    mic_tcp_pdu ack_pdu;
    int pdu_recu;
    
    do {

        //J'envoie mon paquet
        if((sent_size = (IP_send(pdu, SOCKET_LOCAL.addr)))==-1) {
            printf("[MIC-TCP] Erreur dans l'envoi du paquet");
            exit(0);
        }
        
        //J'attends l'ACK
        pdu_recu = IP_recv(&ack_pdu, &ADRESSE_DISTANTE, 1); //timeout d'une milliseconde

    } while ((pdu_recu == -1) || (ack_pdu.header.ack_num != seq_num)) ;
    //Si je n'ai pas recu de PDU ou que le numéro ne correspond pas, je recommence l'envoi

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

    //Si le paquet reçu est celui attendu, je le stocke dans le buffer et j'incrémente mon ack
    if (pdu.header.seq_num == ack_num) {
        app_buffer_put(pdu.payload);
        ack_num = (ack_num + 1)%2 ;
    }

    //Dans les deux cas (erreur ou pas) je renvoie un paquet avec le numéro d'ACK attendu
    //Je remplit le PDU contenant l'ACK
    mic_tcp_pdu pdu_ack;
        //Header (le playload reste vide)
    pdu_ack.header.source_port = ADRESSE_DISTANTE.port;
    pdu_ack.header.dest_port = SOCKET_LOCAL.addr.port;
    pdu_ack.header.seq_num = 0;
    pdu_ack.header.ack_num = ack_num;
    pdu_ack.header.syn = 0;
    pdu_ack.header.ack = 1;
    pdu_ack.header.fin = 0;

    //Je renvoie mon pdu avec l'ACK
    if ((IP_send(pdu_ack, addr))==-1) {
        printf("Erreur envoi ACK\n");
        exit(0);
    }

}