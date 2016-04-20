#include "gnome-search-example.h"

static GVariant *
get_result_set (void)
{
  GVariantBuilder builder;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("as"));
  g_variant_builder_add (&builder, "s", "result1");
  g_variant_builder_add (&builder, "s", "result2");
	
    
  return g_variant_new ("(as)", &builder);
}

static GVariant *
get_result_metas (const gchar **results)
{
  gint idx;
  GIcon *gicon = g_themed_icon_new ("text-x-generic");
  const gchar *name_format = "Result %d";
  gchar *name, *gicon_str;
  GVariantBuilder meta, metas;

  g_variant_builder_init (&metas, G_VARIANT_TYPE ("aa{sv}"));

  for (idx = 0; results[idx] != NULL; idx++)
    {
      g_variant_builder_init (&meta, G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&meta, "{sv}",
                             "id", g_variant_new_string (results[idx]));

      name = g_strdup_printf (name_format, idx);
      g_variant_builder_add (&meta, "{sv}",
                             "name", g_variant_new_string (name));
      g_free (name);

      gicon_str = g_icon_to_string (gicon);
      g_variant_builder_add (&meta, "{sv}",
                             "gicon", g_variant_new_string (gicon_str));
      g_free (gicon_str);

      g_variant_builder_add_value (&metas, g_variant_builder_end (&meta));
    }

  return g_variant_new ("(aa{sv})", &metas);
}

static gboolean
handle_get_initial_result_set (SearchExampleShellSearchProvider2  *skeleton,
                               GDBusMethodInvocation              *invocation,
                               gchar                             **terms,
                               gpointer                            user_data)
{
  gchar *joined_terms = g_strjoinv (" ", terms);

  g_print ("****** GetInitialResultSet() called with %s\n", joined_terms);
  g_free (joined_terms);
  
  g_dbus_method_invocation_return_value (invocation, get_result_set ());
  return TRUE;
}

static gboolean
handle_get_subsearch_result_set (SearchExampleShellSearchProvider2  *skeleton,
                                 GDBusMethodInvocation              *invocation,
                                 gchar                             **previous_results,
                                 gchar                             **terms,
                                 gpointer                            user_data)
{
  gchar *joined_terms = g_strjoinv (" ", terms);

  g_print ("****** GetSubSearchResultSet() called with %s\n", joined_terms);
  g_free (joined_terms);

  g_dbus_method_invocation_return_value (invocation, get_result_set ());
  return TRUE;
}

static gboolean
handle_get_result_metas (SearchExampleShellSearchProvider2  *skeleton,
                         GDBusMethodInvocation              *invocation,
                         gchar                             **results,
                         gpointer                            user_data)
{
  gint idx;

  g_print ("****** GetResultMetas() called for results \n");

  for (idx = 0; results[idx] != NULL; idx++)
    g_print ("   %s\n", results[idx]);

  g_print ("\n");

  g_dbus_method_invocation_return_value (invocation, get_result_metas ((const gchar **) results));
  return TRUE;
}

static gboolean
handle_launch_search (SearchExampleShellSearchProvider2  *skeleton,
                      GDBusMethodInvocation              *invocation,
                      gchar                             **terms,
                      guint32                             timestamp,
                      gpointer                            user_data)
{
  gchar *joined_terms = g_strjoinv (" ", terms);

  g_print ("****** LaunchSearch() called with %s\n", joined_terms);
  g_free (joined_terms);

  g_dbus_method_invocation_return_value (invocation, NULL);
  return TRUE;
}

static gboolean
handle_activate_result (SearchExampleShellSearchProvider2  *skeleton,
                        GDBusMethodInvocation              *invocation,
                        gchar                              *result,
                        gchar                             **terms,
                        guint32                             timestamp,
                        gpointer                            user_data)
{
  gchar *joined_terms = g_strjoinv (" ", terms);

  g_print ("****** ActivateResult() called for %s and result %s\n",
           joined_terms, result);
  g_free (joined_terms);

  g_dbus_method_invocation_return_value (invocation, NULL);
  return TRUE;
}

static void
search_provider_name_acquired_cb (GDBusConnection *connection,
                                  const gchar     *name,
                                  gpointer         user_data)
{
  g_print ("Search provider name acquired: %s\n", name);
}

static void
search_provider_name_lost_cb (GDBusConnection *connection,
                              const gchar     *name,
                              gpointer         user_data)
{
  g_print ("Search provider name lost: %s\n", name);
}

static void
search_provider_bus_acquired_cb (GDBusConnection *connection,
                                 const gchar *name,
                                 gpointer user_data)
{
  SearchExampleProviderApp *self = user_data;


  g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (self->skeleton),
                                    connection,
                                    "/org/gnome/SearchExample/SearchProvider", NULL);
}

static void
search_provider_app_dispose (GObject *obj)
{
  SearchExampleProviderApp *self = SEARCH_EXAMPLE_PROVIDER_APP (obj);
  
  g_print ("****** Get To dispose rid of %d\n", g_application_get_inactivity_timeout (G_APPLICATION(obj)));
  
  
  if (self->name_owner_id != 0) {
    g_bus_unown_name (self->name_owner_id);
    self->name_owner_id = 0;
  }

  if (self->skeleton != NULL) {
    g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON (self->skeleton));
    g_clear_object (&self->skeleton);
  }

  g_clear_object (&self->object_manager);

  G_OBJECT_CLASS (search_example_provider_app_parent_class)->dispose (obj);
}

static void
search_provider_app_startup (GApplication *app)
{
  SearchExampleProviderApp *self = SEARCH_EXAMPLE_PROVIDER_APP (app);

  G_APPLICATION_CLASS (search_example_provider_app_parent_class)->startup (app);

  /* hold indefinitely unless timput cause the app to exit*/
  g_application_hold (app);

  self->name_owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                                        "org.gnome.SearchExample.SearchProvider",
                                        G_BUS_NAME_OWNER_FLAGS_NONE,
                                        search_provider_bus_acquired_cb,
                                        search_provider_name_acquired_cb,
                                        search_provider_name_lost_cb,
                                        app, NULL);
}

static void
search_example_provider_app_init (SearchExampleProviderApp *self)
{
  GApplication *app = G_APPLICATION (self);

  g_application_set_application_id (app, "org.gnome.SearchExample.SearchProvider");
  g_application_set_flags (app, G_APPLICATION_IS_SERVICE);
  
  self->skeleton = search_example_shell_search_provider2_skeleton_new ();


  g_signal_connect_swapped (self->skeleton,
                            "handle-activate-result",
                            G_CALLBACK (handle_activate_result),
                            self);
  g_signal_connect_swapped (self->skeleton,
                            "handle-get-initial-result-set",
                            G_CALLBACK (handle_get_initial_result_set),
                            self);
  g_signal_connect_swapped (self->skeleton,
                            "handle-get-subsearch-result-set",
                            G_CALLBACK (handle_get_subsearch_result_set),
                            self);
  g_signal_connect_swapped (self->skeleton,
                            "handle-get-result-metas",
                            G_CALLBACK (handle_get_result_metas),
                            self);
  g_signal_connect_swapped (self->skeleton,
                            "handle-launch-search",
                            G_CALLBACK (handle_launch_search),
                            self);
}

static void
search_example_provider_app_class_init (SearchExampleProviderAppClass *klass)
{
  GApplicationClass *aclass = G_APPLICATION_CLASS (klass);
  GObjectClass *oclass = G_OBJECT_CLASS (klass);

  aclass->startup = search_provider_app_startup;
  oclass->dispose = search_provider_app_dispose;
}

static GApplication *
search_example_provider_app_new (void)
{
  return g_object_new (search_example_provider_app_get_type (),
                       NULL);
}

int
main (int argc,
      char **argv)
{
  GApplication *app;
  gint res;

  gtk_init (&argc, &argv);

  app = search_example_provider_app_new ();
  res = g_application_run (app, argc, argv);
  g_object_unref (app);

  return res;
}
