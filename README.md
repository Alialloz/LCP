# LCP
 LCP ("Copie Paresseuse et Concurrente de Fichiers") est une application de copie de fichiers avancée conçue pour minimiser les écritures inutiles. Ce projet met en œuvre des concepts de création et de terminaison de processus dans un contexte algorithmique et traite différents types de fichiers de manière efficace.

## Fonctionnalités Principales

### Usage
`lcp [-j nb_proc] [-l] [-b TAILLE] SOURCE... DESTINATION`

### Options
- **-l**: Copie les liens plutôt que les fichiers liés.
- **-j**: Permet la copie concurrente avec des processus enfants.
  - `-j 2` : Un orchestrateur et un processus de copie.
  - `-j 3` : Un orchestrateur et deux processus de copie.
  - Plus de processus sont gérés de manière similaire.

## Détails d'Implémentation

- **Gestion des Liens**: Utilisation de `lstat`, `readlink`, `symlink`.
- **Gestion des Processus**: Utilisation de `fork` et `wait`.
- **Copie Paresseuse**: Seuls les blocs nécessaires sont copiés.
- **Option -b**: Spécifie la taille des blocs à utiliser.

## Code de Retour

- **0** : Succès.
- **1** : Erreur, avec un message d'erreur approprié.

## Développement et Utilisation

- Développement en C.
- Fichier source principal : `lcp.c`.
- Compilation et tests via `make`.

## Tests et Validation

- Tests intégrés pour vérifier le bon fonctionnement.
- Lancement des tests localement pour validation rapide.

## Points Techniques

- **Conception Modulaire** : Développement en modules organisés.
- **Gestion des Erreurs** : Traitement adéquat des cas d'erreurs.
- **Qualité du Code** : Focus sur la lisibilité, robustesse et documentation.
