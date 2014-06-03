import os, sys

TEXT     = 0
VARIABLE = 1
IF       = 2
ELIF     = 3
ELSE     = 4
ENDIF    = 5
FUNC     = 6

def pack_ifs(code, index):
	out = []
	pos = index
	length = len(code)
	restart = False
	while pos < length:
		item = code[pos]
		if item[0] == IF:
			part, pos, restart = pack_ifs(code, pos + 1)
			out.append((IF, item[1], part))
			continue
		elif item[0] == ELIF:
			if restart:
				part, pos, restart = pack_ifs(code, pos + 1)
				out.append((ELSE, item[1], part))
				continue
			else:
				return (out, pos, True)
		elif item[0] == ELSE:
			if restart:
				part, pos, restart = pack_ifs(code, pos + 1)
				out.append((ELSE, None, part))
				continue
			else:
				return (out, pos, True)
		elif item[0] == ENDIF:
			return (out, pos + 1, True)
		elif item[0] == TEXT:
			#print pos, index, restart, "%r" % item[1]
			if (pos == index and index > 0) or restart: out.append((TEXT, item[1].lstrip("\n\r")))
			else: out.append(item)
		else:
			out.append(item)
		restart = False
		pos += 1

	return (out, pos, False)

def print_code(code, indent = ""):
	for item in code:
		if item[0] == IF:
			print "%sif %s:" % (indent, item[1])
			print_code(item[2], indent + "    ")
		elif item[0] == ELIF:
			print "%selif %s:" % (indent, item[1])
			print_code(item[2], indent + "    ")
		elif item[0] == ELSE:
			print "%selse:" % indent
			print_code(item[2], indent + "    ")
		elif item[0] == TEXT:
			print "%s%r" % (indent, item[1])
		elif item[0] == VARIABLE:
			print "%s%%[[%s]]" % (indent, item[1])
		elif item[0] == FUNC:
			print "%s%%%s(%s)" % (indent, item[1], item[2])
		else:
			print "%s%r" % (indent, item)

def stringify(arg):
	return '"' + arg.replace("\\", "\\\\").replace('"', '\\"') + '"'

class FileTemplate:
	def __init__(self, vars):
		self.vars = vars

	def compile(self, path):
		try:
			f = open(path, "r")
		except IOError as e:
			print "%s: error %s: %s" % (path, e.errno, e.strerror)
			sys.exit(1)
		src = f.read()
		f.close()
		src = src.split("%[[")
		out = []
		out.append((TEXT, src[0]))
		for i in range(1, len(src)):
			s = src[i].split("]]")
			if len(s) != 2:
				print "%s: error: bad syntax" % path
				sys.exit(1)
			v = s[0]
			if v[:3] == "if:":
				out.append((IF, v[3:]))
			elif v[:5] == "elif:":
				out.append((ELIF, v[5:]))
			elif v == "else":
				out.append((ELSE, None))
			elif v == "endif":
				out.append((ENDIF, None))
			elif ':' in v:
				v = v.split(':', 1)
				out.append((FUNC, v[0], v[1]))
			else:
				out.append((VARIABLE, v))
			out.append((TEXT, s[1]))
			
		out, index, ignore = pack_ifs(out, 0)
		return out

	def produce(self, compiled):
		#print_code(compiled)
		return "".join(self.run(compiled))

	def _if(self, var):
		if var not in self.vars: return False
		return not not self.vars[var]

	def run(self, code):
		skipping = False
		out = []

		for item in code:
			if item[0] == IF:
				skipping = self._if(item[1])
				if skipping:
					sub = self.run(item[2])
					out += sub
			elif item[0] == ELIF:
				if skipping: continue
				skipping = self._if(item[1])
				if skipping:
					sub = self.run(item[2])
					out += sub
			elif item[0] == ELSE:
				if skipping: continue
				sub = self.run(item[2])
				out += sub
			elif item[0] == TEXT:
				out.append(item[1])
			elif item[0] == VARIABLE:
				if item[1] in self.vars:
					out.append(str(self.vars[item[1]]))
			elif item[0] == FUNC:
				if item[1] == "STR":
					if item[2] in self.vars:
						out.append(stringify(str(self.vars[item[2]])))
				else:
					print "error: unknown function `%s'" % item[1]
					sys.exit(1)
			else:
				print "error: unknown opcode %s" % item[0]
				sys.exit(1)

		return out

	def output(self, text, path):
		try:
			directory = os.path.dirname(path)
			if not os.path.exists(directory):
				os.makedirs(directory)
			f = open(path, "w")
			f.write(text)
			f.close()
		except IOError as e:
			print "%s: error %s: %s" % (path, e.errno, e.strerror)
			sys.exit(1)

	def generate(self, tmplt, dest):
		self.output(self.produce(self.compile(tmplt)), dest)

class Template(FileTemplate):
	def __init__(self, vars, name):
		FileTemplate.__init__(self, vars)
		self.name = name
		self.files = {}
		self.outdir = os.path.realpath(self.vars["PLUGIN_TARGET"])
		if "SOLUTION" in self.vars:
			self.outdir = os.path.realpath(self.vars["SOLUTION"])
		self.source = os.path.join(os.path.dirname(os.path.realpath(__file__)), "template", name)
		files = self.compile(os.path.join(self.source, name + ".files"))
		text = self.produce(files)
		for line in text.split('\n'):
			line = line.split('=', 1)
			if len(line) < 2: continue
			self.files[line[0].strip()] = line[1].strip()

	def install(self):
		keys = self.files.keys()
		keys.sort()
		for infile in keys:
			#print os.path.join(self.source, infile), "->", os.path.join(self.outdir, self.files[infile])
			self.generate(os.path.join(self.source, infile), os.path.join(self.outdir, self.files[infile]))
