# !/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Полная сборка DEB пакета программы nixdbf.
"""

import os
import os.path

__version__ = (0, 0, 2, 1)
__author__ = 'xhermit'

PACKAGENAME='nixdbf'
PACKAGE_VERSION='0.7'
LINUX_VERSION='ubuntu16.04'
LINUX_PLATFORM='i386'
COPYRIGHT='<xhermitone@gmail.com>'
DESCRIPTION='The Linux DBF file command-line control utility'

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


def print_color_txt(sTxt, sColor=NORMAL_COLOR_TEXT):
    if type(sTxt) == unicode:
        sTxt = sTxt.encode('DEFAULT_ENCODING')
    txt = sColor + sTxt + NORMAL_COLOR_TEXT
    print(txt)

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
    sys_cmd('make clean')
    sys_cmd('make')

def build_deb():
    """
    Сборка пакета.
    """
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
        print_color_txt('ERROR! Compile error', RED_COLOR_TEXT)


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
