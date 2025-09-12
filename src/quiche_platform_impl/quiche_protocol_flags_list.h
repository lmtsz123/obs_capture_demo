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
QUICHE_PROTOCOL_FLAG(int32_t, quic_dispatcher_max_ack_sent_per_connection, 10,
                     "Maximum number of ACKs sent per connection")
QUICHE_PROTOCOL_FLAG(bool, quic_use_lower_server_response_mtu_for_test, false,
                     "Use lower server response MTU for testing")
QUICHE_PROTOCOL_FLAG(bool, quic_enforce_single_packet_chlo, false,
                     "Enforce single packet CHLO")
QUICHE_PROTOCOL_FLAG(bool, quic_export_write_path_stats_at_server, false,
                     "Export write path stats at server")
QUICHE_PROTOCOL_FLAG(bool, quic_disable_client_tls_zero_rtt, false,
                     "Disable client TLS zero RTT")
QUICHE_PROTOCOL_FLAG(bool, quic_use_blackhole_detector, false,
                     "Use blackhole detector")
QUICHE_PROTOCOL_FLAG(bool, quic_disable_version_negotiation_grease_randomness, false,
                     "Disable version negotiation grease randomness")
QUICHE_PROTOCOL_FLAG(bool, quic_enable_chaos_protection, false,
                     "Enable chaos protection")
QUICHE_PROTOCOL_FLAG(bool, quic_always_discard_packets_if_per_packet_stores_undecryptable_packets, false,
                     "Always discard packets if per packet stores undecryptable packets")
QUICHE_PROTOCOL_FLAG(bool, quic_use_new_timeout_logic, true,
                     "Use new timeout logic")
QUICHE_PROTOCOL_FLAG(int64_t, quic_key_update_confidentiality_limit, 0,
                     "Key update confidentiality limit")
QUICHE_PROTOCOL_FLAG(int32_t, quic_max_pace_time_into_future_ms, 10,
                     "Max pace time into future in milliseconds")
QUICHE_PROTOCOL_FLAG(float, quic_pace_time_into_future_srtt_fraction, 0.125f,
                     "Pace time into future SRTT fraction")

// 可以在这里添加更多需要的协议标志

#endif
