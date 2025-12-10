#pragma once
#include <vector>

/*~ Animation Data ~*/
struct AnimationEntry
{
	const wchar_t* key;
	const wchar_t* path;
	bool repeat;
};

/*~ Sound Data ~*/
struct SoundEntry
{
	const wchar_t* key;
	const wchar_t* path;
};

/*~ VFX Data ~*/
struct VFXEntry
{
	const wchar_t* key;
	const wchar_t* path;
};
