import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--name", help="Name of the plugin", required=True)
parser.add_argument("--mime-type", metavar="TYPE", help="MIME type to support. Type should be application private (Should start with \"application/x-\")", required=True)
parser.add_argument("--descr", help="Plugin description (default: empty)", default="")
parser.add_argument("--version", help="Plugin version (default: 1.0.0)", default="1.0.0")
parser.add_argument("--namespace", metavar="CPP-NS", help="Code namespace (default: based on plugin name)")
parser.add_argument("--target-name", metavar="NAME", help="DLL name (default: same as namespace)")
parser.add_argument("--instance", metavar="CLASS", help="Class to use for the instance (default: Instance)", default="Instance")
parser.add_argument("--module", metavar="CLASS", help="Class to use for the module (default: Module)", default="Module")
args = parser.parse_args()
print(args)

#PLUGIN_NAME == ?
#PLUGIN_MIMETYPE = ?
#PLUGIN_DESCRIPTION == ""
#PLUGIN_VERSION = 1.0.0
#PLUGIN_NAMESPACE == PLUGIN_NAME -> camel to gtk
#PLUGIN_TARGET == PLUGIN_NAMESPACE
#INSTANCE_CLASS == Instance
#MODULE_CLASS == Module
#YEAR == today().year
#PROJ_GUID = generate
#SHAKER_HOME = cwd
