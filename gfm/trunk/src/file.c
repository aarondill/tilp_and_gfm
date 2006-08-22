/*
  Name: Group File Manager
  Copyright (C) 2006 Tyler Cassidy
  Copyright (C) 2006 Romain Lievin
  29/05/06 17:51 - file.c

  This program is free software you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include "tilibs.h"

#ifdef __WIN32__
#include <windows.h>
#include <dirent.h>
#endif

#ifndef __WIN32__
#include <pwd.h>
#include <grp.h>
#endif

#include "dialog.h"
#include "file.h"
#include "folder_tree.h"
#include "gfm.h"
#include "paths.h"

/* File Exist Function */
int file_exists(const char *filename)
{
    FILE *fpointer;

    // Does it exist?
    if ((fpointer = fopen(filename, "r")) == NULL)
        return 0;
    else
        fclose(fpointer);

    return 1;
}

/* Check to see if we have all of our Glade Files */
void glade_files_check(void)
{
    const char *glade_files[] = {
        "dialogs.glade",
        "gfm.glade",
        "manpage.glade",
        "options.glade",
        "pbars.glade",
        "release.glade"
    };
    short i, num_files;
    gchar *msg;
    num_files = 6; // Currently 4 files :)

    // Loop em all up!
    for (i=0; i<num_files; i++)
    {
        if (!file_exists((const char *)gfm_paths_build_glade(glade_files[i])))
        {
            msg = g_strconcat("Could not find: <b>",
                              glade_files[i],
                              "</b> GUI File in '",
                              inst_paths.glade_dir,
                              "' directory!", NULL);
            msgbox_error(msg);
            g_free(msg);
            exit(-1); // Exit
        }
    }

    // Return
    return;
}

/* Clear the List Structures */
static void free_file_info_struct(gpointer data)
{
	FileEntry *fi = data;

	g_free(fi->name);
	g_free(fi);
}

/* Get Directory List Function */
int file_get_dirlist(void)
{
    GDir *dir;
    GError *error;
    G_CONST_RETURN gchar *dirname;
    struct stat f_info;
    FileEntry *fi;

    // Open the Directory
    dir = g_dir_open(settings.cur_dir, 0, &error);

    // Couldnt open Directory
    if (dir == NULL)
    {
        msgbox_error("Could not open the Directory!");
        return -1;
    }

    // Clear the Directory List
    if (ftree_info.dir_list != NULL)
    {
        g_list_foreach(ftree_info.dir_list, (GFunc)free_file_info_struct, NULL);
        g_list_free(ftree_info.dir_list);
        ftree_info.dir_list = NULL;
    }

    // Add the '..' entry
    fi = (FileEntry *)g_malloc0(sizeof(FileEntry));
    fi->name = g_strdup("..");
    if (!stat(fi->name, &f_info))
    {
        fi->date = f_info.st_mtime; // Date
        fi->size = f_info.st_size; // Size
        fi->attrib = f_info.st_mode; // Mode
    }
    ftree_info.dir_list = g_list_prepend(ftree_info.dir_list, (gpointer)fi); // Added to List

    // Add the Rest
    while ((dirname = g_dir_read_name(dir)) != NULL)
    {
        // Valid?
        if (dirname[0] == '.')
            continue;

        // Create Entry
        fi = (FileEntry *)g_malloc0(sizeof(FileEntry));
        fi->name = g_strdup(dirname);
        fi->path = g_strjoin(NULL, settings.cur_dir, G_DIR_SEPARATOR_S, fi->name, NULL);
        if (!g_stat(fi->path, &f_info))
		{
			fi->date = f_info.st_mtime;
			fi->size = f_info.st_size;
			fi->attrib = f_info.st_mode;
		}

		// Add to the List
		ftree_info.dir_list = g_list_prepend(ftree_info.dir_list, (gpointer)fi); // Added to list
    }

    // Close the Directory
    g_dir_close(dir);

    // Return
    return 0; // Good
}

/* Replace invalid characters with _ */
char *file_fix_letters(char *string)
{
    int i, j;
	char tokens[] = "/\\:*?\"<>|- ";

	// Replace
    for (i=0; i<(int)strlen(string); i++) {
		for (j=0; j<(int)strlen(tokens); j++) {
			if (string[i] == tokens[j])
				string[i] = '_'; // Replace
		}
	}

	// Return
	return string;
}

/* Get File Type */
const char *file_get_type(FileEntry *fe)
{
    static char buffer[32];

    // Is a directory?
    if (S_ISDIR(fe->attrib))
      return "";

    // Copy
    strncpy(buffer, tifiles_file_get_type(fe->path), 32);

    // Return
    return buffer;
}

/* Get File Size */
const char *file_get_size(FileEntry *fe)
{
    static char buffer[32];

    // Bytes
    if (fe->size < 1024)
        snprintf(buffer, 32, "  %i", (int)fe->size);

    // Kilobytes
    if ((fe->size >= 1024) && (fe->size < 1048576))
        snprintf(buffer, 32, "  %i KB", (int)(fe->size / 1024));

    // Megabytes
    if (fe->size >= 1048576)
        snprintf(buffer, 32, "  %i MB", (int)(fe->size / 1048576));

    // Return
    return buffer;
}

/* Get File Date */
const char *file_get_date(FileEntry *fe)
{
    static char buffer[32];
    int i;
    char *p;

    // Add
    p = ctime(&(fe->date));
    for (i=4; i<11; i++)
        buffer[i-4] = *(p + i);
    for (i=20; i<24; i++)
        buffer[i-13] = *(p + i);
    buffer[i-13] = '\0';

    // Return
    return buffer;
}

/* g_chdir, changes current directory, only run if less than GLIB 2.8.0 */
#if !GLIB_CHECK_VERSION(2, 8, 0)
#include <errno.h>
#include <direct.h>
// Code taken from Glib
static int g_chdir (const gchar *path)
{
#ifdef __WIN32__
  if (G_WIN32_HAVE_WIDECHAR_API ())
    {
      wchar_t *wpath = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
      int retval;
      int save_errno;

      if (wpath == NULL)
	{
	  errno = EINVAL;
	  return -1;
	}

      retval = _wchdir (wpath);
      save_errno = errno;

      g_free (wpath);

      errno = save_errno;
      return retval;
    }
  else
    {
      gchar *cp_path = g_locale_from_utf8 (path, -1, NULL, NULL, NULL);
      int retval;
      int save_errno;

      if (cp_path == NULL)
	{
	  errno = EINVAL;
	  return -1;
	}

      retval = chdir (cp_path);
      save_errno = errno;

      g_free (cp_path);

      errno = save_errno;
      return retval;
    }
#else
  return chdir (path);
#endif
}
#endif

/* Change Current Working Directory */
int gfm_change_cwd(const char *path)
{
  if (g_chdir(path))
			return -1; // Error
	
	// Return with Success
	return 0;
}

/* Copy File */
int gfm_copy_file(const char *source, const char *destination)
{
	// Does file exist?
	if (file_exists(destination))
	{
		int action;
		gchar *message;
		
		// Make a message
		message = g_strjoin(NULL, "The File '", destination, "' already exists!", NULL);
		action = msgbox_three("Overwrite", "Rename", message);
		
		// Perform Action
		if (action == MSGBOX_NO)
			return 0; // Exit Silently
		if (action == MSGBOX_BUTTON2)
		{
			msgbox_error("Needs to be coded, ~line 331 in file.c");
		}
	}
	
	// Windows Method
#ifdef __WIN32__
	{
	int ret = 0; // Return Variable, 0 is good
	gchar *src_utf8 = g_filename_to_utf8(source, -1, NULL, NULL, NULL); // Source Location in UTF-8
	gchar *dst_utf8 = g_filename_to_utf8(destination, -1, NULL, NULL, NULL); // Dest. in UTF-8

	// UTF-16 System?
	if(G_WIN32_HAVE_WIDECHAR_API())
	{
		gunichar2 *src_utf16 = g_utf8_to_utf16(src_utf8,-1,NULL,NULL,NULL); // UTF8 -> UTF16
		gunichar2 *dst_utf16 = g_utf8_to_utf16(dst_utf8,-1,NULL,NULL,NULL); // UTF8 -> UTF16
		
		// Do we have something?
		if(src_utf16 && dst_utf16)
			if (!CopyFileW(src_utf16, dst_utf16, FALSE)) // Copy
				ret = 1; // Return 1, error!
		
		// Free
		g_free(src_utf16);
		g_free(dst_utf16);
	}
	else
	{
		// Converting back to LOCALE from UTF-8
		gchar *src_loc = g_locale_from_utf8(src_utf8, -1, NULL, NULL, NULL);
		gchar *dst_loc = g_locale_from_utf8(dst_utf8, -1, NULL, NULL, NULL);	
		
		// We have something?
		if(src_loc && dst_loc)
			if (!CopyFile(src_loc, dst_loc, FALSE)) // Copy
				ret = 1; // Return 1, File not copied! Error!
		
		// Free
		g_free(src_loc);
		g_free(dst_loc);
	}
  
	// Free
	g_free(src_utf8);
	g_free(dst_utf8);
  
	// Return hopefully with 0 :)
	return ret;
	}
#else
	FILE *in, *out;
	int c;
	
	// Open Source File
	if ((in = g_fopen(source, "rb")) == NULL) 
		return -1; // Could not Open Source File!

  
	// Open Destination File
	if ((out = g_fopen(destination, "wb")) == NULL) 
		return -2; // Could not Open Destination File!
	
	// Copy the data
	while (!feof(in)) 
	{
		c = fgetc(in);
		if (feof(in))
			break;
		fputc(c, out);
	}
  
	// Close the Files
	fclose(in);
	fclose(out);
  
	// Return with successful copy!
	return 0;
#endif
}

/* Move Files */
int gfm_move_file(const char *source, const char *destination)
{
	// Copy File
	if (gfm_copy_file(source, destination) < 0)
	{
		msgbox_error("Could not move file!");
		return -1; // Bad
	}
	
	// Delete old file
	gfm_delete_file(source);
	
	// Return good
	return 0;
}

/* Delete File */
int gfm_delete_file(const gchar *filename)
{
	// Delete File
	if (g_unlink(filename) < 0)
	{
		msgbox_error("Could not Delete File!");
		return -1; // Bad
	}
	
	// Return
	return 0;
}