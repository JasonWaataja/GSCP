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

#include "libssh2_config.h"
#include <libssh2.h>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include <gtk/gtk.h>

#include "sshconnection.h"

/* Main window. */
GtkWidget *window;
/* The main grid for the window. */
GtkWidget *grid;

/* Gui elements. */
GtkWidget *src_on_lhost_checkbox;
GtkWidget *dest_on_lhost_checkbox;
GtkWidget *src_addr_entry;
GtkWidget *dest_addr_entry;
GtkWidget *src_prt_entry;
GtkWidget *dest_prt_entry;
GtkWidget *src_pwd_entry;
GtkWidget *dest_pwd_entry;
GtkWidget *start_button;
GtkWidget *src_path_entry;
GtkWidget *dest_path_entry;
GtkWidget *src_uname_entry;
GtkWidget *dest_uname_entry;
GtkWidget *progress_bar;

/* The ssh connection info and whether or not it's stored on the local machine.  */
ssh_path_info *src_path_info;
ssh_path_info *dest_path_info;
ssh_connection *src_con;
ssh_connection *dest_con;

/* The function to call to start the file transfer using the information filled
   out in the boxes.  */
static void
start_transfer ();

/* The function to call when the on local filesystem checkbox is toggled.  */
static void
src_on_lhost_checkbox_toggled();

/* The function to call when the on local filesystem checkbox is toggled.  */
static void
dest_on_lhost_checkbox_toggled();

/* The activate function that sets up the gui.  */
static void
activate (GtkApplication* app,
          gpointer        user_data);

/* The shutdown function that does some cleanup.  */
static void
app_shutdown(GApplication *application,
         gpointer      user_data);


static void
start_transfer ()
{
  /* Get paths and check them.  */
  const char *src_path = gtk_entry_get_text(GTK_ENTRY(src_path_entry));
  if (strlen(src_path) == 0)
    {
      popup_message(GTK_WINDOW(window), "Please enter a source path");
      return;
    }
  src_path_info->path = malloc(sizeof(char) * (strlen(src_path) + 1));
  strcpy(src_path_info->path, src_path);
  const char *dest_path = gtk_entry_get_text(GTK_ENTRY(dest_path_entry));
  if (strlen(dest_path) == 0)
    {
      popup_message(GTK_WINDOW(window), "Please enter a destination path");
      return;
    }
  dest_path_info->path = malloc(sizeof(char) * (strlen(dest_path) + 1));
  strcpy(dest_path_info->path, dest_path);
  /* If the on local filesystem button is on, make sure the correct connection
     code is done.  */
  if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(src_on_lhost_checkbox)))
    {
      src_path_info->on_lhost = 0;
      const char *src_addr = gtk_entry_get_text(GTK_ENTRY(src_addr_entry));
      if (strlen(src_addr) == 0)
        {
          popup_message(GTK_WINDOW(window), "Please enter a source address");
          return;
        }
      src_con->hostaddr = inet_addr(gtk_entry_get_text(GTK_ENTRY(src_addr_entry)));
      if (strlen(gtk_entry_get_text(GTK_ENTRY(src_prt_entry))) == 0)
        {
          src_con->port = SSH_DEFAULT_PORT;
        }
      else
        src_con->port = atoi(gtk_entry_get_text(GTK_ENTRY(src_prt_entry)));
      const char *src_uname = gtk_entry_get_text(GTK_ENTRY(src_uname_entry));
      if (strlen(src_uname) == 0)
        {
          popup_message(GTK_WINDOW(window), "Please enter a username for the source");
          return;
        }
      src_con->username = malloc(sizeof(char) * (strlen(src_uname) + 1));
      strcpy(src_con->username, src_uname);
      const char *src_pwd = gtk_entry_get_text(GTK_ENTRY(src_pwd_entry));
      src_con->password = malloc(sizeof(char) * (strlen(src_pwd) + 1));
      strcpy(src_con->password, src_pwd);
    }
  else
    {
      src_path_info->on_lhost = 1;
    }
  if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dest_on_lhost_checkbox)))
    {
      dest_path_info->on_lhost = 0;
      const char *dest_addr = gtk_entry_get_text(GTK_ENTRY(dest_addr_entry));
      if (strlen(dest_addr) == 0)
        {
          popup_message(GTK_WINDOW(window), "Please enter a destination address");
          return;
        }
      dest_con->hostaddr = inet_addr(gtk_entry_get_text(GTK_ENTRY(dest_addr_entry)));
      if (strlen(gtk_entry_get_text(GTK_ENTRY(dest_prt_entry))) == 0)
        {
          dest_con->port = SSH_DEFAULT_PORT;
        }
      else
        dest_con->port = atoi(gtk_entry_get_text(GTK_ENTRY(dest_prt_entry)));
      const char *dest_uname = gtk_entry_get_text(GTK_ENTRY(dest_uname_entry));
      if (strlen(dest_uname) == 0)
        {
          popup_message(GTK_WINDOW(window), "Please enter a username for the destinaction");
          return;
        }
      dest_con->username = malloc(sizeof(char) * (strlen(dest_uname) + 1));
      strcpy(dest_con->username, dest_uname);
      const char *dest_pwd = gtk_entry_get_text(GTK_ENTRY(dest_pwd_entry));
      dest_con->password = malloc(sizeof(char) * (strlen(dest_pwd) + 1));
      strcpy(dest_con->password, dest_pwd);
    }
  else
    {
      dest_path_info->on_lhost = 1;
    }
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
  int result;
  result = gscp(src_path_info, dest_path_info, GTK_PROGRESS_BAR(progress_bar), 1, GTK_WINDOW(window));
  if (result)
    {
      popup_message(GTK_WINDOW(window), "Transfer Completed");
    }
  else
    {
      /*popup_message(GTK_WINDOW(window), "Transfer Failed");*/
    }
  /*gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);*/
}

static void
src_on_lhost_checkbox_toggled()
{
  /* When the on local filesystem is checked, hide the ssh information.  */
  gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(src_on_lhost_checkbox));
  if (is_checked)
    {
      gtk_editable_set_editable(GTK_EDITABLE(src_addr_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(src_addr_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(src_prt_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(src_prt_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(src_uname_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(src_uname_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(src_pwd_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(src_pwd_entry), FALSE);
    }
  else
    {
      gtk_editable_set_editable(GTK_EDITABLE(src_addr_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(src_addr_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(src_prt_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(src_prt_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(src_uname_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(src_uname_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(src_pwd_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(src_pwd_entry), TRUE);
    }
}

static void
dest_on_lhost_checkbox_toggled()
{
  /* When the on local filesystem is checked, hide the ssh information.  */
  gboolean is_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dest_on_lhost_checkbox));
  if (is_checked)
    {
      gtk_editable_set_editable(GTK_EDITABLE(dest_addr_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(dest_addr_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_prt_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(dest_prt_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_uname_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(dest_uname_entry), FALSE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_pwd_entry), FALSE);
      gtk_widget_set_visible(GTK_WIDGET(dest_pwd_entry), FALSE);
    }
  else
    {
      gtk_editable_set_editable(GTK_EDITABLE(dest_addr_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(dest_addr_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_prt_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(dest_prt_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_uname_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(dest_uname_entry), TRUE);
      gtk_editable_set_editable(GTK_EDITABLE(dest_pwd_entry), TRUE);
      gtk_widget_set_visible(GTK_WIDGET(dest_pwd_entry), TRUE);
    }

}

static void
activate (GtkApplication* app,
          gpointer        user_data)
{
  /* Allocate the new objects.  */
  src_path_info = (ssh_path_info *) malloc(sizeof(ssh_path_info));
  dest_path_info = (ssh_path_info *) malloc(sizeof(ssh_path_info));
  src_con = (ssh_connection *) malloc(sizeof(ssh_connection));
  dest_con = (ssh_connection *) malloc(sizeof(ssh_connection));
  if (!src_path_info || !dest_path_info || !src_con || !dest_con)
    {
      fprintf(stderr, "Error allocating memory\n");
      return;
    }
  src_path_info->con = src_con;
  dest_path_info->con = dest_con;
  src_path_info->path = NULL;
  src_con->username = NULL;
  src_con->password = NULL;
  dest_path_info->path = NULL;
  dest_con->username = NULL;
  dest_con->password = NULL;

  /* Setup the new window.  */
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "GSCP");
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);
  /*gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);*/

  /* Setup other gui elements.  */
  src_on_lhost_checkbox = gtk_check_button_new_with_label("On Local Filesystem");
  g_signal_connect(G_OBJECT(src_on_lhost_checkbox), "toggled",
                   G_CALLBACK(src_on_lhost_checkbox_toggled), NULL);
  gtk_grid_attach(GTK_GRID(grid), src_on_lhost_checkbox, 0, 0, 1, 1);
  src_path_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_path_entry), "Path");
  gtk_grid_attach(GTK_GRID(grid), src_path_entry, 0, 1, 1, 1);
  src_addr_entry = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(src_addr_entry), GTK_INPUT_PURPOSE_URL);
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_addr_entry), "Address");
  gtk_grid_attach(GTK_GRID(grid), src_addr_entry, 0, 2, 1, 1);
  /*gtk_widget_set_hexpand(src_addr_entry, TRUE);*/
  src_prt_entry = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(src_prt_entry), GTK_INPUT_PURPOSE_DIGITS);
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_prt_entry), "Port");
  gtk_grid_attach(GTK_GRID(grid), src_prt_entry, 0, 3, 1, 1);
  src_uname_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_uname_entry), "Username");
  gtk_grid_attach(GTK_GRID(grid), src_uname_entry, 0, 4, 1, 1);
  src_pwd_entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(src_pwd_entry), FALSE);
  gtk_entry_set_input_purpose(GTK_ENTRY(src_pwd_entry), GTK_INPUT_PURPOSE_PASSWORD);
  gtk_entry_set_placeholder_text(GTK_ENTRY(src_pwd_entry), "Password");
  gtk_grid_attach(GTK_GRID(grid), src_pwd_entry, 0, 5, 1, 1);
  


  dest_on_lhost_checkbox = gtk_check_button_new_with_label("On Local Filesystem");
  g_signal_connect(G_OBJECT(dest_on_lhost_checkbox), "toggled",
                   G_CALLBACK(dest_on_lhost_checkbox_toggled), NULL);
  gtk_grid_attach(GTK_GRID(grid), dest_on_lhost_checkbox, 1, 0, 1, 1);
  dest_path_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_path_entry), "Path");
  gtk_grid_attach(GTK_GRID(grid), dest_path_entry, 1, 1, 1, 1);
  dest_addr_entry = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(dest_addr_entry), GTK_INPUT_PURPOSE_URL);
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_addr_entry), "Address");
  gtk_grid_attach(GTK_GRID(grid), dest_addr_entry, 1, 2, 1, 1);
  /*gtk_widget_set_hexpand(dest_addr_entry, TRUE);*/
  dest_prt_entry = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(dest_prt_entry), GTK_INPUT_PURPOSE_DIGITS);
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_prt_entry), "Port");
  gtk_grid_attach(GTK_GRID(grid), dest_prt_entry, 1, 3, 1, 1);
  dest_uname_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_uname_entry), "Username");
  gtk_grid_attach(GTK_GRID(grid), dest_uname_entry, 1, 4, 1, 1);
  dest_pwd_entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(dest_pwd_entry), FALSE);
  gtk_entry_set_input_purpose(GTK_ENTRY(dest_pwd_entry), GTK_INPUT_PURPOSE_PASSWORD);
  gtk_entry_set_placeholder_text(GTK_ENTRY(dest_pwd_entry), "Password");
  gtk_grid_attach(GTK_GRID(grid), dest_pwd_entry, 1, 5, 1, 1);

  start_button = gtk_button_new_with_label("Start Transfer");
  gtk_widget_set_hexpand(GTK_WIDGET(start_button), TRUE);
  gtk_widget_set_vexpand(GTK_WIDGET(start_button), TRUE);
  gtk_grid_attach(GTK_GRID(grid), start_button, 0, 6, 2, 1);
  g_signal_connect(G_OBJECT(start_button), "clicked", G_CALLBACK(start_transfer), NULL);

  progress_bar = gtk_progress_bar_new();
  gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "Progress:");
  gtk_grid_attach(GTK_GRID(grid), progress_bar, 0, 7, 2, 1);
  /*gtk_widget_set_visible(progress_bar, FALSE);*/

  /* Show the window and all its elements.  */
  gtk_widget_show_all (window);
}

static void
app_shutdown(GApplication *application,
         gpointer      user_data)
{
  /* Free all of the elements.  */

  /*printf("Shutting down\n");*/
  /*printf("src path info path %p\n", src_path_info->path);*/
  free(src_path_info->path);
  src_path_info->path = NULL;
  /*printf("src path info %p\n", src_path_info);*/
  free(src_path_info);
  src_path_info = NULL;
  /*printf("src_con %p\n", src_con);*/
  ssh_connection_free(src_con);
  src_con = NULL;
  /*printf("dest_path_info %p\n", dest_path_info->path);*/
  free(dest_path_info->path);
  dest_path_info->path = NULL;
  /*printf("dest_path_info %p\n", dest_path_info);*/
  free(dest_path_info);
  dest_path_info = NULL;
  /*printf("dest_con %p\n", dest_con);*/
  ssh_connection_free(dest_con);
  dest_con = NULL;
}

int main(int argc, char *argv[])
{
  /* Initialize the libssh2 library.  */
  libssh2_init(0);

  GtkApplication *app;
  int status;

  /* Create a new application and tell it what to do on startup and shutdown.
  */
  app = gtk_application_new ("com.waataja.gscp", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect(app, "shutdown", G_CALLBACK(app_shutdown), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  /* Exit the libssh2 library.  */
  libssh2_exit();

  /* Return the same status that the gapplication returned.  */
  return status;
}
