#! /usr/bin/env python3

# Copyright (c) 2013 Blaze <blaze@jabster.pl>
# Licensed under the GNU General Public License, version 3 or later.
# See the file http://www.gnu.org/copyleft/gpl.txt.

import os
import re
import subprocess
import notify2

def find_year(path):
    try:
        return re.split('([12]{1}[09]{1}\d{2})', path, 1)[1]
    except IndexError:
        return 'none'

def cover_path(path):
    entries = os.listdir(path)
    for entry in entries:
        if re.match('([^\s]+(?=\.(jpg|jpeg|png))\.)', entry):
            return path + '/' + entry
        else:
            return ''

def main():
    output = lambda x : subprocess.getoutput(x)
    filename = output('mocp -Q %file')
    if not filename.find('/') == 0:
        os._exit(0) #radio stream is playing
    artist = output('mocp -Q %artist')
    album = output('mocp -Q %album')
    tail = output("mocp -Q '%song (%tt)'")
    path = filename.rsplit('/', 1)[0]
    year = find_year(path)
    cover = cover_path(path)
    if not notify2.init('moc'):
        os._exit(0) #notify2 is out of reach

    ni = notify2.Notification('music on console', '%s <i>%s</i> %s<br /><b>%s</b>' % (artist, year, album, tail), cover)
    ni.timeout = 5000
    if not ni.show():
        os._exit(1) #notify2 is not operational

if __name__ == '__main__':
    main()
