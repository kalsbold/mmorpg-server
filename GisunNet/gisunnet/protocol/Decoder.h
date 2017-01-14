#pragma once

namespace gisunnet
{
namespace protocol
{



}
}

class Decoder
{
	void Decode();

	struct MsgHeader
	{
		uint8_t opCode;
		int16_t length;
	};

	const int HeaderSize = 3;

	//---------------------------------------------------------------
	// 메시지 디코드 관련 함수들
	//---------------------------------------------------------------
	void DecodeMessage(gisunnet::ByteBuf& read_buf)
	{
		// 처리할 데이터가 있으면 반복
		while (read_buf.IsReadable())
		{
			// Decode MsgHeader
			if (!headerDecoded)
			{
				// 디코드가 실패하면 나간다.
				if (!DecodeHeader(read_buf))
					break;
			}

			// Decode Body
			if (headerDecoded)
			{
				if (!DecodeBody(read_buf))
					break;
			}
		}
	}

	// 헤더 디코드
	bool DecodeHeader(gisunnet::ByteBuf& read_buf)
	{
		// 헤더 사이즈 만큼 받지 못했으면 리턴
		if (!read_buf.IsReadable(HeaderSize))
		{
			return false;
		}

		// TO DO : 헤더 검증? 복호화?
		read_buf.Read(header.opCode);
		read_buf.Read(header.length);

		headerDecoded = true;
		return true;
	}

	// 바디 디코드
	bool DecodeBody(gisunnet::ByteBuf& read_buf)
	{
		if (!read_buf.IsReadable(header.length))
			return false;

		OnReceive(header, read_buf, read_buf.ReaderIndex(), header.length);

		// body 사이즈 만큼 ReaderIndex 전진.
		read_buf.SkipBytes(header.length);
		// 헤더 초기화
		headerDecoded = false;
		header.opCode = 0;
		header.length = 0;

		return true;
	}

	// 완성된 메시지 처리
	void OnReceive(MsgHeader& header, const gisunnet::ByteBuf& body, int bodyIndex, int bodyLength)
	{
		// Deserialize

		//this.MessageCallback(message);
	}

	MsgHeader header;
	bool headerDecoded = false;
};