#include "qpcpp.h"
#include "bsp.h"
#include "blinky.h"

#include <stdio.h>

Q_DEFINE_THIS_FILE

#ifdef Q_SPY
#error Simple Blinky Application does not provide Spy build configuration
#endif

//............................................................................
void BSP_init(void) {
    printf("QP/C++ test: Simple Blinky example\r\n");
    printf("QP/C++ test: QP version: %s\r\n", QP_VERSION_STR);
}
//............................................................................
void BSP_ledOff(void) {
    printf("QP/C++ test: LED OFF\r\n");
}
//............................................................................
void BSP_ledOn(void) {
    printf("QP/C++ test: LED ON\r\n");
}

//............................................................................
void QF::onStartup(void) {
    printf("QF::onStartup\r\n");
    QF_setTickRate(BSP_TICKS_PER_SEC, osPriorityNormal);  // desired tick rate/prio
}
//............................................................................
void QF::onCleanup(void) {
    printf("QP/C++ test: Bye Bye!!!\r\n");
}
//............................................................................
void QP::QF_onClockTick(void) {
#if 0
    printf("QP/C++ test: tick\r\n");
#endif
    QF::TICK_X(0U, (void*)0);  // perform the QF clock tick processing
}
//............................................................................
extern "C" void Q_onAssert(char const* const module, int loc) {
    printf("QP/C++ test: Assertion failed in %s location %d\r\n", module, loc);
    QS_ASSERTION(module, loc, static_cast<uint32_t>(10000U));
}
