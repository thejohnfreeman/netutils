#include <err.h>

#include "jobs.h"

void
qmap_ctor(struct qmap* qm, int num, int size) {
  qm->threads  = NULL;
  qm->nthreads = 0;
  pcq_ctor(qm->q, num, size);
  pthread_mutex_init(&qm->mutex, /*attr=*/NULL);
}

void
qmap_map(struct qmap* qm, void* it, struct iter_vtable* vt, int nworkers,
    bool (*f)(void* in, void* out))
{
  qm->threads  = (pthread_t*)malloc(nworkers * sizeof(pthread_t));
  qm->nthreads = nworkers;

  int error;
  for (int i = 0; i < nworkers; ++i) {
    if ((error = pthread_create(qm->threads + i, /*attr=*/NULL, &worker,
            &wargs)))
    {
      errc(error, error, "could not create thread");
    }
  }
}

bool
qmap_empty(struct qmap* qm) {
  return pcq_empty(&qm->q);
}

void
qmap_pop(struct qmap* qm, void* out) {
  pcq_pop(&qm->q, out);
}

void
qmap_dtor(struct qmap* qm) {
  free(qm->threads);
}

#endif


struct worker_args {
  void*               it;
  struct iter_vtable* vt;
  struct pcq_queue*   q;
  bool (*f)(void* in, void* out);
  pthread_mutex_t*    mutex;
  int                 nworkers_alive;
  pthread_cond_t*     no_workers_alive;
};

void* worker(void* vargs) {
  struct worker_args* args = (struct worker_args*)vargs;
  void*               it   = args->it;
  struct iter_vtable* vt   = args->vt;
  struct pcq_queue*   q    = args->q;

  struct pcq_token tok;
  void*            out = malloc(pcq_sizeoft(q));

  while (1) {
    /* Allocation should be optimized out of loop. */
    void* in = NULL;

    /* Get the next piece of input, if it exists. */
    pthread_mutex_lock(args->mutex);
    if (!q->is_closing && vt->is_not_end(it)) {
      tok = pcq_claim(q);
      in  = vt->get(it);
    }
    pthread_mutex_unlock(args->mutex);

    /* If there is no more input, close the queue. */
    if (!in) {
      break;
    }

    /* Transform input and store output. */
    bool is_last_output = f(in, out);
    pcq_push(q, &tok, out);

    /* If function signaled last output, close the queue. */
    if (is_last_output) {
      break;
    }
  }

  /* Close the output queue, clean up, die, and signal death. */
  pthread_mutex_lock(args->mutex);
  q->is_closing = true;
  if (!(--args->nworkers_alive)) {
    pthread_cond_signal(args->no_workers_alive);
  }
  pthread_mutex_unlock(args->mutex);

  free(out);
  return;
}

struct master_args {
  void*               it;
  struct iter_vtable* vt;
  struct pcq_queue*   q;
  bool (*f)(void* in, void* out);
  int                 nworkers;
};

void* master(void* vargs) {
  struct master_args* args = (struct master_args*)vargs;

  pthread_mutex_t mutex;
  /* Use condition variable to handle joining. Not sure what is better. */
  pthread_cond_t  no_workers_alive;
  pthread_mutex_init(&mutex, /*attr=*/NULL);
  pthread_cond_init(&no_workers_alive, /*attr=*/NULL);

  struct worker_args wargs = {
    args->it,
    args->vt,
    args->q,
    args->f,
    &mutex,
    /*nworkers_alive=*/args->nworkers,
    &no_workers_alive
  };

  int error;
  for (int i = 1; i < args->nworkers; ++i) {
    pthread_t thread;
    if ((error = pthread_create(&thread, /*attr=*/NULL, &worker, &wargs))) {
      errc(error, error, "could not create thread");
    }
  }

  worker(&wargs);

  if (wargs.nworkers_alive) {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&no_workers_alive, &mutex);
    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&no_workers_alive);
}

void
pcq_map(void* it, struct iter_vtable* vt, struct pcq_queue* q,
    bool (*f)(void* in, void* out), int nworkers)
{
  assert(nworkers >= 1);

