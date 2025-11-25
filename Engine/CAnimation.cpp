#include "pch.h"
#include <fstream>
#include "JSON/inc/json.hpp"
#include "StringHelper.h"
#include "CAnimation.h"

using json = nlohmann::json;

CAnimation::CAnimation()
{
	image	= nullptr;
	repeat	= false;
}

CAnimation::~CAnimation()
{
}

void CAnimation::Load(const wstring& key, const wstring& path)
{
    filesystem::path filePath(path);

    if (!filesystem::exists(filePath))
    {
        return;
    }

    // JSON 파일
    ifstream file(filePath);
    if (!file.is_open()) 
        return;

    json data;
    try 
    {
        file >> data;
    }
    catch (const json::parse_error&) 
    {
        file.close();
        return;
    }
    file.close();


    // 이미지 경로 (string -> wstring 변환)
    string imgPathStr = data["image"];
    wstring imgPath = ToWString(imgPathStr);

    // 프레임 간격 시간
    float interval = 0.1f; // 기본값
    if (data.contains("frame_interval")) 
    {
        interval = data["frame_interval"];
    }

    // 이미지 로드
	this->image = LOADIMAGE(key, imgPath);

    // 프레임 데이터 파싱
    const auto& jsonFrames = data["frames"];
    
    this->frames.clear();
    this->frames.reserve(jsonFrames.size());

    for (const auto& f : jsonFrames)
    {
        // JSON 구조: [x, y, w, h]
        float x = f[0];
        float y = f[1];
        float w = f[2];
        float h = f[3];

        AniFrame frame;
        frame.pos = Vec2(x, y);
        frame.scale = Vec2(w, h);
        frame.time = interval;

        this->frames.push_back(frame);
    }

    this->repeat = false;
}

void CAnimation::Create(CImage* image, float stepTime, UINT count, bool repeat, Vec2 pos, Vec2 scale, Vec2 step)
{
	this->image = image;
	this->repeat = repeat;

	AniFrame frame;
	for (UINT i = 0; i < count; i++)
	{
		frame.pos = pos + step * i;
		frame.scale = scale;
		frame.time = stepTime;

		frames.push_back(frame);
	}
}
