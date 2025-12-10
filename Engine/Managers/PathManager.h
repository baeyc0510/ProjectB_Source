#pragma once
class PathManager : public SingleTon<PathManager>
{
	friend SingleTon<PathManager>;
private:
	PathManager();
	virtual ~PathManager();

public:
	void Init();
	void Release();

	const wstring& GetPath();

private:
	wstring path;
};

#define PATH			PathManager::GetInstance()->GetPath()
