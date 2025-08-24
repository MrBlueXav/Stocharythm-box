/*
 * ring_buffer.h
 *
 *  Created on: Aug 24, 2025
 *      Author: ChatGPT 5
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

/*
 * Buffer circulaire de chars, taille puissance de 2 recommandée (facilite le masquage).
 * Modèle d'usage sûr : SINGLE PRODUCER / SINGLE CONSUMER
 *  - Le producteur (p.ex. ISR RX) appelle rb_push / rb_write.
 *  - Le consommateur (p.ex. main loop) appelle rb_pop / rb_read / rb_peek.
 *  - head n'est modifié que par le producteur; tail que par le consommateur.
 *  - Les fonctions de vérification (size, empty, full) lisent head/tail sans les modifier.
 *
 * Si vous devez écrire ET lire depuis plusieurs contextes concurrents,
 * protégez les appels critiques (désactivation IRQ ou mutex RTOS).
 */

typedef struct {
    volatile uint32_t head;   // index de la prochaine écriture (prod)
    volatile uint32_t tail;   // index de la prochaine lecture (cons)
    char *buf;                // stockage
    uint32_t capacity;        // taille totale du buffer (N)
    uint32_t mask;            // = capacity - 1 (si capacity puissance de 2), sinon inutilisé
    bool use_mask;            // true si capacité est une puissance de 2
} ring_buffer_t;

/* Utils internes */
static inline bool rb_is_pow2(uint32_t x) { return x && ((x & (x - 1u)) == 0u); }
static inline uint32_t rb_mod(const ring_buffer_t *rb, uint32_t x) {
    return rb->use_mask ? (x & rb->mask) : (x % rb->capacity);
}

/* Initialisation : buffer externe fourni par l'appelant */
static inline void rb_init(ring_buffer_t *rb, char *storage, uint32_t capacity) {
    rb->buf = storage;
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->use_mask = rb_is_pow2(capacity);
    rb->mask = rb->use_mask ? (capacity - 1u) : 0u;
}

/* Remise à zéro (vide le buffer) */
static inline void rb_reset(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
}

/* Capacités & état */
static inline uint32_t rb_capacity(const ring_buffer_t *rb) { return rb->capacity; }

/* Nombre d'éléments présents (non bloquant). */
static inline uint32_t rb_size(const ring_buffer_t *rb) {
    return (uint32_t)(rb->head - rb->tail);  // fonctionne modulo 2^32
}

static inline uint32_t rb_free_space(const ring_buffer_t *rb) {
    return rb->capacity - rb_size(rb);
}

static inline bool rb_is_empty(const ring_buffer_t *rb) { return rb->head == rb->tail; }
static inline bool rb_is_full (const ring_buffer_t *rb) { return rb_size(rb) == rb->capacity; }

/* Écriture d’un caractère (retourne false si plein).  Contexte producteur. */
static inline bool rb_push(ring_buffer_t *rb, char c) {
    if (rb_is_full(rb)) return false;
    uint32_t h = rb->head;
    rb->buf[ rb_mod(rb, h) ] = c;
    rb->head = h + 1;
    return true;
}

/* Lecture d’un caractère (retourne false si vide).  Contexte consommateur. */
static inline bool rb_pop(ring_buffer_t *rb, char *out) {
    if (rb_is_empty(rb)) return false;
    uint32_t t = rb->tail;
    if (out) *out = rb->buf[ rb_mod(rb, t) ];
    rb->tail = t + 1;
    return true;
}

/* Peek: lire sans consommer, offset 0 = prochain char. */
static inline bool rb_peek(const ring_buffer_t *rb, uint32_t offset, char *out) {
    if (offset >= rb_size(rb)) return false;
    uint32_t idx = rb_mod(rb, rb->tail + offset);
    if (out) *out = rb->buf[idx];
    return true;
}

/* Écriture d’un bloc. Retourne le nombre écrit (peut être partiel si plein). */
static inline uint32_t rb_write(ring_buffer_t *rb, const char *data, uint32_t len) {
    uint32_t free = rb_free_space(rb);
    if (len > free) len = free;
    uint32_t h = rb->head;
    for (uint32_t i = 0; i < len; ++i) {
        rb->buf[ rb_mod(rb, h + i) ] = data[i];
    }
    rb->head = h + len;
    return len;
}

/* Lecture d’un bloc. Retourne le nombre lu (peut être partiel si vide). */
static inline uint32_t rb_read(ring_buffer_t *rb, char *out, uint32_t len) {
    uint32_t have = rb_size(rb);
    if (len > have) len = have;
    uint32_t t = rb->tail;
    for (uint32_t i = 0; i < len; ++i) {
        if (out) out[i] = rb->buf[ rb_mod(rb, t + i) ];
    }
    rb->tail = t + len;
    return len;
}

/* Recherche d’un caractère jusqu’à max_scan éléments; retourne l’offset ou -1. */
static inline int32_t rb_find_char(const ring_buffer_t *rb, char target, uint32_t max_scan) {
    uint32_t n = rb_size(rb);
    if (max_scan < n) n = max_scan;
    for (uint32_t i = 0; i < n; ++i) {
        if (rb->buf[ rb_mod(rb, rb->tail + i) ] == target) return (int32_t)i;
    }
    return -1;
}

#endif /* RING_BUFFER_H */




#endif /* INC_RING_BUFFER_H_ */
