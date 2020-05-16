#! /usr/bin/env python3
# -*- coding: utf-8 -*-

"""======HEAD======"""

import os
import sys

path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.abspath(path + "/../py_utility"))
from gr_module import *

"""======DECOR====="""

def print_usage(ex=True):
    print(
"""Usage: git-reword one (-d | -m) <rev> <new_message>
       git-reword list (-d | -m) (-s | -c | -n) <path_to_list-file>
    
Commands "one" & "list":
    
    -d  (not yet implemented)
        Delete merge-commits on the edited path.
    
    -m
        Save merge-commits on the edited path.

Command "list":
    
    -s (not yet implemented)
        Automatically format the list because it does not satisfy some conditions.
    
    -c (not yet implemented)
        Check that the list satisfies the conditions.
    
    -n
        The list fully meets all the requirements, the user knows what he is doing.

    list-file:

        format:
            <rev_1> <new_message_1>
            <rev_2> <new_message_2>
            ...
            <rev_n> <new_message_n>
        
        The list must satisfy certain conditions, which depend on the version.
"""
    )
    if ex:
        sys.exit(2)

"""======WORK======"""

def work_single(id, msg, del_merge):
    if del_merge:
        print_nyi() #TODO
    
    if git_get_commit_type(id) != "commit":
        print_err("error: input: it's not a commit?")
    
    id = git_get_full_id(id)
    
    workx_line(id, {id: msg}, del_merge)

def work_list(lst_path, sorting, del_merge):
    if sorting != 2 or del_merge:
        print_nyi() #TODO
    
    lst_file = open(lst_path, "rt")
    msg_dict = {}
    last_id = ""
    for line in lst_file:
        line = line[:-1]
        if line == "":
            continue
        words = line.split(" ")
        if len(words) != 2:
            print_err("error: list format is wrong")
        id, msg = git_get_full_id(words[0]), words[1]
        msg_dict[id] = msg
        last_id = id
    
    workx_line(last_id, msg_dict, del_merge)

def workx_line(id, msg_dict, del_merge):
    if DEBUG:
        print(msg_dict)
    
    ancs = git_get_commit_ancestors(id)
    if DEBUG:
        print("acns", ancs)
    
    id_new_dict = {}
    for i in range(len(ancs)):
        id = ancs[i]["id"]
        c_dict = git_get_commit_data_dict(id)
        if DEBUG:
            print("---for---")
            print(i, id)
            print(c_dict)
        
        msg_new = ""
        if id in msg_dict:
            msg_new = msg_dict[id]
        else:
            msg_new = c_dict["_msg"]
        
        parent_new = []
        if i == 0:
            parent_new = c_dict["parent"]
        else:
            parent_new = []
            for j in range(len(c_dict["parent"])):
                id_old = c_dict["parent"][j]
                if id_old in id_new_dict:
                    parent_new.append(id_new_dict[id_old])
                else:
                    parent_new.append(id_old)
        
        author = c_dict["author"][0].split(" ")
        author_name, author_email, author_date = author[0], author[1], " ".join(author[2:4])
        id_new = git_new_commit(c_dict["tree"][0], msg_new, parent_new, author_name, author_email, author_date)
        id_new_dict[id] = id_new
        
        if i == len(ancs) - 1:
            git_set_new_head(id_new)

"""======MAIN======"""

if len(sys.argv) == 5 and sys.argv[1] == "one" and (sys.argv[2] in ["-d", "-m"]):
    print("Start")
    del_merge = sys.argv[2] == "-d"
    id = sys.argv[3]
    msg = sys.argv[4]
    if DEBUG:
        print(id, msg, del_merge)
    work_single(id, msg, del_merge)
    print("OK")
elif len(sys.argv) == 5 and sys.argv[1] == "list" and (sys.argv[2] in ["-d", "-m"]) and (sys.argv[3] in ["-s", "-c", "-n"]):
    print("Start")
    del_merge = sys.argv[2] == "-d"
    sorting = ["-s", "-c", "-n"].index(sys.argv[3])
    lst_path = sys.argv[4]
    if DEBUG:
        print(lst_path, sorting, del_merge)
    work_list(lst_path, sorting, del_merge)
    print("OK")
else:
    print_usage()
