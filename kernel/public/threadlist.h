/*===========================================================================
     _____        _____        _____        _____
 ___|    _|__  __|_    |__  __|__   |__  __| __  |__  ______
|    \  /  | ||    \      ||     |     ||  |/ /     ||___   |
|     \/   | ||     \     ||     \     ||     \     ||___   |
|__/\__/|__|_||__|\__\  __||__|\__\  __||__|\__\  __||______|
    |_____|      |_____|      |_____|      |_____|

--[Mark3 Realtime Platform]--------------------------------------------------

Copyright (c) 2012-2015 Funkenstein Software Consulting, all rights reserved.
See license.txt for more information
===========================================================================*/
/*!

    \file   threadlist.h    

    \brief  Thread_t linked-list declarations

*/

#ifndef __THREADLIST_H__
#define __THREADLIST_H__

#include "kerneltypes.h"
#include "ll.h"

#ifdef __cplusplus
    extern "C" {
#endif

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_Init

    Default constructor - zero-initializes data before use.

    \param pstList_ ThreadList object to manipulate
*/
void ThreadList_Init( ThreadList_t *pstList_ );

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_SetPriority

    Set the priority of this threadlist (if used for a scheduler).

    \param pstList_ ThreadList object to manipulate
    \param ucPriority_ Priority level of the thread list
*/
void ThreadList_SetPriority( ThreadList_t *pstList_, K_UCHAR ucPriority_ );

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_SetFlagPointer

    Set the pointer to a bitmap to use for this threadlist.  Once again,
    only needed when the threadlist is being used for scheduling purposes.

    \param pstList_ ThreadList object to manipulate
    \param pucFlag_ Pointer to the bitmap flag
*/
void ThreadList_SetFlagPointer( ThreadList_t *pstList_, K_UCHAR *pucFlag_ );

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_Add

    Add a thread to the threadlist.

    \param pstList_ ThreadList object to manipulate
    \param node_ Pointer to the thread (link list node) to add to the list
*/
void ThreadList_Add( ThreadList_t *pstList_, Thread_t *node_ );

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_AddEX

    \fn void Add(LinkListNode_t *node_,
                    K_UCHAR *pucFlag_,
                    K_UCHAR ucPriority_)

    Add a thread to the threadlist, specifying the flag and priority at
    the same time.

    \param pstList_ ThreadList object to manipulate
    \param node_        Pointer to the thread to add (link list node)
    \param pucFlag_     Pointer to the bitmap flag to set (if used in
                        a scheduler context), or NULL for non-scheduler.
    \param ucPriority_  Priority of the threadlist
*/
void ThreadList_AddEX( ThreadList_t *pstList_, Thread_t *node_, K_UCHAR *pucFlag_, K_UCHAR ucPriority_);

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_Remove

    Remove the specified thread from the threadlist

    \param pstList_ ThreadList object to manipulate
    \param node_ Pointer to the thread to remove
*/
void ThreadList_Remove( ThreadList_t *pstList_, Thread_t *node_);

//---------------------------------------------------------------------------
/*!
    \brief ThreadList_HighestWaiter

    Return a pointer to the highest-priority thread in the thread-list.

    \param pstList_ ThreadList object to manipulate
    \return Pointer to the highest-priority thread
*/
Thread_t *ThreadList_HighestWaiter( ThreadList_t *pstList_ );


#ifdef __cplusplus
    }
#endif

#endif

