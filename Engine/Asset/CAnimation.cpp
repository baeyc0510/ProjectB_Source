#include "pch.h"
#include "JSON/inc/json.hpp"
#include "CAnimation.h"
#include <fstream>
#include <filesystem>

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
    if (!filesystem::exists(filePath)) return;

    ifstream file(filePath);
    if (!file.is_open()) return;

    json data;
    try { file >> data; }
    catch (const json::parse_error&) { file.close(); return; }
    file.close();

    // 1. 이미지 로드
    string imgPathStr = data["image"];
    this->image = LOADIMAGE(key, filesystem::path(imgPathStr));

    // 2. 옵션 설정
    float interval = 0.1f;
    if (data.contains("frame_interval")) interval = data["frame_interval"];

    // 3. 프레임 데이터 파싱
    const auto& jsonFrames = data["frames"];

    this->frames.clear();
    this->frames.reserve(jsonFrames.size());

    for (const auto& f : jsonFrames)
    {
        const auto& r = f["rect"]; // [x, y, w, h]

        AniFrame frame;
        frame.pos = Vec2(r[0], r[1]);
        frame.scale = Vec2(r[2], r[3]);
        frame.time = interval;

        // 이벤트 (추후 구현)
        if (f.contains("events"))
        {
            const auto& events = f["events"];
            for (const auto& e : events)
            {
                frame.events.push_back(ToWString(e.get<string>()));
            }
        }

        this->frames.push_back(frame);
    }

    this->repeat = true;
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
