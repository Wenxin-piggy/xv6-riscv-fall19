// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define MAXN 13

struct {
  struct spinlock lock[MAXN];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // head.next is most recently used.
  struct buf head[MAXN];
} bcache;

void
binit(void)
{
  struct buf *b;

  for(int i = 0;i < MAXN;i ++){
    initlock(&bcache.lock[i], "bcachemy");
    // Create linked list of buffers
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }
  //也没太懂，好像是把所有的归到第一组里
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}
int myhash(int nodeId){
  return (nodeId % MAXN);
}
// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int id = myhash(blockno);
  acquire(&bcache.lock[id]);

  // Is the block already cached?
  //  能在维护的队列中找到b的情况
  for(b = bcache.head[id].next; b != &bcache.head[id]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  //不能在维护的队列里找到b,从没用过的里面拉一个出来
  // Not cached; recycle an unused buffer.
  int next_id = myhash((id+1));
  while(next_id != id){
    //还没有找回一圈
    //循环链表，从后往前找
    acquire(&bcache.lock[next_id]);
    int find = 0;
    for(b = bcache.head[next_id].prev;b != &bcache.head[next_id];b = b -> prev){
      if(b->refcnt == 0){
        //能够找到一个空的，可用的，把它拿出来
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        find = 1;
        break;
      }
    }
    if(find == 1){
      //将其从原来的地方断开
      b -> next -> prev = b -> prev;
      b -> prev -> next = b -> next;
      release(&bcache.lock[next_id]);
      //然后把它移动到它该去的地方
      b->next = bcache.head[id].next;
      b->prev = &bcache.head[id];
      bcache.head[id].next->prev = b;
      bcache.head[id].next = b;
      release(&bcache.lock[id]);
      acquiresleep(&b->lock);
      return b;
    }
    //在这一次勇敢的尝试中没找到空的
    release(&bcache.lock[next_id]);
    next_id = myhash((next_id+1));
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b->dev, b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b->dev, b, 1);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  int id = myhash(b->blockno);
  acquire(&bcache.lock[id]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[id].next;
    b->prev = &bcache.head[id];
    bcache.head[id].next->prev = b;
    bcache.head[id].next = b;
  }
  
  release(&bcache.lock[id]);
}

void
bpin(struct buf *b) {
  int id = myhash(b->blockno);
  acquire(&bcache.lock[id]);
  b->refcnt++;
  release(&bcache.lock[id]);
}

void
bunpin(struct buf *b) {
  int id = myhash(b->blockno);
  acquire(&bcache.lock[id]);
  b->refcnt--;
  release(&bcache.lock[id]);
}


