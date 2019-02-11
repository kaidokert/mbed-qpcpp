//****************************************************************************
// Product: Simple Blinky example
// Last Updated for Version: 5.4.0
// Date of the Last Update:  2015-05-04
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Web  : https://state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "bsp.h"
#include "blinky.h"

void qpcpp_test() {
    static QEvt const* blinkyQSto[10];  // Event queue storage for Blinky

    printf("QP/C++ test: Hello!\r\n");

    BSP_init();  // initialize the Board Support Package
    printf("QP/C++ test: after BSP_init()\r\n");

    QF::init();  // initialize the framework and the underlying RT kernel
    printf("QP/C++ test: after QF::init()\r\n");

    // publish-subscribe not used, no call to QF::psInit()
    // dynamic event allocation not used, no call to QF::poolInit()

    // instantiate and start the active objects...
    AO_Blinky->start(osPriorityNormal,               // priority
                     blinkyQSto, Q_DIM(blinkyQSto),  // event queue
                     (void*)0, 0U);                  // stack (unused)

    printf("QP/C++ test: before QF::run()\r\n");
    QF::run();  // run the QF application
}
