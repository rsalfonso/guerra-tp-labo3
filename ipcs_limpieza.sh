#!/bin/bash

USUARIO=`whoami`

MEMORIAS_COMPARTIDAS=`ipcs -m | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`
SEMAFOROS=`ipcs -s | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`
COLAS_DE_MENSAJES=`ipcs -q | egrep "0x[0-9a-f]+ [0-9]+" | grep $USUARIO | cut -f2 -d" "`


for id in $MEMORIAS_COMPARTIDAS; do
  ipcrm -m $id;
done

for id in $SEMAFOROS; do
  ipcrm -s $id;
done

for id in $COLAS_DE_MENSAJES; do
  ipcrm -q $id;
done