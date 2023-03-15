#pragma once

class SHA1Context
{
public:
	SHA1Context(size_t count, const std::vector<uint32_t>& tracks);

	std::string RFC822_Binary();

private:
	std::vector<uint8_t> GetDigest();
	uint32_t CircularShift(int bits, uint32_t word);
	void Input(size_t num, std::string_view format);
	void PadMessage();
	void ProcessMessageBlock();

	int16_t Message_Block_Index{};
	std::array<uint8_t, 64> Message_Block;
	std::array<uint32_t, 5> Intermediate_Hash = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
	uint32_t Length_High{};
	uint32_t Length_Low{};
};
