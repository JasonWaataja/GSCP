/*  This file is part of GSCP.

    GSCP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GSCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GSCP.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <stdint.h>
#include <stdlib.h>
#include <libssh2.h>
#include <gtk/gtk.h>

#define SSH_DEFAULT_PORT 22

#define SSH_READ_SIZE 32000
#define SSH_WRITE_SIZE 32000

#define FILE_READ_SIZE 4000
#define FILE_WRITE_SIZE 4000

/* A structure that represents the information necessary for connecting to a specific
   ssh server.  Note, it doesn't yet support authentication files.  */
struct _ssh_connection
{
  uint32_t hostaddr;
  unsigned int port;
  int auth_pw;
  //char *fingerprint;
  char *username;
  char *password;
};

typedef struct _ssh_connection ssh_connection;

/* A structure to represent a way to access a specific path which is
   either on the local machine or via an ssh connection.  Note, if
   on_lhost is true, the connection should probably be NULL.  */
struct _ssh_path_info
{
  /* boolean is on local host */
  int on_lhost;
  char *path;
  ssh_connection *con;
};

typedef struct _ssh_path_info ssh_path_info;

/* Allocate a new ssh_connection with the given username and password.  The important
   thing is that it uses malloc to allocate both the structure itself and the memory
   used to store the username and password.  It is up to the caller to free it as well
   as the username and password, or call ssh_connection_free ().  */
ssh_connection *ssh_connection_new (const char *username, const char *password);
/* Frees the username, password, and ssh_connection given.  This is best used for
   freeing a pointer allocated with ssh_connection_new ().  */
void ssh_connection_free (ssh_connection *con);

/* Executes the code for connecting to a session given by con and stores it in session.
   the return value can be interpreted as a boolean representing whether or not the
   connection was successful.  */
int ssh_connection_session_open (ssh_connection *con, int *sock, LIBSSH2_SESSION **session);
/* Executes the code for closing a session in session.
   the return value can be interpreted as a boolean representing whether or not the
   connection was successful.  */
int ssh_connection_session_close (LIBSSH2_SESSION **session, int sock, const char *message);
/* Sends the eof signal to channel that is assumed is being written to.  This is usually
   called right before closing the stream.  */
int ssh_channel_send_send_eof (LIBSSH2_CHANNEL *channel);

/* Makes sure that info is a valid path, althougth it doesn't check if the file
   actually exists.  */
int is_valid_ssh_path (ssh_path_info *info);
/* Makes sure that the connection is not NULL and that it is valid.  */
int is_valid_ssh_connection (ssh_connection *con);

/* Closes the given socket.  The reason that this is a function is that the code
   is different for UNIX as it is for Windows, so it executes the correct
   procedure.  */
void close_socket (int socket);
//void close_libssh2_session(LIBSSH2_SESSION *session, const char *close_messg);
//void close_libssh2_channel(LIBSSH2_CHANNEL *channel);

/* Reads the given file info into data, which is allocated with malloc.  mem_size
   is not allocated and is the size of the array pointed to by data.  Returns
   whether or not reading was successful.  */
int read_from_ssh (ssh_path_info *info, char **data, size_t *mem_size);
/* Writes the bytes given in data to the given path in info.  If writes the amount
   given in mem_size and returns whether or not is was successful.  */
int write_to_ssh (ssh_path_info *info, const char *data, size_t mem_size);

/* A function that is supposed to give an ssh_path from a string of the form
   jason@waataja.com:/home/jason/file or something like that but I never
   wrote it because I never needed it.  */
int parse_ssh_path (const char *ssh_path, ssh_path_info *info);

/* Copies the contents of the file pointed to by src to dest. This can be done for ssh
   or from the local filesystem so this can act as a normal copy function for your
   computer or copy from one ssh connection to another.  The main use, though, is to
   copy from the local filesystem to an ssh destination or copy from an ssh source
   to a local filesystem.  progress_bar will be set after every read and write but
   will be ignored if it's NULL.  If make_popups is true, then error dialogs will be
   created with the given parent.  The errors are always written to stderr.  */
int gscp (ssh_path_info *src, ssh_path_info *dest, GtkProgressBar *progress_bar, int make_popups, GtkWindow *parent);

/* A function to ask the user for info about a path, either on
   the local filesystem or via ssh.  I didn't need it, thought.  */
int get_ssh_path_info_from_user (ssh_path_info *info);

/* Give a popup message stored in message with the parent parent.  It
   will have an okay button to return to the program.  */
void popup_message (GtkWindow *parent, const gchar *message);
/* Writes to both stderr and gives a popup message with popup_message
   is true.  */
void error_message (const char *message, int make_popup, GtkWindow *parent);

#endif /* SSHCONNECTION_H */ 
