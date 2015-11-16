#ifndef VIMOL_SPI_H
#define VIMOL_SPI_H

/* spatial index */
struct spi;

struct spi *spi_create(void);
void spi_free(struct spi *);
void spi_add_point(struct spi *, vec_t);
int spi_get_point_count(struct spi *);
vec_t spi_get_point(struct spi *, int);
void spi_clear(struct spi *);
void spi_compute(struct spi *, double);
int spi_get_pair_count(struct spi *);
struct pair spi_get_pair(struct spi *, int);

#endif /* VIMOL_SPI_H */
