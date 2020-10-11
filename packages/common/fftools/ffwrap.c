/*
** Copyright 2017, Felix Schmidt                       
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
**
*/

/* DOC_START
 * This tool is meant to be used as symlink target to provide wrappers
 * for binaries in other root filesystems, so that they are linked 
 * with their native libraries.
 * 
 * Example:
 * /usr/local/lib/tool 
 *  is a symlink to 
 *    .../other_rootfs/usr/bin/ffwrap
 *  This will execute:
 *    .../other_rootfs/usr/bin/tool
 * 
 * If the file .fflibs exists in the link target directory it
 * is used to specify LD_LIBRARY_PATH for execution of tool.
 * Each line is interpreted as:
 *  
 *   [(%|?)tool_name:]*path_spec:path_spec..
 * 
 * If one or more tool-names are given, the subsequent path specs apply for
 * these tools only. If omitted, path specs apply to all tools. The line
 * matching a tool is used, subsequent lines are ignored.
 * The tool_name prefix determines the string match mode: % indicates exact
 * match, ? indicates substring match.
 * A path spec is either relative to the link target directory (beginning
 * with .), or absolute (beginning with /, i.e. it refers to a host filesystem 
 * directory).
 * Each colon-separated item can be a environment variable if starting with $,
 * which will be resolved before further parsing as described.
 * 
 * A global /tmp/.fflibs file is parsed _before_ the one in the tool directory.
 * However, here only exact tool name matches are allowed. It is ment for
 * testing and workarounds.
 * 
 * Additional environment variables are read from:
 *   /tmp/.ffwenv
 * which can contain name=value pairs. value is optional, and itself can be
 * a environment variable if starting with $. It will be resolved before
 * being assigned to name.
 * 
 * execvp() is invoked to run other_rootfs/usr/bin/tool, with tool as argv[0]
 * (no directory part).
 * 
 * Execution parameters affecting ffwrap itself can be given in the
 * _ffw_exec_option environment variable, which is a space separated list of
 * parameters:
 *  -c: Behaves like cash_wrap by setting LD_PRELOAD to target_bin/cash_wrap.so
 *      An existing LD_PRELOAD is saved to CASH_ORG_LD_PRELOAD.
 *  -C: Like 4, sets cash_wrap_autoexec=1 to not invoke the cash shell.
 * 
 * This should not be a global environment variable, it should be prefixed
 * for the specific invocation. For example:
 *   _ffw_exec_option=-c ffdaemon
 * 
 * DOC_END 
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <stdarg.h>


#define BASENAME "ffwrap"
#define NAMEVAR  "_ffw_path"
#define FFWOPT   "_ffw_options"

#ifndef FFW_GLOBAL_DIR
#define FFW_GLOBAL_DIR "/tmp"
#endif

#include "ffwrap-help.c"


static void err (char *msg, ...) 
{
  va_list ap;

  va_start(ap, msg);
  vfprintf (stderr, msg, ap);
  va_end(ap);
  fprintf (stderr, "\n");

  exit (2);
}

static char *crop(char *str)
{
  int i;

  while (*str != 0 && (isblank(*str) || !isprint(*str)))
    str++;

  for (i = strlen(str)-1; i > 0; i--)
  {
    if (isblank (str[i]) || !isprint(str[i]))
      str[i] = 0;
    else
      break;
  }
  return (str);
}

static char* getexename(char* buf, size_t size)
{
    int ret;

    ret = readlink("/proc/self/exe", buf, size);

    /* In case of an error, leave the handling up to the caller */
    if (ret == -1)
        return NULL;

    /* Report insufficient buffer size */
    if (ret >= size)
    {
        return NULL;
    }

    /* Ensure proper NUL termination */
    buf[ret] = 0;

    return buf;
}

static void load_env()
{
  FILE *efd;
  char *line = NULL;
  int rc;
  size_t llen = 0;
  const char *ffenv = FFW_GLOBAL_DIR "/.ffwenv";

  /* add environent data from /tmp/.ffwenv */
  efd = fopen (ffenv, "r");
  if (efd)
  {
    while ((rc = getline(&line, &llen, efd)) != -1)
    {
      char *val = NULL;
      int i;
      char *s = crop(line);
      char *var = s;

      for (i = 0; i < strlen(s); i++)
      {
	      if (s[i] == '=' && val == NULL)
	      {
          s[i] = 0;
          val = &s[i+1];
          i++;
        }
      }

      if (val == NULL)
      	continue;

      var = crop(var);
      if (strlen(var) == 0)
	      continue;

      val = crop(val);
      
      if (*val == '$')
        val = getenv(val+1);

      setenv (var, val, 1);
    }

    if (line)
      free (line);

    fclose (efd);
  }
}

/* Attempt to load and interpret dir/.fflibs file, resolve paths relative to 
 * dir.
 * If fdir is set, load fdir/.fflibs and require exact match mode.
 * Return 1 if a match for toolname has been found and LD_LIBRRY_PATH has been
 * set, 0 if not.
 */
static int load_fflibs(char *toolname, char *dir, char *fdir)
{
  char tmp_str[PATH_MAX+1];
  char resolved_path[PATH_MAX+1];
  char env_val[PATH_MAX+1];
  FILE *efd;
  char *line = NULL;
  int rc = 0;
  size_t llen = 0;
  int exact_match_only = 0;

  if (fdir)
    exact_match_only = 1;
  else
    fdir = dir;

  if (snprintf(tmp_str, PATH_MAX, "%s/.fflibs", fdir) >= PATH_MAX)
    err ("fflibs path name too long");

  efd = fopen (tmp_str, "r");
  if (!efd)
    return 0;

  while ((rc = getline(&line, &llen, efd)) != -1)
  {
    char *s = crop(line);
    int skip_line = 0;
    int matchmode = 0;
    int tool_name_match = 0;
    char *item;
    char *sep = "";

    strcpy (env_val, "");
    while (!skip_line && ((item = strtok (s, ":")) != NULL))
    {
      int l = strlen(env_val);
      s = NULL;

      if (item[0] == '$')
      {
        item = getenv(item+1);
        if (!item)
          continue;
      }

      switch (item[0])
      {
        case '%':
          /* exact tool name match */
          matchmode = 1;
          if (toolname && !strcmp(item+1, toolname))
            tool_name_match = 1;
          break;
        
        case '?':
          if (exact_match_only)
            break;

          /* tool name substring match */
          matchmode = 1;
          if (toolname && strstr(toolname, item+1))
            tool_name_match = 1;
          break;
        
        case '.':
          if (exact_match_only && !matchmode)
          {
            /* tool match required but missing, skip rest of line */
            skip_line = 1;
            break;
          }
          if (matchmode && !tool_name_match)
          {
            /* no tool match, skip rest of line */
            skip_line = 1;
            break;
          }

          if (snprintf (tmp_str, PATH_MAX, "%s/%s", dir, item) >= PATH_MAX)
            err ("path from %s/.fflibs too long", dir);

          if (realpath (tmp_str, resolved_path))
          {
            if (l + strlen(resolved_path) + 1 >= PATH_MAX)
                err ("envname from %s/.fflibs too long", dir);

            strcat (env_val, sep);
            strcat (env_val, resolved_path);
            sep = ":";
          }
          break;
        
        case '/':
          if (exact_match_only && !matchmode)
          {
            /* tool match required but missing, skip rest of line */
            skip_line = 1;
            break;
          }
          if (matchmode && !tool_name_match)
          {
            /* no tool match, skip rest of line */
            skip_line = 1;
            break;
          }
          
          if (l + strlen(item) + 1 >= PATH_MAX)
            err ("envname from %s/.fflibs too long", dir);
              
          strcat (env_val, sep);
          strcat (env_val, item);
          sep = ":";
          break;

        default:
          /* format error, just ignore the whole line */
          skip_line = 1;
          break;
      }
    } /* process line */

    if (!skip_line && env_val[0] != 0)
    {
      /* found match */
      setenv ("LD_LIBRARY_PATH", env_val, 1);
      rc = 1;
      break;
    }
  }

  if (line)
    free (line);

  fclose (efd);
  return rc;
}

static void parse_options(char *exedir)
{
  char *s, *opt;
  struct stat stb;
  char tmp[PATH_MAX+1];

  if ((opt = getenv (FFWOPT)) == NULL)
    return;
  setenv (FFWOPT, "", 1);
  
  while ((s = strtok(opt, "-")) != NULL)
  {
    opt = NULL;
    switch (*s)
    {
      case 'C':
        setenv ("cash_wrap_autoexec", "1", 1);
        /* fall through */
      case 'c':
        if (getenv ("LD_PRELOAD"))
        {
          setenv ("CASH_ORG_LD_PRELOAD", getenv("LD_PRELOAD"), 1);
        }
        if (snprintf (tmp, PATH_MAX, "%s/cash_wrap.so", exedir) >= PATH_MAX)
          err ("path for LD_PRELOAD too long");

        if (stat(tmp, &stb))
        {
          perror (tmp);
          exit(1);
        }
        setenv ("CASH_LIB", tmp, 1);
        setenv ("LD_PRELOAD", tmp, 1);
        break;
    }
  }
}


int main(int argc, char** argv) {
  char exedir[PATH_MAX+1];
  char exepath[PATH_MAX+1];
  char *toolname = NULL;
  char *ffwrap_name = BASENAME;
  int i;

  /* Get basename of argv[0] */
  toolname = argv[0];
  for (i = strlen(argv[0]); i >= 0; i--)
  {
    if (argv[0][i] == '/')
    {
      toolname = &argv[0][i+1];
      break;
    }
  }

  if (!strncmp (toolname, BASENAME, strlen(BASENAME)))
  {
    if (argc > 1 && !strcmp(argv[1], "-h"))
    {
      printf (help);
      exit (0);
    }
    fprintf (stderr, "This tool is meant to be executed as symlink target. See %s -h\n", toolname);
    exit(1);
  }

  /* Full path of this executable
   */
  if (NULL == getexename(exedir, PATH_MAX))
    err ("getexename");

#if 0
  /* prevent recursion */
  if (getenv(NAMEVAR) && !strcmp (getenv(NAMEVAR), exedir))
    err ("ffwrap recursion detected");

  setenv (NAMEVAR, exedir, 1);
#endif

  /* Extract path and basename of the ffwrap executable,
   * construct full path/name of binary to exec later.  */
  for (i = strlen(exedir); i > 0; i--)
  {
    if (exedir[i] == '/')
    {
      ffwrap_name = strdup (&exedir[i+1]);
      exedir[i] = 0;
      sprintf (exepath, "%s/%s", exedir, toolname);
      break;
    }
  }

  if (i == 0)
    err ("exedir");

  if (strncmp (ffwrap_name, BASENAME, strlen(BASENAME)))
    err ("Name of link target must start with " BASENAME);

  /* determine options based on _ffw_exec_option
   */
  parse_options(exedir);

  /* load environment */
  load_env();

  /* set up libs */
  if (!load_fflibs(toolname, exedir, FFW_GLOBAL_DIR))
      load_fflibs(toolname, exedir, NULL);

  /* Execute application with proper environment and argv[0] */
  argv[0] = toolname;

  if (ffwrap_name)
    free (ffwrap_name);

  if (!getenv("_ffw_test_mode"))
  {
    execvp((const char*)exepath, argv);

    /* only reached when execvp returns due to an error */
    perror (exepath);
    exit (1);
  }
  return 0;
}
