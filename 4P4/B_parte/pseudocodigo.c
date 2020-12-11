
mutex mtx;
condvar prod,cons;
int prod_count=0, cons_count=0;
struct kfifo cbuffer;

void fifoproc_open(bool abre_para_lectura) { 
	lock(mtx)
	if(abre_para_lectura){
		cons_count++;
		cond_signal(prod);
		
		/*Al abrir FIFO en modo lectura (consumidor) se bloquea al proceso hasta que
		  productor haya abierto su extremo de escritura */
		while(prod_count == 0)
			cond_wait(cons, mtx);
	}
	else {
		prod_count++;
		cond_signal(cons);
		/*Al abrir FIFO en modo escritura (productor) se bloquea al proceso hasta que
		  consumidor haya abierto su extremo de lectura*/
		while(cons_count == 0)
			cond_wait(prod, mtx);
	}
	unlock(mtx);
}

void fifoproc_release(bool lectura) { 
	lock(mtx)
	
	/* Avisa a posible productor bloqueado*/
	if(lectura){
		cons_count--;
		cond_signal(prod);
	}
	/* Avisa a posible consumidor bloqueado*/
	else {
		prod_count--;
		cond_signal(cons);
		
	}
	/* Cuando todos los procesos (productores y consumidores) finalicen su actividad
       con el FIFO, el buffer circular ha de vaciarse*/
	if (cons_count == 0 && prod_count == 0) kfifo_reset(&cbuf);
	unlock(mtx)
}

int fifoproc_write(char* buff, int len) { 
	char kbuffer[MAX_KBUF];
	
	if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) { return Error;}
	if (copy_from_user(kbuffer,buff,len)) { return Error;}

	lock(mtx);	
	/* Esperar hasta que haya hueco para insertar (debe haber consumidores) */
	/* El productor se bloquea si no hay hueco en el buffer para insertar el número
	   de bytes solicitados mediante write()*/
	while (kfifo_avail(&cbuffer)<len && cons_count>0){
		cond_wait(prod,mtx);
	}
	
	/* Detectar fin de comunicación por error (consumidor cierra FIFO antes) */
	/* Si se intenta escribir en el FIFO cuando no hay consumidores (extremo de lectura
	   cerrado), el módulo devolverá un error*/
	if (cons_count==0) {
		unlock(mtx); 
		return -EPIPE;
	}
	
	kfifo_in(&cbuffer,kbuffer,len);
	/* Despertar a posible consumidor bloqueado */
	cond_signal(cons);
	unlock(mtx);
	return len;
}
int fifoproc_read(const char* buff, int len) { 
	
	char kbuffer[MAX_KBUF];
	
	if (len> MAX_CBUFFER_LEN || len> MAX_KBUF) { return Error;}

	lock(mtx);	
	/* Esperar hasta que haya hueco para insertar (debe haber productores) */
	/* El consumidor se bloquea si el buffer contiene menos bytes que los solicitados
       mediante read()*/
	while (kfifo_len(&cbuffer)<len && prod_count>0){
		cond_wait(cons,mtx);
	}
	
	/* Detectar fin de comunicación por error (consumidor cierra FIFO antes) */
	/* Si se intenta hacer una lectura del FIFO cuando el buffer circular esté vacío y no
       haya productores, el módulo devolverá el valor 0 (EOF)*/
	if (prod_count==0 && kfifo_is_empty(&cbuf)) {
		unlock(mtx); 
		return 0;
	}

	
	kfifo_out(&cbuffer,kbuffer,len);
	/* Despertar a posible productor bloqueado */
	cond_signal(prod);
	unlock(mtx);
	
	if (copy_to_user(kbuffer,buff,len)) { return Error;}
	
	return len;
}

