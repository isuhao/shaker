import argparse, re, os, uuid, tmplt, gitlib
from datetime import date

def convert(name):
	s1 = re.sub('(.)\s(.)', r'\1_\2', name)
	s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', s1)
	s1 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()
	return re.sub('_+', '_', s1)

def switches():
	parser = argparse.ArgumentParser()
	parser.add_argument("--name", help="Name of the plugin", required=True)
	parser.add_argument("--mime-type", metavar="TYPE", help="MIME type to support. Type should be application private (Should start with \"application/x-\")", required=True)
	parser.add_argument("--descr", help="Plugin description (default: empty)", default="")
	parser.add_argument("--version", help="Plugin version (default: 1.0.0)", default="1.0.0")
	parser.add_argument("--namespace", metavar="CPP-NS", help="Code namespace (default: based on plugin name)")
	parser.add_argument("--target-name", metavar="NAME", help="DLL name (default: same as namespace)")
	parser.add_argument("--instance", metavar="CLASS", help="Class to use for the instance (default: Instance)", default="Instance")
	parser.add_argument("--module", metavar="CLASS", help="Class to use for the module (default: Module)", default="Module")
	parser.add_argument("--solution", action="store_true", help="If present, will create solution with a project")
	parser.add_argument("--solution-name", metavar="SLN", help="Name of the solution (default: same as target)")
	if gitlib.has_git:
		parser.add_argument("--git", action="store_true", help="Adds the project to or creates a new Git repo")

	args = parser.parse_args()
	if args.namespace is None:
		args.namespace = convert(args.name)

	if args.target_name is None:
		args.target_name = args.namespace

	if args.solution and args.solution_name is None:
		args.solution_name = args.target_name

	if args.solution_name is not None:
		args.solution = True

	return args

def make_vars(args):
	vars = {}
	vars['PLUGIN_NAME'] = args.name
	vars['PLUGIN_MIMETYPE'] = args.mime_type
	vars['PLUGIN_DESCRIPTION'] = args.descr
	vars['PLUGIN_VERSION'] = args.version
	vars['PLUGIN_NAMESPACE'] = args.namespace
	vars['PLUGIN_TARGET'] = args.target_name
	vars['INSTANCE_CLASS'] = args.instance
	vars['MODULE_CLASS'] = args.module
	if args.solution:
		vars['SOLUTION'] = args.solution_name
	vars['YEAR'] = date.today().year
	vars['PROJ_GUID'] = str(uuid.uuid1()).upper()
	vars['SHAKER_HOME'] = os.path.dirname(os.path.realpath(__file__))

	return vars

def add_files(repo, files):
	ignore = [".user"]

	if not repo.exists():
		repo.init()

	keys = files.keys()
	keys.sort()
	known = repo.known_files()
	for key in keys:
		if os.path.splitext(files[key])[1] in ignore:
			continue
		if files[key] in known:
			continue

		repo.add(files[key])

def main():
	args = switches()

	t = tmplt.Template(make_vars(args), "default")
	t.install()

	if gitlib.has_git and args.git:
		add_files(gitlib.Repo(t.outdir), t.files)

if __name__ == "__main__": main()