#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Utilities for MODGUI X11UI, based on Carla code
# Copyright (C) 2015 Filipe Coelho <falktx@falktx.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# For a full copy of the GNU General Public License see the doc/GPL.txt file.

# ------------------------------------------------------------------------------------------------------------
# Imports (Global)

from ctypes import *
from sys import argv, platform

# ------------------------------------------------------------------------------------------------------------
# Set Platform

if platform == "darwin":
    HAIKU   = False
    LINUX   = False
    MACOS   = True
    WINDOWS = False
elif "haiku" in platform:
    HAIKU   = True
    LINUX   = False
    MACOS   = False
    WINDOWS = False
elif "linux" in platform:
    HAIKU   = False
    LINUX   = True
    MACOS   = False
    WINDOWS = False
elif platform in ("win32", "win64", "cygwin"):
    HAIKU   = False
    LINUX   = False
    MACOS   = False
    WINDOWS = True
else:
    HAIKU   = False
    LINUX   = False
    MACOS   = False
    WINDOWS = False

# ------------------------------------------------------------------------------------------------------------
# Convert a ctypes c_char_p into a python string

def charPtrToString(charPtr):
    if not charPtr:
        return ""
    if isinstance(charPtr, str):
        return charPtr
    return charPtr.decode("utf-8", errors="ignore")

# ------------------------------------------------------------------------------------------------------------
# Convert a ctypes POINTER(c_char_p) into a python string list

def charPtrPtrToStringList(charPtrPtr):
    if not charPtrPtr:
        return []

    i       = 0
    charPtr = charPtrPtr[0]
    strList = []

    while charPtr:
        strList.append(charPtr.decode("utf-8", errors="ignore"))

        i += 1
        charPtr = charPtrPtr[i]

    return strList

# ------------------------------------------------------------------------------------------------------------
# Convert a ctypes POINTER(c_<num>) into a python number list

def numPtrToList(numPtr):
    if not numPtr:
        return []

    i       = 0
    num     = numPtr[0] #.value
    numList = []

    while num not in (0, 0.0):
        numList.append(num)

        i += 1
        num = numPtr[i] #.value

    return numList

# ------------------------------------------------------------------------------------------------------------
# Convert a ctypes value into a python one

c_int_types    = (c_int, c_int8, c_int16, c_int32, c_int64, c_uint, c_uint8, c_uint16, c_uint32, c_uint64, c_long, c_longlong)
c_float_types  = (c_float, c_double, c_longdouble)
c_intp_types   = tuple(POINTER(i) for i in c_int_types)
c_floatp_types = tuple(POINTER(i) for i in c_float_types)

def toPythonType(value, attr):
    if isinstance(value, (bool, int, float)):
        return value
    if isinstance(value, bytes):
        return charPtrToString(value)
    if isinstance(value, c_intp_types) or isinstance(value, c_floatp_types):
        return numPtrToList(value)
    if isinstance(value, POINTER(c_char_p)):
        return charPtrPtrToStringList(value)
    print("..............", attr, ".....................", value, ":", type(value))
    return value

# ------------------------------------------------------------------------------------------------------------
# Convert a ctypes struct into a python dict

def structToDict(struct):
    return dict((attr, toPythonType(getattr(struct, attr), attr)) for attr, value in struct._fields_)

# ------------------------------------------------------------------------------------------------------------
# Carla Utils API (C stuff)

CarlaPipeClientHandle = c_void_p
CarlaPipeCallbackFunc = CFUNCTYPE(None, c_void_p, c_char_p)

# ------------------------------------------------------------------------------------------------------------
# Carla Utils object using a DLL

class CarlaUtils(object):
    def __init__(self, filename):
        object.__init__(self)

        self.lib = cdll.LoadLibrary(filename)

        self.lib.carla_set_process_name.argtypes = [c_char_p]
        self.lib.carla_set_process_name.restype = None

        self.lib.carla_pipe_client_new.argtypes = [POINTER(c_char_p), CarlaPipeCallbackFunc, c_void_p]
        self.lib.carla_pipe_client_new.restype = CarlaPipeClientHandle

        self.lib.carla_pipe_client_idle.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_idle.restype = None

        self.lib.carla_pipe_client_is_running.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_is_running.restype = c_bool

        self.lib.carla_pipe_client_lock.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_lock.restype = None

        self.lib.carla_pipe_client_unlock.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_unlock.restype = None

        self.lib.carla_pipe_client_readlineblock.argtypes = [CarlaPipeClientHandle, c_uint]
        self.lib.carla_pipe_client_readlineblock.restype = c_char_p

        self.lib.carla_pipe_client_write_msg.argtypes = [CarlaPipeClientHandle, c_char_p]
        self.lib.carla_pipe_client_write_msg.restype = c_bool

        self.lib.carla_pipe_client_write_and_fix_msg.argtypes = [CarlaPipeClientHandle, c_char_p]
        self.lib.carla_pipe_client_write_and_fix_msg.restype = c_bool

        self.lib.carla_pipe_client_flush.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_flush.restype = c_bool

        self.lib.carla_pipe_client_flush_and_unlock.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_flush_and_unlock.restype = c_bool

        self.lib.carla_pipe_client_destroy.argtypes = [CarlaPipeClientHandle]
        self.lib.carla_pipe_client_destroy.restype = None

    # --------------------------------------------------------------------------------------------------------

    def set_process_name(self, name):
        self.lib.carla_set_process_name(name.encode("utf-8"))

    def pipe_client_new(self, func):
        argc      = len(argv)
        cagrvtype = c_char_p * argc
        cargv     = cagrvtype()

        for i in range(argc):
            cargv[i] = c_char_p(argv[i].encode("utf-8"))

        self._pipeClientCallback = CarlaPipeCallbackFunc(func)
        return self.lib.carla_pipe_client_new(cargv, self._pipeClientCallback, None)

    def pipe_client_idle(self, handle):
        self.lib.carla_pipe_client_idle(handle)

    def pipe_client_is_running(self, handle):
        return bool(self.lib.carla_pipe_client_is_running(handle))

    def pipe_client_lock(self, handle):
        self.lib.carla_pipe_client_lock(handle)

    def pipe_client_unlock(self, handle):
        self.lib.carla_pipe_client_unlock(handle)

    def pipe_client_readlineblock(self, handle, timeout):
        return charPtrToString(self.lib.carla_pipe_client_readlineblock(handle, timeout))

    def pipe_client_write_msg(self, handle, msg):
        return bool(self.lib.carla_pipe_client_write_msg(handle, msg.encode("utf-8")))

    def pipe_client_write_and_fix_msg(self, handle, msg):
        return bool(self.lib.carla_pipe_client_write_and_fix_msg(handle, msg.encode("utf-8")))

    def pipe_client_flush(self, handle):
        return bool(self.lib.carla_pipe_client_flush(handle))

    def pipe_client_flush_and_unlock(self, handle):
        return bool(self.lib.carla_pipe_client_flush_and_unlock(handle))

    def pipe_client_destroy(self, handle):
        self.lib.carla_pipe_client_destroy(handle)

# ------------------------------------------------------------------------------------------------------------
