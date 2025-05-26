# Compilador y flags
CC = gcc
CFLAGS = -pthread

# Ejecutables
RECEPTOR = Receptor
SOLICITANTE = Solicitante

# Archivos fuente
RECEPTOR_SRC = Receptor.c InterfazReceptor.c
SOLICITANTE_SRC = Solicitante.c InterfazSolicitante.c

# Objetos
RECEPTOR_OBJ = $(RECEPTOR_SRC:.c=.o)
SOLICITANTE_OBJ = $(SOLICITANTE_SRC:.c=.o)

# Regla por defecto: compilar ambos
all: $(RECEPTOR) $(SOLICITANTE)

# Cómo compilar Receptor
$(RECEPTOR): $(RECEPTOR_OBJ)
	$(CC) $(CFLAGS) -o $(RECEPTOR) $(RECEPTOR_OBJ)

# Cómo compilar Solicitante
$(SOLICITANTE): $(SOLICITANTE_OBJ)
	$(CC) $(CFLAGS) -o $(SOLICITANTE) $(SOLICITANTE_OBJ)

# Limpieza de archivos
clean:
	rm -f *.o $(RECEPTOR) $(SOLICITANTE)
