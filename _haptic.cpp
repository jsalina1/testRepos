/* START: This is this force feedback aka haptic section ---------------------------------------------------
SDL_Haptic* haptic;
printf("\nHaptic Count: %d\n", SDL_NumHaptics());
printf("Is Joystick Haptic? %d\n", SDL_JoystickIsHaptic(controller1));
haptic=SDL_HapticOpenFromJoystick(controller1); if (haptic==NULL) printf("%s\n\n", SDL_GetError());
//SDL_HapticRumbleInit(haptic);
//SDL_HapticRumblePlay(haptic, 0.5, 2000);
END: This is this force feedback aka haptic section --------------------------------------------------- */



