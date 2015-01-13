#!/bin/bash
for f in jobs/*.sh
do
    qsub $f
done
