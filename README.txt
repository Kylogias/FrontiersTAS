Frontiers TAS. I believe it's very possible for the codebase as it currently is to be used for other games

Writing a TAS:
	TAS Support is very rudimentary at the moment. Each TAS consists of inputs. These may be
		press <BUTTON> - Press a button for 1 frame
		hold <BUTTON> - Hold a button until released
		toggle <BUTTON> - Toggle a button between held and released
		release <BUTTON> - Release a button. All buttons are automatically released at the end of TAS
		spam <BUTTON> - Alternate between button pressed and button unpressed each frame until released
		idle - Reset the state of the controller
		wait <FRAMES> - Wait for the specified number of frames, spamming and holding along the way
			The end of a line automatically appends a "wait 1" after it. Put "nodelay" at the end of the line to prevent this
		leftstick <X> <Y> - Tilt the left stick into the specified position
		rightstick <X> <Y> - Tilt the right stick into the specified position
	Each line may have multiple inputs, for example "press RB press LT"
	The buttons are
		A,  B,  X,  Y,
		U,  D,  L,  R,
		LT, RT, LB, RB
		ST, SE, LS, RS
	The buttons may have any characters after them, but they are case sensitive

Building the project:
	Good luck. I'd recommend a linux environment (like Windows Subsystem for Linux, on the Microsoft Store)
	Then you need to figure out how to install MinGW in said environment
	After that, run the "make" command