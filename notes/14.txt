### travail sur poll

je pense qu'on peut définir nos constantes directement dans (la déclaration de) la classe Server, en static const. sinon, on peut faire des #define dans le main.hpp (comme en C mais c'est pas ouf), ou bien des variables globales dans le main.hpp (c'est très moche), ou bien un namespace spécial dans le main.hpp :
namespace irc {
    static const size_t MAX_READ_PER_CYCLE = 65536;
    static const size_t MAX_CLIENT_INPUT_BUFFER = 262144;
}

j'ai ajouté une petite exception personnalisée dans main.hpp : struct StopRequested {};

j'ai gardé la logique de la boucle principale dans main avec g_stop_requested et j'ai ajouté une petite fonction checkSignals dans signal.cpp pour checker g_stop_requested aussi n'importe où dans le code et remonter au bloc try catch principal

normalement c'est poll qui sera interrompu par SIGINT car c'est la seule fonction bloquante. peu importe le signal qui l'interrompt, il revient au try catch du main qui check g_stop_requested. si c'est un autre signal qui l'interrompt, il se relancera normalement (mais bon, ça n'arrivera probablement jamais).

dans la fonction acceptClients, on a une boucle "infinie" qui s'arrête seulement quand il n'y a plus de client à accepter (ou qu'il y a une erreur avec accept). si JAMAIS on a un SIGINT qui arrive pile poil au lancement de accept, j'ai mis un checkSignals pour remonter les appels. si jamais le client a tout de suite annulé sa connexion (ECONNABORTED) ou qu'un autre signal inconnu a interrompu accept, on recommence accept.

dans handleClientEvents, on boucle sur le tableau de poll pour checker les évènements qui se sont produits sur les clients. s'il y a une erreur sur un client, on le supprime. normalement, un client qui se déco va trigger POLLHUP, mais il peut aussi juste trigger POLLIN et on le verra dans handleClientPOLLIN avec recv qui renvoie 0. quand on supprime un client, on le delete et on ne change pas i (puisqu'on fait des swap and pop).

pour un client donné, si POLLIN est signalé par poll, alors on sait qu'il veut nous envoyer des données. on appelle handleClientPOLLIN.

pour handleClientPOLLIN, on a :
- un buffer qui sert à recv, de taille BUFFER_RECV_SIZE.
- un nombre limite d'octets qu'on peut recevoir à la fois par client (donc par appelle de handleClientPOLLIN), c'est MAX_READ_PER_CYCLE.
- un nombre max d'octets par ligne de commande irc, c'est MAX_LINE_LENGTH.
- un nombre max d'octets qu'on peut avoir dans le buffer de réception d'un client, c'est MAX_CLIENT_INPUT_BUFFER.

dans handleClientPOLLIN, on a une boucle "infinie" qui :
- appelle recv, ce qui remplie le buffer de recv.
- append le buffer de recv au buffer de réception du client.
- appelle handleCommand pour examiner le buffer de réception du client. handleCommand va détecter s'il y a une / plusieurs lignes complètes dans le buffer, exécuter les commandes et les retirer du buffer du client. attention, handleCommand peut supprimer le client (par exemple si la ligne dépasse MAX_LINE_LENGTH ou si la commande c'est QUIT). je propoes que si elle supprime un client, elle renvoie 1, sinon elle renvoie 0.

- on sort quand on a plus rien à lire (ou qu'il y a eu une erreur).
- si le client s'est déconnecté ou qu'on a fini de lire, c'est normal. sinon, c'est une autre erreur improbable et on peut supprimer le client.
- si jamais on a un signal qui interrompt recv, on fait checkSignals pour détecter SIGINT et on relance recv pour les autres signaux improbables.
- si jamais on a détecté un POLLIN sur ce client mais que recv renvoie EAGAIN, on fait comme si de rien n'était et on passe au prochain client. ça veut sûrement dire que le client avait envoyé quelque chose mais qu'entre le moment où on a fait poll et celui où on a fait recv, les données se sont miraculeusement envolées. pour le bénéfice du doute, on ne touche pas à ce client.

après un recv > 0 :
- si on a dépassé MAX_READ_PER_CYCLE, on passe aux clients suivants pour ne pas les faire poireauter trop longtemps (si un client envoye beaucoup de commandes d'un coup, il ne faut pas s'occuper exclusivement de lui en abandonnant les autres clients).
- si handleCommand détecte qu'une ligne fait plus de MAX_LINE_LENGTH caractères (\r\n compris), je pense qu'on peut kicker le client.
- si jamais un client arrive à faire saturer son buffer de réception (la taile dépasse MAX_CLIENT_INPUT_BUFFER) (en envoyant d'un coup plein de commandes par exemple), il faut le kick.

### fusion

-
main, signal


ajout du StopRequested

signal -> signals

ajout de void checkSignals();

ajout du catch (StopRequested&)
-
classe server


ajout des constantes en static

hostname -> const

ajout du _buffer_recv en attribut

delete client passe en public ? juste besoin de passe l'indice du client à supprimer ?
supprimer le nickname lors du delete

getChannel pass en privé, on garde getOrCreateChannel en public ?

pas de processClientBuffer dans le server

getHostname et getPassword renvoient const string&

addClientToNickMap
pas besoin du nickname en paramètre, juste le pointeur sur le client

sendNamesList
pourquoi une ref à un client en paramètre ?

modif des fonctions poll : acceptClients, handleClientErrors, handleClientPOLLIN, handleClientEvents
-
classe Channel

getOperators renvoie une copie ?
-
CommandHandler

changement du type de handleCommand
renvoie si la commande a supprimé le client
prend un client et son indice (pour le delete)
-

### makefile

- `d` : lancer un conteneur docker temporaire
- `make test` : recompiler et lancer le serveur
- `make nc` : connecter un client avec nc
