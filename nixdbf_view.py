# !/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
NixDBF viewer.

Command line parameters:

        python3 nixdbf_view.py [Launch parameters]

Launch parameters:

    [Help and debugging]
        --help|-h|-?        Print help lines
        --version|-v        Print version of the program

    [Options]
        --filename=         DBF filename
"""

import sys
import os
import os.path
import getopt
import locale
import subprocess
import time

try:
    import rich.console
    import rich.table
    import rich.live
    import rich.align
except ImportError:
    print('Error import rich. For install execute <pip3 install rich>')
    sys.exit(2)

__version__ = (0, 0, 0, 1)

NIXDBF_FIELDS_FMT = 'nixdbf --dbf="%s" --cmd=FIELDS'
NIXDBF_RECORD_COUNT_FMT = 'nixdbf --dbf="%s" --cmd=LENGTH'
NIXDBF_RECORDS_FMT = 'nixdbf --dbf="%s" --cmd=SELECT'
NIXDBF_FIELD_DELIMETER = '|'

# Объект консоли
CONSOLE = rich.console.Console()


def getCommandLines(cmd):
    """
    Получить список строк результата выполнения команды.

    :param cmd: Команда ОС.
    """
    try:
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        console_encoding = locale.getpreferredencoding()
        stdout, stderror = process.communicate()
        output = stdout.decode(console_encoding, 'ignore')
        lines = output.split(os.linesep)
        lines = [line.strip() for line in lines if line.strip()]
        return lines
    except:
        print('Error get subprocess lines. Command <%s>' % cmd)
        raise
    return list()


def getDBFTitle(dbf_filename, cur_record_idx=0):
    """
    Определить заголовок DBF файла для отображения.
    
    :param dbf_filename: Полное наименование DBF файла.
    :param cur_record_idx: Индекс текущей строки.
    """
    dbf_basename = os.path.basename(dbf_filename)
    title = dbf_basename + ' ' * (CONSOLE.size.width - len(dbf_basename))
    return title

    
def getDBFFields(dbf_filename):
    """
    Определить список колонок/полей DBF файла.
    
    :param dbf_filename: Полное наименование DBF файла.
    """
    cmd = NIXDBF_FIELDS_FMT % dbf_filename
    lines = getCommandLines(cmd)
    lines = [line.split(NIXDBF_FIELD_DELIMETER) for line in lines]
    fields = [dict(name=line[0], type=line[1], length=line[2], decimal=line[3]) for line in lines]
    return fields
    

def getDBFRecords(dbf_filename, fields=None):
    """
    Получить список строк DBF файла.

    :param dbf_filename: Полное наименование DBF файла.
    :param fields: Список полей. Если не определен, то определяется по функции getDBFFields.
    """
    if fields is None:
        fields = getDBFFields(dbf_filename)

    cmd = NIXDBF_RECORDS_FMT % dbf_filename
    lines = getCommandLines(cmd)
    rows = [line.split(NIXDBF_FIELD_DELIMETER) for line in lines]
    records = [dict([(field['name'], row[i].strip()) for i, field in enumerate(fields)]) for row in rows]
    return records


def createTable(table, dbf_filename, fields=None):
    """
    """
    if fields is None:
        fields = getDBFFields(dbf_filename)

    for field in fields:
        table.add_column(field.get('name', 'Unknown'),  justify='left', no_wrap=True)

    # rows = CONSOLE.size.height - 2

    records = getDBFRecords(dbf_filename, fields=fields)
    for record in records:
        row = [record[field['name']] for field in fields]
        table.add_row(*row)

    return table


def viewDBFTable(dbf_filename):
    """
    Просмотр DBF файла в табличном представлении.
    
    :param dbf_filename: Полное наименование DBF файла.
    """
    table = rich.table.Table(show_footer=False)
    # table_centered = rich.align.Align.center(table)

    title = getDBFTitle(dbf_filename)
    table.title = title

    fields = getDBFFields(dbf_filename)

    CONSOLE.clear()

    with rich.live.Live(table, console=CONSOLE, screen=False, refresh_per_second=4) as live:

        for i in range(40):
            time.sleep(0.4)
            live.update(createTable(table, dbf_filename, fields))

    # CONSOLE.print(table, justify="center")

    
def main(*argv):
    """
    Основная запускаемая функция.

    :param argv: Параметры коммандной строки.
    """
    # Parse command line arguments
    try:
        options, args = getopt.getopt(argv, 'h?v',
                                      ['help', 'version', 'filename='])
    except getopt.error as msg:
        print(str(msg))
        print(__doc__)
        sys.exit(2)

    dbf_filename = None
    for option, arg in options:
        if option in ('-h', '--help', '-?'):
            print(__doc__)
            sys.exit(0)
        elif option in ('-v', '--version'):
            str_version = 'NixDBF viewer %s' % '.'.join([str(sign) for sign in __version__])
            print(str_version)
            sys.exit(0)
        elif option in ('--filename',):
            dbf_filename = arg
    
    if dbf_filename:
        viewDBFTable(dbf_filename)


if __name__ == '__main__':
    main(*sys.argv[1:])

