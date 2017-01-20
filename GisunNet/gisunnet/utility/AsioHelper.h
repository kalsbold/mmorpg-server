#pragma once

#include <boost/asio.hpp>

using namespace boost;

namespace gisunnet {

// Get a buffer that asio input sequence.
template <typename BufferType>
asio::const_buffers_1 const_buffer(const BufferType& buf)
{
	return asio::buffer(boost::asio::const_buffer(buf.Data() + buf.ReaderIndex(), buf.ReadableBytes()));
}

// Get a buffer that asio output sequence.
template <typename BufferType>
asio::mutable_buffers_1 mutable_buffer(BufferType& buf)
{
	return asio::buffer(asio::mutable_buffer(buf.Data() + buf.WriterIndex(), buf.WritableBytes()));
}

} // namespace gisunnet