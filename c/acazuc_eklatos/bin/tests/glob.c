#include "tests.h"

#include <sys/stat.h>

#include <fnmatch.h>
#include <wordexp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>

void test_wordexp(void)
{
	wordexp_t we;

	unsetenv("TEST_VAR");
	ASSERT_EQ(wordexp("$TEST_VAR", &we, WRDE_UNDEF), WRDE_BADVAL);
	wordfree(&we);

	ASSERT_EQ(wordexp("\"$TEST_VAR\"", &we, WRDE_UNDEF), WRDE_BADVAL);
	wordfree(&we);

	ASSERT_EQ(wordexp("'$TEST_VAR'", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "$TEST_VAR");
	wordfree(&we);

	ASSERT_EQ(wordexp("a b c", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 3);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "b");
	ASSERT_STR_EQ(we.we_wordv[2], "c");
	wordfree(&we);

	setenv("TEST_VAR", "TEST_VAL", 1);

	ASSERT_EQ(wordexp("a $TEST_VAR c", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 3);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "TEST_VAL");
	ASSERT_STR_EQ(we.we_wordv[2], "c");
	wordfree(&we);

	ASSERT_EQ(wordexp("\"a $TEST_VAR c\"", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "a TEST_VAL c");
	wordfree(&we);

	ASSERT_EQ(wordexp("'a $TEST_VAR c'", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "a $TEST_VAR c");
	wordfree(&we);

	setenv("TEST_VAR", "TEST1 TEST*2", 1);

	ASSERT_EQ(wordexp("a $TEST_VAR c", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 4);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "TEST1");
	ASSERT_STR_EQ(we.we_wordv[2], "TEST*2");
	ASSERT_STR_EQ(we.we_wordv[3], "c");
	wordfree(&we);

	ASSERT_EQ(wordexp("a \"$TEST_VAR\" c", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 3);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "TEST1 TEST*2");
	ASSERT_STR_EQ(we.we_wordv[2], "c");
	wordfree(&we);

	ASSERT_EQ(wordexp("a '$TEST_VAR' c", &we, WRDE_UNDEF), 0);
	ASSERT_EQ(we.we_wordc, 3);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "$TEST_VAR");
	ASSERT_STR_EQ(we.we_wordv[2], "c");
	wordfree(&we);

	setenv("HOME", "/root", 1);

	ASSERT_EQ(wordexp("~", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], getenv("HOME"));
	wordfree(&we);

	ASSERT_EQ(wordexp("~/test", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "/root/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("\"~/test\"", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "~/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("'~/test'", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "~/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("a ~/test", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 2);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "~/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("a /~/test", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 2);
	ASSERT_STR_EQ(we.we_wordv[0], "a");
	ASSERT_STR_EQ(we.we_wordv[1], "/~/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("~user/test", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "/home/user/test");
	wordfree(&we);

	ASSERT_EQ(wordexp("\"\"", &we, 0), 0);
	ASSERT_EQ(we.we_wordc, 1);
	ASSERT_STR_EQ(we.we_wordv[0], "");
	wordfree(&we);

	ASSERT_EQ(wordexp("\"salut", &we, 0), WRDE_SYNTAX);
	wordfree(&we);

	ASSERT_EQ(wordexp("'salut", &we, 0), WRDE_SYNTAX);
	wordfree(&we);

	we.we_offs = 2;
	ASSERT_EQ(wordexp("a b", &we, WRDE_DOOFFS), 0);
	ASSERT_EQ(we.we_wordc, 2);
	ASSERT_NE(we.we_wordv,NULL);
	ASSERT_EQ(we.we_wordv[0], NULL);
	ASSERT_EQ(we.we_wordv[1], NULL);
	ASSERT_STR_EQ(we.we_wordv[2], "a");
	ASSERT_STR_EQ(we.we_wordv[3], "b");
	ASSERT_EQ(we.we_wordv[4], NULL);
	ASSERT_EQ(wordexp("c d", &we, WRDE_APPEND), 0);
	ASSERT_EQ(we.we_wordc, 4);
	ASSERT_NE(we.we_wordv, NULL);
	ASSERT_EQ(we.we_wordv[0], NULL);
	ASSERT_EQ(we.we_wordv[1], NULL);
	ASSERT_STR_EQ(we.we_wordv[2], "a");
	ASSERT_STR_EQ(we.we_wordv[3], "b");
	ASSERT_STR_EQ(we.we_wordv[4], "c");
	ASSERT_STR_EQ(we.we_wordv[5], "d");
	ASSERT_EQ(we.we_wordv[6], NULL);
	wordfree(&we);
}

void test_fnmatch(void)
{
	ASSERT_EQ(fnmatch("test", "test", 0), 0);
	ASSERT_EQ(fnmatch("tset", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t*", "test", 0), 0);
	ASSERT_EQ(fnmatch("u*", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("te*", "test", 0), 0);
	ASSERT_EQ(fnmatch("tet*", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("*", "test", 0), 0);
	ASSERT_EQ(fnmatch("*t", "test", 0), 0);
	ASSERT_EQ(fnmatch("*u", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("*st", "test", 0), 0);
	ASSERT_EQ(fnmatch("*tt", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("te*st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t*t", "test", 0), 0);
	ASSERT_EQ(fnmatch("test*", "test", 0), 0);
	ASSERT_EQ(fnmatch("*test", "test", 0), 0);
	ASSERT_EQ(fnmatch("te?t", "test", 0), 0);
	ASSERT_EQ(fnmatch("te?s", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[e]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[r]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[]]st", "t]st", 0), 0);
	ASSERT_EQ(fnmatch("t[[]st", "t[st", 0), 0);
	ASSERT_EQ(fnmatch("t[]st", "tst", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[!e]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[!f]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[d-f]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[!d-f]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[!f-g]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[e-e]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[!e-e]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:alnum:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:alnum:]]st", "t3st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:alnum:]]st", "t#st", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:alnum:]#]st", "t#st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:alpha:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:alpha:]]st", "t3st", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:blank:]]st", "t st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:blank:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:cntrl:]]st", "t\bst", 0), 0);
	ASSERT_EQ(fnmatch("t[[:cntrl:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:digit:]]st", "t3st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:digit:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "t3st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "t3st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "t#st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:graph:]]st", "t st", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:lower:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:lower:]]st", "tEst", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:print:]]st", "test", 0), 0);
	ASSERT_EQ(fnmatch("t[[:print:]]st", "t\bst", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:punct:]]st", "t.st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:punct:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:space:]]st", "t st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:space:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:upper:]]st", "tEst", 0), 0);
	ASSERT_EQ(fnmatch("t[[:upper:]]st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[[:xdigit:]]st", "t0st", 0), 0);
	ASSERT_EQ(fnmatch("t[[:xdigit:]]st", "tast", 0), 0);
	ASSERT_EQ(fnmatch("t[[:xdigit:]]st", "tAst", 0), 0);
	ASSERT_EQ(fnmatch("t[[:xdigit:]]st", "tgst", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t?st", "t/st", 0), 0);
	ASSERT_EQ(fnmatch("t?st", "t/st", FNM_PATHNAME), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t*st", "t/st", 0), 0);
	ASSERT_EQ(fnmatch("t*st", "t/st", FNM_PATHNAME), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("t[/]st", "t/st", 0), 0);
	ASSERT_EQ(fnmatch("t[/]st", "t/st", FNM_PATHNAME), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("te\\*st", "test", 0), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("te\\*st", "te*st", 0), 0);
	ASSERT_EQ(fnmatch("te\\*st", "te\\st", FNM_NOESCAPE), 0);
	ASSERT_EQ(fnmatch("te\\*st", "te*st", FNM_NOESCAPE), FNM_NOMATCH);
	ASSERT_EQ(fnmatch("*", ".oui", 0), 0);
	ASSERT_EQ(fnmatch("*", ".oui", FNM_PERIOD), FNM_NOMATCH);
	ASSERT_EQ(fnmatch(".*", ".oui", FNM_PERIOD), 0);
	ASSERT_EQ(fnmatch("\\.*", ".oui", FNM_PERIOD), 0);
	ASSERT_EQ(fnmatch("\\.*", ".oui", FNM_PERIOD | FNM_NOESCAPE), FNM_NOESCAPE);
	ASSERT_EQ(fnmatch("*d", "abcded", 0), 0);
}

void test_glob(void)
{
	glob_t gl;

	system("rm -rf /tmp/glob_test");
	ASSERT_EQ(mkdir("/tmp/glob_test", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir100", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir200", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir300", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir100/dir110", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir100/dir120", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir100/dir130", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir200/dir210", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir200/dir220", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir200/dir230", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir300/dir310", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir300/dir320", 0755), 0);
	ASSERT_EQ(mkdir("/tmp/glob_test/dir300/dir330", 0755), 0);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir110/file111", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir110/file112", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir110/file113", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir120/file121", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir120/file122", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir120/file123", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir130/file131", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir130/file132", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir100/dir130/file133", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir210/file211", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir210/file212", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir210/file213", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir220/file221", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir220/file222", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir220/file223", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir230/file231", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir230/file232", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir200/dir230/file233", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir310/file311", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir310/file312", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir310/file313", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir320/file321", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir320/file322", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir320/file323", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir330/file331", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir330/file332", 0644), -1);
	ASSERT_NE(creat("/tmp/glob_test/dir300/dir330/file333", 0644), -1);

	ASSERT_EQ(glob("/tmp/glob_testy", 0, NULL, &gl), GLOB_NOMATCH);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_testy", GLOB_NOCHECK, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 1);
	ASSERT_NE(gl.gl_pathv, NULL);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_testy");
	ASSERT_EQ(gl.gl_pathv[1], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 1);
	ASSERT_NE(gl.gl_pathv, NULL);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test");
	ASSERT_EQ(gl.gl_pathv[1], NULL);
	globfree(&gl);

	gl.gl_offs = 2;
	ASSERT_EQ(glob("/tmp///glob_test", GLOB_DOOFFS, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 1);
	ASSERT_NE(gl.gl_pathv, NULL);
	ASSERT_EQ(gl.gl_pathv[0], NULL);
	ASSERT_EQ(gl.gl_pathv[1], NULL);
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp///glob_test");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	ASSERT_EQ(glob("/tmp//", GLOB_APPEND | GLOB_NOSORT, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 2);
	ASSERT_NE(gl.gl_pathv, NULL);
	ASSERT_EQ(gl.gl_pathv[0], NULL);
	ASSERT_EQ(gl.gl_pathv[1], NULL);
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp///glob_test");
	ASSERT_STR_EQ(gl.gl_pathv[3], "/tmp//");
	ASSERT_EQ(gl.gl_pathv[4], NULL);
	ASSERT_EQ(glob("/tmp//", GLOB_APPEND, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_NE(gl.gl_pathv, NULL);
	ASSERT_EQ(gl.gl_pathv[0], NULL);
	ASSERT_EQ(gl.gl_pathv[1], NULL);
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp//");
	ASSERT_STR_EQ(gl.gl_pathv[3], "/tmp//");
	ASSERT_STR_EQ(gl.gl_pathv[4], "/tmp///glob_test");
	ASSERT_EQ(gl.gl_pathv[5], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/*", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir200");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir300");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/*/", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir200/");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir300/");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/*", GLOB_MARK, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir200/");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir300/");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 2);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir300");
	ASSERT_EQ(gl.gl_pathv[2], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00/???1*", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/dir110");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir100/dir120");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir100/dir130");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00/???1[2]*/*", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/dir120/file121");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir100/dir120/file122");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir100/dir120/file123");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00/???1[2]*/*", GLOB_MARK, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 3);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/dir120/file121");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir100/dir120/file122");
	ASSERT_STR_EQ(gl.gl_pathv[2], "/tmp/glob_test/dir100/dir120/file123");
	ASSERT_EQ(gl.gl_pathv[3], NULL);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00/???1[2]*/*/", 0, NULL, &gl), GLOB_NOMATCH);
	globfree(&gl);

	ASSERT_EQ(glob("/tmp/glob_test/d?r[13]00/dir?2*/*3", 0, NULL, &gl), 0);
	ASSERT_EQ(gl.gl_pathc, 2);
	ASSERT_STR_EQ(gl.gl_pathv[0], "/tmp/glob_test/dir100/dir120/file123");
	ASSERT_STR_EQ(gl.gl_pathv[1], "/tmp/glob_test/dir300/dir320/file323");
	ASSERT_EQ(gl.gl_pathv[2], NULL);
	globfree(&gl);

	system("rm -rf /tmp/glob_test");
}
