#include "stdafx.hpp"
#include "SHA1.hpp"

#pragma warning(disable : 4244)

namespace
{
	constexpr uint32_t SHA1CircularShift(int bits, uint32_t word)
	{
		return word << bits | word >> (32 - bits);
	}

	void SHA1ProcessMessageBlock(SHA1Context* context)
	{
		static constexpr std::array<uint32_t, 4> K = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };

		auto [A, B, C, D, E] = context->Intermediate_Hash;
		std::array<uint32_t, 80> W;
		uint32_t temp;

		for (size_t i = 0; i < 80; ++i)
		{
			if (i < 16) W[i] = context->Message_Block[i * 4] << 24 | context->Message_Block[i * 4 + 1] << 16 | context->Message_Block[i * 4 + 2] << 8 | context->Message_Block[i * 4 + 3];
			else W[i] = SHA1CircularShift(1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
		}

		for (size_t i = 0; i < 80; ++i)
		{
			if (i < 20) temp = SHA1CircularShift(5, A) + ((B & C) | ((~B) & D)) + E + W[i] + K[0];
			else if (i < 40) temp = SHA1CircularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[1];
			else if (i < 60) temp = SHA1CircularShift(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[i] + K[2];
			else temp = SHA1CircularShift(5, A) + (B ^ C ^ D) + E + W[i] + K[3];

			E = D;
			D = C;
			C = SHA1CircularShift(30, B);
			B = A;
			A = temp;
		}

		context->Intermediate_Hash[0] += A;
		context->Intermediate_Hash[1] += B;
		context->Intermediate_Hash[2] += C;
		context->Intermediate_Hash[3] += D;
		context->Intermediate_Hash[4] += E;
		context->Message_Block_Index = 0;
	}

	void SHA1PadMessage(SHA1Context* context)
	{
		context->Message_Block[context->Message_Block_Index] = 0x80;
		std::fill(context->Message_Block.begin() + context->Message_Block_Index + 1, context->Message_Block.begin() + 56, 0);
		context->Message_Block[56] = context->Length_High >> 24;
		context->Message_Block[57] = context->Length_High >> 16;
		context->Message_Block[58] = context->Length_High >> 8;
		context->Message_Block[59] = context->Length_High;
		context->Message_Block[60] = context->Length_Low >> 24;
		context->Message_Block[61] = context->Length_Low >> 16;
		context->Message_Block[62] = context->Length_Low >> 8;
		context->Message_Block[63] = context->Length_Low;
		SHA1ProcessMessageBlock(context);
	}
}

void SHA1Input(SHA1Context* context, const uint8_t* message_array, uint32_t length)
{
	while (length--)
	{
		context->Message_Block[context->Message_Block_Index++] = (*message_array & 0xFF);
		context->Length_Low += 8;

		if (context->Message_Block_Index == 64)
		{
			SHA1ProcessMessageBlock(context);
		}

		message_array++;
	}
}

void SHA1Result(SHA1Context* context, uint8_t Message_Digest[SHA1HashSize])
{
	SHA1PadMessage(context);
	context->Message_Block.fill(0);
	context->Length_Low = 0;
	context->Length_High = 0;

	for (uint32_t i = 0; i < SHA1HashSize; ++i)
	{
		Message_Digest[i] = context->Intermediate_Hash[i >> 2] >> 8 * (3 - (i & 0x03));
	}
}
