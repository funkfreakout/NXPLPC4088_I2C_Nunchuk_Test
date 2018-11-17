#ifndef SONIDO_H
#define SONIDO_H

struct sonido_t
{
    int tamano;
    const short* muestras;
};

void sonido_inicializar(void);
int sonido_reproducir(const struct sonido_t *sptr);
int sonido_en_reproduccion(void);
void isr_timer0(void);

#endif /* SONIDO_H */
