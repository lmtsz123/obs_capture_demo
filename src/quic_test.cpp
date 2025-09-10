#include <iostream>
#include "quiche/common/platform/api/quiche_logging.h"
#include "quiche/common/quiche_endian.h"
#include "quiche/common/quiche_data_reader.h"
#include "quiche/common/quiche_data_writer.h"

void TestQuicheIntegration() {
    std::cout << "Testing QUICHE integration..." << std::endl;
    
    // 测试日志系统
    QUICHE_LOG(INFO) << "QUICHE logging system working!";
    
    // 测试字节序转换
    uint32_t value = 0x12345678;
    uint32_t network_value = quiche::QuicheEndian::HostToNet32(value);
    uint32_t host_value = quiche::QuicheEndian::NetToHost32(network_value);
    
    std::cout << "Original: 0x" << std::hex << value << std::endl;
    std::cout << "Network: 0x" << std::hex << network_value << std::endl;
    std::cout << "Back to host: 0x" << std::hex << host_value << std::endl;
    
    // 测试数据读写
    std::string buffer;
    buffer.resize(64);
    
    quiche::QuicheDataWriter writer(buffer.size(), &buffer[0]);
    writer.WriteUInt32(0x12345678);
    writer.WriteUInt16(0xABCD);
    
    quiche::QuicheDataReader reader(buffer.data(), writer.length());
    uint32_t read_uint32;
    uint16_t read_uint16;
    reader.ReadUInt32(&read_uint32);
    reader.ReadUInt16(&read_uint16);
    
    std::cout << "Written/Read uint32: 0x" << std::hex << read_uint32 << std::endl;
    std::cout << "Written/Read uint16: 0x" << std::hex << read_uint16 << std::endl;
    
    std::cout << "QUICHE integration test completed successfully!" << std::endl;
}