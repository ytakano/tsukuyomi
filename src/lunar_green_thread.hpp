#ifndef LUNAR_GREEN_THREAD_HPP
#define LUNAR_GREEN_THREAD_HPP

#include "lunar_common.hpp"
#include "lunar_spin_lock.hpp"
#include "lunar_shared_stream.hpp"
#include "lunar_ringq.hpp"
#include "lunar_shared_type.hpp"
#include "lunar_slab_allocator.hpp"

#ifdef __linux__
#include "hopscotch.hpp"
#endif // __linux__

#ifndef __linux__
#include "lunar_slub_stack.hpp"
#endif // __linux__

#include <unistd.h>
#include <setjmp.h>

#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

#if (defined(__unix__) || defined(unix) || (defined __APPLE__)) && !defined(USG)
#include <sys/param.h>
#endif

#if (defined BSD)
    #define KQUEUE
#elif (defined __linux__)
    #define EPOLL
#else
    #error unsupported platform!
#endif

#ifdef KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#elif (defined EPOLL)
#include <sys/epoll.h>
#endif // KQUEUE
/*
#define TIMESPECCMP(tvp, uvp, cmp)                  \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?             \
        ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :       \
        ((tvp)->tv_sec cmp (uvp)->tv_sec))
*/
#define TIMESPECADD(vvp, uvp)                       \
    do {                                            \
        (vvp)->tv_sec += (uvp)->tv_sec;             \
        (vvp)->tv_nsec += (uvp)->tv_nsec;           \
        if ((vvp)->tv_nsec >= 1000000000) {         \
            (vvp)->tv_sec++;                        \
            (vvp)->tv_nsec -= 1000000000;           \
        }                                           \
    } while (0)

#define TIMESPECSUB(vvp, uvp)                       \
    do {                                            \
        (vvp)->tv_sec -= (uvp)->tv_sec;             \
        (vvp)->tv_nsec -= (uvp)->tv_nsec;           \
        if ((vvp)->tv_nsec < 0) {                   \
            (vvp)->tv_sec--;                        \
            (vvp)->tv_nsec += 1000000000;           \
        }                                           \
    } while (0)

#ifdef __APPLE__
#define GETTIME(ts)                                 \
    do {                                            \
        timeval tv;                                 \
        gettimeofday(&tv, nullptr);                 \
        (ts)->tv_sec  = tv.tv_sec;                  \
        (ts)->tv_nsec = tv.tv_usec * 1000;          \
    } while (0)
#elif (defined BSD)
#define GETTIME(ts) clock_gettime(CLOCK_MONOTONIC_FAST, ts)
#elif (defined __linux__)
#define GETTIME(ts) clock_gettime(CLOCK_MONOTONIC_COARSE, ts)
#endif // __APPLE__

#ifdef KQUEUE
    #define FD_EV_READ             EVFILT_READ
    #define FD_EV_WRITE            EVFILT_WRITE
    #define FD_EV_VNODE            EVFILT_VNODE
    #define FD_EV_PROC             EVFILT_PROC
    #define FD_EV_SIGNAL           EVFILT_SIGNAL
    #define FD_EV_USER             EVFILT_USER
#ifdef __APPLE__
    #define FD_EV_MACHPORT         EVFILT_MACHPORT
#endif // __APPLE__

    // for read or write events
    #define FD_EV_FLAG_EOF         EV_EOF

    // for files
    #define FD_EV_FFLAG_DELETE     NOTE_DELETE
    #define FD_EV_FFLAG_WRITE      NOTE_WRITE
    #define FD_EV_FFLAG_EXTEND     NOTE_EXTEND
    #define FD_EV_FFLAG_ATTRIB     NOTE_ATTRIB
    #define FD_EV_FFLAG_LINK       NOTE_LINK
    #define FD_EV_FFLAG_RENAME     NOTE_RENAME
    #define FD_EV_FFLAG_REVOKE     NOTE_REVOKE

    // for processes
    #define FD_EV_FFLAG_EXIT       NOTE_EXIT
    #define FD_EV_FFLAG_FORK       NOTE_FORK
    #define FD_EV_FFLAG_EXEC       NOTE_EXEC
#ifdef __APPLE__
    #define FD_EV_FFLAG_EXITSTATUS NOTE_EXITSTATUS
    #define FD_EV_FFLAG_SIGNAL     NOTE_SIGNAL
    #define FD_EV_FFLAG_REAP       NOTE_REAP
#else
    #define FD_EV_FFLAG_TRAC       NOTE_TRACK
#endif // __APPLE__

#endif // KQUEUE

#ifdef EPOLL
    #define FD_EV_READ             EPOLLIN
    #define FD_EV_WRITE            EPOLLOUT

    // for read or write events
    #define FD_EV_FLAG_EOF         1

    // for files
    #define FD_EV_FFLAG_DELETE     0x0001
    #define FD_EV_FFLAG_WRITE      0x0002
    #define FD_EV_FFLAG_EXTEND     0x0004
    #define FD_EV_FFLAG_ATTRIB     0x0008
    #define FD_EV_FFLAG_LINK       0x0010
    #define FD_EV_FFLAG_RENAME     0x0020
    #define FD_EV_FFLAG_REVOKE     0x0040

    // for processes
    #define FD_EV_FFLAG_EXIT       0x0080
    #define FD_EV_FFLAG_FORK       0x0100
    #define FD_EV_FFLAG_EXEC       0x0200
    #define FD_EV_FFLAG_EXITSTATUS 0x0400
    #define FD_EV_FFLAG_SIGNAL     0x0800
    #define FD_EV_FFLAG_REAP       0x1000
    #define FD_EV_FFLAG_TRAC       0x2000
#endif // EPOLL

namespace lunar {

class green_thread;

extern "C" {
    uint64_t get_clock();
    bool init_green_thread(uint64_t thid, int qlen, int vecsize); // thid is user defined thread ID
    void schedule_green_thread();
    void spawn_green_thread(void (*func)(void*), void *arg = nullptr);
    void run_green_thread();
    uint64_t get_thread_id();
    void* get_green_thread(uint64_t thid);
    bool is_timeout_green_thread();

#ifdef KQUEUE
    void select_green_thread(struct kevent *kev, int num_kev,
                      void * const *stream, int num_stream,
                      bool is_threadq, int64_t timeout);
#elif (defined EPOLL)
    void select_green_thread(epoll_event *eev, int num_eev,
                      void * const *stream, int num_stream,
                      bool is_threadq, int64_t timeout);
#endif // KQUEUE

    void*       get_threadq_green_thread(uint64_t thid);
    STRM_RESULT push_threadq_green_thread(void *thq, char *p);
    STRM_RESULT pop_threadq_green_thread(char *p);
    STRM_RESULT push_stream_ptr(void *p, void *data);
    STRM_RESULT push_stream_bytes(void *p, char *data);
    STRM_RESULT pop_stream_ptr(void *p, void **data);
    STRM_RESULT pop_stream_bytes(void *p, char *data);
    void        push_stream_eof(void *p);

    struct fdevent_green_thread {
#ifdef KQUEUE
        uintptr_t fd;
        int16_t   event;
#elif (defined EPOLL)
        int       fd;
        uint32_t  event;
#endif // KQUEUE
        uint16_t  flags;
        uint32_t  fflags;
        intptr_t  data;
    };

    void get_streams_ready_green_thread(void ***streams, ssize_t *len);
    bool is_timeout_green_thread();
    bool is_ready_threadq_green_thread();
    void get_fds_ready_green_thread(fdevent_green_thread **events, ssize_t *len);
}

class green_thread {
public:
    green_thread(int qsize, int vecsize);
    virtual ~green_thread();

    void schedule();
    int  spawn(void (*func)(void*), void *arg = nullptr, int stack_size = 4096 * 50);
    void run();
    STRM_RESULT push_threadq(char *p) { return m_threadq->push(p); }
    STRM_RESULT pop_threadq(char *p) { return m_threadq->pop(p); }

    void* get_threadq()
    {
        incref_shared_type(m_threadq);
        return m_threadq;
    }

#ifdef KQUEUE
    void select_stream(struct kevent *kev, int num_kev,
                       void * const *stream, int num_stream,
                       bool is_threadq, int64_t timeout);
#elif (defined EPOLL)
    void select_stream(epoll_event *kev, int num_eev,
                       void * const *stream, int num_stream,
                       bool is_threadq, int64_t timeout);
#endif // KQUEUE

    template<typename T> STRM_RESULT pop_stream(shared_stream *p, T &ret);
    template<typename T> STRM_RESULT pop_streamN(shared_stream *p, T *ret);
    template<typename T> STRM_RESULT push_stream(shared_stream *p, T data);
    template<typename T> STRM_RESULT push_streamN(shared_stream *p, T *data);
    template<typename T> void        push_eof_stream(shared_stream *p);

    struct ev_key {
#ifdef KQUEUE
        uintptr_t m_fd;
        int16_t   m_event;
#elif (defined EPOLL)
        int       m_fd;
        uint32_t  m_event;
#endif // KQUEUE

#ifdef KQUEUE
        ev_key(uintptr_t fd, int16_t event) : m_fd(fd), m_event(event) { }
#elif (defined EPOLL)
        ev_key(int fd, uint32_t event) : m_fd(fd), m_event(event) { }
#endif // KQUEUE

        bool operator== (const ev_key &rhs) const {
            return (m_fd == rhs.m_fd) && (m_event == rhs.m_event);
        }
    };

    struct ev_key_hasher {
        std::size_t operator()(const ev_key& k) const {
            using std::size_t;
            using std::hash;
            using std::string;

            return hash<uintptr_t>()(k.m_fd) ^ ((std::size_t)hash<int16_t>()(k.m_event) << 16);
        }
    };

    struct event_data {
        uint16_t m_flags;
        uint32_t m_fflags;
        intptr_t m_data;

        event_data(uint16_t flags, uint32_t fflags, intptr_t data)
            : m_flags(flags), m_fflags(fflags), m_data(data) { }
    };

    // get functions for invoked events
    void get_fds_ready(fdevent_green_thread **events, ssize_t *len) {
        *events = &m_running->m_events[0];
        *len    = m_running->m_events.size();
    }

    void get_streams_ready(void ***streams, ssize_t *len) {
        *streams = &m_running->m_ev_stream[0];
        *len    =   m_running->m_ev_stream.size();
    }

    bool is_timeout() { return m_running->m_is_ev_timeout; }
    bool is_ready_threadq() { return m_running->m_is_ev_thq; }

private:
    struct context {
        // states of contexts
        static const int READY           = 0x0001;
        static const int RUNNING         = 0x0002;
        static const int SUSPENDING      = 0x0004;
        static const int WAITING_FD      = 0x0008;
        static const int WAITING_STREAM  = 0x0010;
        static const int WAITING_THQ     = 0x0020;
        static const int WAITING_TIMEOUT = 0x0040;
        static const int STOP            = 0x0080;

        uint32_t   m_state;
        sigjmp_buf m_jmp_buf;

        // waiting events
        std::vector<ev_key> m_fd;       // waiting file descriptors to read
        std::vector<void*>  m_stream;   // waiting streams to read

        // invoked events
        std::vector<void*> m_ev_stream; // streams ready to read
        std::vector<fdevent_green_thread> m_events; // file descriptors ready to read
        bool m_is_ev_thq;     // is the thread queue ready to read
        bool m_is_ev_timeout; // is timeout

        int64_t m_id; // m_id must not be less than or equal to 0
        uint64_t *m_stack;
        int m_stack_size;
    };

    struct ctx_time {
        uint64_t  m_clock;
        context  *m_ctx;

        ctx_time(uint64_t clock, context *ctx) : m_clock(clock), m_ctx(ctx) { }
    };

    typedef boost::multi_index::multi_index_container<
        ctx_time,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_non_unique<
                boost::multi_index::tag<>,
                boost::multi_index::member<ctx_time, uint64_t, &ctx_time::m_clock>>,
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<>,
                boost::multi_index::member<ctx_time, context*, &ctx_time::m_ctx>>
        >
    > timeout_t;

    sigjmp_buf m_jmp_buf;
    int64_t    m_count;
    context*   m_running;
    context*   m_wait_thq;
    timeout_t  m_timeout;
    std::deque<context*> m_suspend;
    std::deque<context*> m_stop;
    std::unordered_map<int64_t, std::unique_ptr<context>,
                       std::hash<int64_t>,
                       std::equal_to<int64_t>,
                       lunar::slab_allocator<std::pair<const int64_t, std::unique_ptr<context>>>> m_id2context;

#ifdef __linux__
    nanahan::Map<ev_key,
                 std::unordered_set<context*>,
                 ev_key_hasher, std::equal_to<ev_key>,
                 lunar::slab_allocator<std::pair<const ev_key, std::unordered_set<context*>>>> m_wait_fd;
    nanahan::Map<void*,
                 context*,
                 std::hash<void*>,
                 std::equal_to<void*>,
                 lunar::slab_allocator<std::pair<void * const, context*>>> m_wait_stream;
#else
    std::unordered_map<ev_key,
                       std::unordered_set<context*>,
                       ev_key_hasher, std::equal_to<ev_key>,
                       lunar::slab_allocator<std::pair<const ev_key, std::unordered_set<context*>>>> m_wait_fd;
    std::unordered_map<void*,
                       context*,
                       std::hash<void*>,
                       std::equal_to<void*>,
                       lunar::slab_allocator<std::pair<void * const, context*>>> m_wait_stream;
#endif // __linux__

    // for circular buffer
    class threadq {
    public:
        enum qwait_type {
            QWAIT_COND,
            QWAIT_PIPE,
            QWAIT_NONE,
        };

        threadq(int qsize, int vecsize);
        virtual ~threadq();

        inline STRM_RESULT push(char *p) {
            if (m_qlen == m_max_qlen)
                return STRM_NO_VACANCY;
            else if (m_is_closed)
                return STRM_CLOSED;

            spin_lock_acquire_unsafe lock(m_qlock);

            if (m_qlen == m_max_qlen)
                return STRM_NO_VACANCY;

            for (int i = 0; i < m_vecsize; i++)
                m_qtail[i] = p[i];

            m_qlen++;
            m_qtail += m_vecsize;

            if (m_qtail == m_qend) {
                m_qtail = m_q;
            }

            if (! m_is_qnotified) {
                m_is_qnotified = true;
                if (m_qwait_type == QWAIT_COND) {
                    lock.unlock();
                    std::unique_lock<std::mutex> mlock(m_qmutex);
                    m_qcond.notify_one();
                } else {
                    lock.unlock();
                    char c = '\0';
                    if (write(m_qpipe[1], &c, sizeof(c)) < 0) {
                        PRINTERR("could not write data to pipe");
                        exit(-1);
                    }
                }

                return STRM_SUCCESS;
            }

            lock.unlock();

            return STRM_SUCCESS;
        }

        inline STRM_RESULT pop(char *p) {
            int n = 0;
            while (m_qlen == 0) {
                if (n++ > 1000)
                    return STRM_NO_MORE_DATA;
            }

            for (int i = 0; i < m_vecsize; i++)
                p[i] = m_qhead[i];

            {
                spin_lock_acquire lock(m_qlock);
                m_qlen--;
            }

            m_qhead += m_vecsize;

            if (m_qhead == m_qend) {
                m_qhead = m_q;
            }

            return STRM_SUCCESS;
        }

        int get_len() { return m_qlen; }
        int get_read_fd() { return m_qpipe[0]; }
        qwait_type get_wait_type() { return m_qwait_type; }
        void set_wait_type(qwait_type t) { m_qwait_type = t; }

        void pop_pipe(ssize_t len) {
            char buf[16];
            ssize_t n;
            do {
                n = read(m_qpipe[0], buf, sizeof(buf));
                if (n < 0) {
                    if (errno == EINTR)
                        continue;
                    else if (errno == EAGAIN)
                        break;

                    PRINTERR("could not read data from pipe");
                    exit(-1);
                }

                assert(n != 0);
                assert(n <= len);

                len -= n;
            } while (len > 0);
        }

    private:
        volatile int  m_qlen;
        volatile bool m_is_qnotified;
        volatile qwait_type m_qwait_type;
        int   m_max_qlen;
        int   m_vecsize;
        char *m_q;
        char *m_qend;
        char *m_qhead;
        char *m_qtail;
        int   m_qpipe[2];
        volatile bool m_is_closed;
        spin_lock  m_qlock;
        std::mutex m_qmutex;
        std::condition_variable m_qcond;

        friend void green_thread::schedule();
    };

    threadq *m_threadq;

#ifdef KQUEUE
    int m_kq;
#elif (defined EPOLL)
    int m_epoll;
#endif // KQUEUE
    void select_fd(bool is_block);
    void resume_timeout();
    void remove_stopped();

#ifndef __linux__
    slub_stack m_slub_stack;
#endif // __linux__

    int m_pagesize;

    friend void spawn_green_thread(void (*func)(void*), void *arg);

    friend STRM_RESULT push_threadq_green_thread(void *thq, char *p);
};

}

#endif // LUNAR_GREEN_THREAD_HPP