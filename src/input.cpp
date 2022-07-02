#include "input.h"
#include "pinout.h"

Input::Input()
{
    btnMode = Button2(pins::button1, INPUT_PULLUP);
}