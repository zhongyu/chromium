// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_CLIENT_H_
#define CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_CLIENT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/lazy_instance.h"
#include "base/memory/scoped_ptr.h"
#include "chrome/browser/extensions/chrome_notification_observer.h"
#include "extensions/browser/extensions_browser_client.h"

namespace base {
class CommandLine;
}

namespace content {
class BrowserContext;
}

namespace extensions {

class ChromeExtensionsAPIClient;
class ContentSettingsPrefsObserver;

// Implementation of extensions::BrowserClient for Chrome, which includes
// knowledge of Profiles, BrowserContexts and incognito.
//
// NOTE: Methods that do not require knowledge of browser concepts should be
// implemented in ChromeExtensionsClient even if they are only used in the
// browser process (see chrome/common/extensions/chrome_extensions_client.h).
class ChromeExtensionsBrowserClient : public ExtensionsBrowserClient {
 public:
  ChromeExtensionsBrowserClient();
  virtual ~ChromeExtensionsBrowserClient();

  // BrowserClient overrides:
  virtual bool IsShuttingDown() OVERRIDE;
  virtual bool AreExtensionsDisabled(const base::CommandLine& command_line,
                                     content::BrowserContext* context) OVERRIDE;
  virtual bool IsValidContext(content::BrowserContext* context) OVERRIDE;
  virtual bool IsSameContext(content::BrowserContext* first,
                             content::BrowserContext* second) OVERRIDE;
  virtual bool HasOffTheRecordContext(
      content::BrowserContext* context) OVERRIDE;
  virtual content::BrowserContext* GetOffTheRecordContext(
      content::BrowserContext* context) OVERRIDE;
  virtual content::BrowserContext* GetOriginalContext(
      content::BrowserContext* context) OVERRIDE;
  virtual bool IsGuestSession(content::BrowserContext* context) const OVERRIDE;
  virtual bool IsExtensionIncognitoEnabled(
      const std::string& extension_id,
      content::BrowserContext* context) const OVERRIDE;
  virtual bool CanExtensionCrossIncognito(
      const extensions::Extension* extension,
      content::BrowserContext* context) const OVERRIDE;
  virtual PrefService* GetPrefServiceForContext(
      content::BrowserContext* context) OVERRIDE;
  virtual void GetEarlyExtensionPrefsObservers(
      content::BrowserContext* context,
      std::vector<ExtensionPrefsObserver*>* observers) const OVERRIDE;
  virtual bool DeferLoadingBackgroundHosts(
      content::BrowserContext* context) const OVERRIDE;
  virtual bool IsBackgroundPageAllowed(
      content::BrowserContext* context) const OVERRIDE;
  virtual scoped_ptr<ExtensionHostDelegate> CreateExtensionHostDelegate()
      OVERRIDE;
  virtual bool DidVersionUpdate(content::BrowserContext* context) OVERRIDE;
  virtual scoped_ptr<AppSorting> CreateAppSorting() OVERRIDE;
  virtual bool IsRunningInForcedAppMode() OVERRIDE;
  virtual ApiActivityMonitor* GetApiActivityMonitor(
      content::BrowserContext* context) OVERRIDE;
  virtual ExtensionSystemProvider* GetExtensionSystemFactory() OVERRIDE;
  virtual void RegisterExtensionFunctions(
      ExtensionFunctionRegistry* registry) const OVERRIDE;

 private:
  friend struct base::DefaultLazyInstanceTraits<ChromeExtensionsBrowserClient>;

  // Observer for Chrome-specific notifications.
  ChromeNotificationObserver notification_observer_;

#if defined(ENABLE_EXTENSIONS)
  // Client for API implementations.
  scoped_ptr<ChromeExtensionsAPIClient> api_client_;
#endif

  DISALLOW_COPY_AND_ASSIGN(ChromeExtensionsBrowserClient);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CHROME_EXTENSIONS_BROWSER_CLIENT_H_
