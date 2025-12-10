#include "pch.h"
#include "PathManager.h"

PathManager::PathManager()
{
}

PathManager::~PathManager()
{
}

void PathManager::Init()
{
	WCHAR pathArray[255] = {};				// 경로를 받기 위한 문자열
	GetCurrentDirectory(255, pathArray);	// 현재 경로를 확인
	path = wstring(pathArray);				// 문자열을 wstring으로 변환

	Logger::Debug(TEXT("게임 경로"));
	Logger::Debug(path);
}

void PathManager::Release()
{
}

const wstring& PathManager::GetPath()
{
	return path;
}
