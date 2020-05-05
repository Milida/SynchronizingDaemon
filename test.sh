#!/bin/bash
gcc demon.c filelib.c filelib.h -o Demon

mkdir -p ~/folder1/qwe/nowyFolder/nowy
mkdir -p ~/folder1/qwe/kolejnyFolder
mkdir -p ~/folder1/qwe/folderX
mkdir -p ~/folder1/asd
mkdir -p ~/folder2/aa/123/foolder
mkdir -p ~/folder2/innyfolder

touch ~/folder1/qwe/nowyFolder/nowy/a.txt
echo "Zawartosc pliku a.txt" > ~/folder1/qwe/nowyFolder/nowy/a.txt
touch ~/folder1/qwe/nowyFolder/nowy/new.doc
echo "Tym razem inny plik" > ~/folder1/qwe/nowyFolder/nowy/new.doc
touch ~/folder1/qwe/nowyFolder/b.txt
echo "Kolejny nowy plik" > ~/folder1/qwe/nowyFolder/b.txt
touch ~/folder1/filee
echo "Nowy plik w folderze folder1" > ~/folder1/filee
touch ~/folder2/pliczek.txt
echo "Plik w folderze folder2" > ~/folder2/pliczek.txt
touch ~/folder2/aa/123/so
echo "Jeszcze inny plik" > ~/folder2/aa/123/so

echo "Utworzono środowisko testowe - foldery ~/folder1 i ~/folder2"

./Demon ~/folder1 ~/folder2 -r -s 20

echo "Uruchomiono demona synchronizującego rekurencyjnie, budzącego się co 20 s"