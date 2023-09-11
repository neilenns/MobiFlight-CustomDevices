Import('env')
from os.path import join

env.Append(CPPPATH=[join(env.subst("${PROJECT_DIR}"))])
