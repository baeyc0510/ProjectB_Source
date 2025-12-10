#pragma once

//========================================
//##		디자인 패턴 FlyWeight		##
//========================================
/*
	한개의 고유 데이터를 다른 객체들에서 공유하게 만들어 메모리 사용량을 줄임
	고유 데이터를 참조하여 사용하기에 불러오는 시간을 줄임
*/

class ImageResource;
class SoundResource;
class AnimationResource;

class ResourceManager : public SingleTon<ResourceManager>
{
	friend SingleTon<ResourceManager>;
private:
	ResourceManager();
	virtual ~ResourceManager();

public:
	void			Init();
	void			Release();

	ImageResource*			ImageFind(const wstring& key);
	ImageResource*			ImageLoad(const wstring& key, const wstring& file);
	
	AnimationResource*		AnimationFind(const wstring& key);
	AnimationResource*		AnimationLoad(const wstring& key, const wstring file);

	SoundResource*			SoundFind(const wstring& key);
	SoundResource*			SoundLoad(const wstring& key, const wstring& file);

	const wstring&	GetResourceFolder()							{ return resourceFolder; }
	void			SetResourceFolder(const wstring& folder)	{ this->resourceFolder = folder; }

private:
	unordered_map<wstring, ImageResource*>	images;
	unordered_map<wstring, SoundResource*>	sounds;
	unordered_map<wstring, AnimationResource*>	animations;

	wstring resourceFolder;
};

#define LOADIMAGE(key, file)	ResourceManager::GetInstance()->ImageLoad(key, file)
#define LOADSOUND(key, file)	ResourceManager::GetInstance()->SoundLoad(key, file)
#define LOADANIMATION(key, file)	ResourceManager::GetInstance()->AnimationLoad(key, file)