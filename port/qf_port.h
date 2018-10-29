#ifndef __mbed_qf_port_h
#define __mbed_qf_port_h

#define QF_EQUEUE_TYPE QEQueue
#define QF_THREAD_TYPE int

#include "mbed.h"

#define QF_OS_OBJECT_TYPE Queue<void*, 16>

#define QF_MAX_ACTIVE 32

#define QF_INT_DISABLE() \
    {}
#define QF_INT_ENABLE() \
    {}

#define QF_CRIT_ENTRY(stat_) \
    {}
#define QF_CRIT_EXIT(stat_) \
    {}

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

// extern Mutex QF_pThreadMutex_; // mutex for QF critical section

}  // namespace QP

#ifdef QP_IMPL
#define QACTIVE_EQUEUE_WAIT_(me_)                                 \
    while ((me_)->m_eQueue.m_frontEvt == static_cast<QEvt*>(0)) { \
        osEvent evt = (me_)->m_osObject.get();                    \
    }

#define QACTIVE_EQUEUE_SIGNAL_(me_)  \
    do {                             \
        (me_)->m_osObject.put(NULL); \
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