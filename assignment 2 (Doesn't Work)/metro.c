#include "cs744_thread.h"

int locked;

struct station {
  int noOfThreadsWaiting, ThreadsYetToBoard, noOfThreads, threadPresent, threadBoarded; 
  struct condition TrainCame, ThreadReadyToBoard, AllThreadsOnBoard, TrainNotFull;
  struct lock mutex;
};

void
station_init(struct station *station)
{
  lock_init(&(station->mutex));
  cond_init(&(station->AllThreadsOnBoard));
  cond_init(&(station->TrainNotFull));
  cond_init(&(station->ThreadReadyToBoard));
  cond_init(&(station->TrainCame));
  station->ThreadsYetToBoard = 0;
  station->noOfThreadsWaiting = 0;
  station->threadPresent=0;
  station->threadBoarded=0;

}

void
station_load_train(struct station *station, int count)
{
  // FILL ME IN
  lock_acquire(&(station->mutex));
  station->threadBoarded=0;
  locked=1;

    if(!count && locked){
      lock_release(&(station->mutex));
      return;
    }
    if(!station->noOfThreadsWaiting)
    {
      lock_release(&(station->mutex));
      return;
    }
    if(!station->threadPresent)
    {
           lock_release(&(station->mutex));
      return; 
    }
  else if(count){
  while (station->threadPresent && count--){
    cond_signal(&(station->TrainCame),&(station->mutex));
    cond_signal(&(station->TrainNotFull),&(station->mutex));
    cond_wait(&(station->ThreadReadyToBoard), &(station->mutex));
  }
}
 //if a passenger enters then wait for the train to become full.
// else load_train returns
  if(station->ThreadsYetToBoard) 
    cond_wait(&(station->AllThreadsOnBoard), &(station->mutex));
  lock_release(&(station->mutex));
  locked=0;
}

void
station_wait_for_train(struct station *station)
{
  // FILL ME IN
  lock_acquire(&(station->mutex));
  station->threadPresent=1;
  station->noOfThreadsWaiting=station->noOfThreadsWaiting+1;
  cond_wait(&(station->TrainCame),&(station->mutex));
  if(!--(station->noOfThreadsWaiting))
    station->threadPresent=0;
  // ++(station->ThreadsYetToBoard);
  if(++station->ThreadsYetToBoard)
    cond_signal(&(station->ThreadReadyToBoard),&(station->mutex));
  lock_release(&(station->mutex));
  locked=0;
}

void
station_on_board(struct station *station)
{
  // FILL ME IN
    lock_acquire(&(station->mutex));
    station->threadBoarded=1;
  if (!(--(station->ThreadsYetToBoard))){
    cond_signal(&(station->AllThreadsOnBoard),&(station->mutex));
  }
  lock_release(&(station->mutex));
}
