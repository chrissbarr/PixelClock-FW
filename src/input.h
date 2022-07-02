#ifndef input_h
#define input_h

#include <Button2.h>

enum ButtonState {
    IDLE,
    PRESSED,
    HELD
};

struct ButtonStates {
    ButtonState mode;
    ButtonState up;
    ButtonState down;
    ButtonState select;
    ButtonState brightness;
};

class Input {
public:
    Input();

    void update() {};

    ButtonStates getStates() const { return {}; }

private:
    Button2 btnMode;
    Button2 btnUp;
    Button2 btnDown;
    Button2 btnSelect;
    Button2 btnBrightness;
};


#endif // input_h