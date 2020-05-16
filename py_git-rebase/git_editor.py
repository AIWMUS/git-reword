#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""======HEAD======"""

import os
import sys

path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.abspath(path + "/../py_utility"))
from gr_module import *

"""======---======"""

file_path = sys.argv[-1]
file = open(file_path, "r+")

if sys.argv[2] == "one":
    id = sys.argv[4]
    msg = sys.argv[5]
    
    id = git_get_full_id(id)
    
    if sys.argv[1] == "seq":
        lines = []
        for line in file:
            lines.append(line)
            if line == "\n" or line.startswith("#"):
                continue
            words = line.split(" ")
            if not id.startswith(words[1]):
                continue
            words[0] = "reword"
            lines[-1] = " ".join(words)
        file.seek(0)
        file.truncate()
        file.write("".join(lines))
        file.close()
        if DEBUG:
            print(id)
            print("".join(lines))
    elif sys.argv[1] == "none":
        file.seek(0)
        file.truncate()
        file.write(msg)
        file.close()
    
elif sys.argv[2] == "list":
    lst_path = sys.argv[5]
    lst_file = open(lst_path, "r")
    
    msg_dict = {}
    id_arr = []
    for line in lst_file:
        line = line[:-1]
        if line == "":
            continue
        words = line.split(" ")
        if len(words) != 2:
            print_err("error: list format is wrong")
        id, msg = git_get_full_id(words[0]), words[1]
        msg_dict[id] = msg
        id_arr.append(id)
    
    if sys.argv[1] == "seq":
        lines = []
        k = len(id_arr) - 1
        for line in file:
            lines.append(line)
            if line == "\n" or line.startswith("#"):
                continue
            words = line.split(" ")
            if DEBUG:
                print(str(words))
            if k < 0 or not id_arr[k].startswith(words[1]):
                continue
            k -= 1
            words[0] = "reword"
            lines[-1] = " ".join(words)
        file.seek(0)
        file.truncate()
        file.write("".join(lines))
        file.close()
        
        filek = open(path + "/k", "w")
        filek.write(str(len(id_arr) - 1))
        filek.close()
    elif sys.argv[1] == "none":
        filek = open(path + "/k", "r+")
        k = int(filek.read())
        filek.seek(0)
        filek.truncate()
        filek.write(str(k - 1))
        filek.close()
        if k == 0:
            os.remove(path + "/k")
        
        file.seek(0)
        file.truncate()
        file.write(msg_dict[id_arr[k]])
        file.close()
    
file.close()
