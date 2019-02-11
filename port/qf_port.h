#ifndef __mbed_qf_port_h
#define __mbed_qf_port_h

#include "mbed.h"

#define QF_EQUEUE_TYPE QEQueue
#define QF_THREAD_TYPE int

#define QF_OS_OBJECT_TYPE EventFlags

#define QF_MAX_ACTIVE 32

#if 0
#define QF_INT_DISABLE() \
    {}
#define QF_INT_ENABLE() \
    {}

#define QF_CRIT_ENTRY(stat_) \
    {}
#define QF_CRIT_EXIT(stat_) \
    {}
#endif

#define QF_INT_DISABLE() \
    { CriticalSectionLock::enable(); }
#define QF_INT_ENABLE() \
    { CriticalSectionLock::disable(); }

#if 0
// QF critical section
// #define QF_CRIT_STAT_TYPE not defined
#define QF_CRIT_ENTRY(stat_) { printf("QF_CRIT_ENTRY %s:%d\r\n", __FILE__, __LINE__); CriticalSectionLock::enable(); /* CriticalSectionLock::disable(); */ }
#define QF_CRIT_EXIT(stat_)  { CriticalSectionLock::disable(); printf("QF_CRIT_EXIT %s:%d\r\n", __FILE__, __LINE__); } 
#endif

#if 0
// QF critical section
#define QF_CRIT_STAT_TYPE Mutex
#define QF_CRIT_ENTRY(mutex_) mutex_.lock()
#define QF_CRIT_EXIT(mutex_)  mutex_.unlock()
#endif

class OneSlotSemaphore : public Semaphore {
   public:   
    OneSlotSemaphore() : Semaphore(1) {}
};

// QF critical section
#define QF_CRIT_STAT_TYPE OneSlotSemaphore
#define QF_CRIT_ENTRY(semaphore) semaphore.wait()
#define QF_CRIT_EXIT(semaphore)  semaphore.release()

// Activate the QF ISR API
//#define QF_ISR_API    1

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // this QP port uses the native QF event queue
#include "qmpool.h"    // this QP port uses the native QF memory pool
#include "qf.h"        // QF platform-independent public interface

namespace QP {

// set clock tick rate and p-thread priority
void QF_setTickRate(uint32_t ticksPerSec, int_t tickPrio);

// clock tick callback (provided in the app)
void QF_onClockTick(void);

}  // namespace QP

#ifdef QP_IMPL
#define QACTIVE_EQUEUE_WAIT_(me_)                                 \
    while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt*>(0)) { \
        (me_)->m_osObject.wait_all(0x1);                          \
    }

#define QACTIVE_EQUEUE_SIGNAL_(me_)  \
    do {                             \
        (me_)->m_osObject.set(0x1);  \
    } while (0)

// Mbed-specific scheduler locking (not used at this point)
#define QF_SCHED_STAT_
#define QF_SCHED_LOCK_(dummy) ((void)0)
#define QF_SCHED_UNLOCK_() ((void)0)

#define QF_EPOOL_TYPE_ QMPool
#define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) (p_).init((poolSto_), (poolSize_), (evtSize_))
#define QF_EPOOL_EVENT_SIZE_(p_) static_cast<uint_fast16_t>((p_).getBlockSize())
#define QF_EPOOL_GET_(p_, e_, m_) ((e_) = static_cast<QEvt*>((p_).get((m_))))
#define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif /* ifdef QP_IMPL */

#endif