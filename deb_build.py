# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Полная сборка DEB пакета программы nixdbf.
"""

import os
import os.path
import platform

__version__ = (0, 0, 4, 1)
__author__ = 'xhermit'

#Цвета в консоли
RED_COLOR_TEXT      =   '\x1b[31;1m'    # red
GREEN_COLOR_TEXT    =   '\x1b[32m'      # green
YELLOW_COLOR_TEXT   =   '\x1b[33m'      # yellow
BLUE_COLOR_TEXT     =   '\x1b[34m'      # blue
PURPLE_COLOR_TEXT   =   '\x1b[35m'      # purple
CYAN_COLOR_TEXT     =   '\x1b[36m'      # cyan
WHITE_COLOR_TEXT    =   '\x1b[37m'      # white
NORMAL_COLOR_TEXT   =   '\x1b[0m'       # normal

DEFAULT_ENCODING = 'utf-8'


def print_color_txt(sTxt, sColor=NORMAL_COLOR_TEXT):
    txt = sColor + sTxt + NORMAL_COLOR_TEXT
    print(txt)

def getPlatform():
    """
    Get platform name.
    """
    return platform.uname()[0].lower()

def isWindowsPlatform():
    return getPlatform() == 'windows'


def isLinuxPlatform():
    return getPlatform() == 'linux'

def getOSVersion():
    """
    Get OS version.
    """
    try:
        if isLinuxPlatform():
            import distro
            return distro.linux_distribution()
        elif isWindowsPlatform():
            try:
                cmd = 'wmic os get Caption'
                p = subprocess.Popen(cmd.split(' '), stdout=subprocess.PIPE)
            except FileNotFoundError:
                print_color_txt('WMIC.exe was not found. Make sure \'C:\Windows\System32\wbem\' is added to PATH', RED_COLOR_TEXT)
                return None

            stdout, stderror = p.communicate()

            output = stdout.decode('UTF-8', 'ignore')
            lines = output.split('\r\r')
            lines = [line.replace('\n', '').replace('  ', '') for line in lines if len(line) > 2]
            return lines[-1]
    except:
        print_color_txt(u'Error get OS version', RED_COLOR_TEXT)
        raise
    return None

def getPlatformKernel():
    """
    Get kernel.
    """
    try:
        return platform.release()
    except:
        print_color_txt(u'Error get platform kernel', RED_COLOR_TEXT)
        raise
    return None


def getCPUSpec():
    """
    Get CPU specification.
    """
    try:
        return platform.processor()
    except:
        print_color_txt(u'Error get CPU specification', RED_COLOR_TEXT)
        raise
    return None

def is64Linux():
    """
    Определить разрядность Linux.
    @return: True - 64 разрядная ОС Linux. False - нет.
    """
    cpu_spec = getCPUSpec()
    return cpu_spec == 'x86_64'

PACKAGENAME='nixdbf'
PACKAGE_VERSION='1.1'
LINUX_VERSION='-'.join([str(x).lower() for x in getOSVersion()[:-1]])
LINUX_PLATFORM = 'amd64' if is64Linux() else 'i386'
COPYRIGHT='<xhermitone@gmail.com>'
DESCRIPTION='The Linux DBF file command-line control utility'

DEBIAN_CONTROL_FILENAME = './deb/DEBIAN/control'
DEBIAN_CONTROL_BODY = '''Package: %s
Version: %s
Architecture: %s
Maintainer: %s
Depends: 
Section: contrib/otherosfs
Priority: optional
Description: %s 
''' % (PACKAGENAME, PACKAGE_VERSION, LINUX_PLATFORM, COPYRIGHT, DESCRIPTION)


def sys_cmd(sCmd):
    """
    Выполнить комманду ОС.
    """
    print_color_txt('System command: <%s>' % sCmd, GREEN_COLOR_TEXT)
    os.system(sCmd)

def compile_and_link():
    """
    Компиляция и сборка.
    """
    if not os.path.exists('./obj'):
        os.makedirs('./obj')
    if not os.path.exists('./lib'):
        os.makedirs('./lib')
    if not os.path.exists('./include'):
        os.makedirs('./include')

    sys_cmd('make clean')
    sys_cmd('make')

def build_deb():
    """
    Сборка пакета.
    """
    if not os.path.exists('./deb/DEBIAN'):
        os.makedirs('./deb/DEBIAN')
    if not os.path.exists('./deb/usr/lib'):
        os.makedirs('./deb/usr/lib')
    if not os.path.exists('./deb/usr/include'):
        os.makedirs('./deb/usr/include')
    if not os.path.exists('./deb/usr/bin'):
        os.makedirs('./deb/usr/bin')

    # Прописать файл control
    try:
        control_file = None 
        control_file = open(DEBIAN_CONTROL_FILENAME, 'w')
        control_file.write(DEBIAN_CONTROL_BODY)
        control_file.close()
        control_file = None        
        print_color_txt('Save file <%s>' % DEBIAN_CONTROL_FILENAME, GREEN_COLOR_TEXT)
    except:
        if control_file:
           control_file.close()
           control_file = None 
        print_color_txt('ERROR! Write control', RED_COLOR_TEXT)
        raise
        
    if os.path.exists('./include/nixdbf.h'):
        sys_cmd('cp ./include/nixdbf.h ./deb/usr/include')
    else:
        print_color_txt('File <./include/nixdbf.h> not found', RED_COLOR_TEXT)
    if os.path.exists('./lib/libnixdbf.so'):
        sys_cmd('cp ./lib/libnixdbf.so ./deb/usr/lib')
    else:
        print_color_txt('File <./lib/libnixdbf.so> not found', RED_COLOR_TEXT)
    if os.path.exists('nixdbf'):
        # Копировать в папку сборки файл программы
        sys_cmd('cp ./nixdbf ./deb/usr/bin')

        sys_cmd('fakeroot dpkg-deb --build deb')

        if os.path.exists('./deb.deb'):
            deb_filename = '%s-%s-%s.%s.deb' % (PACKAGENAME, PACKAGE_VERSION, LINUX_VERSION, LINUX_PLATFORM)
            sys_cmd('mv ./deb.deb ./%s' % deb_filename)
        else:
            print_color_txt('ERROR! DEB build error', RED_COLOR_TEXT)
    else:
        print_color_txt('ERROR! Compile error. File <nixdbf> not found', RED_COLOR_TEXT)


def build():
    """
    Запуск полной сборки.
    """
    import time

    start_time = time.time()
    # print_color_txt(__doc__,CYAN_COLOR_TEXT)
    compile_and_link()
    build_deb()
    sys_cmd('ls *.deb')
    print_color_txt(__doc__, CYAN_COLOR_TEXT)
    print_color_txt('Time: <%d>' % (time.time()-start_time), BLUE_COLOR_TEXT)


if __name__=='__main__':
    build()
