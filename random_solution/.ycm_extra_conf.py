"""
Configuration file for YCM plugin (https://github.com/Valloric/YouCompleteMe)
"""

from os import path
import json
import re

ROOT = path.dirname(path.realpath(__file__))
COMPILATION_DB = path.join(ROOT, 'build/compile_commands.json')

def distance(file_path, candidate_path):
  fd, fn = path.split(file_path)
  cd, cn = path.split(candidate_path)
  cp = path.commonprefix([fd, cd])
  return (fd == cd, fn == cn, cp == cd, -(path.relpath(fd, cp) + path.relpath(cd, cp)).count("/"))

def FlagsForFile(file_path):
  file_path = re.sub(r'\.h(pp)?$', '.cpp', file_path)
  db = json.load(open(COMPILATION_DB))
  command = max((distance(file_path, r['file']), r['command']) for r in db)[1]
  flags = command.split()[1:-1] + \
      ['-x', 'c++',
       '-Wno-error=sign-compare',
       '-Wno-error=sign-conversion']
  return {
    'flags': flags,
    'do_cache': True
  }
