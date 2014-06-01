import argparse

parser = argparse.ArgumentParser()
args = parser.parse_args()
print(args)

#PLUGIN_NAME == ?
#PLUGIN_MIMETYPE = ?
#PLUGIN_TARGET == PLUGIN_NAME -> camel to gtk
#PLUGIN_DESCRIPTION == ""
#PLUGIN_VERSION = 1.0.0
#PLUGIN_NAMESPACE == PLUGIN_TARGET
#INSTANCE_CLASS == Instance
#MODULE_CLASS == Module
#YEAR == today().year
#PROJ_GUID = generate
#SHAKER_HOME = cwd
