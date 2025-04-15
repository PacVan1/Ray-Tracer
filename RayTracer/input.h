#pragma once

int constexpr INPUT_KEY_COUNT = 256; 

class Input
{
public:   
	bool mKeyStates[INPUT_KEY_COUNT];
	bool mPrevKeyStates[INPUT_KEY_COUNT];

	bool mMouseState;
	bool mPrevMouseState;
	int2 mMousePos;

	bool mShouldUpdate; 

public:
						Input(); 
	void				Update();

	[[nodiscard]] bool	IsKeyDown(uint const key) const;
	[[nodiscard]] bool	IsKeyUp(uint const key) const;
	[[nodiscard]] bool	IsKeyReleased(uint const key) const;

	[[nodiscard]] bool	IsMouseDown() const; 
	[[nodiscard]] bool	IsMouseUp() const; 
	[[nodiscard]] bool	IsMouseReleased() const; 
};

