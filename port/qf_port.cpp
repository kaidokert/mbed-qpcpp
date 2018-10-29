
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
Mutex QF_pThreadMutex_;

// Local objects *************************************************************
static bool l_isRunning;  // flag indicating when QF is running
static Mutex l_startupMutex;

#if 0    
static struct timespec l_tick;
static int_t l_tickPrio;
enum { NANOSLEEP_NSEC_PER_SEC = 1000000000 }; // see NOTE05
#endif

//****************************************************************************
void QF::init(void) {
    // lock memory so we're never swapped out to disk
    // mlockall(MCL_CURRENT | MCL_FUTURE); // uncomment when supported

    // init the global mutex with the default non-recursive initializer
    // QF_pThreadMutex_

    // init the startup mutex with the default non-recursive initializer
    // l_startupMutex

    // lock the startup mutex to block any active objects started before
    // calling QF::run()
    l_startupMutex.lock();

    // clear the internal QF variables, so that the framework can (re)start
    // correctly even if the startup code is not called to clear the
    // uninitialized data (as is required by the C++ Standard).
    extern uint_fast8_t QF_maxPool_;
    QF_maxPool_ = static_cast<uint_fast8_t>(0);
    bzero(&QF::timeEvtHead_[0], static_cast<uint_fast16_t>(sizeof(QF::timeEvtHead_)));
    bzero(&active_[0], static_cast<uint_fast16_t>(sizeof(active_)));

#if 0    
    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC/100L; // default clock tick
    l_tickPrio = sched_get_priority_min(SCHED_FIFO); // default tick prio
#endif
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

    // unlock the startup mutex to unblock any active objects started before
    // calling QF::run()
    l_startupMutex.unlock();

    l_isRunning = true;
    while (l_isRunning) {  // the clock tick loop...
        QF_onClockTick();  // clock tick callback (must call QF_TICK_X())

        wait(0.01);

        // nanosleep(&l_tick, NULL); // sleep for the number of ticks, NOTE05
    }
    onCleanup();  // invoke cleanup callback
    // pthread_mutex_destroy(&l_startupMutex);
    // pthread_mutex_destroy(&QF_pThreadMutex_);
    return static_cast<int_t>(0);  // return success
}
//****************************************************************************
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio) {
    Q_REQUIRE_ID(300, ticksPerSec != static_cast<uint32_t>(0));
    // l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC / ticksPerSec;
    // l_tickPrio = tickPrio;
}
//****************************************************************************
void QF::stop(void) {
    l_isRunning = false;  // stop the loop in QF::run()
}
//............................................................................
void QF::thread_(QActive* act) {
    // block this thread until the startup mutex is unlocked from QF::run()
    l_startupMutex.lock();
    l_startupMutex.unlock();

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

#if 0    
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // SCHED_FIFO corresponds to real-time preemptive priority-based scheduler
    // NOTE: This scheduling policy requires the superuser privileges
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    // see NOTE04
    struct sched_param param;
    param.sched_priority = prio
                           + (sched_get_priority_max(SCHED_FIFO)
                              - QF_MAX_ACTIVE - 3);

    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // stack size not provided?
    if (stkSize == 0U) {
        stkSize = (uint_fast16_t)PTHREAD_STACK_MIN; // the minimum
    }
    pthread_t thread;
    if (pthread_create(&thread, &attr, &ao_thread, this) != 0) {

        // Creating the p-thread with the SCHED_FIFO policy failed.
        // Most probably this application has no superuser privileges,
        // so we just fall back to the default SCHED_OTHER policy
        // and priority 0.

        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        Q_ALLEGE(pthread_create(&thread, &attr, &ao_thread, this)== 0);
    }
    pthread_attr_destroy(&attr);
#endif

    Thread* thread = new Thread();
    thread->start(mbed::callback(&ao_thread, this));
    m_thread = static_cast<uint8_t>(1);
}
//............................................................................
void QActive::stop(void) {
    unsubscribeAll();
    m_thread = static_cast<uint8_t>(0);  // stop the QF::thread_() loop
}

}  // namespace QP

//****************************************************************************
// NOTE01:
// In Linux, the scheduler policy closest to real-time is the SCHED_FIFO
// policy, available only with superuser privileges. QF::run() attempts to set
// this policy as well as to maximize its priority, so that the ticking
// occurrs in the most timely manner (as close to an interrupt as possible).
// However, setting the SCHED_FIFO policy might fail, most probably due to
// insufficient privileges.
//
// NOTE02:
// On some Linux systems nanosleep() might actually not deliver the finest
// time granularity. For example, on some Linux implementations, nanosleep()
// could not block for shorter intervals than 20ms, while the underlying
// clock tick period was only 10ms. Sometimes, the select() system call can
// provide a finer granularity.
////............................................................................

// NOTE03:
// Any blocking system call, such as nanosleep() or select() system call can
// be interrupted by a signal, such as ^C from the keyboard. In this case this
// QF port breaks out of the event-loop and returns to main() that exits and
// terminates all spawned p-threads.
//
// NOTE04:
// According to the man pages (for pthread_attr_setschedpolicy) the only value
// supported in the Linux p-threads implementation is PTHREAD_SCOPE_SYSTEM,
// meaning that the threads contend for CPU time with all processes running on
// the machine. In particular, thread priorities are interpreted relative to
// the priorities of all other processes on the machine.
//
// This is good, because it seems that if we set the priorities high enough,
// no other process (or thread running within) can gain control over the CPU.
//
// However, QF limits the number of priority levels to QF_MAX_ACTIVE.
// Assuming that a QF application will be real-time, this port reserves the
// three highest Linux priorities for the ISR-like threads (e.g., the ticker,
// I/O), and the rest highest-priorities for the active objects.
//
// NOTE05:
// In some (older) Linux kernels, the POSIX nanosleep() system call might
// deliver only 2*actual-system-tick granularity. To compensate for this,
// you would need to reduce (by 2) the constant NANOSLEEP_NSEC_PER_SEC.
//
