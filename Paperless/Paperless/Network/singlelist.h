#ifndef __SINGLE_LIST_H__
#define __SINGLE_LIST_H__

#define USE_SYS_SINGLELIST

#if defined(USE_SYS_SINGLELIST)

#define S_LIST_HEADER             SLIST_HEADER
#define S_LIST_ENTRY              SINGLE_LIST_ENTRY
#define PS_LIST_ENTRY             PSINGLE_LIST_ENTRY

#define INIT_SLIST_HEAD           InitializeSListHead
#define RELEASE_SLIST_HEAD(x)     ((void*)0)
#define PUSH_SLIST_ENTRY          InterlockedPushEntrySList
#define POP_SLIST_ENTRY           InterlockedPopEntrySList
#define FLUSH_SLIST               InterlockedFlushSList
#define QUERY_SLIST               QueryDepthSList

#else // !USE_SYS_SINGLELIST

typedef struct _S_LIST_ENTRY
{
    struct _S_LIST_ENTRY  *next;

} S_LIST_ENTRY, *PS_LIST_ENTRY;

typedef struct _S_LIST_HEADER
{
    CRITICAL_SECTION      cs;
    LONG                  length;
    PS_LIST_ENTRY         head;

} S_LIST_HEADER, *PS_LIST_HEADER;

void INIT_SLIST_HEAD(PS_LIST_HEADER ListHead)
{
    InitializeCriticalSection(&ListHead->cs);
    ListHead->length = 0;
    ListHead->head = NULL;
}

void RELEASE_SLIST_HEAD(PS_LIST_HEADER ListHead)
{
    EnterCriticalSection(&ListHead->cs);
    ListHead->length = 0;
    ListHead->head = NULL;
    LeaveCriticalSection(&ListHead->cs);
    DeleteCriticalSection(&ListHead->cs);
}

PS_LIST_ENTRY PUSH_SLIST_ENTRY(PS_LIST_HEADER ListHead, PS_LIST_ENTRY ListEntry)
{
    EnterCriticalSection(&ListHead->cs);
    ListEntry->next = ListHead->head;
    ListHead->head = ListEntry;
    ListHead->length++;
    LeaveCriticalSection(&ListHead->cs);
    return ListEntry->next;
}

PS_LIST_ENTRY POP_SLIST_ENTRY(PS_LIST_HEADER ListHead)
{
    PS_LIST_ENTRY ret;
    EnterCriticalSection(&ListHead->cs);
    ret = ListHead->head;
    if (ret)
    {
        ListHead->head = ret->next;
        ListHead->length--;
    }
    LeaveCriticalSection(&ListHead->cs);
    return ret;
}

PS_LIST_ENTRY FLUSH_SLIST(PS_LIST_HEADER ListHead)
{
    PS_LIST_ENTRY ret;
    EnterCriticalSection(&ListHead->cs);
    ret = ListHead->head;
    ListHead->head = NULL;
    ListHead->length = 0;
    LeaveCriticalSection(&ListHead->cs);
    return ret;
}

LONG QUERY_SLIST(PS_LIST_HEADER ListHead)
{
    LONG length;
    EnterCriticalSection(&ListHead->cs);
    length = ListHead->length;
    LeaveCriticalSection(&ListHead->cs);
    return length;
}

#endif // USE_SYS_SINGLELIST

#endif // __SINGLE_LIST_H__
