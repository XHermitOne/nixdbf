# !/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import os.path
import shutil
import time


def do_tst(cmd):
    print('Start command: <%s>' % cmd)
    start_time = time.time()
    cmd_list = cmd.split(' ')
    os.spawnv(os.P_WAIT, cmd_list[0], cmd_list)
    print('Stop command: <%s> Time = %d' % (cmd, time.time() - start_time))


commands=(
          # './nixdbf --debug --dbf=./tst/SHIPPING.DBF',
          #'./nixdbf --debug --dbf=./tst/ADDROBJ.DBF',
          #'./nixdbf --debug --dbf=./tst/ADDROBJ.DBF --start_rec=1000000 --limit=10',
          # './nixdbf --debug --dbf=./tst/SB1111.DBF --cmd=FIELDS',
          #'./nixdbf --debug --dbf=./tst/SHIPPING.DBF --src_codepage=CP866 --dst_codepage=UTF-8',
          # './nixdbf --debug --dbf=./tst/SHIPPING.DBF --cmd=SELECT',
          './nixdbf --debug --dbf=./tst/kladr.dbf --cmd=LENGTH',
          )


def test():
    print('NixDBF test start .. ok')    

    for i, command in enumerate(commands):
            
        do_tst(command)
        
    print('NixDBF test stop .. ok')

    
if __name__=='__main__':
    test()
