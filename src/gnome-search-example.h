#ifndef __SEARCH_TYPE_EXAMPLE_PROVIDER_H__
#define __SEARCH_TYPE_EXAMPLE_PROVIDER_H__
#include <config.h>

#include <gio/gio.h>
#include <string.h>
#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "search-example-provider-generated.h"

#define SEARCH_PROVIDER_INACTIVITY_TIMEOUT 120000 /* milliseconds */

typedef GApplicationClass SearchExampleProviderAppClass;
typedef struct _SearchExampleProviderApp SearchExampleProviderApp;

struct _SearchExampleProviderApp {
  GApplication parent;

  guint name_owner_id;
  GDBusObjectManagerServer *object_manager;
  SearchExampleShellSearchProvider2 *skeleton;
};

GType search_example_provider_app_get_type (void);

#define SEARCH_TYPE_EXAMPLE_PROVIDER_APP search_example_provider_app_get_type()
#define SEARCH_EXAMPLE_PROVIDER_APP(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), SEARCH_TYPE_EXAMPLE_PROVIDER_APP, SearchExampleProviderApp))

G_DEFINE_TYPE (SearchExampleProviderApp, search_example_provider_app, G_TYPE_APPLICATION)


#endif /* __SEARCH_TYPE_EXAMPLE_PROVIDER_H__ */
