// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/api/test/test_api.h"

#include <string>

#include "base/command_line.h"
#include "base/memory/singleton.h"
#include "chrome/browser/chrome_notification_types.h"
#include "content/public/browser/notification_service.h"
#include "content/public/common/content_switches.h"
#include "extensions/browser/extension_function_dispatcher.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/quota_service.h"
#include "extensions/common/api/test.h"

namespace {

// If you see this error in your test, you need to set the config state
// to be returned by chrome.test.getConfig().  Do this by calling
// TestGetConfigFunction::set_test_config_state(Value* state)
// in test set up.
const char kNoTestConfigDataError[] = "Test configuration was not set.";

const char kNotTestProcessError[] =
    "The chrome.test namespace is only available in tests.";

}  // namespace

namespace extensions {

namespace Log = core_api::test::Log;
namespace NotifyFail = core_api::test::NotifyFail;
namespace PassMessage = core_api::test::PassMessage;
namespace WaitForRoundTrip = core_api::test::WaitForRoundTrip;

TestExtensionFunction::~TestExtensionFunction() {}

void TestExtensionFunction::Run() {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(switches::kTestType)) {
    error_ = kNotTestProcessError;
    SendResponse(false);
    return;
  }
  SendResponse(RunImpl());
}

TestNotifyPassFunction::~TestNotifyPassFunction() {}

bool TestNotifyPassFunction::RunImpl() {
  content::NotificationService::current()->Notify(
      chrome::NOTIFICATION_EXTENSION_TEST_PASSED,
      content::Source<content::BrowserContext>(dispatcher()->browser_context()),
      content::NotificationService::NoDetails());
  return true;
}

TestNotifyFailFunction::~TestNotifyFailFunction() {}

bool TestNotifyFailFunction::RunImpl() {
  scoped_ptr<NotifyFail::Params> params(NotifyFail::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  content::NotificationService::current()->Notify(
      chrome::NOTIFICATION_EXTENSION_TEST_FAILED,
      content::Source<content::BrowserContext>(dispatcher()->browser_context()),
      content::Details<std::string>(&params->message));
  return true;
}

TestLogFunction::~TestLogFunction() {}

bool TestLogFunction::RunImpl() {
  scoped_ptr<Log::Params> params(Log::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  VLOG(1) << params->message;
  return true;
}

TestResetQuotaFunction::~TestResetQuotaFunction() {}

bool TestResetQuotaFunction::RunImpl() {
  QuotaService* quota =
      ExtensionSystem::Get(browser_context())->quota_service();
  quota->Purge();
  quota->violation_errors_.clear();
  return true;
}

bool TestSendMessageFunction::RunImpl() {
  scoped_ptr<PassMessage::Params> params(PassMessage::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());
  content::NotificationService::current()->Notify(
      chrome::NOTIFICATION_EXTENSION_TEST_MESSAGE,
      content::Source<TestSendMessageFunction>(this),
      content::Details<std::string>(&params->message));
  return true;
}

TestSendMessageFunction::~TestSendMessageFunction() {}

void TestSendMessageFunction::Reply(const std::string& message) {
  SetResult(new base::StringValue(message));
  SendResponse(true);
}

// static
void TestGetConfigFunction::set_test_config_state(
    base::DictionaryValue* value) {
  TestConfigState* test_config_state = TestConfigState::GetInstance();
  test_config_state->set_config_state(value);
}

TestGetConfigFunction::TestConfigState::TestConfigState()
    : config_state_(NULL) {}

// static
TestGetConfigFunction::TestConfigState*
TestGetConfigFunction::TestConfigState::GetInstance() {
  return Singleton<TestConfigState>::get();
}

TestGetConfigFunction::~TestGetConfigFunction() {}

bool TestGetConfigFunction::RunImpl() {
  TestConfigState* test_config_state = TestConfigState::GetInstance();

  if (!test_config_state->config_state()) {
    error_ = kNoTestConfigDataError;
    return false;
  }

  SetResult(test_config_state->config_state()->DeepCopy());
  return true;
}

TestWaitForRoundTripFunction::~TestWaitForRoundTripFunction() {}

bool TestWaitForRoundTripFunction::RunImpl() {
  scoped_ptr<WaitForRoundTrip::Params> params(
      WaitForRoundTrip::Params::Create(*args_));
  SetResult(new base::StringValue(params->message));
  return true;
}

}  // namespace extensions
