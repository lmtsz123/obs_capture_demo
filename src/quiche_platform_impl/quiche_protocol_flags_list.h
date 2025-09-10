// Copyright (c) 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// NOLINT(build/header_guard)
// This file intentionally does not have header guards, it's intended to be
// included multiple times, each time with a different definition of
// QUICHE_PROTOCOL_FLAG.

#if defined(QUICHE_PROTOCOL_FLAG)

// 只包含我们需要的标志，避免包含所有标志
QUICHE_PROTOCOL_FLAG(bool, quic_interval_set_enable_add_optimization, true,
                     "If true, enable an optimization in QuicIntervalSet")

// 可以在这里添加更多需要的协议标志

#endif
