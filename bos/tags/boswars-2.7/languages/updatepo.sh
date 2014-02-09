#!/bin/bash
for i in bos-??.po bos-??-*.po; do msgmerge -U  $i bos.pot; done
for i in ??.po ??-*.po; do msgmerge -U  $i engine.pot; done
