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
	// �޽��� ���ڵ� ���� �Լ���
	//---------------------------------------------------------------
	void DecodeMessage(gisunnet::ByteBuf& read_buf)
	{
		// ó���� �����Ͱ� ������ �ݺ�
		while (read_buf.IsReadable())
		{
			// Decode MsgHeader
			if (!headerDecoded)
			{
				// ���ڵ尡 �����ϸ� ������.
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

	// ��� ���ڵ�
	bool DecodeHeader(gisunnet::ByteBuf& read_buf)
	{
		// ��� ������ ��ŭ ���� �������� ����
		if (!read_buf.IsReadable(HeaderSize))
		{
			return false;
		}

		// TO DO : ��� ����? ��ȣȭ?
		read_buf.Read(header.opCode);
		read_buf.Read(header.length);

		headerDecoded = true;
		return true;
	}

	// �ٵ� ���ڵ�
	bool DecodeBody(gisunnet::ByteBuf& read_buf)
	{
		if (!read_buf.IsReadable(header.length))
			return false;

		OnReceive(header, read_buf, read_buf.ReaderIndex(), header.length);

		// body ������ ��ŭ ReaderIndex ����.
		read_buf.SkipBytes(header.length);
		// ��� �ʱ�ȭ
		headerDecoded = false;
		header.opCode = 0;
		header.length = 0;

		return true;
	}

	// �ϼ��� �޽��� ó��
	void OnReceive(MsgHeader& header, const gisunnet::ByteBuf& body, int bodyIndex, int bodyLength)
	{
		// Deserialize

		//this.MessageCallback(message);
	}

	MsgHeader header;
	bool headerDecoded = false;
};