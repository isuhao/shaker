import os
from subprocess import Popen, PIPE

def check_git():
	try:
		p = Popen(['git', 'version'], stdout=PIPE)
		p.wait()
		return True
	except:
		return False

def repo_dir(dirname):
	while dirname != "":
		repo = os.path.join(dirname, ".git")
		if os.path.exists(repo):
			return dirname
		tmp = os.path.dirname(dirname)
		if tmp == dirname:
			return None
		dirname = tmp

has_git = check_git()

class Repo:
	def __init__(self, dirname):
		self.dirname = dirname
		self.repo = repo_dir(dirname)

	def exists(self):
		return self.repo is not None

	def init(self):
		p = Popen(['git', 'init', self.dirname])
		p.wait()
		
		try:
			f = open(os.path.join(self.dirname, ".gitignore"), "w")
			f.writelines([
				"int/\n",
				"out/\n",
				"*.opensdf\n",
				"*.sdf\n",
				"*.suo\n",
				"*.orig\n",
				"*.user\n"
			])
			f.close()
			self.add(".gitignore")
		except IOError:
			pass

	def add(self, file):
		p = Popen(['git', 'add', file], cwd = self.dirname)
		p.wait()

	def known_files(self):
		p = Popen(['git', 'ls-files'], stdout=PIPE, cwd = self.dirname)
		f, ignore = p.communicate()
		return f.split("\n")
