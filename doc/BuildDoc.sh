#!/bin/sh
latex2html -local_icons UserGuide.tex
latex2html -local_icons DeveloperGuide.tex

#tar cvpf UserGuide.tar UserGuide
#gzip UserGuide.tar
#rm -r UserGuide

#tar cvpf DeveloperGuide.tar DeveloperGuide
#gzip DeveloperGuide.tar
#rm -r DeveloperGuide

#mv html tacppd-api
#tar cvpf tacppd-api.tar tacppd-api
#rm -r tacppd-api
#gzip tacppd-api.tar
