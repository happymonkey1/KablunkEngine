#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Buffer.h"

#include "Kablunk/serialize/file/stream_reader.h"

#include <filesystem>
#include <fstream>

namespace kb::serialize::file
{ // start namespace kb::serialize::file

class file_stream_reader : public stream_reader
{
public:
    file_stream_reader() = default;
    file_stream_reader(const std::filesystem::path& p_path);
    file_stream_reader(const file_stream_reader&) = delete;
    virtual ~file_stream_reader();

    virtual bool is_good() const noexcept override { return m_stream.good(); };
    virtual u64 get_stream_cursor() noexcept { return m_stream.tellg(); }
    virtual void set_stream_cursor(u64 p_new_cursor) noexcept { m_stream.seekg(p_new_cursor); }
    virtual bool read_data_unsafe(char* p_destination_buffer, size_t p_size) noexcept override;

private:
    std::filesystem::path m_path{};
    std::ifstream m_stream{};
};

class file_stream_writer
{
    file_stream_writer() = default;
    file_stream_writer(const std::filesystem::path& p_path);
    file_stream_writer(const file_stream_writer&) = delete;
    virtual ~file_stream_writer();

    virtual void is_good() const noexcept override { return m_stream.good(); };
    virtual u64 get_stream_cursor() noexcept override { return m_stream.tellp(); }
    virtual void set_stream_cursor(u64 p_new_cursor) noexcept override { m_stream.seekp(p_new_cursor); }
    virtual bool write_data(const Buffer& p_source_buffer) noexcept override;
private:
    std::filesystem::path m_path{};
    std::ofstream m_stream{};
};

} // end namespace kb::serialize::file 
