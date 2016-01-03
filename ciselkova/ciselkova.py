#!/usr/bin/env python
import itertools

results = [11, 7, 17, 22, 9, 18, 11, 12, 8, 9]

class Vertice:
	
	def __init__(self, name):
		self.name = name
		self.key = -1
		self.neighbors = None

	@staticmethod
	def all_correct(vertices):
		for v in vertices:
			if not v.is_correct():
				return False
		return True
	
	def set_key(self, key):
		self.key = key

	def set_neighbors(self, neighbors):
		self.neighbors = neighbors

	def is_correct(self):
		if self.key == -1:
			print "[-] You must set key first for %c!" % self.name
			return False
		if self.neighbors is None:
			print "[-] You must set some neighbors for %c!" % self.name
			return false

		result = sum([x.key for x in self.neighbors])
		return result == results[self.key]

	def __str__(self):
		if self.neighbors is None:
			return "  [.] Vertice %c with no neighbors." % self.name

		neitotal = sum([x.key for x in self.neighbors])
		return "  [.] Vertice %c with key %d (value %d), neig = %d (value %d)" % (self.name, self.key, results[self.key], len(self.neighbors), neitotal)



def bruteforce(vertices):
	print "[+] Bruteforcing the solution"
	keys = [x for x in range(0, 10)]
	iteration = 1
	for permutation in itertools.permutations(keys):
		# update keys
		for n in range(0, len(vertices)):
			vertices[n].set_key(permutation[n])
		#check for correct solution
		if Vertice.all_correct(vertices):
			print "[+] Result found (iteration %d)!" % iteration
			return
		iteration += 1
	# not found?
	print "[-] Solution not found. Bug somewhere?"
	return

# ------------------------------------------------------------------------ #

# create vertices
a = Vertice("A")
b = Vertice("B")
c = Vertice("C")
d = Vertice("D")
e = Vertice("E")
f = Vertice("F")
g = Vertice("G")
h = Vertice("H")
i = Vertice("I")
j = Vertice("J")

vertices = [a, b, c, d, e, f, g, h, i, j]

a.set_neighbors([b, c])
b.set_neighbors([a, c, e, f])
c.set_neighbors([a, b, d, j])
d.set_neighbors([c, e, g])
e.set_neighbors([b, d])
f.set_neighbors([b, g, h])
g.set_neighbors([d, f, h])
h.set_neighbors([f, g, i])
i.set_neighbors([h, j])
j.set_neighbors([c, i])


bruteforce(vertices)
print "[.] Printing result:"
for v in vertices:
	print v


