#define PLUGINYEAR		2025
#define PLUGINMAJOR 	1
#define PLUGINMINOR 	0
#define PLUGINREVISION	1
#define PLUGINBUILD		4

#define _PLUGINVERSIONSTR2(x)	#x
#define _PLUGINVERSIONSTR(x)	_PLUGINVERSIONSTR2(x)
#define PLUGINVERSION			_PLUGINVERSIONSTR(PLUGINMAJOR.PLUGINMINOR.PLUGINREVISION.PLUGINBUILD)
