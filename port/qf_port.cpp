
#define QP_IMPL       // this is QP implementation
#include "qf_port.h"  // QF port
#include "qf_pkg.h"   // QF package-scope interface
#include "qassert.h"  // QP embedded systems-friendly assertions
#ifdef Q_SPY          // QS software tracing enabled?
#include "qs_port.h"  // include QS port
#else
#include "qs_dummy.h"  // disable the QS software tracing
#endif                 // Q_SPY

#include "mbed.h"

namespace QP {

Q_DEFINE_THIS_MODULE("qf_port")

/* Global objects ==========================================================*/

// Local objects *************************************************************
static bool is_running = false;  // flag indicating when QF is running
static Mutex startup_mutex;
static int tick_period_ms = 1000;    
static osPriority tick_priority = osPriorityNormal;
static events::EventQueue event_queue(2 * EVENTS_EVENT_SIZE);
    
//****************************************************************************
void QF::init(void) {
    // Init the startup mutex with the default non-recursive initializer
    // startup_mutex

    // Lock the startup mutex to block any active objects started before
    // calling QF::run()
    startup_mutex.lock();

    // clear the internal QF variables, so that the framework can (re)start
    // correctly even if the startup code is not called to clear the
    // uninitialized data (as is required by the C++ Standard).
    extern uint_fast8_t QF_maxPool_;
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&QF::timeEvtHead_[0], static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));

    tick_priority = osPriorityNormal;
}

//****************************************************************************
int_t QF::run(void) {
    
    onStartup();  // invoke startup callback

#if 0    
    // try to set the priority of the ticker thread
    struct sched_param sparam;
    sparam.sched_priority = l_tickPrio;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam) == 0) {
        // success, this application has sufficient privileges
    }
    else {
        // setting priority failed, probably due to insufficient privieges
    }
#endif
    
    printf("QF::run after onStartup();\r\n");
    
    // unlock the startup mutex to unblock any active objects started before
    // calling QF::run()
    startup_mutex.unlock();

    printf("QF::run after startup_mutex.unlock();\r\n");
        
    is_running = true;    
#if 0    
    while (is_running) {  // the clock tick loop...
        QF_onClockTick();  // clock tick callback (must call QF_TICK_X())
        wait((float) tick_period_ms/1000.0);
    }
#endif
    
#if 1    
    event_queue.call_every(tick_period_ms, QF_onClockTick);    
    // Process the event queue.
    event_queue.dispatch_forever();
#endif
    
    onCleanup();  // invoke cleanup callback
    
    // Destroy startup_mutex
    
    return static_cast<int_t>(0);  // return success
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio) {
    Q_REQUIRE_ID(300, ticksPerSec != static_cast<uint32_t>(0));
    tick_period_ms = 1000/ticksPerSec;
    tick_priority = static_cast<osPriority>(tickPrio);
}
//****************************************************************************
void QF::stop(void) {
    if (!is_running) {
        return;
    }    
    is_running = false;  // stop the loop in QF::run()
    event_queue.break_dispatch();
}
//............................................................................
void QF::thread_(QActive* act) {
    // block this thread until the startup mutex is unlocked from QF::run()
    startup_mutex.lock();
    startup_mutex.unlock();

    // loop until m_thread is cleared in QActive::stop()
    do {
        QEvt const* e = act->get_();  // wait for event
        act->dispatch(e);             // dispatch to the active object's state machine
        gc(e);                        // check if the event is garbage, and collect it if so
    } while (act->m_thread != static_cast<uint8_t>(0));

    QF::remove_(act);  // remove this object from the framework
    // pthread_cond_destroy(&act->m_osObject); // cleanup the condition variable
}
//............................................................................
// thread routine for all AOs
static void ao_thread(QActive* arg) {
    QF::thread_(arg);
    // return static_cast<void *>(0); // return success
}
//............................................................................
void QActive::start(uint_fast8_t prio,
                    QEvt const* qSto[],
                    uint_fast16_t qLen,
                    void* stkSto,
                    uint_fast16_t stkSize,
                    QEvt const* ie) {
    // p-threads allocate stack internally
    Q_REQUIRE_ID(600, stkSto == static_cast<void*>(0));

    // pthread_cond_init(&m_osObject, 0);

    m_eQueue.init(qSto, qLen);
    m_prio = static_cast<uint8_t>(prio);  // set the QF priority of this AO
    QF::add_(this);                       // make QF aware of this AO
    this->init(ie);                       // execute initial transition (virtual call)
       
    Thread* thread = new Thread();
    //!TODO thread->set_priority(static_cast<osPriority>(prio));
    thread->start(mbed::callback(&ao_thread, this));
    m_thread = static_cast<uint8_t>(1);
}
//............................................................................
void QActive::stop(void) {
    unsubscribeAll();
    m_thread = static_cast<uint8_t>(0);  // stop the QF::thread_() loop
}

}  // namespace QP

