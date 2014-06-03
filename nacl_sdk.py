import argparse, os

def switches():
	parser = argparse.ArgumentParser()
	parser.add_argument("PPAPI", help="NaCl SDK Pepper API path")

	args = parser.parse_args()
	args.PPAPI = os.path.dirname(os.path.join(args.PPAPI, "."))
	return args

class PPAPI:
	def __init__(self, api):
		self.api = api
		self.files = []
		self.filters = []

	def scan(self, dirs):
		api_len = len(self.api) + len(os.sep)
		exts = [".h", ".hh", ".hpp", ".c", ".cc", ".cpp"]
		for sub in dirs:
			path = os.path.join(args.PPAPI, *sub.split("/"))
			if not os.path.exists(path):
				print "error: %s does not exist" % path
				exit(1)
			for root, dir, files in os.walk(path):
				for name in files:
					if os.path.splitext(name)[1] not in exts: continue
					self.add(os.path.join(root, name)[api_len:])
		self.filters.sort()
		self.files.sort()

	def add(self, file):
		self.files.append(file)
		filter = os.path.dirname(file)
		while filter != "":
			if filter not in self.filters:
				self.filters.append(filter)
			tmp = os.path.dirname(filter)
			if tmp == filter: break
			filter = tmp

def main():
	args = switches()
	dirs = [
		"include",
		"src/jsoncpp",
		"src/ppapi",
		"src/ppapi_cpp",
		"src/pthread",
		"src/sdk_util"
	]
	api = PPAPI(args.PPAPI)
	print "PPAPI:", args.PPAPI
	api.scan(dirs)
	#for filter in api.filters: print "NaCl SDK\\%s" % filter
	#for fname in api.files: print "$(NACL_SDK)\\%s" % fname

if __name__ == "__main__": main()