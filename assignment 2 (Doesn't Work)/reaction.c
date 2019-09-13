#include "cs744_thread.h"

// Forward declaration. This function is implemented in reaction-runner.c,
// but you needn't care what it does. Just be sure it's called when
// appropriate within reaction_o()/reaction_h().
void make_water();

struct reaction {
	// FILL ME IN
	struct lock mutex;
	struct condition hyderogen_atom, oxygen_atom,watermade;
	int noOfHyderogenAtoms, noOfOxygenAtoms, noOfWater, hyderogenInUse, oxygenInUse, totalAtoms, noOfSigByH;

};

void
reaction_init(struct reaction *reaction)
{
	// FILL ME IN
	lock_init(&(reaction->mutex));
	cond_init(&(reaction->hyderogen_atom));
	cond_init(&(reaction->oxygen_atom));
	cond_init(&(reaction->watermade));
	reaction->noOfWater=0;
	reaction->noOfHyderogenAtoms=0;
	reaction->noOfOxygenAtoms=0;
	reaction->hyderogenInUse=0;
	reaction->noOfSigByH=0;
	reaction->totalAtoms=0;
}

void
reaction_h(struct reaction *reaction)
{
	// FILL ME IN
	lock_acquire(&(reaction->mutex));
	reaction->noOfHyderogenAtoms++;
	if(reaction->noOfHyderogenAtoms%2==0)
	{	

		(reaction->noOfHyderogenAtoms>=2)?
		 (
		 	cond_signal(&(reaction->hyderogen_atom),&(reaction->mutex))
		 ):
		((reaction->noOfOxygenAtoms==0))?(
			reaction->noOfSigByH++
		):(cond_signal(&(reaction->watermade),&(reaction->mutex)));
	}
	cond_wait(&(reaction->oxygen_atom),&(reaction->mutex));
	lock_release(&(reaction->mutex));
}

void
reaction_o(struct reaction *reaction)
{
	// FILL ME IN
	lock_acquire(&(reaction->mutex));
	(reaction->noOfSigByH)?
	(
	reaction->noOfHyderogenAtoms-=2,
	cond_signal(&(reaction->oxygen_atom),&(reaction->mutex)),
	cond_signal(&(reaction->oxygen_atom),&(reaction->mutex))):
	(cond_signal(&(reaction->watermade),&(reaction->mutex)));
	reaction->noOfOxygenAtoms++;
	cond_wait(&(reaction->hyderogen_atom),&(reaction->mutex));
	reaction->noOfHyderogenAtoms-=2;
	cond_signal(&(reaction->oxygen_atom),&(reaction->mutex));
	cond_signal(&(reaction->oxygen_atom),&(reaction->mutex));
	(**make_water)();
	lock_release(&(reaction->mutex));
}