#include <assert.h>

#include <pcq.h>

#define NUM_SLOTS 8

int main() {
  struct pcq_queue q;
  pcq_ctor(&q, /*num=*/NUM_SLOTS, /*size=*/sizeof(double));

  assert(pcq_sizeoft(&q) == sizeof(double));

  double value;
  struct pcq_token tok;

  for (int i = 0; i < (NUM_SLOTS >> 1); ++i) {
    value = i;
    tok = pcq_claim(&q);
    pcq_push(&q, &tok, /*is_last=*/false, &value);
  }

  for (int i = 0; i < (NUM_SLOTS >> 1); ++i) {
    assert(!pcq_empty(&q));
    value = -1.0;
    pcq_pop(&q, &value);
    assert(value == (double)i);
  }

  assert(!pcq_empty(&q));

  tok = pcq_claim(&q);
  struct pcq_token after = pcq_claim(&q);

  value = 5.0;
  pcq_push(&q, &tok, /*is_last=*/true, &value);
  assert(!pcq_empty(&q));

  struct pcq_token after2 = pcq_claim(&q);
  assert(!after2.is_valid);

  value = -1.0;
  pcq_pop(&q, &value);
  assert(value == 5.0);

  pcq_push(&q, &after, /*is_last=*/false, &value);
  assert(pcq_empty(&q));

  pcq_dtor(&q);

  return 0;
}

