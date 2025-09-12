// NOLINTBEGIN
// clang-format off
// DO NOT EDIT.

// This file intentionally does not have header guards, it is intended to be
// included multiple times, each time with a different definition of
// QUICHE_FLAG.

#if defined(QUICHE_FLAG)

// BBR2拥塞控制相关标志
QUICHE_FLAG(int32_t, quic_bbr2_default_probe_bw_full_loss_count, 2, 2,
           "Default value for Bbr2Params.probe_bw_full_loss_count")

QUICHE_FLAG(float, quic_bbr2_default_probe_rtt_inflight_target_bdp_fraction, 0.5f, 0.5f,
           "Default value for Bbr2Params.probe_rtt_inflight_target_bdp_fraction")

QUICHE_FLAG(int32_t, quic_bbr2_default_probe_rtt_period_ms, 10000, 10000,
           "Default value for Bbr2Params.probe_rtt_period")

QUICHE_FLAG(int32_t, quic_bbr2_default_probe_rtt_duration_ms, 200, 200,
           "Default value for Bbr2Params.probe_rtt_duration")

QUICHE_FLAG(int32_t, quic_bbr2_default_initial_ack_height_filter_window, 10, 10,
           "Default value for Bbr2Params.initial_ack_height_filter_window")

QUICHE_FLAG(float, quic_bbr2_default_inflight_hi_headroom, 0.15f, 0.15f,
           "Default value for Bbr2Params.inflight_hi_headroom")

QUICHE_FLAG(float, quic_bbr2_default_loss_threshold, 0.3f, 0.3f,
           "Default value for Bbr2Params.loss_threshold")

// 拥塞窗口相关
QUICHE_FLAG(int32_t, quic_max_congestion_window, 2000, 2000,
           "Maximum congestion window in packets")

// 其他必需的标志
QUICHE_FLAG(bool, quic_enable_version_rfcv1, true, true,
           "Enable RFC v1 QUIC version")

// QUIC服务器配置
QUICHE_FLAG(bool, quic_always_support_server_preferred_address, false, false,
           "Always support server preferred address")

QUICHE_FLAG(int32_t, quic_anti_amplification_factor, 3, 3,
           "Anti-amplification factor")

QUICHE_FLAG(bool, quic_enforce_strict_amplification_factor, false, false,
           "Enforce strict amplification factor")

// 重传和Ping相关
QUICHE_FLAG(int32_t, quic_max_aggressive_retransmittable_on_wire_ping_count, 1, 1,
           "Max aggressive retransmittable on wire ping count")

QUICHE_FLAG(int32_t, quic_max_retransmittable_on_wire_ping_count, 2, 2,
           "Max retransmittable on wire ping count")

// ACK相关
QUICHE_FLAG(int32_t, quic_ack_decimation_delay, 25, 25,
           "ACK decimation delay")

QUICHE_FLAG(int32_t, quic_default_delayed_ack_time_ms, 25, 25,
           "Default delayed ACK time in milliseconds")

QUICHE_FLAG(int32_t, quic_multiplexer_alarm_granularity_us, 1000, 1000,
           "Multiplexer alarm granularity in microseconds")

// 发送缓冲区相关
QUICHE_FLAG(int32_t, quic_send_buffer_max_data_slice_size, 4096, 4096,
           "Send buffer max data slice size")

// 数据包跟踪（已在protocol flags中定义）

QUICHE_FLAG(bool, quic_preallocate_unacked_packets, true, true,
           "Preallocate unacked packets")

// BBR相关
QUICHE_FLAG(float, quic_bbr_cwnd_gain, 2.0f, 2.0f,
           "BBR congestion window gain")

QUICHE_FLAG(int32_t, quic_bbr2_default_startup_full_loss_count, 8, 8,
           "BBR2 default startup full loss count")

QUICHE_FLAG(int32_t, quic_bbr2_default_probe_bw_base_duration_ms, 2000, 2000,
           "BBR2 default probe BW base duration in milliseconds")

QUICHE_FLAG(int32_t, quic_bbr2_default_probe_bw_max_rand_duration_ms, 1000, 1000,
           "BBR2 default probe BW max random duration in milliseconds")

// 带宽阈值
QUICHE_FLAG(int32_t, quic_ack_aggregation_bandwidth_threshold, 1000000, 1000000,
           "ACK aggregation bandwidth threshold")

// Pacing相关
QUICHE_FLAG(int32_t, quic_lumpy_pacing_size, 2, 2,
           "Lumpy pacing size")

QUICHE_FLAG(float, quic_lumpy_pacing_cwnd_fraction, 0.25f, 0.25f,
           "Lumpy pacing congestion window fraction")

QUICHE_FLAG(int32_t, quic_lumpy_pacing_min_bandwidth_kbps, 1200, 1200,
           "Lumpy pacing minimum bandwidth in kbps")

// 版本控制相关（已在protocol flags中定义）

// 可重新加载标志 (简化实现)
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_pacing_remove_non_initial_burst, false, false,
           "Remove non-initial burst in pacing")

// 添加缺失的reloadable flags
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_default_to_bbr_v2, false, false,
           "Default to BBR v2")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_default_to_bbr, false, false,
           "Default to BBR")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_disable_server_blackhole_detection, false, false,
           "Disable server blackhole detection")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_enable_mtu_discovery_at_server, false, false,
           "Enable MTU discovery at server")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_enable_server_on_wire_ping, false, false,
           "Enable server on wire ping")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_allow_flow_label_blackhole_avoidance_on_server, false, false,
           "Allow flow label blackhole avoidance on server")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_on_packet_header_return_connected, false, false,
           "On packet header return connected")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_use_received_client_addresses_cache, false, false,
           "Use received client addresses cache")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_fail_on_empty_ack, false, false,
           "Fail on empty ACK")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_add_stream_info_to_idle_close_detail, false, false,
           "Add stream info to idle close detail")
QUICHE_FLAG(bool, quiche_reloadable_flag_quic_no_path_degrading_before_handshake_confirmed, false, false,
           "No path degrading before handshake confirmed")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_neuter_packets_on_migration, false, false,
           "Neuter packets on migration")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_least_unacked_plus_1, false, false,
           "Use least unacked plus 1")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_default_enable_5rto_blackhole_detection2, false, false,
           "Enable 5RTO blackhole detection")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_ignore_gquic_probing, false, false,
           "Ignore GQUIC probing")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_limit_new_streams_per_loop_2, false, false,
           "Limit new streams per loop")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_test_peer_addr_change_after_normalize, false, false,
           "Test peer address change after normalize")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_fix_timeouts, false, false,
           "Fix timeouts")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_notify_ack_listener_earlier, false, false,
           "Notify ACK listener earlier")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_conservative_cwnd_and_pacing_gains, false, false,
           "Conservative congestion window and pacing gains")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_enable_version_rfcv2, false, false,
           "Enable RFC v2 QUIC version")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_disable_version_rfcv1, false, false,
           "Disable RFC v1 QUIC version")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_disable_version_draft_29, false, false,
           "Disable draft 29 QUIC version")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_disable_version_q046, false, false,
           "Disable Q046 QUIC version")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_priority_respect_incremental, false, false,
           "Priority respect incremental")

QUICHE_FLAG(bool, quiche_reloadable_flag_quic_disable_batch_write, false, false,
           "Disable batch write")

// 添加更多缺失的FLAGS
QUICHE_FLAG(bool, quic_enable_http3_grease_randomness, false, false,
           "Enable HTTP/3 grease randomness")

QUICHE_FLAG(bool, quic_buffer_packet_till_readable, false, false,
           "Buffer packet until readable")

// quic_enable_chaos_protection 已在protocol flags中定义

QUICHE_FLAG(bool, quic_use_common_stream_check, false, false,
           "Use common stream check")

QUICHE_FLAG(bool, quic_allow_chlo_buffering, true, true,
           "Allow CHLO buffering")

QUICHE_FLAG(int32_t, quic_max_buffered_crypto_bytes, 16384, 16384,
           "Maximum buffered crypto bytes")

QUICHE_FLAG(bool, quic_use_pending_frames_in_alarm, false, false,
           "Use pending frames in alarm")

QUICHE_FLAG(bool, quic_enable_tls_resumption, true, true,
           "Enable TLS resumption")

QUICHE_FLAG(bool, quic_use_write_or_buffer_data_at_level, false, false,
           "Use write or buffer data at level")

QUICHE_FLAG(bool, quic_goaway_with_max_stream_id, false, false,
           "GOAWAY with max stream ID")

QUICHE_FLAG(bool, quic_unified_iw_options, false, false,
           "Unified initial window options")

QUICHE_FLAG(int32_t, quic_time_wait_list_seconds, 200, 200,
           "Time wait list seconds")

QUICHE_FLAG(int32_t, quic_time_wait_list_max_connections, 600000, 600000,
           "Time wait list max connections")

QUICHE_FLAG(bool, quic_use_stream_notifier, false, false,
           "Use stream notifier")

QUICHE_FLAG(int32_t, quic_connection_options, 0, 0,
           "Connection options")

QUICHE_FLAG(bool, quic_enable_pto, true, true,
           "Enable probe timeout")

QUICHE_FLAG(bool, quic_enable_server_on_wire_ping, false, false,
           "Enable server on wire ping")

QUICHE_FLAG(bool, quic_enable_loss_detection_tuner, false, false,
           "Enable loss detection tuner")

QUICHE_FLAG(bool, quic_simplify_stop_waiting, false, false,
           "Simplify stop waiting")

QUICHE_FLAG(bool, quic_fix_bbr_cwnd_in_bandwidth_resumption, false, false,
           "Fix BBR congestion window in bandwidth resumption")

QUICHE_FLAG(bool, quic_enable_token_binding, false, false,
           "Enable token binding")

QUICHE_FLAG(bool, quic_treat_buffer_as_ack_eliciting, false, false,
           "Treat buffer as ACK eliciting")

QUICHE_FLAG(bool, quic_count_bytes_on_alternative_path_separately, false, false,
           "Count bytes on alternative path separately")

QUICHE_FLAG(bool, quic_count_server_designated_connection_id_limit, false, false,
           "Count server designated connection ID limit")

QUICHE_FLAG(bool, quic_enable_mtu_discovery, false, false,
           "Enable MTU discovery")

QUICHE_FLAG(int32_t, quic_max_undecryptable_packets, 10, 10,
           "Maximum undecryptable packets")

QUICHE_FLAG(bool, quic_enable_peer_migration, false, false,
           "Enable peer migration")

QUICHE_FLAG(bool, quic_enable_zero_rtt_for_tls, false, false,
           "Enable 0-RTT for TLS")

QUICHE_FLAG(bool, quic_enable_accept_random_ipn, false, false,
           "Enable accept random IPN")

QUICHE_FLAG(bool, quic_enable_multiport, false, false,
           "Enable multiport")

// 添加更多从编译错误中发现的缺失FLAGS
QUICHE_FLAG(int32_t, FLAGS_quic_dispatcher_max_ack_sent_per_connection, 10, 10,
           "Max ACK frames sent per connection by dispatcher")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_allow_client_enabled_bbr_v2, false, false,
           "Allow client enabled BBR v2")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_default_to_bbr, false, false,
           "Default to BBR congestion control")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_allow_client_enabled_2x_initial_cwnd, false, false,
           "Allow client enabled 2x initial congestion window")

QUICHE_FLAG(bool, FLAGS_quic_disable_pacing_for_perf_tests, false, false,
           "Disable pacing for performance tests")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_conservative_bursts, false, false,
           "Conservative bursts")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_receive_ack_frequency, false, false,
           "Receive ACK frequency")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_no_write_control_frame_upon_connection_close, false, false,
           "No write control frame upon connection close")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_notify_stream_soon_to_destroy, false, false,
           "Notify stream soon to destroy")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_deliver_stop_sending_to_zombie_streams, false, false,
           "Deliver stop sending to zombie streams")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_use_inlining_send_buffer2, false, false,
           "Use inlining send buffer v2")

QUICHE_FLAG(int32_t, FLAGS_quic_buffered_data_threshold, 8192, 8192,
           "Buffered data threshold")

QUICHE_FLAG(int32_t, quic_max_streams_window_divisor, 2, 2,
           "Max streams window divisor")

QUICHE_FLAG(int32_t, FLAGS_quic_time_wait_list_max_pending_packets, 5, 5,
           "Time wait list max pending packets")

QUICHE_FLAG(bool, FLAGS_quic_enforce_single_packet_chlo, false, false,
           "Enforce single packet CHLO")

QUICHE_FLAG(bool, FLAGS_quic_export_write_path_stats_at_server, false, false,
           "Export write path stats at server")

QUICHE_FLAG(bool, FLAGS_quiche_reloadable_flag_quic_no_write_control_frame_upon_connection_close2, false, false,
           "No write control frame upon connection close v2")

QUICHE_FLAG(bool, FLAGS_quic_reject_retry_token_in_initial_packet, false, false,
           "Reject retry token in initial packet")

QUICHE_FLAG(bool, FLAGS_quic_disable_client_tls_zero_rtt, false, false,
           "Disable client TLS 0-RTT")

#endif
// clang-format on
// NOLINTEND
