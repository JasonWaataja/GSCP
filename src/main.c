/*
 * Sample showing how to do a simple SCP transfer.
 */

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

GtkWidget *window;
GtkWidget *grid;

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

ssh_path_info *src_path_info;
ssh_path_info *dest_path_info;
ssh_connection *src_con;
ssh_connection *dest_con;


static void
start_transfer()
{
  const char *src_path = gtk_entry_get_text(GTK_ENTRY(src_path_entry));
  src_path_info->path = malloc(sizeof(char) * (strlen(src_path) + 1));
  strcpy(src_path_info->path, src_path);
  const char *dest_path = gtk_entry_get_text(GTK_ENTRY(dest_path_entry));
  dest_path_info->path = malloc(sizeof(char) * (strlen(dest_path) + 1));
  strcpy(dest_path_info->path, dest_path);
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(src_on_lhost_checkbox)))
    {
      src_path_info->on_lhost = 1;
      src_con->hostaddr = inet_addr(gtk_entry_get_text(GTK_ENTRY(src_addr_entry)));
      src_con->port = atoi(gtk_entry_get_text(GTK_ENTRY(src_prt_entry)));
      const char *src_uname = gtk_entry_get_text(GTK_ENTRY(src_uname_entry));
      src_con->username = malloc(sizeof(char) * (strlen(src_uname) + 1));
      strcpy(src_con->username, src_uname);
      const char *src_pwd = gtk_entry_get_text(GTK_ENTRY(src_uname_entry));
      src_con->password = malloc(sizeof(char) * (strlen(src_pwd) + 1));
      strcpy(src_con->password, src_pwd);
    }
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dest_on_lhost_checkbox)))
    {
      dest_path_info->on_lhost = 1;
      dest_con->hostaddr = inet_addr(gtk_entry_get_text(GTK_ENTRY(dest_addr_entry)));
      dest_con->port = atoi(gtk_entry_get_text(GTK_ENTRY(dest_prt_entry)));
      const char *dest_uname = gtk_entry_get_text(GTK_ENTRY(dest_uname_entry));
      dest_con->username = malloc(sizeof(char) * (strlen(dest_uname) + 1));
      strcpy(dest_con->username, dest_uname);
      const char *dest_pwd = gtk_entry_get_text(GTK_ENTRY(dest_uname_entry));
      dest_con->password = malloc(sizeof(char) * (strlen(dest_pwd) + 1));
      strcpy(dest_con->password, dest_pwd);
    }

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
}

static void
src_on_lhost_checkbox_toggled()
{
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
  src_path_info = (ssh_path_info *) malloc(sizeof(ssh_path_info));
  dest_path_info = (ssh_path_info *) malloc(sizeof(ssh_path_info));
  src_con = (ssh_connection *) malloc(sizeof(ssh_connection));
  dest_con = (ssh_connection *) malloc(sizeof(ssh_connection));
  src_path_info->con = src_con;
  dest_path_info->con = dest_con;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "GSCP");
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);
  /*gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);*/

  src_on_lhost_checkbox = gtk_check_button_new_with_label("On Local Filesystem");
  g_signal_connect(G_OBJECT(src_on_lhost_checkbox), "toggled", G_CALLBACK(src_on_lhost_checkbox_toggled), NULL);
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
  g_signal_connect(G_OBJECT(dest_on_lhost_checkbox), "toggled", G_CALLBACK(dest_on_lhost_checkbox_toggled), NULL);
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
  gtk_grid_attach(GTK_GRID(grid), progress_bar, 0, 7, 2, 1);
  /*gtk_widget_set_visible(progress_bar, FALSE);*/

  gtk_widget_show_all (window);
}

static void
app_shutdown(GApplication *application,
         gpointer      user_data)
{
  free(src_path_info->path);
  src_path_info->path = NULL;
  free(src_path_info);
  src_path_info = NULL;
  ssh_connection_free(src_con);
  src_con = NULL;
  free(dest_path_info->path);
  dest_path_info->path = NULL;
  free(dest_path_info);
  dest_path_info = NULL;
  ssh_connection_free(dest_con);
  dest_con = NULL;
}

int main(int argc, char *argv[])
{

  libssh2_init(0);

  GtkApplication *app;
  int status;

  app = gtk_application_new ("com.waataja.gscp", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect(app, "shutdown", G_CALLBACK(app_shutdown), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  libssh2_exit();

  return status;
}
