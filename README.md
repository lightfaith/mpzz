# mpzz
MPZZ Project - Dijkstra on directed graph

SPF
	run:
		./spf data/test.gml output.txt A 9
		./spf data/astro-ph.gml output.txt 3 1
		./spf gen/1000.gml output.txt 3

	debug:
		valgrind --leak-check=full ./spf gen/1.gml output.txt 0 9

GEN
	run:
		./gen gml 500 500nodes.gml
