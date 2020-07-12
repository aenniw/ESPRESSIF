import os
import shutil
from common import *

Import("env")

if is_target_ffs(BUILD_TARGETS):
    if os.path.exists(RS_DIRECTORY):
        shutil.rmtree(RS_DIRECTORY)
    os.mkdir(RS_DIRECTORY)
