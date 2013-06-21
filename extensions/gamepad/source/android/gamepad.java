/*
java implementation of the gamepad extension.

Add android-specific functionality here.

These functions are called via JNI from native code.
*/
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
import android.view.InputDevice;
import com.ideaworks3d.marmalade.LoaderAPI;
import com.ideaworks3d.marmalade.LoaderActivity;
import java.util.ArrayList;
import android.view.MotionEvent;
import source.android.GamepadInfo;

class gamepad
{
	public final ArrayList<InputDevice> gamepad_devices;
	
	public gamepad()
	{
		int[] devices;

		gamepad_devices = new ArrayList<InputDevice>();

		GamepadInfo.Instance = new ArrayList<GamepadInfo>();

		devices = InputDevice.getDeviceIds();
		for (int i=0; i<devices.length; ++i)
		{
			InputDevice device = InputDevice.getDevice(devices[i]);
			int sources = device.getSources();
			if (((sources & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD)
				|| ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
				|| ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK))
			{
				gamepad_devices.add(device);
				GamepadInfo.Instance.add(new GamepadInfo(device));
			}
		}
	}
    public int gamepadGetNumDevices()
    {
        return gamepad_devices.size();
    }
    public int gamepadGetNumAxes(int index)
    {
	return GamepadInfo.Instance.get(index).GetNumAxes();
    }
    public int gamepadGetNumButtons(int index)
    {
        return 32;
    }
    public float gamepadGetAxis(int index, int axisIndex)
    {
        return GamepadInfo.Instance.get(index).axis[axisIndex];
    }
    public int gamepadGetButtons(int index)
    {
        return GamepadInfo.Instance.get(index).buttons;
    }
    public int gamepadGetDeviceId(int index)
    {
        return gamepad_devices.get(index).getSources();
    }
    public void gamepadInit()
    {
        
    }
    public void gamepadTerminate()
    {
        
    }
    public void gamepadUpdate()
    {
        
    }
}
