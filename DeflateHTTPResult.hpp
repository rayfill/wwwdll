#include <net/HTTPResult.hpp>
#include <util/SmartPointer.hpp>
#include <text/LexicalCast.hpp>
#include <zlib.h>
#include <stdexcept>
#include <cstdint>

typedef std::uint8_t byte;

class ZlibException : std::runtime_error
{
private:
	int errorCode;
	std::string message;

public:

	ZlibException(int code):
		std::runtime_error(""),
		errorCode(code),
		message("zlib process failed: ")
	{
		message + stringCast<int>(errorCode);
	}

	~ZlibException() throw()
	{}

	const char* what() const throw()
	{
		return message.c_str();
	}
};

struct ZlibContext
{
	z_stream z;
	bool is_text;
	bool has_crc;
	bool has_extra;
	bool has_filename;
	bool has_comment;

	ZlibContext():
		z(),
		is_text(false),
		has_crc(false),
		has_extra(false),
		has_filename(false),
		has_comment(false)
	{
		std::memset(&z, 0, sizeof(z));
		z.zalloc = Z_NULL;
		z.zfree = Z_NULL;
		z.opaque = Z_NULL;

		int result = inflateInit2(&z, -15);
		//int result = inflateInit(&z);
		if (Z_OK != result)
			throw ZlibException(result);
	}

	~ZlibContext()
	{
		int result = inflateEnd(&z);
		if (result != Z_OK)
			throw ZlibException(result);
	}

private:
	ZlibContext(const ZlibContext&);
	ZlibContext& operator=(const ZlibContext&);

};

template <size_t BufferSize = 4096>
class DeflateHTTPResult : public HTTPResult<BufferSize>
{
private:
	SmartPointer<ZlibContext> context;

	typedef HTTPResult<BufferSize> super_type;

	enum compress_mode {
		undecided,
		identity,
		compressed
	} mode;

public:
	DeflateHTTPResult():
		super_type(),
		context(new ZlibContext()),
		mode(undecided)
	{}

	DeflateHTTPResult(const DeflateHTTPResult& rhs):
		super_type(rhs),
		context(rhs.context),
		mode(rhs.mode)
	{}

	DeflateHTTPResult& operator=(const DeflateHTTPResult& rhs) 
	{
		super_type::operator=(rhs);
		context = rhs.context;
		mode = rhs.mode;

		return *this;
	}

	virtual ~DeflateHTTPResult() throw()
	{}

protected:
	struct GzipHeader
	{
		byte magicId1; // 0x1f
		byte magicId2; // 0x8b
		byte compressionMethod; // accept by 8(deflate) mode only.

		// reserve, reserve, reserve, fcomment, fname, fextra, fhcrc, ftext
		byte flags;

		int32_t modificationTime;
		
		byte extraFlags;
		byte operatingSystem;
		/**
		   0 FAT filesystem
		   1 Amiga
		   2 VMS (or OpenVMS)
		   3 Unix
		   4 VM/CMS
		   5 Atari OS
		   6 HPFS filesystem (OS/2, NT)
		   7 Macintosh
		   8 z-system
		   9 CP/M
		   10 TOPS-20
		   11 NTFS filesystem(NT)
		   12 QDOS
		   13 Acorn RISCOS
		   255 unknown
		 */
	};

	

	void parseGzipHeader(Socket& socket) const
	{
		GzipHeader header;
		socket.read(&header, 10);
		if (header.magicId1 != 0x1f ||
			header.magicId2 != 0x8b)
			throw std::runtime_error("invalid gzip format");

		this->context->is_text = header.flags & 1;
		this->context->has_crc = header.flags & 2;
		this->context->has_extra = header.flags & 4;
		this->context->has_filename = header.flags & 8;
		this->context->has_comment = header.flags & 16;
	}

	void skipHeader(Socket& socket) const
	{
		if (this->context->has_extra)
		{
			std::uint16_t extraLen = 0;
			socket.read(&extraLen, sizeof(extraLen));

			std::vector<char> dummy;
			dummy.resize(extraLen);
			socket.read(&dummy.front(), extraLen);
		}

		if (this->context->has_filename)
		{
			byte dummy;
			for (;;)
			{
				socket.read(&dummy, sizeof(dummy));
				if (dummy == 0)
					break;
			}
		}

		if (this->context->has_comment)
		{
			byte dummy;
			for (;;)
			{
				socket.read(&dummy, sizeof(dummy));
				if (dummy == 0)
					break;
			}
		}

		if (this->context->has_crc)
		{
			uint16_t crc16;
			socket.read(&crc16, sizeof(crc16));
		}
	}

	virtual void readResource(Socket& socket, const size_t totalSize)
	{
		unsigned char readBuffer[BufferSize];
		unsigned char outBuffer[BufferSize];

		size_t readSize = 0;
		size_t totalReadSize = 0;
		
		assert(z->avail_in == 0);
		assert(z->avail_out == 0);

		if (mode == undecided)
		{
			if (super_type::properties.get("Content-Encoding") == "gzip")
			{
				mode = compressed;
				parseGzipHeader(socket);
				skipHeader(socket);
			}
			else
				mode = identity;
		}

		while (totalSize > totalReadSize)
		{
			readSize = 
				socket.readWithTimeout(
					readBuffer, 
					(totalSize - totalReadSize) < BufferSize ?
					(totalSize - totalReadSize) : BufferSize);

			totalReadSize += readSize;

			context->z.next_in = readBuffer;
			context->z.avail_in = readSize;

			if (mode == compressed)
			{
				while (context->z.avail_in > 0)
				{
					context->z.next_out = outBuffer;
					context->z.avail_out = BufferSize;


					int result = inflate(&context->z, Z_SYNC_FLUSH);
					if (result == Z_STREAM_END)
					{
						if (this->context->has_crc)
						{
							int crc32;
							int modLen;
							socket.read(&crc32, sizeof(crc32));
							socket.read(&modLen, sizeof(modLen));
						}
						int decodeSize = BufferSize - context->z.avail_out;
						super_type::resource.insert(super_type::resource.end(),
									    outBuffer, outBuffer + decodeSize);
							
						break;
					}


					if (result != Z_OK)
						throw ZlibException(result);
			
					int decodeSize = BufferSize - context->z.avail_out;
					super_type::resource.insert(super_type::resource.end(),
						outBuffer, outBuffer + decodeSize);
				}
			}
			else
				super_type::resource.insert(super_type::resource.end(),
					readBuffer, readBuffer + readSize);
		}
	}
};
