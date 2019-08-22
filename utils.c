#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

char*
expandpath(const char *path)
{
	struct passwd *pw;
	char buf[PATH_MAX+1], *p, *s, *r;
	int l;

	p = strdup(path);
	if(p[0] != '~')
		return p;
	if(p[1] == '/' || p[1] == '\0'){
		pw = getpwuid(getuid());
		if(pw == NULL)
			die("cannot expand path (%s)\n", strerror(errno));
		r = p + 1;
	}else{
		s = strchr(p+1, '/');
		if(s != NULL)
			*s = '\0';
		pw = getpwnam(p+1);
		if(pw == NULL)
			die("cannot expand path (user '%s' not found)\n", p+1);
		if(s){
			*s = '/';
			r = s;
		}
	}
	l = snprintf(buf, PATH_MAX + 1, "%s%s", pw->pw_dir, r);
	if(l < 0)
		die("cannot expand path '%s' (%s)\n", path, strerror(errno));
	return strdup(buf);
}

int
dirfilecount(const char *path)
{
	struct dirent *e;
	DIR *d;
	int c;
	
	c = 0;
	d = opendir(path);
	if(d == NULL)
		die("cannot open directory '%s' (%s)\n", path, strerror(errno));
	for(;;){
		e = readdir(d);
		if(e == NULL)
			break;
		if(e->d_type == DT_REG)
			++c;
	}
	closedir(d);
	return c;
}

