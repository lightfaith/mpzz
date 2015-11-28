# mpzz
MPZZ Project - Dijkstra on directed graph

Compile on Linux (preferrably Debian-based) OS:

	make clean && make

SPF

	run:
		./spf <input file> <format> <output file> <root node> <debug level> 

		./spf data/test.gml gml output.txt A 9
		./spf data/astro-ph.gml output.txt 3
		./spf data/OrientOhodG1.csv csv output.txt 30 1

	timing:
		time ./spf data/OrientOhodG2.csv csv output.txt 0 1

GEN

	run:
		./gen <format> <nodes> <output file>

		./gen gml 500 500nodes.gml
		./gen csv 500 500nodes.csv
