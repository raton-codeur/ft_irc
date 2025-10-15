fusion du 14/10


classe Channel

renommer addInvite → invite

pas besoin de remove invite car on ne peut pas annuler l'invitation d'un client à un channel


classe client

plus besoin de l'id

on stocke le hostname et le realname mais on pourra hardcoder "localhost" et ignorer le realname

le nom le plus courant c'est registered, pas authenticated

la fonction register passe définitivement le client en registered


cmdHandler : pas besoin de new

port et password dans le serveur à init dans le constructeur du serveur

une map clients_by_nick pour optimiser ? apparemment c'est ce qui se fait le plus.
ajouter le client qui correspond au nickname lorsqu'on set le nickname

pas besoin de getChannels ? juste getChannel à partir du nom


dans le split de la commande il faut passer le préfixe optionnel au début

noms des méthodes en minuscule mais mots-clés en majuscule

fonction toUpper en mode C, static au fichier, vu que c'est une petite fonction utilitaire.
on peut la mettre dans utils.cpp sinon.
