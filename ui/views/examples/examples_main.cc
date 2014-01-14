// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/run_loop.h"
#include "ui/aura/env.h"
#include "ui/base/ime/input_method_initializer.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/compositor/test/context_factories_for_test.h"
#include "ui/gfx/screen.h"
#include "ui/views/corewm/wm_state.h"
#include "ui/views/examples/examples_window.h"
#include "ui/views/test/desktop_test_views_delegate.h"

#if !defined(OS_CHROMEOS)
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#endif
#if defined(OS_WIN)
#include "ui/base/win/scoped_ole_initializer.h"
#endif

int main(int argc, char** argv) {
#if defined(OS_WIN)
  ui::ScopedOleInitializer ole_initializer_;
#endif

  CommandLine::Init(argc, argv);

  base::AtExitManager at_exit;
  base::MessageLoopForUI message_loop;

  base::i18n::InitializeICU();

  base::FilePath pak_dir;
  PathService::Get(base::DIR_MODULE, &pak_dir);

  base::FilePath pak_file;
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("ui_test.pak"));

  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);

  // The ContextFactory must exist before any Compositors are created.
  bool allow_test_contexts = false;
  ui::InitializeContextFactoryForTests(allow_test_contexts);

  aura::Env::CreateInstance();

  ui::InitializeInputMethodForTesting();

  {
    views::DesktopTestViewsDelegate views_delegate;
    views::corewm::WMState wm_state;

#if !defined(OS_CHROMEOS)
    scoped_ptr<gfx::Screen> desktop_screen(views::CreateDesktopScreen());
    gfx::Screen::SetScreenInstance(gfx::SCREEN_TYPE_NATIVE,
                                   desktop_screen.get());
#endif

    views::examples::ShowExamplesWindow(views::examples::QUIT_ON_CLOSE);

    base::RunLoop().Run();

    ui::ResourceBundle::CleanupSharedInstance();
  }

  ui::ShutdownInputMethod();

  aura::Env::DeleteInstance();

  return 0;
}
