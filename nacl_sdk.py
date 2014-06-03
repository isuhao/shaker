import argparse, os, uuid, tmplt

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
			path = os.path.join(self.api, *sub.split("/"))
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

def includes(files, klass, exts):
	out = []
	for name in files:
		if os.path.splitext(name)[1] not in exts: continue
		out.append("<%s Include=\"$(NACL_SDK)\\%s\" />" % (klass, name))
	return "\n    ".join(out);

def filters(list):
	out = []
	for name in list:
		out.append("<Filter Include=\"NaCl SDK\\%s\">\n      <UniqueIdentifier>{%s}</UniqueIdentifier>\n    </Filter>" % (name, uuid.uuid1()))
	return "\n    ".join(out);

def filtered(files, klass, exts):
	out = []
	for name in files:
		if os.path.splitext(name)[1] not in exts: continue
		out.append("<%s Include=\"$(NACL_SDK)\\%s\">\n      <Filter>NaCl SDK\\%s</Filter>\n    </%s>" % (klass, name, os.path.dirname(name), klass))
	return "\n    ".join(out);

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
	api.scan(dirs)
	libpepper = os.path.join(os.path.dirname(__file__), "libpepper")
	
	

	tmplt.FileTemplate({
		"NACL_SDK" : api.api
	}).generate(
		os.path.join(os.path.dirname(__file__), "vstudio", "salt_props.tmplt"),
		os.path.join(os.path.dirname(__file__), "vstudio", "salt.props")
	)
	tmplt.FileTemplate({
		"NACL_INCLUDES" : includes(api.files, "ClInclude", [".h", ".hh", ".hpp"]),
		"NACL_SOURCES" : includes(api.files, "ClCompile", [".c", ".cc", ".cpp"])
	}).generate(
		os.path.join(libpepper, "libpepper_vcxproj.tmplt"),
		os.path.join(libpepper, "libpepper.vcxproj")
	)
	tmplt.FileTemplate({
		"NACL_FILTERS" : filters(api.filters),
		"NACL_FILTERED_INCLUDES" : filtered(api.files, "ClInclude", [".h", ".hh", ".hpp"]),
		"NACL_FILTERED_SOURCES" : filtered(api.files, "ClCompile", [".c", ".cc", ".cpp"])
	}).generate(
		os.path.join(libpepper, "libpepper_vcxproj_filters.tmplt"),
		os.path.join(libpepper, "libpepper.vcxproj.filters")
	)
	#for filter in api.filters: print "NaCl SDK\\%s" % filter
	#for fname in api.files: print "$(NACL_SDK)\\%s" % fname

if __name__ == "__main__": main()