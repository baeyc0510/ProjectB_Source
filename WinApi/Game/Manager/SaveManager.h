#pragma once

struct CheckpointSaveData
{
	int checkpointID = -1;
	Vec2 position;
	int sceneType = -1;

	bool IsValid() const { return checkpointID != -1; }
	void Clear() { checkpointID = -1; position = Vec2(0, 0); sceneType = -1; }
};

class SaveManager : public SingleTon<SaveManager>
{
	friend SingleTon<SaveManager>;

private:
	SaveManager() = default;
	virtual ~SaveManager() = default;

private:
	CheckpointSaveData lastCheckpoint;

public:
	void SaveCheckpoint(int checkpointID, Vec2 pos, int sceneType)
	{
		lastCheckpoint.checkpointID = checkpointID;
		lastCheckpoint.position = pos;
		lastCheckpoint.sceneType = sceneType;
	}

	const CheckpointSaveData& GetLastCheckpoint() const { return lastCheckpoint; }
	bool HasCheckpoint() const { return lastCheckpoint.IsValid(); }

	void ClearCheckpoint() { lastCheckpoint.Clear(); }

	void NewGame()
	{
		ClearCheckpoint();
	}
};

#define SAVE SaveManager::GetInstance()
