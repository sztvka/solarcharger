import shutil
import os

config_present = os.path.isfile('lib/config/config.h')

if config_present:
    print("Config file found, proceeding...")
else:
    print("Config file not found, using config_default.h ...")
    shutil.copy2('config_default.h', 'lib/config/config.h')


print("Minifying data files....")
minified_data = 'data_minified'
regular_data = 'data'
for filename in os.listdir(minified_data):
    file_path = os.path.join(minified_data, filename)
    try:
        if os.path.isfile(file_path) or os.path.islink(file_path):
            os.unlink(file_path)
        elif os.path.isdir(file_path):
            shutil.rmtree(file_path)
    except Exception as e:
        print('Failed to delete minified file %s. Reason: %s' % (file_path, e))


for filename in os.listdir(regular_data):
    file_path = os.path.join(regular_data, filename)
    minified_path = os.path.join(minified_data, filename)
    if "favicon" in filename:
        shutil.copy2(file_path, minified_path)
    else:
        if os.name == 'nt':
            runstring = ".\minify " + file_path + " -o " + minified_path
            os.system(runstring)
        else:
            runstring = "./minify " + file_path + " -o " + minified_path
            os.system(runstring)

