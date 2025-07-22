
extern char input[21];
extern int index;
extern volatile int enter_pressed;

void keyboardHandler(struct InterruptRegisters *regs);
void initKeyboard();