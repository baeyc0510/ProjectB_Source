#include "pch.h"
#include "ResourceManager.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

ImageResource* ResourceManager::ImageFind(const wstring& key)
{
	auto iter = images.find(key);
	if (iter == images.end())
		return nullptr;
	else
		return iter->second;
}

ImageResource* ResourceManager::ImageLoad(const wstring& key, const wstring& file)
{
	ImageResource* pImage = ImageFind(key);
	if (nullptr != pImage)
		return pImage;

	wstring filePath = resourceFolder + file;
	pImage = new ImageResource;
	pImage->Load(filePath);
	pImage->SetKey(key);
	pImage->SetPath(filePath);
	images.insert(make_pair(key, pImage));

	return pImage;
}

SoundResource* ResourceManager::SoundFind(const wstring& key)
{
	auto iter = sounds.find(key);
	if (iter == sounds.end())
		return nullptr;
	else
		return iter->second;
}

SoundResource* ResourceManager::SoundLoad(const wstring& key, const wstring& file)
{
	SoundResource* pSound = SoundFind(key);
	if (nullptr != pSound)
		return pSound;

	wstring filePath = resourceFolder + file;
	pSound = new SoundResource;
	pSound->Load(filePath);
	pSound->SetKey(key);
	pSound->SetPath(filePath);
	sounds.insert(make_pair(key, pSound));

	return pSound;
}

AnimationResource* ResourceManager::AnimationFind(const wstring& key)
{
	auto iter = animations.find(key);
	if (iter == animations.end())
		return nullptr;
	else
		return iter->second;
}

AnimationResource* ResourceManager::AnimationLoad(const wstring& key, const wstring file)
{
	AnimationResource* pAnimation = AnimationFind(key);
	if (nullptr != pAnimation)
		return pAnimation;

	wstring filePath = resourceFolder + file;
 	pAnimation = new AnimationResource;
	pAnimation->Load(key, filePath);
	
	animations.insert(make_pair(key, pAnimation));
	pAnimation->SetCached(true);
	
	return pAnimation;
}

void ResourceManager::Init()
{
}

void ResourceManager::Release()
{
	for (pair<wstring, ImageResource*> image : images)
	{
		delete image.second;
	}
	images.clear();
	for (pair<wstring, SoundResource*> sound : sounds)
	{
		delete sound.second;
	}
	sounds.clear();
}
