#include "../h/pcb.h"
//#include "../h/asl.h"
#include "../h/const.h"

typedef struct semd_t {
        struct semd_t *s_next; /* next element on the ASL */
        int *s_semAdd;         /* pointer to the semaphore */
        pcb_t *s_procQ;        /*tail pointer to a process queue */
} semd_t;

semd_t *semdFree, *semdFree_h, *semd_h;
static semd_t semdTable[MAXPROC+1];

/*void initASL()
{
	int i;

	semd_h = &semdTable[0]; // dummy
	semd_h->s_next = NULL; // dummy

	semdFree_h = &semdTable[1];

	for(i=1; i<MAXPROC; i++)
	{
		semdFree = &semdTable[i];
                semdFree->s_next = &semdTable[i+1];
	}

	semdFree = &semdTable[MAXPROC];
	semdFree->s_next = NULL;
}*/
void initASL()
{
int i;

semd_h = &semdTable[0]; // dummy
semd_h->s_next = NULL; // dummy

for(i=1; i<MAXPROC+1; i++)
{
semdFree = &semdTable[i];

if(i == 1)
semdFree_h = semdFree;

if(i == MAXPROC)
semdFree->s_next = NULL;

else semdFree->s_next = &semdTable[i+1];
}
}

semd_t *look4sema4(int *semAdd)
{
	semd_t *semdTmp = semd_h;

	while(*((semdTmp->s_next)->s_semAdd) != *semAdd && semdTmp->s_next != NULL)
		semdTmp = semdTmp->s_next;

	return semdTmp;
}

int insertBlocked(int *semAdd, pcb_t *p)
{
	semd_t *semdTmp, *semdList = semd_h;

	semdTmp = look4sema4(semAdd);

	if(semdTmp->s_next == NULL)
	{
		if(semdFree_h == NULL)
			return TRUE;

		if(semdList->s_next == NULL)
		{
			semdList->s_next = semdFree_h;
			semdFree_h = semdFree_h->s_next;

			semdList = semdList->s_next;
			semdList->s_next = NULL;
			*(semdList->s_semAdd) = *semAdd;
			semdList->s_procQ = mkEmptyProcQ();
			insertProcQ(&(semdList->s_procQ), p);

			return FALSE;
		}

		else
		{
			while((*semAdd < *((semdList->s_next)->s_semAdd)) && (semdList->s_next != NULL))
				semdList = semdList->s_next;

			semdList->s_next = semdFree_h;
                        semdFree_h = semdFree_h->s_next;

                        semdList = semdList->s_next;
                        semdList->s_next = NULL;
                        *(semdList->s_semAdd) = *semAdd;
                        semdList->s_procQ = mkEmptyProcQ();
			insertProcQ(&((semdList->s_next)->s_procQ), p);

			return FALSE;
		}
	}

	else
	{
		semdTmp = semdTmp->s_next;

		insertProcQ(&(semdTmp->s_procQ), p);

		return FALSE;
	}	
}

pcb_t *removeBlocked(int *semAdd)
{
	semd_t *semdTmp, *semdRet;
	pcb_t *p;

	semdTmp = look4sema4(semAdd);

	if(semdTmp->s_next == NULL)
		return NULL;

	else
	{
		p = removeProcQ(&((semdTmp->s_next)->s_procQ));

		if(emptyProcQ((semdTmp->s_next)->s_procQ))
		{
			semdRet = semdTmp->s_next;
			semdTmp->s_next = semdRet->s_next;
			semdRet->s_next = semdFree_h;
			semdFree_h = semdRet;
		}

		return p;
	}
}

pcb_t *outBlocked(pcb_t *p)
{
	semd_t *semdTmp;

	semdTmp = (look4sema4(p->p_semAdd))->s_next;

	if((outProcQ(&(semdTmp->s_procQ), p)) == NULL)
		return NULL;

	return p;
}

pcb_t *headBlocked(int *semAdd)
{
	semd_t *semdTmp;

	semdTmp = (look4sema4(semAdd))->s_next;

	if(semdTmp == NULL || semdTmp->s_procQ == NULL)
		return NULL;

	return (semdTmp->s_procQ)->p_next;
}

