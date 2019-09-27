/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/video_engine/vie_window_creator.h"
#include "webrtc/video_engine/vie_autotest_window_manager_interface.h"
#include "webrtc/video_engine/vie_window_manager_factory.h"

#if defined(WIN32)
#include <tchar.h>
#endif

ViEWindowCreator::ViEWindowCreator() {
	window_manager_ =
		ViEWindowManagerFactory::CreateWindowManagerForCurrentPlatform();
}

ViEWindowCreator::~ViEWindowCreator() {
  delete window_manager_;
}

ViEAutoTestWindowManagerInterface*
  ViEWindowCreator::CreateTwoWindows() {
#if defined(WIN32)
  TCHAR window1Title[1024] = _T("ViE test Window 1");
  TCHAR window2Title[1024] = _T("ViE test Window 2");
#else
  char window1Title[1024] = "ViE test Window 1";
  char window2Title[1024] = "ViE test Window 2";
#endif

  AutoTestRect window1Size(608, 544, 400, 200);
  AutoTestRect window2Size(608, 544, 1200, 200);
  window_manager_->CreateWindows(window1Size, window2Size, window1Title,window2Title);
  window_manager_->SetTopmostWindow();

  return window_manager_;
}

void ViEWindowCreator::TerminateWindows() {
  window_manager_->TerminateWindows();
}

//____
mod_ViEWindowCreator::mod_ViEWindowCreator() {
	window_manager_ =
		mod_ViEWindowManagerFactory::CreateWindowManagerForCurrentPlatform();
}

mod_ViEWindowCreator::~mod_ViEWindowCreator() {
	delete window_manager_;
}

mod_ViEAutoTestWindowManagerInterface*
	mod_ViEWindowCreator::CreateOneWindow(char *windowName) {
#if defined(WIN32)
	TCHAR window1Title[1024] = _T("ViE test Window 1");
#else
  char window1Title[1024] = "ViE test Window 1";
#endif

  AutoTestRect window1Size(320, 240, 400, 200);
  window_manager_->CreateOneWindow(window1Size, windowName);

  return window_manager_;
}

void mod_ViEWindowCreator::TerminateWindows() {
	window_manager_->TerminateWindows();
}