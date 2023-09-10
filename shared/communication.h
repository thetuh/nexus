#pragma once

#define COMMUNICATION_KEY 0x6969420

enum class operation : short
{
	read_memory,
	write_memory,
	get_module_base
};

struct communication_t
{
	uint64_t key = COMMUNICATION_KEY;
	operation operation;

	int pid;

	uintptr_t buffer;
	uintptr_t address;
	size_t size;
};