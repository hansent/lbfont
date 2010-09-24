import sys
env = Environment()

## setup build ENV #####################################
env.Append( 
  CPPPATH = ['freetype-2.4.2/include'],
  LIBPATH = ['freetype-2.4.2/objs/.libs'],
  LIBS    = ['z']
)
  
if sys.platform == 'darwin':
	env.Append(FRAMEWORKS= ['OpenGL','GLUT'])
else:
	env.Append(LIBS=['gl','glut'])

	
	
## TARGEST #############################################
lbfont = env.Library(['lbfont.c'])
env.Program( ['examples/example.c', lbfont, 'freetype-2.4.2/objs/.libs/libfreetype.a'])



## build args ###################################
build_mode = ARGUMENTS.get('mode', 'release')
print "build mode", build_mode
if build_mode == 'debug':
	env.Append(CCFLAGS = '-g')





