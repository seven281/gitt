#include "ADT_CircleByteQueue.h"
#include "os.h"
#include "CoreUtil.h"


//循环队列初始化
void ADT_CircleByteQueueInit(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr,uint8_t* bufferStartPtr,uint16_t bufferSize)
{
    //首先检测各个参数是否为空
    if(circleByteQueuePtr == NULL)return;
    if(bufferStartPtr == NULL)return;
    if(bufferSize == 0)return;
    ADT_CIRCLE_BYTE_QUEUE_LOCK_THREAD();
    circleByteQueuePtr->queueArrayBuffer = bufferStartPtr;
    circleByteQueuePtr->frontPos = 0;
    circleByteQueuePtr->rearPos = 0;
    circleByteQueuePtr->maxSize = bufferSize;
    circleByteQueuePtr->currentSize = 0;
    ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
}

//获取循环队列最大容量
uint16_t ADT_CircleByteQueueGetMaxCapacity(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr)
{
    if(circleByteQueuePtr == 0)
    {
        return 0;
    }
    return circleByteQueuePtr->maxSize;
}

//获取当前循环队列已经存储的数据
uint16_t ADT_CircleByteQueueGetCurrentSize(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr)
{
    if(circleByteQueuePtr == 0)
    {
        return 0;
    }
    return circleByteQueuePtr->currentSize;
}

//获取当前循环队列中空数据的量
uint16_t ADT_CircleByteQueueGetCurrentEmptySize(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr)
{
    if(circleByteQueuePtr == 0)
    {
        return 0;
    }
    uint16_t emptySize = 0;
    ADT_CIRCLE_BYTE_QUEUE_LOCK_THREAD();
    emptySize = circleByteQueuePtr->maxSize - circleByteQueuePtr->currentSize;
    ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
    return emptySize;
}

//往循环队列中写入指定数量数据,当空间不足的时候,会将缓冲区数据填满,并返回真实的写入量
void ADT_CircleByteQueueWriteBuffer(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr,uint8_t* bufferWritePtr,uint16_t length,uint16_t* realWriteByteCount)
{
    uint16_t writeCount = 0;
    if(circleByteQueuePtr == NULL)
    {
        *realWriteByteCount = 0;
        return;
    }
    if(bufferWritePtr == NULL)
    {
        *realWriteByteCount = 0;
        return;
    }
    ADT_CIRCLE_BYTE_QUEUE_LOCK_THREAD();
    if(circleByteQueuePtr->maxSize == circleByteQueuePtr->currentSize)
    {
        //当前数据已满
        *realWriteByteCount = 0;
        ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
        return;
    }
    else
    {
        while(circleByteQueuePtr->currentSize < circleByteQueuePtr->maxSize)
        {
            circleByteQueuePtr->queueArrayBuffer[circleByteQueuePtr->rearPos++] = bufferWritePtr[writeCount++];
            circleByteQueuePtr->rearPos %= circleByteQueuePtr->maxSize;
            circleByteQueuePtr->currentSize++;
            if(writeCount == length)
            {
                break;
            }
        }
        *realWriteByteCount = writeCount;
        ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
        return;
    }
    
}

//从循环队列中读取指定数量数据,当数据不足时,已有的数据写到缓冲区,并返回真实读取到的数据
void ADT_CircleByteQueueReadBuffer(ADT_CIRCLE_BYTE_QUEUE* circleByteQueuePtr,uint8_t* bufferReadPtr,uint16_t length,uint16_t* realReadByteCount)
{
    uint16_t readCount = 0;
    if(circleByteQueuePtr == NULL)
    {
        *realReadByteCount = 0;
        return;
    }
    if(bufferReadPtr == NULL)
    {
        *realReadByteCount = 0;
        return;
    }
    ADT_CIRCLE_BYTE_QUEUE_LOCK_THREAD();
    if(0 == circleByteQueuePtr->currentSize)
    {
        *realReadByteCount = 0;
        ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
        return;
    }
    else
    {
        while(circleByteQueuePtr->currentSize > 0)
        {
            bufferReadPtr[readCount++] = circleByteQueuePtr->queueArrayBuffer[circleByteQueuePtr->frontPos++];
            circleByteQueuePtr->frontPos %= circleByteQueuePtr->maxSize;
            circleByteQueuePtr->currentSize--;
            if(readCount == length)
            {
                break;
            }
        }
        *realReadByteCount = readCount;
        ADT_CIRCLE_BYTE_QUEUE_UNLOCK_THREAD();
        return;
    }
}























