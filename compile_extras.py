#Script used for setting config and minifying web files before loading them onto spiffs.

Import("env")
env.Execute("$PYTHONEXE -m pip install css-html-js-minify rjsmin")

import shutil
import os

try:
    from css_html_js_minify import process_single_html_file, process_single_css_file
except ImportError:
    env.Execute("$PYTHONEXE -m pip install css-html-js-minify")
try:
    import rjsmin
except ImportError:
    env.Execute("$PYTHONEXE -m pip install rjsmin")


config_present = os.path.isfile('lib/config/config.h')
dir_present = os.path.isdir('lib/config')
minified_data = 'data_minified'
regular_data = 'data'

if config_present:
    print("Config file found, proceeding...")
else:
    print("Config file not found, using config_default.h ...")
    if not dir_present:
        os.mkdir('lib/config')
        shutil.copy2('config_default.h', 'lib/config/config.h')
    else:
        shutil.copy2('config_default.h', 'lib/config/config.h')


print("Minifying data files....")

if not os.path.isdir(minified_data):
    os.mkdir(minified_data)
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
    if os.path.isfile(file_path):
        if "favicon" in filename:
            shutil.copy2(file_path, minified_path)
        elif ".min" in filename:
            shutil.copy2(file_path, minified_path)
        else:
            if ".js" in filename:
                jsFile = open(file_path, "r")
                data = jsFile.read()
                jsFile.close()
                minified = rjsmin.jsmin(data)
                output = open(minified_path, "w")
                output.write(minified)
                output.close()
                print("Minified %s" % minified_path)
            elif ".html" in filename:
                process_single_html_file(file_path, output_path=minified_path)
            elif ".css" in filename:
                process_single_css_file(file_path, output_path=minified_path)

