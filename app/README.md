# Compte rendu IoT

##### Valentin Legros
##### Souad Mokrani
##### Pierre-Jean Sauvage

## Scénario

On souhaite simuler la récupération de métriques énergétiques de serveurs dans un centre de données. On imagine que chaque serveur dispose d'un wattmètre (noeud) qui lit la consommation d'énergie du serveur. Chaque noeud envoie ses données d'énergie en connexion sans fil, vers un noeud principal, qui récolte ces données et les envoie sur une base de données.

Potentiellement, le noeud principal peut être hors de portée des noeuds. Il est donc nécessaire de faire du multi-saut pour ce cas. Le destinataire intermédiaire pour le multi-saut peut être choisi dynamiquement, afin de pouvoir résister à une panne d'un noeud intermédiaire. 

On peut également imanginer avoir plusieurs noeud principaux de collecte. Dans ce cas il faut
pouvoir répartir tous les noeuds selon un noeud collecteur le plus cohérent.

## Implémentation

L'implémentation utilise comme couche de liaison le `802.15.4`. Le logiciel se repose sur 
RIOT OS.

### Simplifications

Les cartes M3 sur IoT-Lab ne sont pas équipées de capteurs de puissance. Nous utilisons donc 
en simulation un capteur de température. Par ailleurs, nous ne faisons pas de multi-saut, et n'avons qu'un seul noeud collecteur, qui est supposé à portée de communication. Il n'y a pas de routage.

### Améliorations possibles

Beaucoup des simplifications effectuées pourrait être implémentées, même avec les contraintes imposées par la plateforme de tests. Le multi-saut et le routage pourraient être implémentés, ainsi que la multiplication des noeuds collecteurs.

### Difficultés rencontrées

RIOT OS est une couche logicielle assez complexe à prendre en main, avec une courbe 
d'apprentissage assez difficile. Même avec la documentation, il nous est arrivé de passer à côté de petits détails qui empêchaient le fonctionnement de toute l'application (initialisation de la carte au mauvais moment par exemple). 

## Compilation des sources

Pour compiler les sources pour une carte iotlab-m3, il suffit d'aller le dossier `RIOT/app`, et de lancer `make`. Le binaire ELF sera dans `RIOT/app/bin/iotlab-m3/dust.elf`. Pour compiler, il est nécéssaire d'avoir un compilateur ARM.

## Autres remarques

L'adresse du noeud principal est hard-codé en tant que macro dans le fichier `main.c`. L'adresse par défaut est celle du noeud ayant comme identifiant iotlab : `m3-161.lille.iot-lab.info`. On peut donc lancer l'expérience avec ce noeud, ou bien changer la valeur de la macro dans le code ou à la compilation.


