#ifndef __mbed_qf_port_h
#define __mbed_qf_port_h

#define QF_EQUEUE_TYPE        QEQueue
#define QF_THREAD_TYPE        int

#define QF_MAX_ACTIVE         32

#define QF_INT_DISABLE()      {}
#define QF_INT_ENABLE()       {}

#define QF_CRIT_ENTRY(stat_)  {}
#define QF_CRIT_EXIT(stat_)   {}

// Activate the QF ISR API
#define QF_ISR_API    1

#include "qep_port.h"  // QEP port
#include "qequeue.h"   // this QP port uses the native QF event queue
#include "qmpool.h"    // this QP port uses the native QF memory pool
#include "qf.h"        // QF platform-independent public interface


#ifdef QP_IMPL
    #define QACTIVE_EQUEUE_WAIT_(me_) \
        while (0) { \
        }

    #define QACTIVE_EQUEUE_SIGNAL_(me_) do { \
    } while (0)

    #define QF_SCHED_STAT_
    #define QF_SCHED_LOCK_(dummy) {}
    #define QF_SCHED_UNLOCK_()    {}

    #define QF_EPOOL_TYPE_  QMPool
    #define QF_EPOOL_INIT_(p_, poolSto_, poolSize_, evtSize_) \
        (p_).init((poolSto_), (poolSize_), (evtSize_))
    #define QF_EPOOL_EVENT_SIZE_(p_) \
        static_cast<uint_fast16_t>((p_).getBlockSize())
    #define QF_EPOOL_GET_(p_, e_, m_) \
        ((e_) = static_cast<QEvt *>((p_).get((m_))))
    #define QF_EPOOL_PUT_(p_, e_) ((p_).put(e_))

#endif /* ifdef QP_IMPL */

#endif