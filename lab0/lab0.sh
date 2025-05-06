#!/bin/bash
cat /proc/cpuinfo | grep "model name" #Punto 1
cat /proc/cpuinfo | grep "model name" | wc -l #Punto 2
wget https://www.gutenberg.org/files/11/11-0.txt && sed 's/Alice/Guada/g' 11-0.txt > GUADA_in_wonderland.txt && rm 11-0.txt #Punto 3
sort -n -k 5 weather_cordoba.in | head -1 | cut -d " " -f1,2,3  #Punto 4 (Temperatura maxima minima)
sort -n -k 5 weather_cordoba.in | tail -1 | cut -d " " -f1,2,3  #Punto 4 (Temperatura maxima maxima)
sort -n -k 3 atpplayers.in #Punto 5
awk '{print $0, $7-$8}' superliga.in | sort -r -n -k 2,9 #Punto 6
ip addr | grep ^.*link/ether #Punto 7
mkdir Serie && cd Serie && touch fma_S01E{1..10}_es.srt && for f in *srt; do mv -- "$f" "${f%_es.srt}.srt"; done #Punto 8