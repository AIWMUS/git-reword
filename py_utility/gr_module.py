"""======IMPORTS======"""

import sys
import os
import subprocess

"""======UTILITY======"""

DEBUG = False

"""======EXEC======"""

def popen_read_light(cmd):
    return os.popen(cmd).read()

def popen_read(cmd, env=None):
    proc = subprocess.Popen(cmd, shell=True, env=env, stdout=subprocess.PIPE)
    out, err = proc.communicate()
    return out.decode("utf-8")

"""======PRINT======"""

def print_nyi(ex=True):
    print("Not yet implemented")
    if ex:
        sys.exit(3)

def print_err(err, ex=True):
    print(err)
    if ex:
        sys.exit(-1)

"""======GIT======"""

def git_set_new_head(id):
    res = popen_read("git reset --hard {}".format(id))
    if DEBUG:
        print(res)

def git_get_commit_data_dict(id):
    commit_data = popen_read("git cat-file -p {}".format(id))
    c_dict = gitm_parse_commit_data(commit_data)
    return c_dict

def git_get_commit_type(id):
    commit_type = popen_read("git cat-file -t {}".format(id))
    commit_type_s = commit_type.split("\n")[0]
    return commit_type_s

def git_get_commit_ancestors(id):
    lst1 = popen_read("git rev-list --ancestry-path {}~..HEAD --parents".format(id)).split("\n")
    ancs = []
    for i in range(len(lst1) - 1, -1, -1):
        if lst1[i] == "":
            continue
        lst2 = lst1[i].split(" ")
        ancs.append({"id": lst2[0], "parent": lst2[1:]})
    return ancs

def git_new_commit(tree_id, msg, parent, author_name, author_email, author_date):
    parent_s = ""
    for i in range(len(parent)):
        parent_s += " -p {}".format(parent[i])
    env = os.environ.copy()
    env.update({"GIT_AUTHOR_NAME": author_name, "GIT_AUTHOR_EMAIL": author_email, "GIT_AUTHOR_DATE": author_date})
    id = popen_read("git commit-tree {} -m \"{}\"{}".format(tree_id, msg, parent_s),
        env=env)
    return id

def git_get_full_id(id):
    return popen_read("git rev-list {}~..{}".format(id, id)).split("\n")[0]

"""======GIT-MY======"""

def gitm_parse_commit_data(commit_data):
    c_d_l = commit_data.split("\n")
    c_dict = {}
    msg_i = -1
    for i in range(len(c_d_l)):
        c_d_l_l = c_d_l[i].split(" ")
        if len(c_d_l_l) < 2:
            msg_i = i
            break
        key, val = c_d_l_l[0], " ".join(c_d_l_l[1:])
        if key not in c_dict:
            c_dict[key] = []
        c_dict[key].append(val)
    c_dict["_msg"] = "\n".join(c_d_l[msg_i+1:-1])
    return c_dict
