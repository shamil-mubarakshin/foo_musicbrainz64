#include "stdafx.hpp"
#include "SHA1.hpp"

#pragma warning(disable : 4244)

SHA1Context::SHA1Context(size_t count, const std::vector<uint32_t>& tracks)
{
	Input(1, "%02X");
	Input(count, "%02X");

	for (const uint32_t track : tracks)
	{
		Input(track, "%08X");
	}
}

SHA1Context::Digest SHA1Context::GetDigest()
{
	PadMessage();
	Message_Block.fill(0);
	Length_Low = 0;
	Length_High = 0;

	auto view = std::views::iota(0U, 20U) | std::views::transform([this](auto&& index) { return Intermediate_Hash[index >> 2] >> 8 * (3 - (index & 0x03)); });
	return std::ranges::to<Digest>(view);
}

std::string SHA1Context::RFC822_Binary()
{
	static constexpr std::string_view v = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._";
	std::string ret;

	auto digest = GetDigest();
	uint8_t* d = digest.data();
	size_t s = digest.size();

	while (s)
	{
		ret += v[d[0] >> 2];
		ret += v[((d[0] << 4) + (--s ? (d[1] >> 4) : 0)) & 0x3f];
		ret += s ? v[((d[1] << 2) + (--s ? (d[2] >> 6) : 0)) & 0x3f] : '-';
		ret += s ? v[d[2] & 0x3f] : '-';
		if (s) s--;
		d += 3;
	}
	return ret;
}

uint32_t SHA1Context::CircularShift(int bits, uint32_t word)
{
	return word << bits | word >> (32 - bits);
}

void SHA1Context::Input(size_t num, std::string_view format)
{
	size_t length = std::stoul(std::string(format).substr(2, 1));
	char buffer[9]{};
	sprintf_s(buffer, sizeof(buffer), format.data(), num);
	const uint8_t* message_array = reinterpret_cast<const uint8_t*>(buffer);

	while (length--)
	{
		Message_Block[Message_Block_Index++] = (*message_array & 0xFF);
		Length_Low += 8;

		if (Message_Block_Index == 64)
		{
			ProcessMessageBlock();
		}

		message_array++;
	}
}

void SHA1Context::PadMessage()
{
	Message_Block[Message_Block_Index] = 0x80;
	std::fill(Message_Block.begin() + Message_Block_Index + 1, Message_Block.begin() + 56, 0);
	Message_Block[56] = Length_High >> 24;
	Message_Block[57] = Length_High >> 16;
	Message_Block[58] = Length_High >> 8;
	Message_Block[59] = Length_High;
	Message_Block[60] = Length_Low >> 24;
	Message_Block[61] = Length_Low >> 16;
	Message_Block[62] = Length_Low >> 8;
	Message_Block[63] = Length_Low;
	ProcessMessageBlock();
}

void SHA1Context::ProcessMessageBlock()
{
	static constexpr std::array<uint32_t, 4> K = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };

	auto view = std::views::iota(0U, 80U);
	auto [A, B, C, D, E] = Intermediate_Hash;
	std::array<uint32_t, 80> W{};
	uint32_t temp;

	for (const size_t i : view)
	{
		if (i < 16) W[i] = Message_Block[i * 4] << 24 | Message_Block[i * 4 + 1] << 16 | Message_Block[i * 4 + 2] << 8 | Message_Block[i * 4 + 3];
		else W[i] = CircularShift(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
	}

	for (const size_t i : view)
	{
		if (i < 20) temp = CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[i] + K[0];
		else if (i < 40) temp = CircularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[1];
		else if (i < 60) temp = CircularShift(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[i] + K[2];
		else temp = CircularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[3];

		E = D;
		D = C;
		C = CircularShift(30, B);
		B = A;
		A = temp;
	}

	Intermediate_Hash[0] += A;
	Intermediate_Hash[1] += B;
	Intermediate_Hash[2] += C;
	Intermediate_Hash[3] += D;
	Intermediate_Hash[4] += E;
	Message_Block_Index = 0;
}
