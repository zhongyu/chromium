// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/fileapi/safe_audio_video_checker.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/process/process_handle.h"
#include "chrome/common/chrome_utility_messages.h"
#include "content/public/browser/child_process_data.h"
#include "content/public/browser/utility_process_host.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_message_macros.h"
#include "ipc/ipc_platform_file.h"

SafeAudioVideoChecker::SafeAudioVideoChecker(
    const base::PlatformFile& file,
    const fileapi::CopyOrMoveFileValidator::ResultCallback& callback)
    : state_(INITIAL_STATE),
      file_(file),
      file_closer_(&file_),
      callback_(callback) {
  DCHECK(!callback.is_null());
}

void SafeAudioVideoChecker::Start() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  if (state_ != INITIAL_STATE)
    return;
  state_ = PINGED_STATE;

  DCHECK(file_closer_);
  if (*file_closer_.get() == base::kInvalidPlatformFileValue) {
    callback_.Run(base::File::FILE_ERROR_SECURITY);
    state_ = FINISHED_STATE;
    return;
  }

  utility_process_host_ = content::UtilityProcessHost::Create(
      this, base::MessageLoopProxy::current())->AsWeakPtr();
  utility_process_host_->Send(new ChromeUtilityMsg_StartupPing);
}

SafeAudioVideoChecker::~SafeAudioVideoChecker() {}

void SafeAudioVideoChecker::OnProcessStarted() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  if (state_ != PINGED_STATE)
    return;
  state_ = STARTED_STATE;

  if (utility_process_host_->GetData().handle == base::kNullProcessHandle)
    DLOG(ERROR) << "Child process handle is null";
  IPC::PlatformFileForTransit file_for_transit =
      IPC::GetFileHandleForProcess(*file_closer_.release(),
                                   utility_process_host_->GetData().handle,
                                   true /* close_source_handle */);
  const int64 kFileDecodeTimeInMS = 250;
  utility_process_host_->Send(new ChromeUtilityMsg_CheckMediaFile(
      kFileDecodeTimeInMS, file_for_transit));
}

void SafeAudioVideoChecker::OnCheckingFinished(bool valid) {
  DCHECK_CURRENTLY_ON(content::BrowserThread::IO);
  if (state_ != STARTED_STATE)
    return;
  state_ = FINISHED_STATE;

  callback_.Run(valid ? base::File::FILE_OK :
                        base::File::FILE_ERROR_SECURITY);
}

void SafeAudioVideoChecker::OnProcessCrashed(int exit_code) {
  OnCheckingFinished(false);
}

bool SafeAudioVideoChecker::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(SafeAudioVideoChecker, message)
    IPC_MESSAGE_HANDLER(ChromeUtilityHostMsg_ProcessStarted,
        OnProcessStarted)
    IPC_MESSAGE_HANDLER(ChromeUtilityHostMsg_CheckMediaFile_Finished,
        OnCheckingFinished)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}
