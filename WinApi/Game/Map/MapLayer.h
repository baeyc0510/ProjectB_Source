#pragma once

class ImageResource;

class MapLayer
{
public:
	MapLayer();
	~MapLayer();

	void Load(const wstring& imagePath, float parallaxFactor = 1.0f, Vec2 offsetValue = Vec2(0, 0), bool transparent = true);
	void Release();

	void Render(Vec2 cameraPos);

	// 투명 처리 설정
	bool IsTransparent() const { return useTransparent; }
	void SetTransparent(bool value) { useTransparent = value; }

	// Getter/Setter
	float GetParallax() const { return parallax; }
	void SetParallax(float value) { parallax = value; }


	Vec2 GetOffset() const { return offset; }
	void SetOffset(Vec2 value) { offset = value; }

	UINT GetWidth() const;
	UINT GetHeight() const;

	// 이미지 접근 (캐시 빌드용)
	ImageResource* GetImage() const { return layerImage; }

private:
	ImageResource* layerImage;
	float parallax;		// 1.0 = 카메라와 동일, < 1.0 = 느리게 (뒤), > 1.0 = 빠르게 (앞)
	Vec2 offset;		// 이미지 오프셋 (픽셀 단위)
	bool useTransparent;	// true: TransparentBlt, false: BitBlt/StretchBlt (성능 향상)
};
