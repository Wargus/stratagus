#!/bin/bash
for i in bos-??.po; do msgmerge -U  $i bos.pot; done
for i in ??.po; do msgmerge -U  $i engine.pot; done
