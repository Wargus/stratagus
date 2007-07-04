#!/bin/bash
for i in bos-??.po; do msgmerge -U -N $i bos.pot; done
for i in ??.po; do msgmerge -U -N $i engine.pot; done
