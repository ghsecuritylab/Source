/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * gthread.c: posix thread system implementation
 * Copyright 1998 Sebastian Wilhelmi; University of Karlsruhe
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

/* The GMutex, GCond and GPrivate implementations in this file are some
 * of the lowest-level code in GLib.  All other parts of GLib (messages,
 * memory, slices, etc) assume that they can freely use these facilities
 * without risking recursion.
 *
 * As such, these functions are NOT permitted to call any other part of
 * GLib.
 *
 * The thread manipulation functions (create, exit, join, etc.) have
 * more freedom -- they can do as they please.
 */

#include "config.h"

#include "gthread.h"

#include "gthreadprivate.h"
#include "gslice.h"
#include "gmessages.h"
#include "gstrfuncs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

static void
g_thread_abort (gint         status,
                const gchar *function)
{
  fprintf (stderr, "GLib (gthread-posix.c): Unexpected error from C library during '%s': %s.  Aborting.\n",
           function, strerror (status));
  abort ();
}

/* {{{1 GMutex */

static pthread_mutex_t *
g_mutex_impl_new (void)
{
  pthread_mutexattr_t *pattr = NULL;
  pthread_mutex_t *mutex;
  gint status;

  mutex = malloc (sizeof (pthread_mutex_t));
  if G_UNLIKELY (mutex == NULL)
    g_thread_abort (errno, "malloc");

#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
  pthread_mutexattr_t attr;
  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
  pattr = &attr;
#endif

  if G_UNLIKELY ((status = pthread_mutex_init (mutex, pattr)) != 0)
    g_thread_abort (status, "pthread_mutex_init");

#ifdef PTHREAD_ADAPTIVE_MUTEX_NP
  pthread_mutexattr_destroy (&attr);
#endif

  return mutex;
}

static void
g_mutex_impl_free (pthread_mutex_t *mutex)
{
  pthread_mutex_destroy (mutex);
  free (mutex);
}

static pthread_mutex_t *
g_mutex_get_impl (GMutex *mutex)
{
  pthread_mutex_t *impl = mutex->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_mutex_impl_new ();
      if (!g_atomic_pointer_compare_and_exchange (&mutex->p, NULL, impl))
        g_mutex_impl_free (impl);
      impl = mutex->p;
    }

  return impl;
}


/**
 * g_mutex_init:
 * @mutex: an uninitialized #GMutex
 *
 * Initializes a #GMutex so that it can be used.
 *
 * This function is useful to initialize a mutex that has been
 * allocated on the stack, or as part of a larger structure.
 * It is not necessary to initialize a mutex that has been
 * statically allocated.
 *
 * |[
 *   typedef struct {
 *     GMutex m;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = g_new (Blob, 1);
 * g_mutex_init (&b->m);
 * ]|
 *
 * To undo the effect of g_mutex_init() when a mutex is no longer
 * needed, use g_mutex_clear().
 *
 * Calling g_mutex_init() on an already initialized #GMutex leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_mutex_init (GMutex *mutex)
{
  mutex->p = g_mutex_impl_new ();
}

/**
 * g_mutex_clear:
 * @mutex: an initialized #GMutex
 *
 * Frees the resources allocated to a mutex with g_mutex_init().
 *
 * This function should not be used with a #GMutex that has been
 * statically allocated.
 *
 * Calling g_mutex_clear() on a locked mutex leads to undefined
 * behaviour.
 *
 * Sine: 2.32
 */
void
g_mutex_clear (GMutex *mutex)
{
  g_mutex_impl_free (mutex->p);
}

/**
 * g_mutex_lock:
 * @mutex: a #GMutex
 *
 * Locks @mutex. If @mutex is already locked by another thread, the
 * current thread will block until @mutex is unlocked by the other
 * thread.
 *
 * <note>#GMutex is neither guaranteed to be recursive nor to be
 * non-recursive.  As such, calling g_mutex_lock() on a #GMutex that has
 * already been locked by the same thread results in undefined behaviour
 * (including but not limited to deadlocks).</note>
 */
void
g_mutex_lock (GMutex *mutex)
{
  gint status;

  if G_UNLIKELY ((status = pthread_mutex_lock (g_mutex_get_impl (mutex))) != 0)
    g_thread_abort (status, "pthread_mutex_lock");
}

/**
 * g_mutex_unlock:
 * @mutex: a #GMutex
 *
 * Unlocks @mutex. If another thread is blocked in a g_mutex_lock()
 * call for @mutex, it will become unblocked and can lock @mutex itself.
 *
 * Calling g_mutex_unlock() on a mutex that is not locked by the
 * current thread leads to undefined behaviour.
 */
void
g_mutex_unlock (GMutex *mutex)
{
  gint status;

  if G_UNLIKELY ((status = pthread_mutex_unlock (g_mutex_get_impl (mutex))) != 0)
    g_thread_abort (status, "pthread_mutex_unlock");
}

/**
 * g_mutex_trylock:
 * @mutex: a #GMutex
 *
 * Tries to lock @mutex. If @mutex is already locked by another thread,
 * it immediately returns %FALSE. Otherwise it locks @mutex and returns
 * %TRUE.
 *
 * <note>#GMutex is neither guaranteed to be recursive nor to be
 * non-recursive.  As such, calling g_mutex_lock() on a #GMutex that has
 * already been locked by the same thread results in undefined behaviour
 * (including but not limited to deadlocks or arbitrary return values).
 * </note>

 * Returns: %TRUE if @mutex could be locked
 */
gboolean
g_mutex_trylock (GMutex *mutex)
{
  gint status;

  if G_LIKELY ((status = pthread_mutex_trylock (g_mutex_get_impl (mutex))) == 0)
    return TRUE;

//  if G_UNLIKELY (status != EBUSY)
//    g_thread_abort (status, "pthread_mutex_trylock");

  return FALSE;
}

/* {{{1 GRecMutex */

static pthread_mutex_t *
g_rec_mutex_impl_new (void)
{
  pthread_mutexattr_t attr;
  pthread_mutex_t *mutex;

  mutex = g_slice_new (pthread_mutex_t);
  pthread_mutexattr_init (&attr);
  pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init (mutex, &attr);
  pthread_mutexattr_destroy (&attr);

  return mutex;
}

static void
g_rec_mutex_impl_free (pthread_mutex_t *mutex)
{
  pthread_mutex_destroy (mutex);
  g_slice_free (pthread_mutex_t, mutex);
}

static pthread_mutex_t *
g_rec_mutex_get_impl (GRecMutex *rec_mutex)
{
  pthread_mutex_t *impl = rec_mutex->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_rec_mutex_impl_new ();
      if (!g_atomic_pointer_compare_and_exchange (&rec_mutex->p, NULL, impl))
        g_rec_mutex_impl_free (impl);
      impl = rec_mutex->p;
    }

  return impl;
}

/**
 * g_rec_mutex_init:
 * @rec_mutex: an uninitialized #GRecMutex
 *
 * Initializes a #GRecMutex so that it can be used.
 *
 * This function is useful to initialize a recursive mutex
 * that has been allocated on the stack, or as part of a larger
 * structure.
 *
 * It is not necessary to initialise a recursive mutex that has been
 * statically allocated.
 *
 * |[
 *   typedef struct {
 *     GRecMutex m;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = g_new (Blob, 1);
 * g_rec_mutex_init (&b->m);
 * ]|
 *
 * Calling g_rec_mutex_init() on an already initialized #GRecMutex
 * leads to undefined behaviour.
 *
 * To undo the effect of g_rec_mutex_init() when a recursive mutex
 * is no longer needed, use g_rec_mutex_clear().
 *
 * Since: 2.32
 */
void
g_rec_mutex_init (GRecMutex *rec_mutex)
{
  rec_mutex->p = g_rec_mutex_impl_new ();
}

/**
 * g_rec_mutex_clear:
 * @rec_mutex: an initialized #GRecMutex
 *
 * Frees the resources allocated to a recursive mutex with
 * g_rec_mutex_init().
 *
 * This function should not be used with a #GRecMutex that has been
 * statically allocated.
 *
 * Calling g_rec_mutex_clear() on a locked recursive mutex leads
 * to undefined behaviour.
 *
 * Sine: 2.32
 */
void
g_rec_mutex_clear (GRecMutex *rec_mutex)
{
  g_rec_mutex_impl_free (rec_mutex->p);
}

/**
 * g_rec_mutex_lock:
 * @rec_mutex: a #GRecMutex
 *
 * Locks @rec_mutex. If @rec_mutex is already locked by another
 * thread, the current thread will block until @rec_mutex is
 * unlocked by the other thread. If @rec_mutex is already locked
 * by the current thread, the 'lock count' of @rec_mutex is increased.
 * The mutex will only become available again when it is unlocked
 * as many times as it has been locked.
 *
 * Since: 2.32
 */
void
g_rec_mutex_lock (GRecMutex *mutex)
{
  pthread_mutex_lock (g_rec_mutex_get_impl (mutex));
}

/**
 * g_rec_mutex_unlock:
 * @rec_mutex: a #GRecMutex
 *
 * Unlocks @rec_mutex. If another thread is blocked in a
 * g_rec_mutex_lock() call for @rec_mutex, it will become unblocked
 * and can lock @rec_mutex itself.
 *
 * Calling g_rec_mutex_unlock() on a recursive mutex that is not
 * locked by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_rec_mutex_unlock (GRecMutex *rec_mutex)
{
  pthread_mutex_unlock (rec_mutex->p);
}

/**
 * g_rec_mutex_trylock:
 * @rec_mutex: a #GRecMutex
 *
 * Tries to lock @rec_mutex. If @rec_mutex is already locked
 * by another thread, it immediately returns %FALSE. Otherwise
 * it locks @rec_mutex and returns %TRUE.
 *
 * Returns: %TRUE if @rec_mutex could be locked
 *
 * Since: 2.32
 */
gboolean
g_rec_mutex_trylock (GRecMutex *rec_mutex)
{
  if (pthread_mutex_trylock (g_rec_mutex_get_impl (rec_mutex)) != 0)
    return FALSE;

  return TRUE;
}

/* {{{1 GRWLock */

static pthread_rwlock_t *
g_rw_lock_impl_new (void)
{
  pthread_rwlock_t *rwlock;
  gint status;

  rwlock = malloc (sizeof (pthread_rwlock_t));
  if G_UNLIKELY (rwlock == NULL)
    g_thread_abort (errno, "malloc");

  if G_UNLIKELY ((status = pthread_rwlock_init (rwlock, NULL)) != 0)
    g_thread_abort (status, "pthread_rwlock_init");

  return rwlock;
}

static void
g_rw_lock_impl_free (pthread_rwlock_t *rwlock)
{
  pthread_rwlock_destroy (rwlock);
  free (rwlock);
}

static pthread_rwlock_t *
g_rw_lock_get_impl (GRWLock *lock)
{
  pthread_rwlock_t *impl = lock->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_rw_lock_impl_new ();
      if (!g_atomic_pointer_compare_and_exchange (&lock->p, NULL, impl))
        g_rw_lock_impl_free (impl);
      impl = lock->p;
    }

  return impl;
}

/**
 * g_rw_lock_init:
 * @rw_lock: an uninitialized #GRWLock
 *
 * Initializes a #GRWLock so that it can be used.
 *
 * This function is useful to initialize a lock that has been
 * allocated on the stack, or as part of a larger structure.  It is not
 * necessary to initialise a reader-writer lock that has been statically
 * allocated.
 *
 * |[
 *   typedef struct {
 *     GRWLock l;
 *     ...
 *   } Blob;
 *
 * Blob *b;
 *
 * b = g_new (Blob, 1);
 * g_rw_lock_init (&b->l);
 * ]|
 *
 * To undo the effect of g_rw_lock_init() when a lock is no longer
 * needed, use g_rw_lock_clear().
 *
 * Calling g_rw_lock_init() on an already initialized #GRWLock leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_rw_lock_init (GRWLock *rw_lock)
{
  rw_lock->p = g_rw_lock_impl_new ();
}

/**
 * g_rw_lock_clear:
 * @rw_lock: an initialized #GRWLock
 *
 * Frees the resources allocated to a lock with g_rw_lock_init().
 *
 * This function should not be used with a #GRWLock that has been
 * statically allocated.
 *
 * Calling g_rw_lock_clear() when any thread holds the lock
 * leads to undefined behaviour.
 *
 * Sine: 2.32
 */
void
g_rw_lock_clear (GRWLock *rw_lock)
{
  g_rw_lock_impl_free (rw_lock->p);
}

/**
 * g_rw_lock_writer_lock:
 * @rw_lock: a #GRWLock
 *
 * Obtain a write lock on @rw_lock. If any thread already holds
 * a read or write lock on @rw_lock, the current thread will block
 * until all other threads have dropped their locks on @rw_lock.
 *
 * Since: 2.32
 */
void
g_rw_lock_writer_lock (GRWLock *rw_lock)
{
  pthread_rwlock_wrlock (g_rw_lock_get_impl (rw_lock));
}

/**
 * g_rw_lock_writer_trylock:
 * @rw_lock: a #GRWLock
 *
 * Tries to obtain a write lock on @rw_lock. If any other thread holds
 * a read or write lock on @rw_lock, it immediately returns %FALSE.
 * Otherwise it locks @rw_lock and returns %TRUE.
 *
 * Returns: %TRUE if @rw_lock could be locked
 *
 * Since: 2.32
 */
gboolean
g_rw_lock_writer_trylock (GRWLock *rw_lock)
{
  if (pthread_rwlock_trywrlock (g_rw_lock_get_impl (rw_lock)) != 0)
    return FALSE;

  return TRUE;
}

/**
 * g_rw_lock_writer_unlock:
 * @rw_lock: a #GRWLock
 *
 * Release a write lock on @rw_lock.
 *
 * Calling g_rw_lock_writer_unlock() on a lock that is not held
 * by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_rw_lock_writer_unlock (GRWLock *rw_lock)
{
  pthread_rwlock_unlock (g_rw_lock_get_impl (rw_lock));
}

/**
 * g_rw_lock_reader_lock:
 * @rw_lock: a #GRWLock
 *
 * Obtain a read lock on @rw_lock. If another thread currently holds
 * the write lock on @rw_lock or blocks waiting for it, the current
 * thread will block. Read locks can be taken recursively.
 *
 * It is implementation-defined how many threads are allowed to
 * hold read locks on the same lock simultaneously.
 *
 * Since: 2.32
 */
void
g_rw_lock_reader_lock (GRWLock *rw_lock)
{
  pthread_rwlock_rdlock (g_rw_lock_get_impl (rw_lock));
}

/**
 * g_rw_lock_reader_trylock:
 * @rw_lock: a #GRWLock
 *
 * Tries to obtain a read lock on @rw_lock and returns %TRUE if
 * the read lock was successfully obtained. Otherwise it
 * returns %FALSE.
 *
 * Returns: %TRUE if @rw_lock could be locked
 *
 * Since: 2.32
 */
gboolean
g_rw_lock_reader_trylock (GRWLock *rw_lock)
{
  if (pthread_rwlock_tryrdlock (g_rw_lock_get_impl (rw_lock)) != 0)
    return FALSE;

  return TRUE;
}

/**
 * g_rw_lock_reader_unlock:
 * @rw_lock: a #GRWLock
 *
 * Release a read lock on @rw_lock.
 *
 * Calling g_rw_lock_reader_unlock() on a lock that is not held
 * by the current thread leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_rw_lock_reader_unlock (GRWLock *rw_lock)
{
  pthread_rwlock_unlock (g_rw_lock_get_impl (rw_lock));
}

/* {{{1 GCond */

static pthread_cond_t *
g_cond_impl_new (void)
{
  pthread_condattr_t attr;
  pthread_cond_t *cond;
  gint status;

  pthread_condattr_init (&attr);
#if defined (HAVE_PTHREAD_CONDATTR_SETCLOCK) && defined (CLOCK_MONOTONIC)
  pthread_condattr_setclock (&attr, CLOCK_MONOTONIC);
#endif

  cond = malloc (sizeof (pthread_cond_t));
  if G_UNLIKELY (cond == NULL)
    g_thread_abort (errno, "malloc");

  if G_UNLIKELY ((status = pthread_cond_init (cond, &attr)) != 0)
    g_thread_abort (status, "pthread_cond_init");

  pthread_condattr_destroy (&attr);

  return cond;
}

static void
g_cond_impl_free (pthread_cond_t *cond)
{
  pthread_cond_destroy (cond);
  free (cond);
}

static pthread_cond_t *
g_cond_get_impl (GCond *cond)
{
  pthread_cond_t *impl = cond->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_cond_impl_new ();
      if (!g_atomic_pointer_compare_and_exchange (&cond->p, NULL, impl))
        g_cond_impl_free (impl);
      impl = cond->p;
    }

  return impl;
}

/**
 * g_cond_init:
 * @cond: an uninitialized #GCond
 *
 * Initialises a #GCond so that it can be used.
 *
 * This function is useful to initialise a #GCond that has been
 * allocated as part of a larger structure.  It is not necessary to
 * initialise a #GCond that has been statically allocated.
 *
 * To undo the effect of g_cond_init() when a #GCond is no longer
 * needed, use g_cond_clear().
 *
 * Calling g_cond_init() on an already-initialised #GCond leads
 * to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_cond_init (GCond *cond)
{
  cond->p = g_cond_impl_new ();
}

/**
 * g_cond_clear:
 * @cond: an initialised #GCond
 *
 * Frees the resources allocated to a #GCond with g_cond_init().
 *
 * This function should not be used with a #GCond that has been
 * statically allocated.
 *
 * Calling g_cond_clear() for a #GCond on which threads are
 * blocking leads to undefined behaviour.
 *
 * Since: 2.32
 */
void
g_cond_clear (GCond *cond)
{
  g_cond_impl_free (cond->p);
}

/**
 * g_cond_wait:
 * @cond: a #GCond
 * @mutex: a #GMutex that is currently locked
 *
 * Atomically releases @mutex and waits until @cond is signalled.
 *
 * When using condition variables, it is possible that a spurious wakeup
 * may occur (ie: g_cond_wait() returns even though g_cond_signal() was
 * not called).  It's also possible that a stolen wakeup may occur.
 * This is when g_cond_signal() is called, but another thread acquires
 * @mutex before this thread and modifies the state of the program in
 * such a way that when g_cond_wait() is able to return, the expected
 * condition is no longer met.
 *
 * For this reason, g_cond_wait() must always be used in a loop.  See
 * the documentation for #GCond for a complete example.
 **/
void
g_cond_wait (GCond  *cond,
             GMutex *mutex)
{
  gint status;

  if G_UNLIKELY ((status = pthread_cond_wait (g_cond_get_impl (cond), g_mutex_get_impl (mutex))) != 0)
    g_thread_abort (status, "pthread_cond_wait");
}

/**
 * g_cond_signal:
 * @cond: a #GCond
 *
 * If threads are waiting for @cond, at least one of them is unblocked.
 * If no threads are waiting for @cond, this function has no effect.
 * It is good practice to hold the same lock as the waiting thread
 * while calling this function, though not required.
 */
void
g_cond_signal (GCond *cond)
{
  gint status;

  if G_UNLIKELY ((status = pthread_cond_signal (g_cond_get_impl (cond))) != 0)
    g_thread_abort (status, "pthread_cond_signal");
}

/**
 * g_cond_broadcast:
 * @cond: a #GCond
 *
 * If threads are waiting for @cond, all of them are unblocked.
 * If no threads are waiting for @cond, this function has no effect.
 * It is good practice to lock the same mutex as the waiting threads
 * while calling this function, though not required.
 */
void
g_cond_broadcast (GCond *cond)
{
  gint status;

  if G_UNLIKELY ((status = pthread_cond_broadcast (g_cond_get_impl (cond))) != 0)
    g_thread_abort (status, "pthread_cond_broadcast");
}

/**
 * g_cond_wait_until:
 * @cond: a #GCond
 * @mutex: a #GMutex that is currently locked
 * @end_time: the monotonic time to wait until
 *
 * Waits until either @cond is signalled or @end_time has passed.
 *
 * As with g_cond_wait() it is possible that a spurious or stolen wakeup
 * could occur.  For that reason, waiting on a condition variable should
 * always be in a loop, based on an explicitly-checked predicate.
 *
 * %TRUE is returned if the condition variable was signalled (or in the
 * case of a spurious wakeup).  %FALSE is returned if @end_time has
 * passed.
 *
 * The following code shows how to correctly perform a timed wait on a
 * condition variable (extended the example presented in the
 * documentation for #GCond):
 *
 * |[
 * gpointer
 * pop_data_timed (void)
 * {
 *   gint64 end_time;
 *   gpointer data;
 *
 *   g_mutex_lock (&data_mutex);
 *
 *   end_time = g_get_monotonic_time () + 5 * G_TIME_SPAN_SECOND;
 *   while (!current_data)
 *     if (!g_cond_wait_until (&data_cond, &data_mutex, end_time))
 *       {
 *         // timeout has passed.
 *         g_mutex_unlock (&data_mutex);
 *         return NULL;
 *       }
 *
 *   // there is data for us
 *   data = current_data;
 *   current_data = NULL;
 *
 *   g_mutex_unlock (&data_mutex);
 *
 *   return data;
 * }
 * ]|
 *
 * Notice that the end time is calculated once, before entering the
 * loop and reused.  This is the motivation behind the use of absolute
 * time on this API -- if a relative time of 5 seconds were passed
 * directly to the call and a spurious wakeup occurred, the program would
 * have to start over waiting again (which would lead to a total wait
 * time of more than 5 seconds).
 *
 * Returns: %TRUE on a signal, %FALSE on a timeout
 * Since: 2.32
 **/
gboolean
g_cond_wait_until (GCond  *cond,
                   GMutex *mutex,
                   gint64  end_time)
{
  struct timespec ts;
  gint status;

  ts.tv_sec = end_time / 1000000;
  ts.tv_nsec = (end_time % 1000000) * 1000;

  if ((status = pthread_cond_timedwait (g_cond_get_impl (cond), g_mutex_get_impl (mutex), &ts)) == 0)
    return TRUE;

  if G_UNLIKELY (status != ETIMEDOUT)
    g_thread_abort (status, "pthread_cond_timedwait");

  return FALSE;
}

/* {{{1 GPrivate */

/**
 * GPrivate:
 *
 * The #GPrivate struct is an opaque data structure to represent a
 * thread-local data key. It is approximately equivalent to the
 * pthread_setspecific()/pthread_getspecific() APIs on POSIX and to
 * TlsSetValue()/TlsGetValue() on Windows.
 *
 * If you don't already know why you might want this functionality,
 * then you probably don't need it.
 *
 * #GPrivate is a very limited resource (as far as 128 per program,
 * shared between all libraries). It is also not possible to destroy a
 * #GPrivate after it has been used. As such, it is only ever acceptable
 * to use #GPrivate in static scope, and even then sparingly so.
 *
 * See G_PRIVATE_INIT() for a couple of examples.
 *
 * The #GPrivate structure should be considered opaque.  It should only
 * be accessed via the <function>g_private_</function> functions.
 */

/**
 * G_PRIVATE_INIT:
 * @notify: a #GDestroyNotify
 *
 * A macro to assist with the static initialisation of a #GPrivate.
 *
 * This macro is useful for the case that a #GDestroyNotify function
 * should be associated the key.  This is needed when the key will be
 * used to point at memory that should be deallocated when the thread
 * exits.
 *
 * Additionally, the #GDestroyNotify will also be called on the previous
 * value stored in the key when g_private_replace() is used.
 *
 * If no #GDestroyNotify is needed, then use of this macro is not
 * required -- if the #GPrivate is declared in static scope then it will
 * be properly initialised by default (ie: to all zeros).  See the
 * examples below.
 *
 * |[
 * static GPrivate name_key = G_PRIVATE_INIT (g_free);
 *
 * // return value should not be freed
 * const gchar *
 * get_local_name (void)
 * {
 *   return g_private_get (&name_key);
 * }
 *
 * void
 * set_local_name (const gchar *name)
 * {
 *   g_private_replace (&name_key, g_strdup (name));
 * }
 *
 *
 * static GPrivate count_key;   // no free function
 *
 * gint
 * get_local_count (void)
 * {
 *   return GPOINTER_TO_INT (g_private_get (&count_key));
 * }
 *
 * void
 * set_local_count (gint count)
 * {
 *   g_private_set (&count_key, GINT_TO_POINTER (count));
 * }
 * ]|
 *
 * Since: 2.32
 **/

static pthread_key_t *
g_private_impl_new (GDestroyNotify notify)
{
  pthread_key_t *key;
  gint status;

  key = malloc (sizeof (pthread_key_t));
  if G_UNLIKELY (key == NULL)
    g_thread_abort (errno, "malloc");
  status = pthread_key_create (key, notify);
  if G_UNLIKELY (status != 0)
    g_thread_abort (status, "pthread_key_create");

  return key;
}

static void
g_private_impl_free (pthread_key_t *key)
{
  gint status;

  status = pthread_key_delete (*key);
  if G_UNLIKELY (status != 0)
    g_thread_abort (status, "pthread_key_delete");
  free (key);
}

static pthread_key_t *
g_private_get_impl (GPrivate *key)
{
  pthread_key_t *impl = key->p;

  if G_UNLIKELY (impl == NULL)
    {
      impl = g_private_impl_new (key->notify);
      if (!g_atomic_pointer_compare_and_exchange (&key->p, NULL, impl))
        {
          g_private_impl_free (impl);
          impl = key->p;
        }
    }

  return impl;
}

/**
 * g_private_get:
 * @key: a #GPrivate
 *
 * Returns the current value of the thread local variable @key.
 *
 * If the value has not yet been set in this thread, %NULL is returned.
 * Values are never copied between threads (when a new thread is
 * created, for example).
 *
 * Returns: the thread-local value
 */
gpointer
g_private_get (GPrivate *key)
{
  /* quote POSIX: No errors are returned from pthread_getspecific(). */
  return pthread_getspecific (*g_private_get_impl (key));
}

/**
 * g_private_set:
 * @key: a #GPrivate
 * @value: the new value
 *
 * Sets the thread local variable @key to have the value @value in the
 * current thread.
 *
 * This function differs from g_private_replace() in the following way:
 * the #GDestroyNotify for @key is not called on the old value.
 */
void
g_private_set (GPrivate *key,
               gpointer  value)
{
  gint status;

  if G_UNLIKELY ((status = pthread_setspecific (*g_private_get_impl (key), value)) != 0)
    g_thread_abort (status, "pthread_setspecific");
}

/**
 * g_private_replace:
 * @key: a #GPrivate
 * @value: the new value
 *
 * Sets the thread local variable @key to have the value @value in the
 * current thread.
 *
 * This function differs from g_private_set() in the following way: if
 * the previous value was non-%NULL then the #GDestroyNotify handler for
 * @key is run on it.
 *
 * Since: 2.32
 **/
void
g_private_replace (GPrivate *key,
                   gpointer  value)
{
  pthread_key_t *impl = g_private_get_impl (key);
  gpointer old;
  gint status;

  old = pthread_getspecific (*impl);
  if (old && key->notify)
    key->notify (old);

  if G_UNLIKELY ((status = pthread_setspecific (*impl, value)) != 0)
    g_thread_abort (status, "pthread_setspecific");
}

/* {{{1 GThread */

#define posix_check_err(err, name) G_STMT_START{			\
  int error = (err); 							\
  if (error)	 		 		 			\
    g_error ("file %s: line %d (%s): error '%s' during '%s'",		\
           __FILE__, __LINE__, G_STRFUNC,				\
           g_strerror (error), name);					\
  }G_STMT_END

#define posix_check_cmd(cmd) posix_check_err (cmd, #cmd)

typedef struct
{
  GRealThread thread;

  pthread_t system_thread;
  gboolean  joined;
  GMutex    lock;
} GThreadPosix;

void
g_system_thread_free (GRealThread *thread)
{
  GThreadPosix *pt = (GThreadPosix *) thread;

  if (!pt->joined)
    pthread_detach (pt->system_thread);

  g_mutex_clear (&pt->lock);

  g_slice_free (GThreadPosix, pt);
}

GRealThread *
g_system_thread_new (GThreadFunc   thread_func,
                     gulong        stack_size,
                     GError      **error)
{
  GThreadPosix *thread;
  pthread_attr_t attr;
  gint ret;

  thread = g_slice_new0 (GThreadPosix);

  posix_check_cmd (pthread_attr_init (&attr));

#ifdef HAVE_PTHREAD_ATTR_SETSTACKSIZE
  if (stack_size)
    {
#ifdef _SC_THREAD_STACK_MIN
      stack_size = MAX (sysconf (_SC_THREAD_STACK_MIN), stack_size);
#endif /* _SC_THREAD_STACK_MIN */
      /* No error check here, because some systems can't do it and
       * we simply don't want threads to fail because of that. */
      pthread_attr_setstacksize (&attr, stack_size);
    }
#endif /* HAVE_PTHREAD_ATTR_SETSTACKSIZE */

  ret = pthread_create (&thread->system_thread, &attr, (void* (*)(void*))thread_func, thread);

  posix_check_cmd (pthread_attr_destroy (&attr));

  if (ret == EAGAIN)
    {
      g_set_error (error, G_THREAD_ERROR, G_THREAD_ERROR_AGAIN, 
                   "Error creating thread: %s", g_strerror (ret));
      g_slice_free (GThreadPosix, thread);
      return NULL;
    }

  posix_check_err (ret, "pthread_create");

  g_mutex_init (&thread->lock);

  return (GRealThread *) thread;
}

/**
 * g_thread_yield:
 *
 * Causes the calling thread to voluntarily relinquish the CPU, so
 * that other threads can run.
 *
 * This function is often used as a method to make busy wait less evil.
 */
void
g_thread_yield (void)
{
  sched_yield ();
}

void
g_system_thread_wait (GRealThread *thread)
{
  GThreadPosix *pt = (GThreadPosix *) thread;

  g_mutex_lock (&pt->lock);

  if (!pt->joined)
    {
      posix_check_cmd (pthread_join (pt->system_thread, NULL));
      pt->joined = TRUE;
    }

  g_mutex_unlock (&pt->lock);
}

void
g_system_thread_exit (void)
{
  pthread_exit (NULL);
}

void
g_system_thread_set_name (const gchar *name)
{
#ifdef HAVE_SYS_PRCTL_H
#ifdef PR_SET_NAME
  prctl (PR_SET_NAME, name, 0, 0, 0, 0);
#endif
#endif
}

/* {{{1 Epilogue */
/* vim:set foldmethod=marker: */
