#include "kablunkpch.h"

#include "Kablunk/serialize/file/file_stream.h"

namespace kb::serialize::file
{ // start namespace kb::serialize::file


// =======================
//   file_stream_reader
// =======================

file_stream_reader::file_stream_reader(const std::filesystem::path& p_path)
    : m_stream{ p_path, std::ifstream::in | std::ifstream::binary }
{

}

file_stream_reader::~file_stream_reader()
{
    m_stream.close();
}

bool file_stream_reader::read_data_unsafe(char* p_destination, size_t p_size) noexcept
{
    m_stream.read(p_destination, p_size);
    return true;
}

// =======================

// ======================
//   file_stream_writer
// ======================

file_stream_writer::file_stream_writer(const std::filesystem::path& p_path)
    : m_stream{ p_path, std::ofstream::out | std::ifstream::binary }
{

}

file_stream_writer::~file_stream_writer()
{
    m_stream.close();
}

bool file_stream_writer::write_data(const char* p_source, size_t p_size) noexcept
{
    m_stream.write(p_source, p_size);
    return true;
}

} // end namespace kb::serialize::file
