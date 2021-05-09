BE-Reseau : 

Vous pourrez retrouver dans ce repertoire le protocole mictcp. Ce protocole, différent selon les versions, est utilisé dans le but d'une transmission d'informations textuelles ou vidéos. Voici le détail de chaque versions (vous pouvez retrouver toutes les versions dans "./versions\ à\ rendre")


    mictcp_v1 : 

    Protocole assurant le transfert de données sans reprise des pertes

    mictcp_v2 : 

    Protocole assurant le transfert de données avec reprise des pertes totales. 
    Pour la garantie de fiabilité totale, on utilise le mécanisme de reprise des pertes de type "Stop and Wait".

    Pour cela : -> côté source, on envoie un message jusqu'à recevoir l'ACK attendu.
                -> côté puits, on attend le paquet et si son numéro de séquence correspond à celui attendu, alors on renvoie l'ACK demandant le paquet suivant.

    mictcp_v3 : 

    Protocole assurant le transfert de données avec reprise des pertes partielles.
    Pour la garantie de fiabilité partielle, on utilise le mécanisme de reprise des pertes de type "Stop and Wait" à fiabilité "pré cablée" (c'est à dire dont le taux de pertes admissibles est défini de façon statique).

    Pour cela : -> côté source, on envoie le message.   Si on recoit pas d'ACK, on étudie la nécessité d'un renvoi et on agit en conséquence.
                                                        Si on recoit un ACK et que le numéro correspond, on a réussi.
                                                        Si on recoit un ACK et que le numéro ne correspond pas, on étudie la nécessité d'un renvoi et on agit en conséquence

                -> côté puits, on attend le paquet et si c'est un paquet que je n'ai encore jamais recu (i.e. son numéro de séquence est supérieur au dernier reçu) alors on renvoie l'ACK demandant le paquet suivant.


Informations complémentaires : 

Les trois versions fonctionnent. 
Pour en utiliser une ou l'autre, copiez le fichier voulu dans "./src" et renommez le "mictcp.c". Par défaut, la v3 est actuellement utilisée.
Faites ensuite un "make" et vous pouvez utiliser les applications tsock_video ou tsock_texte.

Pistes d'améliorations du code : 

    mictcp_v3 : 
    
    Le protocole pourrait être améliorer de plusieurs manières : 
        - Utiliser une fenêtre glissante pour le calcul des pertes réelles de paquet afin d'éviter des périodes de grosses pertes possibles grâce à des périodes de faibles pertes préalables. De plus, cela permet de borner les entiers paquets_envoyes et paquets_recus et ainsi éviter un débordement dans le cas où la communication se voit transmettre un trop grand nombre de paquets.
        - Utiliser un mécanisme permettant de différencier la perte du paquet ou la perte de l'ACK afin de connaître le nombre réel de paquet reçu par la puits actuellement faussé par cette non-différenciation. (limite : ce mécanisme peut potentiellement s'avérer plus lourd et gourmand en temps que simplement le renvoi du paquet).




