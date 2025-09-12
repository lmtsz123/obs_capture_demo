#ifndef QUICHE_PLATFORM_IMPL_QUICHE_CLIENT_STATS_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_CLIENT_STATS_IMPL_H_

// 客户端统计实现的存根
// 这些宏在生产环境中通常用于收集性能指标

#define QuicheClientSparseHistogramImpl(name, sample, max) \
  do { \
    /* 空实现 - 在实际应用中这里会记录统计数据 */ \
  } while (0)

#define QUICHE_CLIENT_HISTOGRAM_ENUM_IMPL(name, sample, enum_size, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_SERVER_HISTOGRAM_ENUM_IMPL(name, sample, enum_size, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_CLIENT_HISTOGRAM_BOOL_IMPL(name, sample, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_SERVER_HISTOGRAM_BOOL_IMPL(name, sample, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_CLIENT_HISTOGRAM_TIMES_IMPL(name, sample, min, max, bucket_count, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_SERVER_HISTOGRAM_TIMES_IMPL(name, sample, min, max, bucket_count, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_CLIENT_HISTOGRAM_COUNTS_IMPL(name, sample, min, max, bucket_count, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#define QUICHE_SERVER_HISTOGRAM_COUNTS_IMPL(name, sample, min, max, bucket_count, docstring) \
  do { \
    /* 空实现 */ \
  } while (0)

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_CLIENT_STATS_IMPL_H_