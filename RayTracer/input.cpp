#include "precomp.h"
#include "input.h"

Input::Input() :
	mKeyStates{false},
	mPrevKeyStates{false},
	mMouseState(false),
	mPrevMouseState(false),
	mMousePos(0)
{}

void Input::Update()
{
	if (mShouldUpdate)
	{
		memcpy(mPrevKeyStates, mKeyStates, INPUT_KEY_COUNT * sizeof(bool));
		mShouldUpdate = false;
		printf("input updated\n");  
	}
	mPrevMouseState = mMouseState;
}

bool Input::IsKeyDown(uint const key) const
{
	return mKeyStates[key & INPUT_KEY_COUNT - 1];
}

bool Input::IsKeyUp(uint const key) const
{
	return !mKeyStates[key & INPUT_KEY_COUNT - 1];
}

bool Input::IsKeyReleased(uint const key) const
{
	return mPrevKeyStates[key & INPUT_KEY_COUNT - 1] && !mKeyStates[key & INPUT_KEY_COUNT - 1]; 
}