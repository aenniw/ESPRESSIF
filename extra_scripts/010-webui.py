import os
import requests
import json
import shutil
from common import *

Import("env")

UI_RELEASES = "https://api.github.com/repos/aenniw/esp-webui/releases/latest"
UI_DIRECTORY = ".pio/esp-webui"
RSUI_DIRECTORY = RS_DIRECTORY + "/www"


def download(url, directory):
    file_name = '%s/%s' % (directory, url.split('/')[-1:][0])
    if not os.path.isfile(file_name):
        print("Downloading %s" % url)
        with open(file_name, 'wb') as file:
            file.write(requests.get(url).content)


if is_target_ffs(BUILD_TARGETS):
    if not os.path.isdir(UI_DIRECTORY):
        os.mkdir(UI_DIRECTORY)
    releases = requests.get(UI_RELEASES).content.decode('utf-8')
    for asset_url in [
            x['browser_download_url'] for x in json.loads(releases)['assets']
    ]:
        download(asset_url, UI_DIRECTORY)

    if not os.path.isdir(RSUI_DIRECTORY):
        os.mkdir(RSUI_DIRECTORY)

    for file in os.listdir(UI_DIRECTORY):
        shutil.copyfile("%s/%s" % (UI_DIRECTORY, file),
                        "%s/%s" % (RSUI_DIRECTORY, file))
