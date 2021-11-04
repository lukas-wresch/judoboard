#include <windows.h>
#include <xinput.h>
#include "../include/core.h"
#include "../include/xcontroller.h"


//#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")

using namespace ZED;



bool XController::IsConnected() const
{
	XINPUT_STATE state;
	return XInputGetState(m_ID, &state) == ERROR_SUCCESS;
}



bool XController::Update()
{
	XINPUT_STATE state;
	XInputGetState(m_ID, &state);

	if (m_LastPacketNum == state.dwPacketNumber)
		return false;
	
	m_State.wButtons_Previous = m_State.wButtons;

	m_State.wButtons = state.Gamepad.wButtons;
	m_State.bLeftTrigger  = state.Gamepad.bLeftTrigger;
	m_State.bRightTrigger = state.Gamepad.bRightTrigger;
	m_State.sThumbLX = state.Gamepad.sThumbLX;
	m_State.sThumbLY = state.Gamepad.sThumbLY;
	m_State.sThumbRX = state.Gamepad.sThumbRX;
	m_State.sThumbRY = state.Gamepad.sThumbRY;
	return true;
}



bool XController::Vibrate(uint16_t LowFreq, uint16_t HighFreq)
{
	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed  = LowFreq;
	vibration.wRightMotorSpeed = HighFreq;

	return XInputSetState(m_ID, &vibration) == ERROR_SUCCESS;
}