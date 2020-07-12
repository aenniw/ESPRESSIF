RS_DIRECTORY = "./data"


def is_target_ffs(targets=[]):
    return "uploadfs" in targets or "buildfs" in targets