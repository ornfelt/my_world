void *_dl_open(const char *filenae, int flags);
int _dl_close(void *handle);
char *_dl_error(void);
void *_dl_sym(void *handle, const char *symbol);

void *dlopen(const char *filename, int flags)
{
	return _dl_open(filename, flags);
}

int dlclose(void *handle)
{
	return _dl_close(handle);
}

char *dlerror(void)
{
	return _dl_error();
}

void *dlsym(void *handle, const char *symbol)
{
	return _dl_sym(handle, symbol);
}
