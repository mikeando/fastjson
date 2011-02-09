import platform, sys;
import subprocess
import os

# Lets get the version from git
#  first get the base version
git_sha = subprocess.Popen(["git","rev-parse","--short=10","HEAD"], stdout=subprocess.PIPE ).communicate()[0].strip()
p1 = subprocess.Popen(["git", "status"], stdout=subprocess.PIPE )
p2 = subprocess.Popen(["grep", "Changed but not updated\\|Changes to be committed"], stdin=p1.stdout,stdout=subprocess.PIPE)
result = p2.communicate()[0].strip()

if result!="":
  git_sha += "[MOD]"

print "Building version %s"%git_sha

env = Environment()
env.Append( CPPDEFINES={'GITSHAMOD':'"\\"%s\\""'%git_sha} )
env.Append( CPPPATH=['./include'] )

if sys.platform=="linux2" or sys.platform=="linux":
  env.Append( CXXFLAGS='-pthread')
elif sys.platform=="sunos5":
  env.Append( CXXFLAGS='-pthreads')
  env['CXX']='/usr/sfw/bin/g++'
  env.Append( LIBS=['socket','resolv','nsl'])

env.Append( CPPFLAGS=['-g','-Wall'] )

#A library containing several of the objects just to make linking the tests easier

src = Glob('*.cpp')
env.Library('fastjson', src)
